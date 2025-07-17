#!/bin/bash
# Setup script for Linux/macOS to install dependencies for SSIMULACRA2

echo "Setting up dependencies for SSIMULACRA2..."
echo

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    OS="unknown"
fi

# Try to install system packages first
if [[ "$OS" == "linux" ]]; then
    echo "Detected Linux, attempting to install system packages..."

    # Try different package managers
    if command -v apt-get &> /dev/null; then
        echo "Using apt-get..."
        sudo apt-get update
        sudo apt-get install -y libhwy-dev liblcms2-dev cmake build-essential \
                               libjpeg-dev libpng-dev zlib1g-dev libgif-dev
        if [[ $? -eq 0 ]]; then
            echo "Dependencies installed successfully with apt-get!"
            echo
            echo "You can now run: ./build_shared_library.sh"
            exit 0
        fi
    elif command -v yum &> /dev/null; then
        echo "Using yum..."
        sudo yum install -y highway-devel lcms2-devel cmake gcc-c++
        if [[ $? -eq 0 ]]; then
            echo "Dependencies installed successfully with yum!"
            echo
            echo "You can now run: ./build_shared_library.sh"
            exit 0
        fi
    elif command -v pacman &> /dev/null; then
        echo "Using pacman..."
        sudo pacman -S --noconfirm highway lcms2 cmake base-devel
        if [[ $? -eq 0 ]]; then
            echo "Dependencies installed successfully with pacman!"
            echo
            echo "You can now run: ./build_shared_library.sh"
            exit 0
        fi
    fi

elif [[ "$OS" == "macos" ]]; then
    echo "Detected macOS, attempting to install with Homebrew..."

    if command -v brew &> /dev/null; then
        brew install highway lcms2 cmake
        if [[ $? -eq 0 ]]; then
            echo "Dependencies installed successfully with Homebrew!"
            echo
            echo "You can now run: ./build_shared_library.sh"
            exit 0
        fi
    else
        echo "Homebrew not found. Please install Homebrew first:"
        echo "/bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\""
    fi
fi

# Ensure LCMS2 has our CMakeLists.txt
if [ -d "src/third_party/lcms" ] && [ ! -f "src/third_party/lcms/CMakeLists.txt" ]; then
    echo "Adding CMakeLists.txt for LCMS2..."
    cp lcms2_files/CMakeLists.txt src/third_party/lcms/CMakeLists.txt
fi

# Initialize and update submodules
echo "Initializing and updating submodules..."
git submodule update --init --recursive

echo
echo "Dependencies setup complete!"
echo
echo "You can now run: ./build_shared_library.sh"