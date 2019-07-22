import java.util.Scanner;
import java.util.Random;

public class Main {
    public static String getRandom(String[] array) {
        int rnd = new Random().nextInt(array.length);
        return array[rnd];
    }

    public static void main(String args[]) {
        String[] commands = {"left", "right", "up", "down"};
        Scanner scanner = new Scanner(System.in);
        while (true) {
            String input = scanner.nextLine();
            String command = Main.getRandom(commands);
            System.out.printf("{\"command\": \"%s\"}\n", command);
        }
    }
}
