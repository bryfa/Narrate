#!/bin/bash
# Narrate Build Script
# Build script for Narrate project with support for different build targets

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Parse command line arguments
BUILD_TARGET="ALL"
SHOW_HELP=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --target)
            BUILD_TARGET="$2"
            shift 2
            ;;
        --help|-h)
            SHOW_HELP=true
            shift
            ;;
        *)
            echo "Unknown option: $1"
            SHOW_HELP=true
            shift
            ;;
    esac
done

# Show help if requested or if invalid target
if [ "$SHOW_HELP" = true ]; then
    echo "Narrate Build Script"
    echo "===================="
    echo ""
    echo "Usage: ./build.sh [options]"
    echo ""
    echo "Options:"
    echo "  --target <TARGET>    Specify what to build (default: ALL)"
    echo "                       Options: ALL, PLUGIN, STANDALONE, CONSOLE"
    echo "  --help, -h           Show this help message"
    echo ""
    echo "Build Targets:"
    echo "  ALL          Build everything (VST3 + Standalone + Console)"
    echo "  PLUGIN       Build only VST3 plugin"
    echo "  STANDALONE   Build only standalone application"
    echo "  CONSOLE      Build only console export tool"
    echo ""
    echo "Examples:"
    echo "  ./build.sh"
    echo "  ./build.sh --target CONSOLE"
    echo "  ./build.sh --target STANDALONE"
    exit 0
fi

# Validate BUILD_TARGET
case $BUILD_TARGET in
    ALL|PLUGIN|STANDALONE|CONSOLE)
        ;;
    *)
        echo "Error: Invalid BUILD_TARGET: $BUILD_TARGET"
        echo "Valid options are: ALL, PLUGIN, STANDALONE, CONSOLE"
        exit 1
        ;;
esac

echo "========================================"
echo "  Narrate Build Script"
echo "  Target: $BUILD_TARGET"
echo "========================================"
echo ""

# Create build directory if it doesn't exist
BUILD_DIR="$SCRIPT_DIR/build"
if [ ! -d "$BUILD_DIR" ]; then
    echo "Creating build directory..."
    mkdir -p "$BUILD_DIR"
fi

# Navigate to the build directory
cd "$BUILD_DIR"

# Configure CMake if needed (if CMakeCache.txt doesn't exist or target changed)
RECONFIGURE=false
if [ ! -f "CMakeCache.txt" ]; then
    RECONFIGURE=true
else
    # Check if BUILD_TARGET has changed
    CACHED_TARGET=$(grep "BUILD_TARGET:" CMakeCache.txt | cut -d'=' -f2)
    if [ "$CACHED_TARGET" != "$BUILD_TARGET" ]; then
        echo "Build target changed from $CACHED_TARGET to $BUILD_TARGET"
        RECONFIGURE=true
    fi
fi

if [ "$RECONFIGURE" = true ]; then
    echo "Configuring CMake with BUILD_TARGET=$BUILD_TARGET..."
    # Use Unix Makefiles generator for cross-platform compatibility
    cmake -G "Unix Makefiles" -DBUILD_TARGET=$BUILD_TARGET "$SCRIPT_DIR"
    if [ $? -ne 0 ]; then
        echo "CMake configuration failed!"
        cd "$SCRIPT_DIR"
        exit 1
    fi
fi

# Build the project
echo "Building project..."
# Detect number of CPU cores (cross-platform)
if [[ "$OSTYPE" == "darwin"* ]]; then
    NCORES=$(sysctl -n hw.ncpu)
else
    NCORES=$(nproc)
fi
cmake --build . --config Release -j$NCORES

# Check if build succeeded
if [ $? -eq 0 ]; then
    echo "Build successful!"
    cd "$SCRIPT_DIR"
    exit 0
else
    echo "Build failed!"
    cd "$SCRIPT_DIR"
    exit 1
fi
