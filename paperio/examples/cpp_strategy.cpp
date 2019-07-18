#include <iostream>
#include <time.h>

#include "../nlohmann/json.hpp"

using namespace std;

int main() {
    srand(time(NULL));
    string input_string, input_type;
    char commands[4][10] = {"left", "right", "up", "down"};

    while (true) {
        getline(cin, input_string);
        auto index = rand() % 4;
        nlohmann::json command;
        command["command"] = commands[index];
        cout << command.dump() << endl;
    }

    return 0;
}