#!/bin/bash -eu

# Use this script to force Matlab to load the most recent C++ library
# available on your system (GCC 4.6+ min.). You should chmod +x the 
# script and run it from the command line as: ./start_matlab.sh &
# 
# Close all previous instances of Matlab beforehand. You will NOT
# be able to run (or maybe even compile) the sources of the simulation
# if you do not start Matlab using this script.
# 
# Update the path to the appropriate library on your system if necessary.

LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libstdc++.so.6 \
LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu \
matlab $@