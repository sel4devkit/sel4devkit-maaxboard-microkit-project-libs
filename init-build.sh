#!/bin/bash

# Microkit path
microkit_path=${PWD}

# Initialize variables to store the values of the command-line options
MICROKIT_APP=""
PLATFORM=""

# Function to display usage information
usage() {
    echo "Usage: $0 -DMICROKIT_APP=app_name -DPLATFORM=platform"
    exit 1
}

# Parse command-line options
while [ "$#" -gt 0 ]; do
    case $1 in
        -DMICROKIT_APP=*)
            MICROKIT_APP="${1#*=}"
            ;;
        -DPLATFORM=*)
            PLATFORM="${1#*=}"
            ;;
        *)
            usage
            ;;
    esac
    shift
done

# Check if mandatory options are provided
if [ -z "$MICROKIT_APP" ] || [ -z "$PLATFORM" ]; then
    echo "Error: Missing mandatory arguments."
    usage
fi

echo "MICROKIT_APP: $MICROKIT_APP"
echo "PLATFORM: $PLATFORM"


# Build SDK
EXTRACTED_DIR="gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf"
if [ ! -d "$EXTRACTED_DIR" ]; then
    curl --output compiler.xz https://armkeil.blob.core.windows.net/developer/Files/downloads/gnu-a/10.2-2020.11/binrel/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf.tar.xz
    tar -xvf compiler.xz
    rm -r compiler.xz
fi
export PATH=${PWD}/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin:$PATH
echo "Do you want to build the sdk? (yes/no)"
read user_input

# Convert the input to lowercase 
user_input=$(echo "$user_input" | tr '[:upper:]' '[:lower:]')
if [ "$user_input" = "yes" ]; then
    python3 -m venv "pyenv"
    ./pyenv/bin/pip install --upgrade pip setuptools wheel
    ./pyenv/bin/pip install -r "requirements.txt"
    ./pyenv/bin/python build_sdk.py --sel4 ../seL4
    python3 build_sdk.py --sel4 ../seL4
fi

# Build picolibc
echo "Do you want to build picolibc? (yes/no)"
read user_input1

# Convert the input to lowercase 
user_input1=$(echo "$user_input1" | tr '[:upper:]' '[:lower:]')

if [ "$user_input1" = "yes" ]; then
    rm -rf ../picolibc/picolibc-microkit/
    mkdir ../picolibc/picolibc-microkit/
    cd ../picolibc/picolibc-microkit/
    rm -rf ../../picolibc_build
    mkdir ../../picolibc_build
    ../scripts/do-aarch64-configure-nocrt -Dprefix=${PWD}/../../picolibc_build
    sudo ninja 
    sudo ninja install
fi

# Build application 
cd ../../microkit
rm -rf example/maaxboard/$MICROKIT_APP/build
mkdir example/maaxboard/$MICROKIT_APP/build
rm -rf example/maaxboard/$MICROKIT_APP/example-build
mkdir example/maaxboard/$MICROKIT_APP/example-build
cd example/maaxboard/$MICROKIT_APP/build
cmake -DMICROKIT_APP=$MICROKIT_APP -DPLATFORM=$PLATFORM $microkit_path
make 
echo "Built image is here: ${microkit_path}/example/${PLATFORM}/uboot-driver-example/example-build/sel4_image.img" 