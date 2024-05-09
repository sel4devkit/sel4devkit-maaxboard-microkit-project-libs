/*
 * Copyright 2022, Capgemini Engineering
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <camkes.h>
#include <autoconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <picoserver.h>
#include <assert.h>

#include <camkes/io.h>
#include <sel4platsupport/io.h>
#include <platsupport/delay.h>

#include <uboot_drivers.h>
#include <mmc_platform_devices.h>

#define LOG_FILENAME "transmitter_log.txt" // Filename of the log file to use
#define LOG_FILE_DEVICE "mmc 0:2"          // The U-Boot designation of the disk partition to write to
#define LOG_FILE_WRITE_PERIOD_US 30000000  // Time between log file writes (30 seconds)

#define ETH_PORT 1234 // Port number of the ethernet socket to transmit to

extern void *eth_send_buf;

/* A buffer of encrypted characters to transmit over ethernet */
#define ETH_TX_BUF_LEN 4096
char eth_pending_tx_buf[ETH_TX_BUF_LEN];
uint eth_pending_length = 0;

/* A buffer of encrypted characters to log to the SD/MMC card */
#define MMC_TX_BUF_LEN 4096
char mmc_pending_tx_buf[ETH_TX_BUF_LEN];
uint mmc_pending_length = 0;

/* File descriptor of socket to transmit to. A value of -1 indicates no socket is connected. */
int eth_socket = -1;


void listen_for_socket(void)
{
    printf("%s instance starting up, going to be listening on %s:%d\n",
           get_instance_name(), ip_addr, ETH_PORT);

    int socket_in = eth_control_open(false);
    if (socket_in == -1) {
        assert(!"Failed to open a socket for listening!");
    }

    int ret = eth_control_bind(socket_in, PICOSERVER_ANY_ADDR_IPV4, ETH_PORT);
    if (ret) {
        assert(!"Failed to bind a socket for listening!");
    }

    ret = eth_control_listen(socket_in, 1);
    if (ret) {
        assert(!"Failed to listen for incoming connections!");
    }
}


void handle_picoserver_notification(void)
{
    picoserver_event_t server_event = eth_control_event_poll();
    int ret = 0;
    int socket = 0;
    uint16_t events = 0;
    char ip_string[16] = {0};

    while (server_event.num_events_left > 0 || server_event.events) {
        socket = server_event.socket_fd;
        events = server_event.events;
        if (events & PICOSERVER_CONN) {
            if (socket != 0) {
                picoserver_peer_t peer = eth_control_accept(socket);
                if (peer.result == -1) {
                    assert(!"Failed to accept a peer");
                }
                pico_ipv4_to_string(ip_string, peer.peer_addr);
                printf("%s: Connection established with %s on socket %d\n", get_instance_name(), ip_string, socket);
                /* Store the file descriptor of connected socket */
                eth_socket = peer.socket;
            }
        }
        if (events & PICOSERVER_CLOSE) {
            ret = eth_control_shutdown(socket, PICOSERVER_SHUT_RDWR);
            printf("%s: Connection closing on socket %d\n", get_instance_name(), socket);
            /* Socket no longer connected, clear the stored file descriptor */
            eth_socket = -1;
        }
        if (events & PICOSERVER_FIN) {
            printf("%s: Connection closed on socket %d\n", get_instance_name(), socket);
            /* Socket no longer connected, clear the stored file descriptor */
            eth_socket = -1;
        }
        if (events & PICOSERVER_ERR) {
            printf("%s: Error with socket %d, going to die\n", get_instance_name(), socket);
            assert(0);
        }
        server_event = eth_control_event_poll();
    }
}


void receive_data_from_crypto_component(void)
{
    /* Retrieve data from the circular buffer held in the dataport. As the dataport
     * is shared with another component we ensure only one component accesses it
     * at a time through use of a mutex accessed via the 'circular_buffer_lock_xxx'
     * RPC calls. */

    /* The *_acquire() and *_release() functions are used to maintain coherency of
     * shared memory between components. See the CAmkES manual for details
     * (https://docs.sel4.systems/projects/camkes/manual.html).
     */

    circular_buffer_lock_lock(); // Start of critical section

    circular_buffer_data_acquire();
    while(circular_buffer_data->head != circular_buffer_data->tail) {
        /* Buffer is not empty, read the next character from it */
        char encrypted_char = circular_buffer_data->data[circular_buffer_data->tail];
        circular_buffer_data->tail += 1;
        /* Store the read character in the buffer of pending data to send over ethernet. */
        /* If the buffer is full then discard the character */
        if (eth_pending_length < ETH_TX_BUF_LEN) {
            eth_pending_tx_buf[eth_pending_length] = encrypted_char;
            eth_pending_length += 1;
        }
        /* Store the read character in the buffer of pending data to log to SD/MMC. */
        /* If the buffer is full then discard the character */
        if (mmc_pending_length < MMC_TX_BUF_LEN) {
            mmc_pending_tx_buf[mmc_pending_length] = encrypted_char;
            mmc_pending_length += 1;
        }
    }
    circular_buffer_data_release();

    circular_buffer_lock_unlock(); // End of critical section
}


void transmit_pending_eth_buffer(void)
{
    /* Copy all keypresses stored in the 'eth_pending_tx_buf' buffer to
     * the ethernet send buffer ('eth_send_buf') then instruct the
     * PicoServer component holding the PicoTCP stack to transmit the
     * data */

    strncpy(eth_send_buf, eth_pending_tx_buf, eth_pending_length);
    eth_send_send(eth_socket, eth_pending_length, 0);
    /* All pending characters have now been sent. Clear the buffer */
    memset(eth_send_buf, 0, eth_pending_length);
    memset(eth_pending_tx_buf, 0, eth_pending_length);
    eth_pending_length = 0;
}


void write_pending_mmc_log(void)
{
    /* Track the total number of bytes written to the log file*/
    static uint total_bytes_written = 0;

    /* Write all keypresses stored in the 'mmc_pending_tx_buf' buffer to the log file */
    char uboot_cmd[64];
    sprintf(uboot_cmd, "fatwrite %s 0x%x %s %x %x",
        LOG_FILE_DEVICE,        // The U-Boot partition designation
        &mmc_pending_tx_buf,    // Address of the buffer to write
        LOG_FILENAME,           // Filename to log to
        mmc_pending_length,     // The number of bytes to write
        total_bytes_written);   // The offset in the file to start writing from
    int ret = run_uboot_command(uboot_cmd);

    /* Clear the buffer if writing to the file was successful */
    if (ret >= 0) {
        total_bytes_written += mmc_pending_length;

        /* All pending characters have now been sent. Clear the buffer */
        memset(mmc_pending_tx_buf, 0, mmc_pending_length);
        mmc_pending_length = 0;
    }
}


int run(void)
{
    /* Perform initialisation prior to the main loop (in the following order):
     *
     * 1. Instruct the PicoTCP library to listen on the required socket for
     *    connections.
     * 2. Get the CAmkES IO operations; these need to be provided to the U-Boot
     *    driver library in the following step.
     * 3. Initialise the U-Boot driver library which provides driver support
     *    for accessing the MMC/SD card and filesystem support.
     * 4. Use the U-Boot driver library (initialised in the previous step) to
     *    delete any previous log file from the MMC/SD card using the U-Boot
     */

    /* Listen for connections on the ethernet socket we wish to transmit to */
    listen_for_socket();

    /* Get the CAmkES IO operations (needed by the U-Boot driver library) */
    ps_io_ops_t io_ops;
    if (camkes_io_ops(&io_ops)) {
        assert(!"Failed to initialize io_ops");
    }

    /* Start the U-Boot driver library */
    const char *const_reg_paths[] = REG_PATHS;
    const char *const_dev_paths[] = DEV_PATHS;
    assert(!initialise_uboot_drivers(
        /* Provide the platform support IO operations */
        &io_ops,
        /* List the device tree paths that need to be memory mapped */
        const_reg_paths, REG_PATH_COUNT,
        /* List the device tree paths for the devices */
        const_dev_paths, DEV_PATH_COUNT));

    /* Delete any existing log file to ensure we start with an empty file */
    char uboot_cmd[64];
    sprintf(uboot_cmd, "fatrm %s %s", LOG_FILE_DEVICE, LOG_FILENAME);
    run_uboot_command(uboot_cmd);

    /* Now poll for events and handle them */
    bool idle_cycle;
    seL4_Word badge;
    unsigned long last_log_file_write_time = 0;

    while(1) {
        idle_cycle = true;

        /* Process notification of receipt of encrypted characters */
        if (circular_buffer_notify_poll()) {
            idle_cycle = false;
            receive_data_from_crypto_component();
        }

        /* Send any received encrypted characters to the socket (if connected) */
        if (eth_socket >= 0 && eth_pending_length > 0) {
            idle_cycle = false;
            transmit_pending_eth_buffer();
        }

        /* Write any received encrypted characters to the log file (if sufficient
         * time has passed from the previous write) */
        if (mmc_pending_length > 0 &&
               (uboot_monotonic_timer_get_us() - last_log_file_write_time) >= LOG_FILE_WRITE_PERIOD_US) {
            idle_cycle = false;
            last_log_file_write_time = uboot_monotonic_timer_get_us();
            write_pending_mmc_log();
        }

        /* Process notification of any events from the PicoTCP stack */
        seL4_Poll(eth_control_notification(), &badge);
        if (badge) {
            idle_cycle = false;
            handle_picoserver_notification();
        }

        /* Sleep on idle cycles to prevent busy looping */
        if (idle_cycle) {
            ps_mdelay(10);
        }
    }

    return 0;
}
