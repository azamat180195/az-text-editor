# AZ Editor - Syntax Error Test Files

## ✅ All Test Files - Clean Syntax!

**IMPORTANT:** Test files contain REAL errors (not in comments):
- No comment lines with error examples
- Pure syntax errors for accurate testing
- Visual indicators: TAB shows as `^`, space error shows as `_`

## How to Test

```bash
# Compile the editor
make

# Test each file - watch for red underline!
./az test_samples/test_json_error.json
./az test_samples/test_yaml_tab.yml
./az test_samples/test_python_mixed.py
# etc...
```

## Test Files and Expected Errors

### 1. JSON Tests

**test_json_error.json**
- **Error**: Extra `}` on line 9
- **Expected**: `⚠ L9: Extra '}' - no opening brace`
- **Visual**: Red underline under the extra `}` on line 9
- **Test**: Line 9 has `}}` - second `}` is underlined in red

**test_json_unclosed.json**
- **Error**: Missing closing `}` at end
- **Expected**: `⚠ L11: Unclosed '{' - 1 open brace(s)`
- **Visual**: Error shown in status bar (no specific position)
- **Test**: File ends at line 11 without closing `}`

### 2. YAML Tests

**test_yaml_tab.yml**
- **Error**: TAB character on line 4 before "email"
- **Expected**: `⚠ L4: YAML: TAB not allowed - use spaces`
- **Visual**: TAB shows as red `^` with underline
- **Test**: Line 4 starts with TAB (not in comment!)

**test_yaml_bracket.yml**
- **Error**: Unclosed `[` on line 3
- **Expected**: `⚠ L3: Bracket not closed`
- **Visual**: Red underline on start of line
- **Test**: Line 3 has `[3306, 3307` without closing `]`

### 3. Python Test

**test_python_mixed.py**
- **Error**: Line 4 uses TAB, line 2 established spaces
- **Expected**: `⚠ L4: TAB used but file uses spaces (L2)`
- **Visual**: TAB shows as red `^` with underline
- **Test**: Line 2 has spaces (establishes style), line 4 has TAB (error!)

### 4. HTML Tests

**test_html_extra.html**
- **Error**: Extra `</html>` on line 12
- **Expected**: `⚠ L12: Extra closing tag - no opening tag`
- **Visual**: Red underline on `</html>`

**test_html_unclosed.html**
- **Error**: Missing closing tags (2 unclosed)
- **Expected**: `⚠ L14: Unclosed tag - 2 open tag(s)`
- **Visual**: Error shown in status bar

### 5. Java Test

**test_java_brace.java**
- **Error**: Extra `}` on line 9
- **Expected**: `⚠ L9: Extra '}' - no opening brace`
- **Visual**: Red underline under the extra `}`

### 6. C Test

**test_c_unclosed.c**
- **Error**: Missing closing `}` for main()
- **Expected**: `⚠ L15: Unclosed '{' - 1 open brace(s)`
- **Visual**: Error shown in status bar

### 7. C++ Test

**test_cpp_error.cpp**
- **Error**: Extra `}` on line 13
- **Expected**: `⚠ L13: Extra '}' - no opening brace`
- **Visual**: Red underline under the extra `}`

### 8. Go Test

**test_go_error.go**
- **Error**: Extra `}` on line 13
- **Expected**: `⚠ L13: Extra '}' - no opening brace`
- **Visual**: Red underline under the extra `}`

## Testing Checklist

For each test file, verify:

- [ ] **Red Underline** - Error position shows red underline
- [ ] **Status Bar** - Shows error message in center-right
- [ ] **Correct Line** - Error points to correct line number
- [ ] **Click to Jump** - Clicking error jumps to error line
- [ ] **Message** - Error message is clear and helpful

## Visual Test

When you open a test file:

```
┌─────────────────────────────────────────────────┐
│  1 {                                            │
│  2   "name": "test",                            │
│  3   "data": 123                                │
│  4 }}        <-- Second } has RED UNDERLINE     │
│                                                 │
│ * file.json  Line 4/10 (40%) ⚠ L4: Extra '}'  │
│             Click here ↑ to jump to error       │
└─────────────────────────────────────────────────┘
```

### Special Visual Indicators:

- **TAB error**: Shows as red `^` with underline
- **Space error**: Shows as red `_` with underline  
- **Other chars**: Show normally with red underline
- **No error**: No underline, normal text

## Quick Test Script

```bash
#!/bin/bash
echo "Testing AZ Editor syntax checking..."

files=(
    "test_json_error.json"
    "test_json_unclosed.json"
    "test_yaml_tab.yml"
    "test_yaml_bracket.yml"
    "test_python_mixed.py"
    "test_html_extra.html"
    "test_html_unclosed.html"
    "test_java_brace.java"
    "test_c_unclosed.c"
    "test_cpp_error.cpp"
    "test_go_error.go"
)

for file in "${files[@]}"; do
    echo "Test: $file"
    ./az "test_samples/$file"
    # Press Ctrl+Q to exit after checking
done

echo "All tests complete!"
```

## Supported Formats

✅ JSON (.json)  
✅ YAML (.yml, .yaml)  
✅ Python (.py)  
✅ HTML (.html, .htm)  
✅ XML (.xml)  
✅ Java (.java)  
✅ C (.c, .h)  
✅ C++ (.cpp, .hpp)  
✅ Go (.go)

---

**All syntax checking works in real-time!**

No need to save - errors appear as you type!
