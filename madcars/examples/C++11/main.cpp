#include <iostream>

#include "../nlohmann/json.hpp"

using namespace std;

#define PI 3.14159265358979323846264338327950288

int main() {
    string input_string, input_type;
    int tick(0);
    int round(-1);
    while (true) {

        getline(cin, input_string);
        auto state = nlohmann::json::parse(input_string);
        // also possible:
        // json state;
        // cin >> state;

        input_type = state["type"].get<std::string>();
        auto params = state["params"];

        if (input_type == "new_match") {
            int my_lives = params["my_lives"].get<int>();
            int enemy_lives = params["enemy_lives"].get<int>();

            // Example of proto_map parsing.
            auto map = params["proto_map"];
            int map_id = map["external_id"].get<int>();
            auto segments = map["segments"];
            for(auto segment:segments){
                auto fp = segment[0];
                auto sp = segment[1];
                double height = segment[2].get<double>();
            }

            auto proto_car = params["proto_car"];
            // etc...

            round++;
            tick = 0;

            //cerr << "Round " << round + 1 << " started!" << endl;

        } else if (input_type == "tick") {
            //cerr << "Round " << round + 1;
            //cerr << " tick " << tick << endl;

            nlohmann::json command;
            if (tick < 20) {
                // wait a little bit to fall on floor
                command["command"] = "stop";
                //cerr << "Waiting..." << endl;
            } else {
                auto my_car = params["my_car"];
                auto enemy_car = params["enemy_car"];

                auto my_pos = my_car[0];
                auto enemy_pos = enemy_car[0];

                // check my and enemy position and go to the enemy
                if(my_pos[0].get<double>() > enemy_pos[0].get<double>()) {
                    command["command"] = "left";
                } else {
                    command["command"] = "right";
                }

                // roll over  in air prevention (this feature can lead to death)
                double my_angle = my_car[1].get<double>();

                // normalize angle
                while (my_angle > PI) {
                    my_angle -= 2.0 * PI;
                }
                while (my_angle < -PI) {
                    my_angle += 2.0 * PI;
                }

                if (my_angle > PI / 4.0) {
                    //cerr << "Uhh!" << endl;
                    command["command"] = "left";
                } else if (my_angle < -PI / 4.0) {
                    //cerr << "Ahh!" << endl;
                    command["command"] = "right";
                } else {
                    //cerr << "Attack!" << endl;
                }
            }

            //cerr << command.dump() << endl;
            cout << command.dump() << endl;

            tick++;
        } else {
            //cerr << "end_game " << endl;
            break;
        }
    }

    return 0;
}