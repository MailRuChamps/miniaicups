import json

class Strategy():
    def run(self):
        config = json.loads(raw_input())
        while True:
            data = json.loads(raw_input())
            cmd = self.on_tick(data, config)
            print json.dumps(cmd)

    def find_food(self, objects):
        for obj in objects:
            if obj.get('T') == 'F':
                return obj
        return None

    def on_tick(self, data, config):
        mine, objects = data.get('Mine'), data.get('Objects')
        if mine:
            mine = mine[0]
            food = self.find_food(objects)
            if food:
                return {'X': food.get('X'), 'Y': food.get('Y')}
            return {'X': 0, 'Y': 0, 'Debug': 'No food'}
        return {'X': 0, 'Y': 0, 'Debug': 'Died'}

if __name__ == '__main__':
    Strategy().run()