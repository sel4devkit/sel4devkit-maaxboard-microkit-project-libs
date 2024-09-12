cmake_minimum_required(VERSION 3.7.2)

set(CPU cortex-a53)
set(TOOLCHAIN aarch64-linux-gnu)
set(CMAKE_C_COMPILER ${TOOLCHAIN}-gcc)
set(CMAKE_C_COMPILER_WORKS 1)
project(${MICROKIT_APP} LANGUAGES C)

set(LD ${TOOLCHAIN}-ld)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}-as)
set(MICROKIT_DIR /host/sel4devkit-maaxboard-microkit)

# Microkit specific information
set(MICROKIT_SDK ${MICROKIT_DIR}/out/microkit-sdk-1.4.1)
set(MICROKIT_BOARD ${PLATFORM})
set(MICROKIT_CONFIG debug)

# Set dtb path
set(DTB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/boards/${PLATFORM}.dtb)

set(BOARD_DIR ${MICROKIT_SDK}/board/${MICROKIT_BOARD}/${MICROKIT_CONFIG})
set(PICOLIBC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../picolibc_build/picolibc/aarch64-linux-gnu)

set(CFLAGS -mcpu=${CPU} "-DDTB_PATH=\"${DTB_PATH}\"" -nostdlib -ffreestanding -Wall -Wno-unused-function)
set(LDFLAGS -L${BOARD_DIR}/lib -nostdlib)
set(LIBS -lmicrokit -Tmicrokit.ld -lgcc -L${PICOLIBC_DIR}/lib -lc -lm -lgcc)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../examples/${PLATFORM}/${MICROKIT_APP})
set(BUILD_DIR ${PROJECT_DIR}/build)
set(EXAMPLE_BUILD_DIR ${PROJECT_DIR}/example-build)
set(IMAGE_FILE ${EXAMPLE_BUILD_DIR}/sel4_image.img)
set(REPORT_FILE ${EXAMPLE_BUILD_DIR}/report.txt)

set(libUbootPlatform ${PLATFORM})

# sddf specific information
set(DRIV_DIR imx)
set(UART_DRIV_DIR imx)
set(TIMER_DRV_DIR imx)
set(CPU cortex-a53)
set(SDDF ${CMAKE_CURRENT_SOURCE_DIR}/sddf)
set(ETHERNET_DRIVER ${SDDF}/drivers/network/${DRIV_DIR})
set(NETWORK_COMPONENTS ${SDDF}/network/components/)
set(ECHO_SERVER ${SDDF}/examples/echo_server)
set(LWIP ${SDDF}/network/ipstacks/lwip)
set(BENCHMARK ${SDDF}/benchmark)
set(SERIAL ${SDDF}/drivers/serial/${UART_DRIV_DIR})
set(SERIAL_COMPONENTS ${SDDF}/serial/components)
set(TIMER_DRIVER ${SDDF}/drivers/timer/${TIMER_DRV_DIR})
  
# Set Macros depending on the application 
if(${MICROKIT_APP} STREQUAL "uboot_driver_example")
    set(IMAGES "${MICROKIT_APP}.elf")
    add_definitions(-DUBOOT_DRIVER_EXAMPLE)
endif()

if(${MICROKIT_APP} STREQUAL "security_demo")
    set(IMAGES "keyreader.elf" "transmitter.elf" "crypto.elf" "eth_driver.elf" "lwip.elf" "network_virt_rx.elf" "network_virt_tx.elf" "copy.elf")
    add_definitions(-DSECURITY_DEMO)
endif()

# Add libraries 
add_subdirectory(libubootdrivers)
add_subdirectory(libutils)
add_subdirectory(libmicrokitdma)
target_include_directories(microkitdma PUBLIC 
    "${BOARD_DIR}/include" 
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/")

# Add sddf library
# add_subdirectory(sddf)

# Add lwip library
add_subdirectory(${SDDF}/network/ipstacks/lwip)

# Transform image files
list(TRANSFORM IMAGES PREPEND "${BUILD_DIR}/")

if(${MICROKIT_APP} STREQUAL "uboot_driver_example")
add_executable(${MICROKIT_APP}.elf ${PROJECT_DIR}/${MICROKIT_APP}.c)
endif()

if(${MICROKIT_APP} STREQUAL "security_demo")
add_executable(keyreader.elf ${PROJECT_DIR}/keyreader/keyreader.c)
add_executable(transmitter.elf ${PROJECT_DIR}/transmitter/transmitter.c src/circular_buffer.c)
add_executable(crypto.elf ${PROJECT_DIR}/crypto/crypto.c src/circular_buffer.c)
add_executable(eth_driver.elf ${ETHERNET_DRIVER}/ethernet.c ${SDDF}/util/assert.c ${SDDF}/util/printf.c ${SDDF}/util/putchar_debug.c)
add_executable(lwip.elf ${ECHO_SERVER}/lwip.c ${ECHO_SERVER}/utilization_socket.c ${ECHO_SERVER}/udp_echo_socket.c ${ECHO_SERVER}/tcp_echo_socket.c ${SDDF}/util/assert.c ${SDDF}/util/printf.c ${SDDF}/util/putchar_serial.c)
add_executable(network_virt_rx.elf ${NETWORK_COMPONENTS}/virt_rx.c ${SDDF}/util/assert.c ${SDDF}/util/cache.c ${SDDF}/util/printf.c ${SDDF}/util/putchar_serial.c)
add_executable(network_virt_tx.elf ${NETWORK_COMPONENTS}/virt_tx.c ${SDDF}/util/assert.c ${SDDF}/util/cache.c ${SDDF}/util/printf.c ${SDDF}/util/putchar_serial.c)
add_executable(copy.elf ${NETWORK_COMPONENTS}/copy.c ${SDDF}/util/assert.c ${SDDF}/util/printf.c ${SDDF}/util/putchar_serial.c)
add_executable(timer_driver.elf ${TIMER_DRIVER}/timer.c ${SDDF}/util/assert.c ${SDDF}/util/printf.c ${SDDF}/util/putchar_serial.c ${SDDF}/util/putchar_serial.c)
endif()

if(${MICROKIT_APP} STREQUAL "uboot_driver_example")
# Linker - app
target_link_options(${MICROKIT_APP}.elf PRIVATE ${LDFLAGS})
target_compile_options(${MICROKIT_APP}.elf PRIVATE ${CFLAGS})
target_include_directories(${MICROKIT_APP}.elf PRIVATE 
    "${BOARD_DIR}/include" 
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
)
target_link_libraries(${MICROKIT_APP}.elf PUBLIC ubootdrivers utils microkitdma)
get_target_property(UBOOTDRIVERS_INCLUDE_DIRS ubootdrivers INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${MICROKIT_APP}.elf PRIVATE UBOOTDRIVERS_INCLUDE_DIRS)
target_link_libraries(${MICROKIT_APP}.elf PRIVATE ${LIBS})
endif()

if(${MICROKIT_APP} STREQUAL "security_demo")
# Linker - keyreader
target_link_options(keyreader.elf PRIVATE ${LDFLAGS})
target_compile_options(keyreader.elf PRIVATE ${CFLAGS})
target_include_directories(keyreader.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "include"
    "include/plat/${PLATFORM}/"
)
target_link_libraries(keyreader.elf PUBLIC ubootdrivers utils microkitdma)
get_target_property(UBOOTDRIVERS_INCLUDE_DIRS ubootdrivers INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(keyreader.elf PRIVATE UBOOTDRIVERS_INCLUDE_DIRS)
target_link_libraries(keyreader.elf PRIVATE ${LIBS})

# Linker - transmitter
target_link_options(transmitter.elf PRIVATE ${LDFLAGS})
target_compile_options(transmitter.elf PRIVATE ${CFLAGS})
target_include_directories(transmitter.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "include"
    "include/plat/${PLATFORM}/"
)
target_link_libraries(transmitter.elf PUBLIC ubootdrivers utils microkitdma)
get_target_property(UBOOTDRIVERS_INCLUDE_DIRS ubootdrivers INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(transmitter.elf PRIVATE UBOOTDRIVERS_INCLUDE_DIRS)
target_link_libraries(transmitter.elf PRIVATE ${LIBS})

# Linker - crypto
target_link_options(crypto.elf PRIVATE ${LDFLAGS})
target_compile_options(crypto.elf PRIVATE ${CFLAGS})
target_include_directories(crypto.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "${PICOLIBC_DIR}/include/"
    "include"
)
target_link_libraries(crypto.elf PUBLIC ubootdrivers utils)
get_target_property(UBOOTDRIVERS_INCLUDE_DIRS ubootdrivers INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(crypto.elf PRIVATE UBOOTDRIVERS_INCLUDE_DIRS)
target_link_libraries(crypto.elf PRIVATE ${LIBS})

# Linker - eth_driver
target_link_options(eth_driver.elf PRIVATE ${LDFLAGS})
target_compile_options(eth_driver.elf PRIVATE ${CFLAGS})
target_include_directories(eth_driver.elf PRIVATE 
    "${BOARD_DIR}/include"
    "include"
    "${ETHERNET_DRIVER}"
    "${SDDF}/include"
    "${ECHO_SERVER}/include/ethernet_config"
)
target_link_libraries(eth_driver.elf PUBLIC utils)
target_link_libraries(eth_driver.elf PRIVATE ${LIBS})

# Linker - lwip
target_link_options(lwip.elf PRIVATE ${LDFLAGS})
target_compile_options(lwip.elf PRIVATE ${CFLAGS})
target_include_directories(lwip.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "include"
    "${SDDF}/include"
    "${ECHO_SERVER}/include/ethernet_config"
    "${ECHO_SERVER}/include/serial_config"
    "${ECHO_SERVER}/include/lwip"
    "${LWIP}/src/include"
    "${PICOLIBC_DIR}/include/"
)
target_link_libraries(lwip.elf PUBLIC lwIP utils)
target_link_libraries(lwip.elf PRIVATE ${LIBS})

# Linker - network_virt_rx
target_link_options(network_virt_rx.elf PRIVATE ${LDFLAGS})
target_compile_options(network_virt_rx.elf PRIVATE ${CFLAGS})
target_include_directories(network_virt_rx.elf PRIVATE 
    "${BOARD_DIR}/include"
    "include"
    "${ETHERNET_DRIVER}"
    "${SDDF}/include"
    "${ECHO_SERVER}/include/ethernet_config"
)
target_link_libraries(network_virt_rx.elf PUBLIC utils)
target_link_libraries(network_virt_rx.elf PRIVATE ${LIBS})

# Linker - network_virt_tx
target_link_options(network_virt_tx.elf PRIVATE ${LDFLAGS})
target_compile_options(network_virt_tx.elf PRIVATE ${CFLAGS})
target_include_directories(network_virt_tx.elf PRIVATE 
    "${BOARD_DIR}/include"
    "include"
    "${ETHERNET_DRIVER}"
    "${SDDF}/include"
    "${ECHO_SERVER}/include/ethernet_config"
)
target_link_libraries(network_virt_tx.elf PUBLIC utils)
target_link_libraries(network_virt_tx.elf PRIVATE ${LIBS})

# Linker - copy
target_link_options(copy.elf PRIVATE ${LDFLAGS})
target_compile_options(copy.elf PRIVATE ${CFLAGS})
target_include_directories(copy.elf PRIVATE 
    "${BOARD_DIR}/include"
    "include"
    "${ETHERNET_DRIVER}"
    "${SDDF}/include"
    "${ECHO_SERVER}/include/ethernet_config"
)
target_link_libraries(copy.elf PUBLIC utils)
target_link_libraries(copy.elf PRIVATE ${LIBS})

# Linker - timer_driver
target_link_options(timer_driver.elf PRIVATE ${LDFLAGS})
target_compile_options(timer_driver.elf PRIVATE ${CFLAGS})
target_include_directories(timer_driver.elf PRIVATE 
    "${BOARD_DIR}/include"
    "include"
    "${SDDF}/include"
)
target_link_libraries(timer_driver.elf PUBLIC utils)
target_link_libraries(timer_driver.elf PRIVATE ${LIBS})
endif()

add_custom_target(make-img 
    ALL DEPENDS ${IMAGES} "${SYSTEM_FILE}" COMMAND bash -c
    "${MICROKIT_SDK}/bin/microkit ../${MICROKIT_APP}.system --search-path ${EXAMPLE_BUILD_DIR} --board ${MICROKIT_BOARD} --config ${MICROKIT_CONFIG} --output ${IMAGE_FILE} --report ${REPORT_FILE}")


