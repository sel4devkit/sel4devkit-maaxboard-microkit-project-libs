cmake_minimum_required(VERSION 3.7.2)

set(CPU cortex-a53)
set(TOOLCHAIN aarch64-linux-gnu)
set(CMAKE_C_COMPILER ${TOOLCHAIN}-gcc)
set(CMAKE_C_COMPILER_WORKS 1)
project(${MICROKIT_APP} LANGUAGES C)

set(LD ${TOOLCHAIN}-ld)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}-as)
set(MICROKIT_DIR /packages/microkit/auts_microkit_latest_plus_big_stack)

# Microkit specific information
set(MICROKIT_SDK ${MICROKIT_DIR}/release/microkit-sdk-1.2.6)
set(MICROKIT_BOARD ${PLATFORM})
set(MICROKIT_CONFIG debug)
# Set python variables
set(PYTHONPATH ${MICROKIT_DIR}/tool)
set(MICROKIT_TOOL "python3 -m microkit")
# Set dtb path
set(DTB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/boards/${PLATFORM}.dtb)

set(BOARD_DIR ${MICROKIT_SDK}/board/${MICROKIT_BOARD}/${MICROKIT_CONFIG})
set(PICOLIBC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../picolibc_build/picolibc/aarch64-linux-gnu)

set(CFLAGS -mcpu=${CPU} "-DDTB_PATH=\"${DTB_PATH}\"" -nostdlib -ffreestanding -Wall -Wno-unused-function)
set(LDFLAGS -L${BOARD_DIR}/lib -nostdlib)
set(LIBS -lmicrokit -Tmicrokit.ld -lgcc -L${PICOLIBC_DIR}/lib -lc -lm -lgcc)

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/example/${PLATFORM}/${MICROKIT_APP})
set(BUILD_DIR ${PROJECT_DIR}/example-build)
set(IMAGE_FILE ${BUILD_DIR}/sel4_image.img)
set(REPORT_FILE ${BUILD_DIR}/report.txt)

set(libUbootPlatform ${PLATFORM})

# Add libraries 
add_subdirectory(libubootdrivers)
add_subdirectory(libutils)
add_subdirectory(libmicrokitdma)
add_subdirectory(liblwip)
target_include_directories(microkitdma PUBLIC 
    "${BOARD_DIR}/include" 
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/")
target_include_directories(lwip PUBLIC
    "${BOARD_DIR}/include" 
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/lwip"
    "sddf/include/ethernet_config")

add_custom_target(make-img 
    ALL COMMAND bash -c
    "${MICROKIT_SDK}/bin/microkit ../${MICROKIT_APP}.system --board ${MICROKIT_BOARD} --config ${MICROKIT_CONFIG} --output ${MICROKIT_APP}.img --report report.txt")

add_executable(${MICROKIT_APP}.elf example/${PLATFORM}/${MICROKIT_APP}/${MICROKIT_APP}.c)
add_executable(virt_rx.elf network/virt_rx.c sddf/util/cache.c)
add_executable(virt_tx.elf network/virt_tx.c sddf/util/cache.c)
add_executable(eth.elf ethernet/ethernet.c)
add_executable(lwip.elf lwip.c utilization_socket.c udp_echo_socket.c timer/client/client.c)
add_executable(copy.elf network/copy.c)
add_executable(arp.elf network/arp.c)

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

# Linker - virt_rx
target_link_options(virt_rx.elf PRIVATE ${LDFLAGS})
target_compile_options(virt_rx.elf PRIVATE ${CFLAGS})
target_include_directories(virt_rx.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/ethernet_config"
)
target_link_libraries(virt_rx.elf PRIVATE ${LIBS})

# Linker - virt_tx
target_link_options(virt_tx.elf PRIVATE ${LDFLAGS})
target_compile_options(virt_tx.elf PRIVATE ${CFLAGS})
target_include_directories(virt_tx.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/ethernet_config"
)
target_link_libraries(virt_tx.elf PRIVATE ${LIBS})

# Linker - ethernet driver
target_link_options(eth.elf PRIVATE ${LDFLAGS})
target_compile_options(eth.elf PRIVATE ${CFLAGS})
target_include_directories(eth.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/ethernet_config"
    "ethernet"
)
target_link_libraries(eth.elf PRIVATE ${LIBS})

# Linker - lwip
target_link_options(lwip.elf PRIVATE ${LDFLAGS})
target_compile_options(lwip.elf PRIVATE ${CFLAGS})
target_include_directories(lwip.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/ethernet_config"
)
target_link_libraries(lwip.elf PUBLIC lwip)
target_link_libraries(lwip.elf PRIVATE ${LIBS})

# Linker - copy
target_link_options(copy.elf PRIVATE ${LDFLAGS})
target_compile_options(copy.elf PRIVATE ${CFLAGS})
target_include_directories(copy.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/ethernet_config"
    "ethernet"
)
target_link_libraries(copy.elf PRIVATE ${LIBS})

# Linker - arp
target_link_options(arp.elf PRIVATE ${LDFLAGS})
target_compile_options(arp.elf PRIVATE ${CFLAGS})
target_include_directories(arp.elf PRIVATE 
    "${BOARD_DIR}/include"  
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/"
    "sddf/include"
    "sddf/include/ethernet_config"
    "ethernet"
)
target_link_libraries(arp.elf PRIVATE ${LIBS})



