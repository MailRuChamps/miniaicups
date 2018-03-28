#include "../nlohmann/json.hpp"
#include <iostream>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

struct Strategy {

	void run() {
		string data;
		cin >> data;
		auto config = json::parse(data);
		while (true) {
			cin >> data;
			auto parsed = json::parse(data);
			auto command = on_tick(parsed, config);
			cout << command.dump() << endl;
		}
	}

	json on_tick(const json &data, const json &config) {
		auto mine = data["Mine"];
		auto objects = data["Objects"];
		if (! mine.empty()) {
			auto first = mine[0];
			auto food = find_food(objects);
			if (! food.empty()) {
				return {{"X", food["X"]}, {"Y", food["Y"]}};
			}
			return {{"X", 0}, {"Y", 0}, {"Debug", "No food"}};
		}
		return {{"X", 0}, {"Y", 0}, {"Debug", "Died"}};
	}

    template <class T>
	json find_food(const T &objects) {
		for (auto &obj : objects) {
			if (obj["T"] == "F") {
				return obj;
			}
		}
		return json({});
	}
};

int main() {
	Strategy strategy;
	strategy.run();
	return 0;
}