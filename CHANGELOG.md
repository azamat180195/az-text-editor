# Changelog

All notable changes to AZ Editor will be documented in this file.

## [1.8.0] - 2024-10-17

### Added
- **Universal Linux Support** - Auto-detects and installs on Debian, Ubuntu, Fedora, RHEL, CentOS, Arch, Manjaro, openSUSE, Alpine
- **Right-Click Paste** - Paste clipboard with right mouse button
- **English Interface** - All messages translated to English
- **One-Line Installer** - `curl ... | bash` installation
- **System-Wide Install** - `az` command available everywhere
- **GitHub Repository** - Published at https://github.com/azamat180195/az-text-editor

### Changed
- Status bar messages now in English
- Error messages more professional ("Extra '}' - no opening brace")
- Help line updated with "RClick:Paste"
- Improved Makefile with user-friendly output

### Fixed
- Install script now supports all major Linux distributions
- Better dependency detection (works with/without ldconfig)

## [1.7.2] - 2024-10-17

### Added
- Error message centered in status bar (better visibility)
- Auto-copy on mouse selection (drag to copy automatically)

### Changed
- Error format: "⚠ Q4:" (shorter, fits better)
- Error messages truncated if too long with "..."

## [1.7.1] - 2024-10-17

### Added
- Line number in error messages ("⚠ Qator 4:")
- Click error to jump to line
- Percentage display in status bar (15%)

### Fixed
- Mouse selection properly clears on arrow key press
- Terminal state restored on exit

## [1.7.0] - 2024-10-17

### Added
- **Real-Time Syntax Checking** for 6 formats (JSON, YAML, Python, HTML/XML, C/Java/Go)
- Red underline on syntax errors
- Detailed Uzbek error messages
- Position-specific error highlighting

## [1.6.1] - 2024-10-17

### Fixed
- Mouse selection lingering after release
- Second launch hanging
- Terminal cleanup between sessions

## [1.6.0] - 2024-10-17

### Added
- Search (Ctrl+F) with occurrence count
- Replace (Ctrl+R) one/all
- Undo (Ctrl+Z) with 100 levels
- Real-time mouse selection
- Basic syntax checking

### Fixed
- Backspace artifacts in search/replace
- Echo mode issues

## [1.5.0] - 2024

### Added
- Mouse support (click, drag, selection)
- Real-time selection update

## [1.4.0] - 2024

### Added
- Search and Replace functionality
- Wrapped search

## [1.3.0] - 2024

### Added
- Undo system (100 levels)
- Word boundary detection

## [1.2.0] - 2024

### Added
- Clipboard operations (Copy, Cut, Paste)
- Select All (Ctrl+A)
- Cut/Uncut line (Ctrl+K/U)

## [1.1.0] - 2024

### Added
- Ctrl key support (Save, Quit)
- Signal handling

## [1.0.0] - 2024

### Added
- Initial release
- Basic text editing
- Line numbers
- Status bar
- Word wrapping
- Navigation (arrow keys, Page Up/Down)

---

**Format**: [Semantic Versioning](https://semver.org/)

**Repository**: https://github.com/azamat180195/az-text-editor
