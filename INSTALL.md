# Installation Guide

## One-Line Install (Recommended)

```bash
curl -fsSL https://raw.githubusercontent.com/USERNAME/az-editor/main/install.sh | bash
```

This will:
1. Check and install dependencies (gcc, libncurses)
2. Download source code
3. Compile
4. Install to `/usr/local/bin/az`

## Manual Installation

### 1. Install Dependencies

**Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install build-essential libncurses-dev
```

**Fedora/RHEL:**
```bash
sudo yum install gcc ncurses-devel
```

**Arch Linux:**
```bash
sudo pacman -S gcc ncurses
```

### 2. Clone Repository

```bash
git clone https://github.com/USERNAME/az-editor.git
cd az-editor
```

### 3. Compile

```bash
make
```

This creates `./az` binary (~85-95 KB)

### 4. Install System-Wide

```bash
sudo make install
```

Now `az` command is available everywhere!

### 5. Test

```bash
az test.txt
```

## Verify Installation

```bash
which az
# Should output: /usr/local/bin/az

az --version
# Should output: AZ Editor v1.8.0
```

## Uninstall

```bash
cd az-editor
sudo make uninstall
```

## Usage

```bash
# Open file
az myfile.txt

# New file
az

# Edit JSON with syntax check
az config.json

# Edit Python
az script.py
```

## First Steps

1. **Edit text**: Type normally
2. **Save**: `Ctrl+S`
3. **Find**: `Ctrl+F`
4. **Undo**: `Ctrl+Z`
5. **Mouse**: Drag to select (auto-copies!), right-click to paste
6. **Quit**: `Ctrl+Q`

## Troubleshooting

### Error: "gcc: command not found"
Install build tools:
```bash
sudo apt-get install build-essential
```

### Error: "ncurses.h: No such file"
Install ncurses development files:
```bash
sudo apt-get install libncurses-dev
```

### Error: "Permission denied"
Use sudo for system-wide install:
```bash
sudo make install
```

Or install to local directory:
```bash
make
cp az ~/.local/bin/
# Make sure ~/.local/bin is in your PATH
```

## Update

```bash
cd az-editor
git pull
make clean
make
sudo make install
```

## Custom Install Location

```bash
# Install to custom directory
make install PREFIX=/opt/az

# Or just copy binary
make
sudo cp az /usr/bin/az
```

---

**Need help?** Open an issue: https://github.com/USERNAME/az-editor/issues
