#!/bin/bash

# Exit on error
set -e

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
  mkdir build
fi

# Go to build directory
cd build

# Run CMake
cmake ..

# Run Make
make

echo "Build successful! Executable is in build/create_raster"
