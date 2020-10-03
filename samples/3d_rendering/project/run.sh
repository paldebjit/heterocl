#!/bin/bash

options () {
    echo "MODE: EMU/BSTREAM"
    echo "BOARD: pac_s10_dc/pac_a10"
    echo "KERNEL (File name except .cl extension)"
    echo -e "\n"
    return
}

welcome () {
    echo -e "\n"
    echo "Welcome to HeteroCL + Intel vLab compilation framework"
    echo "Run this script inside HCL project directory"
    echo -e "\n"
}

welcome

if (( $# < 3 ))
then
    printf "%b" "Error. Not enough arguments.\n"
    printf "%b" "Usage: run.sh MODE BOARD KERNEL\n"
    options
    exit 1
elif (( $# > 4 ))
then
    printf "%b" "Error. Too many arguments.\n"
    printf "%b" "Usage: run.sh MODE BOARD KERNEL\n\n"
    options
    exit 2
elif [ $# -eq 3 ]
then
    printf "%b" "Continuing processing\n\n"
    MODE=$1
    BOARD=$2
    KERNEL=$3
fi

# Cleanup all the trashes created from first run of HCL
rm -rf $KERNEL ${KERNEL}.aoc* ${KERNEL}.source time.out

# Make directories for device and host program compilation
mkdir -pv device_${KERNEL} hostbin

# Move kernel code to device directory
mv ${KERNEL}.cl ./device_${KERNEL}

# Move host file to host directory
mv host host.cpp Makefile Makefile.host obj save ./hostbin

# Time to compile kernel code
cd device_${KERNEL}

if [ $MODE == "BSTREAM" ]
then
    aoc -board=$BOARD -time time.out -time-passes -regtest_mode -v -fpc -fp-relaxed -opt-arg -nocaching -report -profile=all -I $INTELFPGAOCLSDKROOT/include/kernel_headers ${KERNEL}.cl
elif [ $MODE == "EMU" ]
then
    aoc -board=$BOARD -march=emulator -time time.out -time-passes -regtest_mode -v -fpc -fp-relaxed -opt-arg -nocaching -report -profile=all -I $INTELFPGAOCLSDKROOT/include/kernel_headers ${KERNEL}.cl
else
    echo "Unknown compilation options :("
fi

# Before proceed further, check for the bitstream file
AOCX_FILE=./${KERNEL}.aocx
if [ -f "$AOCX_FILE" ]; then
    printf "%b" "Kernel bitstream found. Continuing."
    exit 0
else
    printf "%b" "Kernel bitstream not found. Exiting now."
    printf "%b" "See build.log for more details."
    exit 3
fi

# Get out of the device directory and reach root of the project directory
cd ../host
make clean
make
cd ..

# Received FPGA devices via ``aocl list-devices" command. Program the FPGA
#aocl program ac10 device/kernel.aocx

