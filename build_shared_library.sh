#!/bin/bash
# Build script for Linux/macOS to create SSIMULACRA2 shared library

echo "Building SSIMULACRA2 shared library for $(uname)..."

# Create build directory
mkdir -p build
cd build

# Try to find vcpkg toolchain file
VCPKG_TOOLCHAIN=""
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    echo "Found vcpkg at: $VCPKG_ROOT"
elif [ -f "/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_TOOLCHAIN="/usr/local/share/vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "Found vcpkg at: /usr/local/share/vcpkg"
elif [ -f "$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_TOOLCHAIN="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "Found vcpkg at: $HOME/vcpkg"
else
    echo "Warning: vcpkg not found. Image format libraries may not be available."
    echo "To use vcpkg, set VCPKG_ROOT environment variable or install vcpkg"
    echo ""
    echo "You can install vcpkg by running:"
    echo "  git clone https://github.com/Microsoft/vcpkg.git ~/vcpkg"
    echo "  ~/vcpkg/bootstrap-vcpkg.sh"
fi

# Configure with CMake
if [ -n "$VCPKG_TOOLCHAIN" ]; then
    echo "Using vcpkg toolchain: $VCPKG_TOOLCHAIN"
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSSIMULACRA2_BUILD_EXECUTABLE=ON -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" ../src
else
    echo "Building without vcpkg integration"
    cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DSSIMULACRA2_BUILD_EXECUTABLE=ON ../src
fi

# Build the project
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)