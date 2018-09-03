/**
 * Интерфейс типового робота, получающего на вход конфигурацию и состояние мира в формате JSON,
 * и отправляющего на выход своё решение на текущем шаге (тоже в JSON) *
 */
public interface Bot {
    void onMatchStarted(MatchConfig matchConfig);
    void onNextTick(TickState tickState);
    void onParsingError(String message);
}
