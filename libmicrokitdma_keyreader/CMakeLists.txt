#
# Copyright 2018, Data61, CSIRO (ABN 41 687 119 230)
#
# SPDX-License-Identifier: BSD-2-Clause
#

cmake_minimum_required(VERSION 3.8.2)

project(libmicrokitdma_keyreader C)

add_library(microkitdma_keyreader STATIC EXCLUDE_FROM_ALL src/dma.c)
target_include_directories(microkitdma_keyreader PUBLIC include)
target_link_libraries(microkitdma_keyreader PUBLIC utils ubootdrivers)
