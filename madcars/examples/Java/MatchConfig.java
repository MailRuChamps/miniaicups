import org.json.JSONObject;

/**
 * Параметры матча (характеристики машин, контуры карт и т.д.), присылаемые в начале каждого матча.
 * Передается на вход обработчика {@code onMatchStarted} интерфейса {@link Bot}
 */

public class MatchConfig {
    //todo добавить нужные поля и классы и реализовать десериализацию json-объекта
    int myLives;
    int enemyLives;
    // ...

    public MatchConfig(JSONObject params) {
        myLives = params.getInt("my_lives");
        enemyLives = params.getInt("enemy_lives");
        // ...
    }
}
