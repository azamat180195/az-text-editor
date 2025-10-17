#!/bin/bash
# AZ Editor - One-line Install Script
# curl -fsSL https://raw.githubusercontent.com/azamat180195/az-text-editor/main/install.sh | bash

set -e

echo "╔═══════════════════════════════════════════════════╗"
echo "║   AZ EDITOR v1.8.0 - Installation                ║"
echo "║   Universal Linux Text Editor                     ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""

# Detect Linux distribution
if [ -f /etc/os-release ]; then
    . /etc/os-release
    OS=$ID
    echo "→ Detected: $PRETTY_NAME"
else
    echo "→ Cannot detect Linux distribution"
    OS="unknown"
fi
echo ""

# Check dependencies
echo "→ Checking dependencies..."
if ! command -v gcc &> /dev/null; then
    echo "✗ gcc not found. Installing..."
    case "$OS" in
        debian|ubuntu|mint|pop)
            sudo apt-get update -qq
            sudo apt-get install -y build-essential libncurses-dev
            ;;
        fedora|rhel|centos|rocky|alma)
            sudo dnf install -y gcc ncurses-devel make 2>/dev/null || \
            sudo yum install -y gcc ncurses-devel make
            ;;
        arch|manjaro)
            sudo pacman -S --noconfirm gcc ncurses make
            ;;
        opensuse*)
            sudo zypper install -y gcc ncurses-devel make
            ;;
        alpine)
            sudo apk add gcc ncurses-dev make musl-dev
            ;;
        *)
            echo "✗ Cannot install dependencies automatically"
            echo "  Please install: gcc, ncurses-dev/ncurses-devel, make"
            exit 1
            ;;
    esac
fi

# Check ncurses
if ! ldconfig -p 2>/dev/null | grep -q libncurses && ! find /usr/lib* /lib* -name "libncurses*" 2>/dev/null | grep -q .; then
    echo "✗ libncurses not found. Installing..."
    case "$OS" in
        debian|ubuntu|mint|pop)
            sudo apt-get install -y libncurses-dev
            ;;
        fedora|rhel|centos|rocky|alma)
            sudo dnf install -y ncurses-devel 2>/dev/null || \
            sudo yum install -y ncurses-devel
            ;;
        arch|manjaro)
            sudo pacman -S --noconfirm ncurses
            ;;
        opensuse*)
            sudo zypper install -y ncurses-devel
            ;;
        alpine)
            sudo apk add ncurses-dev
            ;;
    esac
fi

echo "✓ Dependencies OK"
echo ""

# Download source
echo "→ Downloading source code..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

curl -fsSL https://raw.githubusercontent.com/azamat180195/az-text-editor/main/az.c -o az.c
curl -fsSL https://raw.githubusercontent.com/azamat180195/az-text-editor/main/Makefile -o Makefile

echo "✓ Downloaded"
echo ""

# Compile
echo "→ Compiling..."
make

if [ ! -f "az" ]; then
    echo "✗ Compilation failed"
    exit 1
fi

echo "✓ Compiled successfully"
echo ""

# Install system-wide
echo "→ Installing to /usr/local/bin..."
sudo install -m 755 az /usr/local/bin/az

echo "✓ Installed"
echo ""

# Cleanup
cd /
rm -rf "$TEMP_DIR"

echo "╔═══════════════════════════════════════════════════╗"
echo "║   ✓ Installation Complete!                       ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""
echo "Usage:"
echo "  az filename.txt       # Open file"
echo "  az                    # New file"
echo ""
echo "Features:"
echo "  • Real-time syntax checking (JSON, YAML, Python, HTML, C/Java)"
echo "  • Mouse support (drag to copy, right-click to paste)"
echo "  • Search/Replace (Ctrl+F/R)"
echo "  • Undo (Ctrl+Z, 100 levels)"
echo "  • Auto-copy on mouse selection"
echo ""
echo "Commands:"
echo "  az --version          # Show version"
echo "  az --help             # Show help"
echo ""
