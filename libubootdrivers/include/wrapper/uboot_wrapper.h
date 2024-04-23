/* 
 * Copyright 2022, Capgemini Engineering 
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 *
 */

 #include <io_dma.h>

/* Routines to perform initialisation and shutdown of the U-Boot wrapper.
 * The initialise routine performs the actions that would normally be
 * performed by U-Boot when it is started.
 */


int initialise_uboot_wrapper(char* fdt_blob);

void shutdown_uboot_wrapper(void);

/* Routines for start up and shutdown of DMA management */

void sel4_dma_initialise(ps_dma_man_t *dma_manager);

void sel4_dma_shutdown(void);

