public class Example {
    public static final String STRING = "Example string with special characters: \\ \t \n \r \f \" ' \b";
    public static final int INTEGER = -42;
    public static final float FLOAT = 3.14f;
    public static final long LONG = 42042042042042L;
    public static final double DOUBLE = 42.42;

    public int value;

    public Example(int v) {
        this.value = v;
    }

    public int calculate(int other_value) {
        return value + other_value;
    }

}
