import java.util.ArrayList;

public class Types {
    public static String[] testA(ArrayList<Integer> param) {
        String[] array = new String[8];
        for (int i = 0; i < 8; ++i) {
            array[i] = "";
        }
        for (Integer i : param) {
            String s = i.toString();
            array[i % 8] += s;
        }
        return array;
    }

    public static ArrayList<String> testB(int[] param) {
        ArrayList<String> l = new ArrayList<String>();
        for (int i = 0; i < param.length; ++i) {
            l.add("" + param[i]);
        }
        return l;
    }
}
