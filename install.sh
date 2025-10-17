#!/bin/bash
# AZ Editor - One-line Install Script
# curl -fsSL https://raw.githubusercontent.com/USERNAME/az-editor/main/install.sh | bash

set -e

echo "╔═══════════════════════════════════════════════════╗"
echo "║   AZ EDITOR v1.8.0 - Installation                ║"
echo "╚═══════════════════════════════════════════════════╝"
echo ""

# Check dependencies
echo "→ Checking dependencies..."
if ! command -v gcc &> /dev/null; then
    echo "✗ gcc not found. Installing..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get update
        sudo apt-get install -y build-essential libncurses-dev
    elif command -v yum &> /dev/null; then
        sudo yum install -y gcc ncurses-devel
    elif command -v pacman &> /dev/null; then
        sudo pacman -S gcc ncurses
    else
        echo "✗ Cannot install dependencies automatically"
        echo "  Please install: gcc, libncurses-dev"
        exit 1
    fi
fi

if ! ldconfig -p | grep -q libncurses; then
    echo "✗ libncurses not found. Installing..."
    if command -v apt-get &> /dev/null; then
        sudo apt-get install -y libncurses-dev
    elif command -v yum &> /dev/null; then
        sudo yum install -y ncurses-devel
    elif command -v pacman &> /dev/null; then
        sudo pacman -S ncurses
    fi
fi

echo "✓ Dependencies OK"
echo ""

# Download source
echo "→ Downloading source code..."
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

curl -fsSL https://raw.githubusercontent.com/USERNAME/az-editor/main/az.c -o az.c
curl -fsSL https://raw.githubusercontent.com/USERNAME/az-editor/main/Makefile -o Makefile

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
