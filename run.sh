#!/bin/bash
# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Create build directory if it doesn't exist
BUILD_DIR="$SCRIPT_DIR/build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Configure CMake if needed (if CMakeCache.txt doesn't exist)
if [ ! -f "CMakeCache.txt" ]; then
    echo "Configuring CMake..."
    cmake "$SCRIPT_DIR"
    if [ $? -ne 0 ]; then
        echo "CMake configuration failed!"
        cd "$SCRIPT_DIR"
        exit 1
    fi
fi

# Build the project
echo "Building project..."
cmake --build . --config Release

# Run the app if build succeeded
if [ $? -eq 0 ]; then
    echo "Launching Narrate..."
    ./Narrate_artefacts/Standalone/Narrate
    # Return to the Narrate root folder
    cd "$SCRIPT_DIR"
else
    echo "Build failed!"
    cd "$SCRIPT_DIR"
    exit 1
fi
