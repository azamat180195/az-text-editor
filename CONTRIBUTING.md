# Contributing to AZ Editor

We welcome contributions! Here's how you can help:

## Ways to Contribute

- 🐛 **Report Bugs** - Open an issue with details
- ✨ **Suggest Features** - Share your ideas
- 📝 **Improve Documentation** - Fix typos, add examples
- 💻 **Submit Code** - Bug fixes, new features

## Getting Started

1. Fork the repository
2. Clone your fork: `git clone https://github.com/YOUR_USERNAME/az-text-editor.git`
3. Create a branch: `git checkout -b feature-name`
4. Make your changes
5. Test thoroughly
6. Commit: `git commit -m "Description"`
7. Push: `git push origin feature-name`
8. Open a Pull Request

## Development Setup

```bash
# Clone
git clone https://github.com/azamat180195/az-text-editor.git
cd az-text-editor

# Compile
make

# Test
./az test_samples/test_json_error.json

# Install locally
make install PREFIX=~/.local
```

## Coding Guidelines

- Follow existing code style
- Use K&R style bracing
- Add comments for complex logic
- Keep functions focused and small
- Test on multiple Linux distributions

## Testing

Before submitting:

```bash
# Compile without warnings
make clean
make

# Test all syntax checkers
./az test_samples/test_json_error.json
./az test_samples/test_yaml_tab.yml
./az test_samples/test_python_mixed.py
# etc...

# Test basic features
# - Edit text
# - Save (Ctrl+S)
# - Search (Ctrl+F)
# - Mouse selection
# - Undo (Ctrl+Z)
```

## Pull Request Process

1. Update README.md if needed
2. Update CHANGELOG.md
3. Ensure code compiles without warnings
4. Test on at least one Linux distribution
5. Describe your changes clearly
6. Reference any related issues

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow

## Questions?

Open an issue or reach out to the maintainers.

---

**Thank you for contributing!** 🎉
