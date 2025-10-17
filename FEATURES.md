# AZ Editor Features

## Real-Time Syntax Checking ✨

Automatically detects and highlights errors in:

### JSON (.json)
- Brace/bracket balance `{} []`
- Detects extra or missing braces
- Shows error line and column

**Example:**
```json
{
  "name": "test"
}}  ← Error: Extra '}' - no opening brace
```

### YAML (.yml, .yaml)
- Detects TAB characters (YAML only allows spaces)
- Bracket balance check
- Line-by-line validation

**Example:**
```yaml
users:
→ name: Ali  ← Error: YAML: TAB not allowed - use spaces
```

### Python (.py)
- Mixed tabs/spaces detection
- Indentation consistency check
- Real-time feedback

**Example:**
```python
def test():
    print("a")
→   print("b")  ← Error: Mixed TAB and spaces
```

### HTML/XML (.html, .xml)
- Tag balance checking
- Detects unclosed/extra tags
- Opening/closing tag matching

**Example:**
```html
<div>
  <p>Hello</p>
</div>
</div>  ← Error: Extra closing tag - no opening tag
```

### C/C++/Java/Go (.c, .cpp, .java, .go)
- Brace balance `{}`
- Skips comments and strings
- Function-level checking

**Example:**
```c
int main() {
    return 0;
}
}  ← Error: Extra '}' - no opening brace
```

---

## Mouse Support 🖱️

### Auto-Copy on Selection
1. **Drag** text with left mouse button
2. **Release** → Text automatically copied to clipboard!
3. **Message**: "Copied (mouse selection)"
4. **Paste** with `Ctrl+V` or right-click

### Right-Click Paste
- **Right-click** anywhere → Pastes clipboard at cursor
- No need for `Ctrl+V`
- Works like modern terminals

### Click Error Navigation
- **Click** on red error in status bar
- **Jumps** to error line automatically
- **Scrolls** if needed
- **Message**: "Jumped to error line"

---

## Search & Replace 🔍

### Search (Ctrl+F)
- **Type** search term
- **Shows count**: "Found 5 occurrences"
- **Wrapped search**: Continues from start after end
- **Highlights** selection
- **Cancel**: `Ctrl+C` or `ESC`

### Replace (Ctrl+R)
- **Enter** search term
- **Enter** replacement
- **Choose**: `1` for one, `a` for all
- **Shows count**: "Replaced: 5 occurrences"
- **Cancel**: `Ctrl+C` or `ESC`

---

## Undo System ↩️

### Features
- **100 levels** of undo
- **Word boundary** detection
- **Preserves** cursor position
- **Fast** state restoration

### Usage
- **Undo**: `Ctrl+Z`
- **Redo**: `Ctrl+Y` (coming soon)

---

## Clipboard Operations 📋

### Copy
- **Manual**: Select text → `Ctrl+C`
- **Auto**: Drag with mouse (automatic!)

### Cut
- **Selection**: `Ctrl+X`
- **Line**: `Ctrl+K` (nano-style)

### Paste
- **Keyboard**: `Ctrl+V`
- **Mouse**: Right-click
- **Line**: `Ctrl+U` (nano-style)

### Select All
- **Keyboard**: `Ctrl+A`

---

## Status Bar 📊

### Left Side
```
 * filename.txt
 ^ Modified indicator
```

### Center
```
Line 15/100 (15%), Col 25
     ^       ^      ^
  Current  Total  Percent
```

### Right Side (Errors)
```
⚠ L4: Extra '}' - no opening brace
^^^^^^^^^^^^^^^ Click to jump!
```

---

## Help Line ℹ️

```
^S:Save  ^Q:Quit  ^Z:Undo  ^F:Find  ^R:Replace  RClick:Paste
```

Shows available commands at bottom

---

## Performance ⚡

- **Syntax check**: <1ms
- **No lag** while typing
- **Low memory**: ~2MB RAM
- **Small binary**: ~85-95KB
- **Fast startup**: Instant

---

## Coming Soon 🚀

- [ ] Redo (Ctrl+Y)
- [ ] Regex search
- [ ] Multi-file tabs
- [ ] Config file (~/.azrc)
- [ ] More syntax formats (JS, Rust, etc.)
- [ ] Auto-indent
- [ ] Line comment toggle
- [ ] Syntax highlighting (colors)

---

**All features work in real-time with no lag!**
