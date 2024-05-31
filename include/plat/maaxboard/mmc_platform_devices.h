/*
 * Copyright 2022, Capgemini Engineering
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

/* List the set of device tree paths that include the 'reg' entries
 * for memory regions that will need to be mapped */

#define REG_TIMER_PATH      "/soc@0/bus@30400000/timer@306a0000"
#define REG_CCM_PATH        "/soc@0/bus@30000000/clock-controller@30380000"
#define REG_SYSCON_PATH     "/soc@0/bus@30000000/syscon@30360000"
#define REG_IOMUXC_PATH     "/soc@0/bus@30000000/iomuxc@30330000"
#define REG_MMC_PATH        "/soc@0/bus@30800000/mmc@30b40000"

#define REG_PATH_COUNT 5

#define REG_PATHS {                                                             \
    REG_MMC_PATH,                                                               \
    REG_TIMER_PATH,                                                             \
    REG_CCM_PATH,                                                               \
    REG_SYSCON_PATH,                                                            \
    REG_IOMUXC_PATH,                                                            \
    };

/* List the set of device tree paths for the devices we wish to access.
 * Note these need ot be the root nodes of each device which the
 * the library supplies a driver */

#define DEV_TIMER_PATH      REG_TIMER_PATH
#define DEV_CCM_PATH        REG_CCM_PATH
#define DEV_SYSCON_PATH     REG_SYSCON_PATH
#define DEV_MMC_PATH        REG_MMC_PATH
#define DEV_IOMUXC_PATH     REG_IOMUXC_PATH
#define DEV_CLK_1_PATH      "/clock-ckil"
#define DEV_CLK_2_PATH      "/clock-osc-25m"
#define DEV_CLK_3_PATH      "/clock-osc-27m"
#define DEV_CLK_4_PATH      "/clock-ext1"
#define DEV_CLK_5_PATH      "/clock-ext2"
#define DEV_CLK_6_PATH      "/clock-ext3"
#define DEV_CLK_7_PATH      "/clock-ext4"

#define DEV_PATH_COUNT 12

#define DEV_PATHS {                                                             \
    DEV_MMC_PATH,                                                               \
    DEV_TIMER_PATH,                                                             \
    DEV_CCM_PATH,                                                               \
    DEV_SYSCON_PATH,                                                            \
    DEV_IOMUXC_PATH,                                                            \
    DEV_CLK_1_PATH,                                                             \
    DEV_CLK_2_PATH,                                                             \
    DEV_CLK_3_PATH,                                                             \
    DEV_CLK_4_PATH,                                                             \
    DEV_CLK_5_PATH,                                                             \
    DEV_CLK_6_PATH,                                                             \
    DEV_CLK_7_PATH                                                              \
};