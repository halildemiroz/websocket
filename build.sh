#!/bin/bash
echo "Building Project..."

# Create build folder if it doesn't exist
mkdir -p build
cd build

# Run CMake and Make
cmake ..
make

echo "---------------------------"
echo "Build complete!"
echo "Client and Server binaries are in build folder"
echo "If you want to use hostChat script make sure server is running on your PC"
echo "Run './hostChat.sh' to start your server."
