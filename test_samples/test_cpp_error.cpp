#include <iostream>

int main() {
    std::cout << "Hello" << std::endl;
    
    {
        int x = 10;
        std::cout << x << std::endl;
    }
    
    return 0;
}}

// ERROR: Extra '}' on line 13
// Should show: ⚠ L13: Extra '}' - no opening brace
