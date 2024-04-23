/* 
 * Copyright 2022, Capgemini Engineering
 * 
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This file performs all of the initialisation and shutdown of the library
 * required from within the seL4 'world', i.e. those actions that require
 * use of seL4 libraries such as mapping of the physical memory for the
 * devices to be used.
 *
 * Following successful initialisation within this file the 'uboot_wrapper'
 * is called to continue within the U-Boot 'world'.
 */

#include <libfdt.h>
#include <uboot_wrapper.h>
#include <utils/page.h>
#include <dma.h>

// The amount of extra space (in bytes) provided to our copy of the FDT to
// allow sufficient space for modifications.
#define EXTRA_FDT_BUFFER_SIZE 1024 * 8;

// Pointer to the FDT.
static void* uboot_fdt_pointer = NULL;

static int set_parent_status(int current_node, char *status_to_set)
{
    // Set status of this node.
    int err = fdt_setprop_string(uboot_fdt_pointer, current_node, "status", status_to_set);
    if (err != 0) {
        UBOOT_LOGE("Failed to set 'status' with error %i. Buffer not big enough?", err);
        return -1;
    };

    // Now set status of parent
    int parent_node = fdt_parent_offset(uboot_fdt_pointer, current_node);
    if (parent_node >= 0)
        if (set_parent_status(parent_node, status_to_set) != 0)
            return -1;

    return 0;
}

static int set_all_child_status(int parent_node, char *status_to_set)
{
    // Set status of this node.
    int err = fdt_setprop_string(uboot_fdt_pointer, parent_node, "status", status_to_set);
    if (err != 0) {
        UBOOT_LOGE("Failed to set 'status' with error %i. Buffer not bid enough?", err);
        return -1;
    };

    // Now set status of all children
    int child_node;
    fdt_for_each_subnode(child_node, uboot_fdt_pointer, parent_node)
        if (set_all_child_status(child_node, status_to_set) != 0)
            return -1;

    return 0;
}

static int disable_not_required_devices(const char **device_paths, uint32_t device_count)
{
    // Start off by recursively disabling all devices in the device tree
    if (set_all_child_status(fdt_path_offset(uboot_fdt_pointer, "/"), "disabled") != 0)
        return -1;

    // Now set the status for all parents and children (recursively) of our used
    // devices to 'okay'. This leaves only the minimum set of devices enabled
    // which we actually require.
    for (int dev_index=0; dev_index < device_count; dev_index++) {
        if (set_parent_status(fdt_path_offset(uboot_fdt_pointer, device_paths[dev_index]), "okay") != 0)
            return -1;
        if (set_all_child_status(fdt_path_offset(uboot_fdt_pointer, device_paths[dev_index]), "okay") != 0)
            return -1;
    }

    return 0;
}

int initialise_uboot_drivers(
    ps_dma_man_t dma_manager,
    const char *orig_fdt_blob,
    const char **dev_paths,
    uint32_t dev_count)
{
    // Return immediately if no devices have been requested.
    if (0 == dev_count || NULL == dev_paths) {
        UBOOT_LOGE("Library initialisation cancelled, no devices supplied");
        return -1;
    }

    int ret;

    // Create a copy of the FDT for U-Boot to use. We do this using
    // 'fdt_open_into' to open the FDT into a larger buffer to allow
    // us extra space to make modifications as required.

    if (orig_fdt_blob == NULL) {
        UBOOT_LOGE("Unable to access FDT");
        return -1;
    }

    int fdt_size = fdt_totalsize(orig_fdt_blob) + EXTRA_FDT_BUFFER_SIZE;
    uboot_fdt_pointer = malloc(fdt_size);
    if (uboot_fdt_pointer == NULL)
        return -ENOMEM;

    ret = fdt_open_into(orig_fdt_blob, uboot_fdt_pointer, fdt_size);
    if (0 != ret)
        goto error;

    // Start by disabling all devices in the FDT that are not required.
    ret = disable_not_required_devices(dev_paths, dev_count);
    if (0 != ret)
        goto error;

    // Map the required device resources for all required devices.
    // ret = map_required_device_resources(reg_paths, reg_count);
    // if (0 != ret)
    //     goto error;

    // Initalise microkit DMA, cached is set to true in the system file
    sel4_dma_initialise(&dma_manager);

    // Start the U-Boot wrapper. Provide it a pointer to the FDT blob.
    ret = initialise_uboot_wrapper(uboot_fdt_pointer);
    
    if (0 != ret)
        goto error;

    // All done.
    return 0;

    error:
        // Failed to initialise library, clean up and return error code.
        free(uboot_fdt_pointer);
        uboot_fdt_pointer = NULL;
        return -1;
}

void shutdown_uboot_drivers(void) {
    if (uboot_fdt_pointer != NULL) {
        free(uboot_fdt_pointer);
        uboot_fdt_pointer = NULL;
    }

    shutdown_uboot_wrapper();

    sel4_dma_shutdown();
}