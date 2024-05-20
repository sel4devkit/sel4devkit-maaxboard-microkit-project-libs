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

uintptr_t data_packet;

#define MMC_TX_BUF_LEN 4096
uintptr_t data_packet;
char* mmc_pending_tx_buf = (void *)(uintptr_t)0x5011000;
uint mmc_pending_length = 0;


void handle_keypress(void) {
    printf("Reading input from the USB keyboard:\n");
    for (int x = 0; x <= 1000; x++) {
        while (uboot_stdin_tstc() > 0) {
            char c = uboot_stdin_getc();
            printf("Received character: %c\n", c, stdout);
            
            // Add character to MMC buffer
            if (mmc_pending_length < MMC_TX_BUF_LEN) {
                mmc_pending_tx_buf[mmc_pending_length++] = c;
            }
        }
        udelay(10000);
    }
    printf("buffer index 1 %c\n", mmc_pending_tx_buf[1]);
}

void
init(void)
{

    printf("data_packet address %x\n", &data_packet);
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

    /* Set USB keyboard as input device */
    int ret = run_uboot_command("setenv stdin usbkbd");
    if (ret < 0) {
        assert(!"Failed to set USB keyboard as the input device");
    }

    /* Start the USB subsystem */
    ret = run_uboot_command("usb start");
    if (ret < 0) {
        assert(!"Failed to start USB driver");
    }
    
    handle_keypress();

    return 0;
}

void
notified(microkit_channel ch)
{
}