# Installation Guide

## One-Line Install (Recommended)

```bash
curl -fsSL https://raw.githubusercontent.com/azamat180195/az-text-editor/main/install.sh | bash
```

This will:
1. **Auto-detect** your Linux distribution
2. **Install dependencies** (gcc, ncurses, make)
3. **Download** source code
4. **Compile** the editor
5. **Install** to `/usr/local/bin/az`

**Supported Distributions:** Debian, Ubuntu, Fedora, RHEL, CentOS, Arch, Manjaro, openSUSE, Alpine

## Manual Installation

### 1. Install Dependencies

**Debian / Ubuntu / Linux Mint / Pop!_OS:**
```bash
sudo apt-get update
sudo apt-get install build-essential libncurses-dev
```

**Fedora / RHEL 8+ / CentOS 8+ / Rocky / AlmaLinux:**
```bash
sudo dnf install gcc ncurses-devel make
```

**RHEL 7 / CentOS 7:**
```bash
sudo yum install gcc ncurses-devel make
```

**Arch Linux / Manjaro:**
```bash
sudo pacman -S gcc ncurses make
```

**openSUSE Leap / Tumbleweed:**
```bash
sudo zypper install gcc ncurses-devel make
```

**Alpine Linux:**
```bash
sudo apk add gcc ncurses-dev make musl-dev
```

### 2. Clone Repository

```bash
git clone https://github.com/azamat180195/az-text-editor.git
cd az-text-editor
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
cd az-text-editor
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

## Update to Latest Version

```bash
cd az-text-editor
git pull
make clean
make
sudo make install
```

Or simply re-run the installer:
```bash
curl -fsSL https://raw.githubusercontent.com/azamat180195/az-text-editor/main/install.sh | bash
```

## Custom Install Location

```bash
# Install to custom directory
make install PREFIX=/opt/az

# Or just copy binary
make
sudo cp az /usr/bin/az
```

## Distribution-Specific Notes

### Debian / Ubuntu
- Uses `apt-get` for packages
- Installs: `build-essential`, `libncurses-dev`

### Fedora / RHEL / CentOS
- Uses `dnf` (RHEL 8+) or `yum` (RHEL 7)
- Installs: `gcc`, `ncurses-devel`, `make`

### Arch / Manjaro  
- Uses `pacman`
- Installs: `gcc`, `ncurses`, `make`

### openSUSE
- Uses `zypper`
- Installs: `gcc`, `ncurses-devel`, `make`

### Alpine
- Uses `apk`
- Installs: `gcc`, `ncurses-dev`, `make`, `musl-dev`

---

**Need help?** Open an issue: https://github.com/azamat180195/az-text-editor/issues
