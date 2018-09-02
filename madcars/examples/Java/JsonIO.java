import org.json.JSONObject;
import java.io.*;

/**
 * Обертка, позволяющая считывать из потока ввода и писать в поток вывода JSON-объекты
 * Для отладочных целей есть возможность читать команды не из STDIN, а из файла, например:
 *
 * <p>{@code gameMessage = JsonIO.readFromFile("messages.json")}</p>
 */

public class JsonIO {
    private static FileInputStream fileStream;
    private static InputStreamReader reader;
    private static BufferedReader bufferedReader;

    public static JSONObject readFromStdIn(){
        return readFromStream(System.in);
    }

    public static JSONObject readFromFile(String fileName){
        try {
            if (fileStream == null)
                fileStream = new FileInputStream(fileName);
            return readFromStream(fileStream);
        }
        catch (FileNotFoundException e) {
            return readFromStdIn();
        }
    }
    private static JSONObject readFromStream(InputStream stream){
        if (reader == null)
            reader = new InputStreamReader(stream);

        if (bufferedReader == null)
            bufferedReader = new BufferedReader(reader);

        try {
            String line = bufferedReader.readLine();
            return (line != null && line.length() != 0)
                    ? new JSONObject(line)
                    : null;
    	}
    	catch (IOException e) {
    		return null;
    	}
    }

    public static void writeToStdOut(JSONObject object){
        writeToStream(System.out, object);
    }

    private static void writeToStream(PrintStream stream, JSONObject object) {
        stream.println(object.toString());
    }
}
