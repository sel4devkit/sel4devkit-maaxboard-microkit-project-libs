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
    cd ../../project_libs
fi

# Build application 
rm -rf example/maaxboard/$MICROKIT_APP/build
mkdir example/maaxboard/$MICROKIT_APP/build
rm -rf example/maaxboard/$MICROKIT_APP/example-build
mkdir example/maaxboard/$MICROKIT_APP/example-build
cd example/maaxboard/$MICROKIT_APP/build
cmake -DMICROKIT_APP=$MICROKIT_APP -DPLATFORM=$PLATFORM $microkit_path
make 
echo "Built image is here: ${microkit_path}/example/${PLATFORM}/${MICROKIT_APP}/example-build/sel4_image.img" 