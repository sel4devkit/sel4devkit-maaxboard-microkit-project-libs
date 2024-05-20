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

uintptr_t data_packet;

/* A buffer of encrypted characters to log to the SD/MMC card */
#define MMC_TX_BUF_LEN 4096
char* mmc_pending_tx_buf = (void *)(uintptr_t)0x5011000;
uint mmc_pending_length = 0;

#define LOG_FILE_DEVICE "mmc 0:1"  // Partition 1 on mmc device 0
#define LOG_FILENAME  "log.txt"

#define STR2(x) #x
#define STR(x) STR2(x)
#define INCBIN_SECTION ".rodata"
#define INCBIN(name, file) \
    __asm__(".section " INCBIN_SECTION "\n" \
            ".global incbin_" STR(name) "_start\n" \
            ".balign 16\n" \
            "incbin_" STR(name) "_start:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global incbin_" STR(name) "_end\n" \
            ".balign 1\n" \
            "incbin_" STR(name) "_end:\n" \
            ".byte 0\n" \
    ); \
    extern __attribute__((aligned(16))) const char incbin_ ## name ## _start[]; \
    extern                              const char incbin_ ## name ## _end[] 
INCBIN(device_tree, DTB_PATH); 

const char* _end = incbin_device_tree_end;

#define REG_TIMER_PATH      "/soc@0/bus@30400000/timer@306a0000"
#define REG_CCM_PATH        "/soc@0/bus@30000000/clock-controller@30380000"
#define REG_IOMUXC_PATH     "/soc@0/bus@30000000/iomuxc@30330000"
#define REG_OCOTP_PATH      "/soc@0/bus@30000000/ocotp-ctrl@30350000"
#define REG_SYSCON_PATH     "/soc@0/bus@30000000/syscon@30360000"
#define REG_USB_2_PATH      "/soc@0/usb@38200000"
#define REG_USB_PHY_2_PATH  "/soc@0/usb-phy@382f0040"
#define REG_MMC_PATH        "/soc@0/bus@30800000/mmc@30b40000"
#define REG_ETH_PATH        "/soc@0/bus@30800000/ethernet@30be0000"
#define REG_GPIO_1_PATH     "/soc@0/bus@30000000/gpio@30200000"
#define REG_GPIO_2_PATH     "/soc@0/bus@30000000/gpio@30210000"
#define REG_GPIO_3_PATH     "/soc@0/bus@30000000/gpio@30220000"
#define REG_GPIO_4_PATH     "/soc@0/bus@30000000/gpio@30230000"
#define REG_GPIO_5_PATH     "/soc@0/bus@30000000/gpio@30240000"
#define REG_I2C_0_PATH      "/soc@0/bus@30800000/i2c@30a20000"
#define REG_I2C_1_PATH      "/soc@0/bus@30800000/i2c@30a30000"
#define REG_I2C_2_PATH      "/soc@0/bus@30800000/i2c@30a40000"
#define REG_I2C_3_PATH      "/soc@0/bus@30800000/i2c@30a50000"
#define REG_SPI_0_PATH      "/soc@0/bus@30800000/spi@30820000"


/* List the set of device tree paths for the devices we wish to access.
 * Note these need ot be the root nodes of each device which the
 * the library supplies a driver */

#define DEV_TIMER_PATH      REG_TIMER_PATH
#define DEV_CCM_PATH        REG_CCM_PATH
#define DEV_IOMUXC_PATH     REG_IOMUXC_PATH
#define DEV_OCOTP_PATH      REG_OCOTP_PATH
#define DEV_SYSCON_PATH     REG_SYSCON_PATH
#define DEV_USB_2_PATH      REG_USB_2_PATH
#define DEV_USB_PHY_2_PATH  REG_USB_PHY_2_PATH
#define DEV_MMC_PATH        REG_MMC_PATH
#define DEV_ETH_PATH        REG_ETH_PATH
#define DEV_GPIO_1_PATH     REG_GPIO_1_PATH
#define DEV_GPIO_2_PATH     REG_GPIO_2_PATH
#define DEV_GPIO_3_PATH     REG_GPIO_3_PATH
#define DEV_GPIO_4_PATH     REG_GPIO_4_PATH
#define DEV_GPIO_5_PATH     REG_GPIO_5_PATH
#define DEV_I2C_0_PATH      REG_I2C_0_PATH
#define DEV_I2C_1_PATH      REG_I2C_1_PATH
#define DEV_I2C_2_PATH      REG_I2C_2_PATH
#define DEV_I2C_3_PATH      REG_I2C_3_PATH
#define DEV_SPI_0_PATH      REG_SPI_0_PATH
#define DEV_LEDS_PATH       "/leds"
#define DEV_CLK_1_PATH      "/clock-ckil"
#define DEV_CLK_2_PATH      "/clock-osc-25m"
#define DEV_CLK_3_PATH      "/clock-osc-27m"
#define DEV_CLK_4_PATH      "/clock-ext1"
#define DEV_CLK_5_PATH      "/clock-ext2"
#define DEV_CLK_6_PATH      "/clock-ext3"
#define DEV_CLK_7_PATH      "/clock-ext4"

#define DEV_PATH_COUNT 27

#define DEV_PATHS {                                                             \
    DEV_USB_2_PATH,                                                             \
    DEV_USB_PHY_2_PATH,                                                         \
    DEV_MMC_PATH,                                                               \
    DEV_ETH_PATH,                                                               \
    DEV_TIMER_PATH,                                                             \
    DEV_CCM_PATH,                                                               \
    DEV_OCOTP_PATH,                                                             \
    DEV_SYSCON_PATH,                                                            \
    DEV_IOMUXC_PATH,                                                            \
    DEV_GPIO_1_PATH,                                                            \
    DEV_GPIO_2_PATH,                                                            \
    DEV_GPIO_3_PATH,                                                            \
    DEV_GPIO_4_PATH,                                                            \
    DEV_GPIO_5_PATH,                                                            \
    DEV_I2C_0_PATH,                                                             \
    DEV_I2C_1_PATH,                                                             \
    DEV_I2C_2_PATH,                                                             \
    DEV_I2C_3_PATH,                                                             \
    DEV_SPI_0_PATH,                                                             \
    DEV_LEDS_PATH,                                                              \
    DEV_CLK_1_PATH,                                                             \
    DEV_CLK_2_PATH,                                                             \
    DEV_CLK_3_PATH,                                                             \
    DEV_CLK_4_PATH,                                                             \
    DEV_CLK_5_PATH,                                                             \
    DEV_CLK_6_PATH,                                                             \
    DEV_CLK_7_PATH                                                              \
    };



// DMA state
static ps_dma_man_t dma_manager;
uintptr_t dma_base;
uintptr_t dma_cp_paddr;
size_t dma_size = 0x100000;


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