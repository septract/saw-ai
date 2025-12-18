#!/bin/bash
# Setup script for SAW formal verification tool
# Downloads and installs SAW locally to tools/
#
# No sudo required - everything stays in the project directory

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
TOOLS_DIR="$PROJECT_DIR/tools"

# Tool version and URL
SAW_VERSION="1.4"

# Detect platform
OS=$(uname -s)
ARCH=$(uname -m)

case "$OS" in
    Darwin)
        if [ "$ARCH" = "arm64" ]; then
            SAW_FILE="saw-${SAW_VERSION}-macos-14-ARM64-with-solvers.tar.gz"
        else
            SAW_FILE="saw-${SAW_VERSION}-macos-14-x86_64-with-solvers.tar.gz"
        fi
        ;;
    Linux)
        SAW_FILE="saw-${SAW_VERSION}-linux-x86_64-with-solvers.tar.gz"
        ;;
    *)
        echo "Unsupported operating system: $OS"
        exit 1
        ;;
esac

SAW_URL="https://github.com/GaloisInc/saw-script/releases/download/v${SAW_VERSION}/${SAW_FILE}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "=========================================="
echo "SAW Formal Verification Tool Setup"
echo "=========================================="
echo ""
echo "This will download and install SAW locally to:"
echo "  $TOOLS_DIR"
echo ""
echo "Detected platform: $OS ($ARCH)"
echo "SAW version: ${SAW_VERSION}"
echo "Download size: ~245MB"
echo ""

# Create tools directory
mkdir -p "$TOOLS_DIR"
cd "$TOOLS_DIR"

# Download and install SAW
echo "Downloading SAW..."
echo "--------------------------------------------"

if [ -d "$TOOLS_DIR/saw" ]; then
    echo -e "${GREEN}Already installed${NC} at $TOOLS_DIR/saw"
    echo ""
    echo "To reinstall, remove the directory first:"
    echo "  rm -rf $TOOLS_DIR/saw"
else
    echo "Downloading SAW v${SAW_VERSION} with solvers..."
    curl -L --progress-bar -o "$SAW_FILE" "$SAW_URL"

    echo "Extracting..."
    tar -xzf "$SAW_FILE"

    # Find and rename the extracted directory
    EXTRACTED_DIR=$(ls -d saw-${SAW_VERSION}-* 2>/dev/null | head -1)
    if [ -n "$EXTRACTED_DIR" ]; then
        mv "$EXTRACTED_DIR" saw
    fi
    rm "$SAW_FILE"

    echo -e "${GREEN}Installed${NC} SAW"
fi

# Verify installation
echo ""
echo "=========================================="
echo "Verifying Installation"
echo "=========================================="
echo ""

export PATH="$TOOLS_DIR/saw/bin:$PATH"

echo -n "SAW: "
if saw --version 2>/dev/null; then
    echo -e "  ${GREEN}OK${NC}"
else
    echo -e "  ${RED}FAILED${NC}"
fi

echo -n "Cryptol: "
if cryptol --version 2>/dev/null; then
    echo -e "  ${GREEN}OK${NC}"
else
    echo -e "  ${RED}FAILED${NC}"
fi

echo -n "Z3 solver: "
if z3 --version 2>/dev/null; then
    echo -e "  ${GREEN}OK${NC}"
else
    echo -e "  ${RED}FAILED${NC}"
fi

echo ""
echo "=========================================="
echo -e "${GREEN}Setup Complete!${NC}"
echo "=========================================="
echo ""
echo "To use SAW, add to your PATH:"
echo ""
echo "  export PATH=\"$TOOLS_DIR/saw/bin:\$PATH\""
echo ""
echo "Or source the environment script:"
echo ""
echo "  source $SCRIPT_DIR/saw-env.sh"
echo ""
