package main

import "fmt"

func main() {
    fmt.Println("Hello World")
    
    if true {
        fmt.Println("True")
    }
}
}

// ERROR: Extra '}' on line 13
// Should show: ⚠ L13: Extra '}' - no opening brace
