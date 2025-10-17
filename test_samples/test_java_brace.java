public class HelloWorld {
    public static void main(String[] args) {
        System.out.println("Hello World");
        if (true) {
            System.out.println("True");
        }
    }
}
}

// ERROR: Extra '}' on line 9
// Should show: ⚠ L9: Extra '}' - no opening brace
