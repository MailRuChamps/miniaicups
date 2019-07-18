import json
import random


while True:
    z = input()
    commands = ['left', 'right', 'up', 'down']
    cmd = random.choice(commands)
    print(json.dumps({"command": cmd, 'debug': str(z)}))
