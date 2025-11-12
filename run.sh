#!/bin/bash
# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build the project using build.sh
"$SCRIPT_DIR/build.sh"

# Run the app if build succeeded
if [ $? -eq 0 ]; then
    echo "Launching Narrate..."
    "$SCRIPT_DIR/build/Narrate_artefacts/Standalone/Narrate"
else
    echo "Build failed!"
    exit 1
fi
