#include <stdio.h>

int main() {
    printf("Hello World\n");
    
    if (1) {
        printf("Inside if\n");
    
    for (int i = 0; i < 10; i++) {
        printf("%d\n", i);
    }
    
    // Missing closing '}' for main function
    
// ERROR: Unclosed '{' for main()
// Should show: ⚠ L15: Unclosed '{' - 1 open brace(s)
