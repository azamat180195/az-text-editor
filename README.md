# AZ Editor v1.8.0

**Professional text editor for Linux with real-time syntax checking**

![Version](https://img.shields.io/badge/version-1.8.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Platform](https://img.shields.io/badge/platform-Linux-lightgrey)

## ✨ Features

- **Real-time Syntax Checking** - JSON, YAML, Python, HTML/XML, C/Java/Go
- **Red Underline Errors** - Visual error highlighting
- **Clickable Error Navigation** - Click status bar to jump to error
- **Mouse Support**
  - Left-click drag → Auto-copy to clipboard
  - Right-click → Paste
  - Click on error → Jump to line
- **Search & Replace** - Ctrl+F/R with occurrence count
- **Undo System** - 100 levels with word boundary detection
- **Lightweight** - <100KB binary, ~2MB RAM
- **Fast** - <1ms syntax checking, no lag

## 🚀 Quick Install

```bash
curl -fsSL https://raw.githubusercontent.com/USERNAME/az-editor/main/install.sh | bash
```

That's it! Now use:
```bash
az filename.txt
```

## 📦 Manual Install

```bash
# Clone
git clone https://github.com/USERNAME/az-editor.git
cd az-editor

# Compile & Install
make
sudo make install

# Run
az test.txt
```

## ⌨️ Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Ctrl+S` | Save |
| `Ctrl+Q` | Quit |
| `Ctrl+Z` | Undo |
| `Ctrl+F` | Find |
| `Ctrl+R` | Replace |
| `Ctrl+C` | Copy |
| `Ctrl+X` | Cut |
| `Ctrl+V` | Paste |
| `Ctrl+A` | Select All |

## 🖱️ Mouse Actions

- **Drag** - Select text (auto-copies to clipboard)
- **Right-click** - Paste at cursor
- **Click error** - Jump to error line in status bar

## 📋 Supported Formats

| Format | Extension | Checks |
|--------|-----------|--------|
| JSON | `.json` | Brace/bracket balance |
| YAML | `.yml`, `.yaml` | No tabs, bracket balance |
| Python | `.py` | Mixed tabs/spaces |
| HTML/XML | `.html`, `.xml` | Tag balance |
| C/C++/Java/Go | `.c`, `.cpp`, `.java`, `.go` | Brace balance |

## 📸 Screenshots

```
┌─────────────────────────────────────────────────┐
│  1 {                                            │
│  2   "name": "test",                            │
│  3   "data": 123                                │
│  4 }}        <-- RED UNDERLINE                  │
│                                                 │
│ * test.json  Line 4/10 (40%) ⚠ L4: Extra '}'  │
│             Click here ↑ to jump to error       │
│ ^S:Save ^Q:Quit ^F:Find  RClick:Paste          │
└─────────────────────────────────────────────────┘
```

## 🔧 Requirements

- **OS**: Linux (Debian, Ubuntu, Arch, etc.)
- **Dependencies**: 
  - `gcc`
  - `libncurses-dev`

Install on Debian/Ubuntu:
```bash
sudo apt-get install build-essential libncurses-dev
```

## 📖 Usage Examples

### Basic Editing
```bash
az myfile.txt
```

### JSON with Syntax Check
```bash
az config.json
# Error shows: ⚠ L4: Extra '}' - no opening brace
# Click error to jump to line 4
```

### YAML Validation
```bash
az app.yml
# Detects: TAB characters (YAML only allows spaces)
```

### Mouse Workflow
1. Drag to select text → Auto-copied!
2. Right-click elsewhere → Pastes
3. No need for Ctrl+C/V

## 🏗️ Building from Source

```bash
# Clone
git clone https://github.com/USERNAME/az-editor.git
cd az-editor

# Compile
make

# Binary created: ./az
./az test.txt

# Install system-wide
sudo make install

# Now use anywhere:
az /path/to/file.txt
```

## 📊 Technical Details

- **Language**: C
- **Library**: ncurses
- **Lines**: ~2050
- **Source Size**: ~63 KB
- **Binary Size**: ~85-95 KB
- **Memory**: ~2 MB runtime
- **Performance**: <1ms syntax check

## 🤝 Contributing

Contributions welcome! Areas:

- [ ] Redo (Ctrl+Y)
- [ ] Regex search
- [ ] Multi-file tabs
- [ ] Config file (~/.azrc)
- [ ] More syntax formats (JS, Rust, etc.)
- [ ] Auto-indent
- [ ] Line comment toggle

## 📝 License

MIT License - See LICENSE file

## 👨‍💻 Author

**Factory AI**

## 🔗 Links

- GitHub: https://github.com/USERNAME/az-editor
- Issues: https://github.com/USERNAME/az-editor/issues
- Documentation: https://github.com/USERNAME/az-editor/wiki

## 🎯 Roadmap

### v1.8.x
- [x] Right-click paste
- [x] English interface
- [x] One-line install

### v1.9.0 (Planned)
- [ ] Redo support
- [ ] Regex search
- [ ] Multi-file tabs

### v2.0.0 (Future)
- [ ] Syntax highlighting (colors)
- [ ] Plugin system
- [ ] LSP support

---

**Made with ❤️ by Factory AI**
