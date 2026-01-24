#!/bin/bash
#
# cc-bash installer
# Usage: ./install.sh [--prefix=/path] [--from-release]
#
# Options:
#   --prefix=PATH     Install to PATH/bin (default: /usr/local)
#   --from-release    Download prebuilt binary from GitHub releases
#   --help            Show this help
#

set -e

# Defaults
PREFIX="/usr/local"
FROM_RELEASE=false
REPO="jcaldwell-labs/smartterm-prototype"
CONFIG_DIR="$HOME/.cc-bash"
CONFIG_FILE="$HOME/.cc-bashrc"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

info() { echo -e "${CYAN}[INFO]${NC} $1"; }
success() { echo -e "${GREEN}[OK]${NC} $1"; }
warn() { echo -e "${YELLOW}[WARN]${NC} $1"; }
error() { echo -e "${RED}[ERROR]${NC} $1"; exit 1; }

# Parse arguments
for arg in "$@"; do
    case $arg in
        --prefix=*)
            PREFIX="${arg#*=}"
            ;;
        --from-release)
            FROM_RELEASE=true
            ;;
        --help|-h)
            echo "cc-bash installer"
            echo ""
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --prefix=PATH     Install to PATH/bin (default: /usr/local)"
            echo "  --from-release    Download prebuilt binary from GitHub releases"
            echo "  --help            Show this help"
            echo ""
            echo "Examples:"
            echo "  $0                        # Build and install to /usr/local/bin"
            echo "  sudo $0                   # Same, with sudo for system install"
            echo "  $0 --prefix=\$HOME/.local  # Install to ~/.local/bin"
            echo "  $0 --from-release         # Download prebuilt binary"
            exit 0
            ;;
        *)
            error "Unknown option: $arg"
            ;;
    esac
done

BINDIR="$PREFIX/bin"

echo ""
echo "╔════════════════════════════════════════╗"
echo "║       cc-bash installer                ║"
echo "╚════════════════════════════════════════╝"
echo ""

# Check dependencies
info "Checking dependencies..."

if ! command -v gcc &> /dev/null && [ "$FROM_RELEASE" = false ]; then
    error "gcc not found. Install with: sudo apt-get install build-essential"
fi

if [ "$FROM_RELEASE" = false ]; then
    # Check for readline
    if ! ldconfig -p 2>/dev/null | grep -q libreadline || \
       ! [ -f /usr/include/readline/readline.h ] && ! [ -f /usr/local/include/readline/readline.h ]; then
        warn "libreadline-dev may not be installed"
        echo "  Install with: sudo apt-get install libreadline-dev"
        echo ""
    fi
fi

success "Dependencies OK"

# Get or build binary
if [ "$FROM_RELEASE" = true ]; then
    info "Downloading latest release..."

    # Detect architecture
    ARCH=$(uname -m)
    case $ARCH in
        x86_64)  ARCH_NAME="linux-x86_64" ;;
        aarch64) ARCH_NAME="linux-arm64" ;;
        arm64)   ARCH_NAME="linux-arm64" ;;
        *)       error "Unsupported architecture: $ARCH" ;;
    esac

    # Get latest release URL
    RELEASE_URL="https://github.com/$REPO/releases/latest/download/cc-bash-$ARCH_NAME"

    # Download
    TEMP_BIN=$(mktemp)
    if curl -fsSL "$RELEASE_URL" -o "$TEMP_BIN"; then
        chmod +x "$TEMP_BIN"
        success "Downloaded cc-bash binary"
    else
        error "Failed to download release. Try building from source instead."
    fi
else
    info "Building from source..."

    # Check if we're in the repo directory
    if [ -f "cc-bash.c" ]; then
        make clean 2>/dev/null || true
        make
        TEMP_BIN="./cc-bash"
    else
        # Clone and build
        TEMP_DIR=$(mktemp -d)
        info "Cloning repository..."
        git clone --depth 1 "https://github.com/$REPO.git" "$TEMP_DIR"
        cd "$TEMP_DIR"
        make
        TEMP_BIN="$TEMP_DIR/cc-bash"
    fi

    success "Build complete"
fi

# Install binary
info "Installing to $BINDIR..."

if [ ! -d "$BINDIR" ]; then
    mkdir -p "$BINDIR" || error "Cannot create $BINDIR. Try with sudo?"
fi

if ! cp "$TEMP_BIN" "$BINDIR/cc-bash" 2>/dev/null; then
    error "Cannot write to $BINDIR. Try: sudo $0"
fi
chmod 755 "$BINDIR/cc-bash"

success "Installed cc-bash to $BINDIR/cc-bash"

# Setup config directory
info "Setting up config..."

mkdir -p "$CONFIG_DIR/plugins"

if [ ! -f "$CONFIG_FILE" ]; then
    # Try to copy template
    if [ -f "config/cc-bashrc.template" ]; then
        cp "config/cc-bashrc.template" "$CONFIG_FILE"
    elif [ -f "$TEMP_DIR/config/cc-bashrc.template" ] 2>/dev/null; then
        cp "$TEMP_DIR/config/cc-bashrc.template" "$CONFIG_FILE"
    else
        # Create minimal config
        cat > "$CONFIG_FILE" << 'ENDCONFIG'
# cc-bash configuration
# See https://github.com/jcaldwell-labs/smartterm-prototype for full options

# Aliases
alias ll='ls -la'
alias gs='git status'

# Workflows
workflow build='make clean && make && make test'
ENDCONFIG
    fi
    success "Created $CONFIG_FILE"
else
    warn "Config file already exists, skipping"
fi

success "Created $CONFIG_DIR/plugins/"

# Cleanup
if [ -n "$TEMP_DIR" ] && [ -d "$TEMP_DIR" ]; then
    rm -rf "$TEMP_DIR"
fi

# Verify installation
echo ""
if command -v cc-bash &> /dev/null; then
    success "Installation complete!"
else
    warn "cc-bash installed but not in PATH"
    echo "  Add to PATH: export PATH=\"$BINDIR:\$PATH\""
fi

echo ""
echo "╔════════════════════════════════════════╗"
echo "║  Installation Summary                  ║"
echo "╠════════════════════════════════════════╣"
echo "║  Binary:  $BINDIR/cc-bash"
echo "║  Config:  $CONFIG_FILE"
echo "║  Plugins: $CONFIG_DIR/plugins/"
echo "╚════════════════════════════════════════╝"
echo ""
echo "Run 'cc-bash' to start!"
echo ""
