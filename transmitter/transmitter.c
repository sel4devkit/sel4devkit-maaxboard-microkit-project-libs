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
#include <mmc_platform_devices.h>

#define LOG_FILE_DEVICE "mmc 0:1"  // Partition 1 on mmc device 0
#define LOG_FILENAME  "log.txt"

/* A buffer of encrypted characters to log to the SD/MMC card */
uintptr_t data_buffer;
uint mmc_pending_length = 0;


void write_mmc_log(char* data_buffer_pointer)
{

    /* Write all keypresses stored in the 'mmc_pending_tx_buf' buffer to the log file */
    char uboot_cmd[64];
    printf("Data buffer pointer %x\n", data_buffer_pointer);
    sprintf(uboot_cmd, "fatwrite %s 0x%x %s %x %x",
        LOG_FILE_DEVICE,        // The U-Boot partition designation
        data_buffer_pointer,    // Address of the buffer to write
        LOG_FILENAME,           // Filename to log to
        mmc_pending_length,     // The number of bytes to write
        0);   // The offset in the file to start writing from
    int ret = run_uboot_command(uboot_cmd);

    // Test string to read the file into
    char read_string[mmc_pending_length];

    // Read then output contents of the file
    sprintf(uboot_cmd, "fatload %s 0x%x %s %x %x",
        LOG_FILE_DEVICE,    // The U-Boot partition designation
        &read_string,       // Address to read the data into
        LOG_FILENAME,       // Filename to read from
        mmc_pending_length, // Max number of bytes to read (0 = to end of file)
        0);                 // The offset in the file to start read from
    run_uboot_command(uboot_cmd);
    printf("String read from file %s: %s\n", LOG_FILENAME, read_string);

    /* Clear the buffer if writing to the file was successful */
    if (ret >= 0) {
        memset(data_buffer, 0, mmc_pending_length);
        mmc_pending_length = 0;
    }

    printf("End of write_mmc_log\n");
}

void
init_post(void)
{
    char* data_buffer_ptr = (char*)data_buffer;
    
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

    write_mmc_log(data_buffer_ptr);

    return 0;
}

void
init(void)
{
}

void
notified(microkit_channel ch)
{
}

seL4_MessageInfo_t
protected(microkit_channel ch, microkit_msginfo msginfo)
{
    switch (ch) {
        case 6:
            mmc_pending_length = (int) microkit_msginfo_get_label(msginfo);
            init_post();
            break;
        default:
            printf("crypto received protected unexpected channel\n");
    }
    return seL4_MessageInfo_new(0,0,0,0);
}