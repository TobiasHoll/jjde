// Structures.java
import java.io.BufferedReader;
import java.io.FileReader;

public class Structures {
    // Try, catch, finally
    public static int tryCatchFinally(int a, int b) {
        try {
            return a / b;
        } catch (ArithmeticException e) {
            System.err.println("Division by zero.");
            return 0;
        } finally {
            System.out.println("In `finally' block.");
        }
    }
    // Try-with-resources, catch and finally
    public static String tryWithResources(String path) {
        try (BufferedReader reader = new BufferedReader(new FileReader(path))) {
            return reader.readLine();
        } catch (Exception e) {
            System.err.println("An error occurred.");
        } finally {
            System.out.println("In `finally' block.");
        }
        return "";
    }

    // Throw exceptions
    public static int throwException(int a, int b) throws IllegalArgumentException {
        if (b == 0) throw new IllegalArgumentException("Division by zero.");
        return a / b;
    }

    // Normal for loop
    public static void forLoop(int count) {
        for (int i = 0; i < count; ++i) {
            System.out.println("Loop iteration " + i);
        }
    }

    // Inverse for loop
    public static void inverseForLoop(int count) {
        for (int i = count - 1; i >= 0; --i) {
            System.out.println("Loop iteration " + i);
        }
    }

    // While loop
    public static void whileLoop(int number) {
        System.out.println(number);
        while (number != 1) {
            if (number % 2 == 0) number /= 2;
            else number = 3 * number + 1;
            System.out.println(number);
        }
    }

    // Do-While loop
    public static void doWhileLoop(int number) {
        int original;
        do {
            System.out.println(number);
            original = number;
            if (number % 2 == 0) number /= 2;
            else number = 3 * number + 1;
        } while (original != 1);
    }

    // Break statement (+ endless loop)
    public static void breakInEndlessLoop(int number) {
        while (true) {
            System.out.println(number);
            if (number == 1) break;
            if (number % 2 == 0) number /= 2;
            else number = 3 * number + 1;
        }
    }

    // Continue
    public static void continueInForLoop(int count, int n) {
        for (int i = 0; i < count; ++i) {
            if (i % n == 0) continue;
            System.out.println("Loop iteration " + i);
        }
    }


    // Self-test main
    public static void main(String[] args) {
        // Tests returning check values
        System.out.println("[             tryCatchFinally] " + tryCatchFinally(5, 1)                + " (should be 5, check for `finally' block)");
        System.out.println("[             tryCatchFinally] " + tryCatchFinally(5, 0)                + " (should be 0, check for division error and `finally' block)");
        System.out.println("[            tryWithResources] " + tryWithResources("Structures.java")  + " (should be \"// Structures.java\", check for `finally' block)");
        System.out.println("[            tryWithResources] " + tryWithResources("Nonexistent.file") + " (should be \"\", check for error message and `finally' block)");
        System.out.println("[              throwException] " + throwException(5, 1)                 + " (should be 5)");

        // Tests throwing exceptions
        try {
            throwException(5, 0);
            System.out.println("[              throwException] FAILED (should be PASSED)");
        } catch (IllegalArgumentException e) {
            System.out.println("[              throwException] PASSED (should be PASSED)");
        }

        // Tests giving output for validation
        forLoop(5);
        System.out.println("[                     forLoop] (should show iterations 0 to 4)");
        inverseForLoop(5);
        System.out.println("[              inverseforLoop] (should show iterations 4 to 0)");
        whileLoop(6);
        System.out.println("[                   whileLoop] (should show the following sequence: 6, 3, 10, 5, 16, 8, 4, 2, 1)");
        doWhileLoop(6);
        System.out.println("[                 doWhileLoop] (should show the following sequence: 6, 3, 10, 5, 16, 8, 4, 2, 1)");
        breakInEndlessLoop(6);
        System.out.println("[          breakInEndlessLoop] (should show the following sequence: 6, 3, 10, 5, 16, 8, 4, 2, 1)");
        continueInForLoop(10, 2);
        System.out.println("[           continueInForLoop] (should show odd iterations from 1 to 9)");
    }

}
