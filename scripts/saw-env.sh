# SAW environment setup
# Source this file to add SAW to your PATH
#
# Usage: source scripts/saw-env.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
TOOLS_DIR="$PROJECT_DIR/tools"

if [ -d "$TOOLS_DIR/saw/bin" ]; then
    export PATH="$TOOLS_DIR/saw/bin:$PATH"
    echo "SAW tools added to PATH"
else
    echo "SAW not installed. Run: ./scripts/install-saw.sh"
fi
