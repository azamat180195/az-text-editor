# AZ Editor - Syntax Error Test Files

## How to Test

```bash
# Compile the editor
make

# Test each file
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
- **Visual**: Red underline under the extra `}`

**test_json_unclosed.json**
- **Error**: Missing closing `}` at end
- **Expected**: `⚠ L11: Unclosed '{' - 1 open brace(s)`
- **Visual**: Error shown in status bar (no specific position)

### 2. YAML Tests

**test_yaml_tab.yml**
- **Error**: TAB character on line 4 before "email"
- **Expected**: `⚠ L4: YAML: TAB not allowed - use spaces`
- **Visual**: Red underline on the TAB character

**test_yaml_bracket.yml**
- **Error**: Unclosed `[` on line 3
- **Expected**: `⚠ L3: Bracket not closed`
- **Visual**: Red underline on entire line

### 3. Python Test

**test_python_mixed.py**
- **Error**: Line 4 uses TAB, others use spaces (mixed indentation)
- **Expected**: `⚠ L4: Mixed TAB and spaces - use one`
- **Visual**: Red underline on the TAB/space at start of line

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
│  4 }}        <-- RED UNDERLINE                  │
│                                                 │
│ * file.json  Line 4/10 (40%) ⚠ L4: Extra '}'  │
│             Click here ↑ to jump to error       │
└─────────────────────────────────────────────────┘
```

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
