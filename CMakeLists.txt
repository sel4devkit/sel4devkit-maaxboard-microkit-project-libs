cmake_minimum_required(VERSION 3.7.2)

set(CPU cortex-a53)
set(TOOLCHAIN aarch64-linux-gnu)
set(CMAKE_C_COMPILER ${TOOLCHAIN}-gcc)
set(CMAKE_C_COMPILER_WORKS 1)
project(${MICROKIT_APP} LANGUAGES C)

set(LD ${TOOLCHAIN}-ld)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}-as)
set(MICROKIT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

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
target_include_directories(microkitdma PUBLIC 
    "${BOARD_DIR}/include" 
    "libubootdrivers/include/public_api/"
    "${PICOLIBC_DIR}/include/")

add_custom_target(make-img 
    ALL COMMAND bash -c
    "PYTHONPATH=${PYTHONPATH} MICROKIT_SDK=${MICROKIT_SDK} ${MICROKIT_TOOL} ../${MICROKIT_APP}.system --search-path ${BUILD_DIR} --board ${MICROKIT_BOARD} --config ${MICROKIT_CONFIG} -o ${IMAGE_FILE} -r ${REPORT_FILE}")

add_executable(${MICROKIT_APP}.elf example/${PLATFORM}/${MICROKIT_APP}/${MICROKIT_APP}.c)

# Linker
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



