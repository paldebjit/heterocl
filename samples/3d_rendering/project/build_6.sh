#!/bin/bash

unset DISPLAY 
cd $PBS_O_WORKDIR
./run.sh BSTREAM pac_s10_dc kernel_6
