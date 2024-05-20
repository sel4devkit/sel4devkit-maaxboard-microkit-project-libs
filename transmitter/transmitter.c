/*
 * Copyright 2021, Breakaway Consulting Pty. Ltd.
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <microkit.h>
#include <sel4_dma.h>
#include <uboot_drivers.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio_microkit.h>
#include <sel4_timer.h>
#include <plat_support.h>

#define LOG_FILE_DEVICE "mmc 0:1"  // Partition 1 on mmc device 0
#define LOG_FILENAME  "log.txt"

/* A buffer of encrypted characters to log to the SD/MMC card */
uintptr_t data_packet;
char* mmc_pending_tx_buf = (void *)(uintptr_t)0x5011000;
uint mmc_pending_length = 0;


void write_pending_mmc_log(void)
{

    /* Track the total number of bytes written to the log file*/
    static uint total_bytes_written = 0;

    // Test string to write to the file
    const char test_string[] = "Hello file!";

    /* Write all keypresses stored in the 'mmc_pending_tx_buf' buffer to the log file */
    char uboot_cmd[64];
    sprintf(uboot_cmd, "fatwrite %s 0x%x %s %x %x",
        LOG_FILE_DEVICE,        // The U-Boot partition designation
        &mmc_pending_tx_buf,    // Address of the buffer to write
        LOG_FILENAME,           // Filename to log to
        mmc_pending_length,     // The number of bytes to write
        0);   // The offset in the file to start writing from
    int ret = run_uboot_command(uboot_cmd);

    /* Clear the buffer if writing to the file was successful */
    if (ret >= 0) {
        total_bytes_written += mmc_pending_length;

        /* All pending characters have now been sent. Clear the buffer */
        memset(mmc_pending_tx_buf, 0, mmc_pending_length);
        mmc_pending_length = 0;
    }

    printf("Enf of write_pending_mmc_log\n");
}

void
init(void)
{
    printf("Transmitter\n");

    printf("buffer index 1 %c\n", mmc_pending_tx_buf[1]);

    const char *const_dev_paths[] = DEV_PATHS;

    // Initalise DMA manager
    microkit_dma_manager(&dma_manager);

    // Initialise DMA
    microkit_dma_init(dma_base, dma_size,
        4096, 1);

    // Initialise uboot library
    initialise_uboot_drivers(
    dma_manager,
    incbin_device_tree_start,
    /* List the device tree paths for the devices */
    const_dev_paths, DEV_PATH_COUNT);

    write_pending_mmc_log();

    return 0;
}

void
notified(microkit_channel ch)
{
}