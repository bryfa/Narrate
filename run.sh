#!/bin/bash
# Narrate Run Script
# Helper script for running different built targets

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_DIR="$SCRIPT_DIR/build"

# Parse command line arguments
TARGET="STANDALONE"  # Default to STANDALONE
SHOW_HELP=false
CONSOLE_ARGS=()

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --target)
            TARGET="$2"
            shift 2
            ;;
        --help|-h)
            SHOW_HELP=true
            shift
            ;;
        *)
            # Collect remaining args for console
            CONSOLE_ARGS+=("$1")
            shift
            ;;
    esac
done

# Show help if requested
if [ "$SHOW_HELP" = true ]; then
    echo "Narrate Run Script"
    echo "=================="
    echo ""
    echo "Usage: ./run.sh [--target <TARGET>] [args...]"
    echo ""
    echo "Options:"
    echo "  --target <TARGET>    Specify which target to run (default: STANDALONE)"
    echo "                       Options: STANDALONE, CONSOLE"
    echo "  --help, -h           Show this help message"
    echo ""
    echo "Targets:"
    echo "  STANDALONE   Run the standalone application (default)"
    echo "  CONSOLE      Run the console export tool"
    echo "               (Pass additional args for console tool)"
    echo ""
    echo "Examples:"
    echo "  ./run.sh                    # Runs standalone by default"
    echo "  ./run.sh --target STANDALONE"
    echo "  ./run.sh --target CONSOLE project.narrate output.json --format json"
    echo "  ./run.sh --target CONSOLE --help"
    exit 0
fi

# Validate target
case $TARGET in
    STANDALONE|CONSOLE)
        ;;
    *)
        echo -e "${RED}Error: Invalid target: $TARGET${NC}"
        echo "Valid options are: STANDALONE, CONSOLE"
        exit 1
        ;;
esac

echo -e "${YELLOW}======================================${NC}"
echo -e "${YELLOW}  Narrate Run Script${NC}"
echo -e "${YELLOW}  Target: $TARGET${NC}"
echo -e "${YELLOW}======================================${NC}"
echo ""

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${RED}Error: Build directory not found${NC}"
    echo "Please run ./build.sh first to build the project"
    exit 1
fi

# Run the appropriate target
case $TARGET in
    STANDALONE)
        STANDALONE_EXE="$BUILD_DIR/Narrate_artefacts/Standalone/Narrate"

        if [ ! -f "$STANDALONE_EXE" ]; then
            echo -e "${RED}Error: Standalone executable not found${NC}"
            echo "Expected location: $STANDALONE_EXE"
            echo ""
            echo "Please build the standalone target first:"
            echo "  ./build.sh --target STANDALONE"
            echo "or"
            echo "  ./build.sh --target ALL"
            exit 1
        fi

        echo -e "${GREEN}Launching Narrate Standalone...${NC}"
        "$STANDALONE_EXE" &
        echo -e "${GREEN}Standalone application launched in background${NC}"
        ;;

    CONSOLE)
        CONSOLE_EXE="$BUILD_DIR/NarrateConsole"

        if [ ! -f "$CONSOLE_EXE" ]; then
            echo -e "${RED}Error: Console executable not found${NC}"
            echo "Expected location: $CONSOLE_EXE"
            echo ""
            echo "Please build the console target first:"
            echo "  ./build.sh --target CONSOLE"
            echo "or"
            echo "  ./build.sh --target ALL"
            exit 1
        fi

        if [ ${#CONSOLE_ARGS[@]} -eq 0 ]; then
            echo -e "${CYAN}Running console tool (no arguments - showing help)...${NC}"
            "$CONSOLE_EXE" --help
        else
            echo -e "${CYAN}Running console tool with arguments...${NC}"
            "$CONSOLE_EXE" "${CONSOLE_ARGS[@]}"
        fi
        ;;
esac

exit $?
