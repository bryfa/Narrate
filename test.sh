#!/bin/bash
# Narrate Test Runner Script
# This script builds and runs tests for the Narrate project

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

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

# Show help if requested
if [ "$SHOW_HELP" = true ]; then
    echo "Narrate Test Runner Script"
    echo "=========================="
    echo ""
    echo "Usage: ./test.sh [options]"
    echo ""
    echo "Options:"
    echo "  --target <TARGET>    Specify build target for tests (default: ALL)"
    echo "                       Options: ALL, PLUGIN, STANDALONE, CONSOLE"
    echo "  --help, -h           Show this help message"
    echo ""
    echo "Notes:"
    echo "  - Tests are built with the same BUILD_TARGET configuration"
    echo "  - The BUILD_TARGET determines which code paths are tested"
    echo ""
    echo "Examples:"
    echo "  ./test.sh"
    echo "  ./test.sh --target CONSOLE"
    echo "  ./test.sh --target STANDALONE"
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

echo -e "${YELLOW}========================================${NC}"
echo -e "${YELLOW}  Narrate Test Runner${NC}"
echo -e "${YELLOW}  Target: $BUILD_TARGET${NC}"
echo -e "${YELLOW}========================================${NC}"
echo ""

# Check if build directory exists
if [ ! -d "build" ]; then
    echo -e "${RED}Error: build directory not found${NC}"
    echo "Please run ./build.sh first to configure the project"
    exit 1
fi

cd build

# Configure with tests enabled and specified target
echo -e "${YELLOW}Configuring with tests enabled (BUILD_TARGET=$BUILD_TARGET)...${NC}"
cmake -DBUILD_TESTS=ON -DBUILD_TARGET=$BUILD_TARGET ..

# Build tests
echo -e "${YELLOW}Building tests...${NC}"
cmake --build . --target NarrateTests

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}Build successful!${NC}"
echo ""

# Run tests
echo -e "${YELLOW}Running tests...${NC}"
echo ""

# Run the test executable directly for better output
if [ -f "./NarrateTests" ]; then
    ./NarrateTests "$@"
    TEST_RESULT=$?
elif [ -f "./NarrateTests.exe" ]; then
    ./NarrateTests.exe "$@"
    TEST_RESULT=$?
else
    echo -e "${RED}Test executable not found!${NC}"
    exit 1
fi

echo ""

# Report results
if [ $TEST_RESULT -eq 0 ]; then
    echo -e "${GREEN}========================================${NC}"
    echo -e "${GREEN}  All tests passed! ✓${NC}"
    echo -e "${GREEN}========================================${NC}"
else
    echo -e "${RED}========================================${NC}"
    echo -e "${RED}  Some tests failed ✗${NC}"
    echo -e "${RED}========================================${NC}"
    exit 1
fi
