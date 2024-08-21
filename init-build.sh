#!/bin/bash

# Project libs path
PROJECT_LIBS_DIR=${PWD}

# Initialize variables to store the values of the command-line options
MICROKIT_APP=""
PLATFORM=""
BUILD_TYPE=""

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
        -DBUILD_TYPE=*)
            BUILD_TYPE="${1#*=}"
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
echo "BUILD_TYPE: $BUILD_TYPE"

# Go back to root dir
cd ..

# Path's
ROOT_DIR="${PWD}"
PROJECT_DIR="$ROOT_DIR/examples/maaxboard/$MICROKIT_APP"

# Build picolibc
echo "Do you want to build picolibc? (yes/no)"
read user_input1

# Convert the input to lowercase 
user_input1=$(echo "$user_input1" | tr '[:upper:]' '[:lower:]')

if [ "$user_input1" = "yes" ]; then
    PICOLIBC_MICROKIT_DIR="picolibc/picolibc-microkit"
    rm -rf $PICOLIBC_MICROKIT_DIR
    mkdir $PICOLIBC_MICROKIT_DIR
    cd $PICOLIBC_MICROKIT_DIR
    sudo rm -rf ../picolibc_build
    mkdir ../picolibc_build
    ../scripts/do-aarch64-configure-nocrt -Dprefix=$ROOT_DIR/picolibc_build
    sudo ninja 
    sudo ninja install
fi

# Build application 
cd $PROJECT_DIR
if [ ! -d "build" ]; then
    mkdir build
fi
if [ ! -d "example-build" ]; then
    mkdir example-build
fi
if [ "$BUILD_TYPE" == "rebuild" ]; then
    rm -rf build
    mkdir build
    rm -rf example-build
    mkdir example-build
fi
cd build
cmake -DMICROKIT_APP=$MICROKIT_APP -DPLATFORM=$PLATFORM $PROJECT_LIBS_DIR
make 
echo "Built image is here: $PROJECT_DIR/example-build/sel4_image.img" 