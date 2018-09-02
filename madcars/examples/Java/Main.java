import org.json.JSONObject;
import java.util.Random;

public class Main {
    private static Bot robot;

    public static void main(String[] args) {

        JSONObject  gameMessage;
        while ((gameMessage = JsonIO.readFromStdIn()) != null) {
            MessageType messageType;
            try {
                messageType = gameMessage.getEnum(MessageType.class, "type");
                switch (messageType) {
                    case tick:
                        TickState tickState = new TickState(gameMessage.getJSONObject("params"));
                        robot().onNextTick(tickState);
                        break;

                    case new_match:
                        MatchConfig matchConfig = new MatchConfig(gameMessage.getJSONObject("params"));
                        robot().onMatchStarted(matchConfig);
                        break;
                }
            }
            catch (Exception e){
                robot().onParsingError(e.getMessage());
            }
        }
    }

    private static Bot robot() {

        if (robot == null)
            robot = new Bot() {
            // todo заменить этот анонимный класс-заглушку реальным классом стратегии
                static final int ON_AIR_PAUSE = 50;
                final String commands[] = {"left", "stop", "right"};

                int thisMathTick = 0;
                int matchCounter = 0;
                String debugMessage = "";

                @Override
                public void onMatchStarted(MatchConfig matchConfig){
                    thisMathTick = 0;
                    matchCounter ++;

                    debugMessage = String.format(".... Match #%d: lives=%d, ", matchCounter, matchConfig.myLives);
                }

                @Override
                public void onNextTick(TickState tickState) {
                    thisMathTick++;

                    if (thisMathTick == 1)
                        debugMessage += String.format("my side=%s.... ", commands[1 - tickState.myCar.mirror]);

                    String cmd = thisMathTick <= ON_AIR_PAUSE ? "stop" : commands[new Random().nextInt(3)];
                    debugMessage += String.format("%d.%d: %s",matchCounter, thisMathTick, cmd);

                    new Answer(cmd, debugMessage).send();
                    debugMessage = "";
                }

                @Override
                public void onParsingError(String message) {
                    debugMessage = message;
                }
            };
        return robot;
    }

    public static class Answer {
        String command;
        String debug;

        public String getCommand()  { return command;}
        public String getDebug()    { return debug;}

        Answer(String cmd, String dbg){
            command = cmd;
            debug = dbg;
        }
        void send(){
            JSONObject json = new JSONObject(this);
            JsonIO.writeToStdOut(json);
        }
    }
    enum MessageType {
        new_match,
        tick
    }
}
