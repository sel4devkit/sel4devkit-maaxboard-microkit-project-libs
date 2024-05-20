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

// fdt initialise 
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

// picolibc setup
seL4_IPCBuffer* __sel4_ipc_buffer_obj;

static ps_dma_man_t dma_manager;

// DMA state
uintptr_t dma_base;
uintptr_t dma_cp_paddr;
size_t dma_size = 0x100000;

uintptr_t data_packet;

#define MMC_TX_BUF_LEN 4096
char* tx_buf = &data_packet;
uint mmc_pending_length = 0;


void handle_keypress(void) {
    printf("Reading input from the USB keyboard:\n");
    for (int x = 0; x <= 1000; x++) {
        while (uboot_stdin_tstc() > 0) {
            char c = uboot_stdin_getc();
            printf("Received character: %c\n", c, stdout);
            
            // Add character to MMC buffer
            if (mmc_pending_length < MMC_TX_BUF_LEN) {
                tx_buf[mmc_pending_length++] = c;
            }
        }
        udelay(10000);
    }
}

void
init(void)
{
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