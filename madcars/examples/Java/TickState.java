import org.json.JSONArray;
import org.json.JSONObject;

/**
 * Состояние мира, присылаемое сервером на каждом тике.
 * Передается на вход обработчика {@code onNextTick} интерфейса {@link Bot}
 */

class TickState {
    Car myCar;
    Car enemyCar;
    float deadLine;

    public TickState(JSONObject params){
        myCar       = new Car(params.getJSONArray("my_car"));
        enemyCar    = new Car(params.getJSONArray("enemy_car"));

        deadLine    = params.getFloat("deadline_position");
    }

    class Car {
        int mirror; // слева = +1, справа = -1

        float x, y;
        float angle;

        WheelPair wheel = new WheelPair();

        Car(JSONArray carParam){
            JSONArray pos = carParam.getJSONArray(0);
                x = pos.getFloat(0);
                y = pos.getFloat(1);

            angle = carParam.getFloat(1);
            mirror = carParam.getInt(2);

            wheel.rear = new Wheel(carParam.getJSONArray(3));
            wheel.front = new Wheel(carParam.getJSONArray(4));
        }
        class WheelPair {
            Wheel rear;
            Wheel front;
        }
        class Wheel {
            float x, y;
            float angle;
            Wheel (JSONArray wheelParam) {
                x = wheelParam.getFloat(0);
                y = wheelParam.getFloat(1);
                angle = wheelParam.getFloat(2);
            }
        }
    }
}