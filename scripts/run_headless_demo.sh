#!/bin/bash
# SmartTerm Headless Demo Runner
#
# This script runs the headless demo in a clean environment
# and generates export files showing SmartTerm's features.

set -e

# Colors for output
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}SmartTerm Headless Demo Runner${NC}"
echo "================================"
echo

# Check if binary exists
if [ ! -f "build/bin/headless_demo" ]; then
    echo "Building SmartTerm library and examples..."
    make -f Makefile.lib clean
    make -f Makefile.lib lib
    make -f Makefile.lib examples
    echo
fi

# Set output directory
OUTPUT_DIR="${1:-./demo_output}"

echo "Running headless demo..."
echo "Output directory: $OUTPUT_DIR"
echo

# Run the demo with TERM set for ncurses
TERM=xterm ./build/bin/headless_demo "$OUTPUT_DIR" 2>&1 | grep -v "^\[" || true

echo
echo -e "${GREEN}✓ Demo complete!${NC}"
echo
echo "Generated files:"
ls -lh "$OUTPUT_DIR"/*.{txt,ansi,md,html} 2>/dev/null | awk '{print "  " $9 " (" $5 ")"}'
echo
echo "View the output:"
echo "  cat $OUTPUT_DIR/demo_output.txt          # Plain text"
echo "  cat $OUTPUT_DIR/demo_output.ansi         # ANSI colored (for terminal)"
echo "  cat $OUTPUT_DIR/demo_output.md           # Markdown format"
echo "  xdg-open $OUTPUT_DIR/demo_output.html    # HTML (in browser)"
