import json

# death by intersect
SCRIPT_COLL_1 = [
    'down',
    'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up',
    'left',
]

# empty hole
SCRIPT_COLL_2 = [
    'down',
    'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up',
    'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down',
]

# paint death
SCRIPT_COLL_3 = [
    'down', 'down',
    'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up', 'up',
    'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down', 'down',
]

# paint death ext
SCRIPT_COLL_3_EXT = [
    'down', 'down',
    'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up', 'up',
    'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down', 'down',
]

# paint inside
SCRIPT_COLL_4 = [
    'down', 'down',
    'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up', 'up',
    'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down',
]

# intersect attempt
SCRIPT_COLL_5 = [
    'down', 'down', 'down',
    'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up', 'up', 'up', 'up',
    'left', 'left', 'left', 'left', 'left',
    'down', 'down', 'down', 'down', 'down', 'down', 'down',
    'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'up', 'up', 'up', 'up', 'up', 'up', 'up',
    'left', 'left', 'left', 'left', 'left', 'left',
]

# paint vs intersect
SCRIPT_COLL_6 = [
    'up', 'right', 'right', 'down', 'down', 'left',
    'up', 'right', 'right', 'down', 'down', 'down', 'down', 'down', 'down', 'left', 'up', 'up', 'up', 'up',
    'up', 'right', 'right', 'right', 'right', 'down',
]

SCRIPT_COLL_7 = [
    'right',
    'down',
    'right', 'right', 'right',
    'up', 'up', 'up', 'up', 'up',
    'left',
    'down', 'down', 'down', 'down',
    'left',
    'up', 'up', 'up', 'up',
    'left',
    'down', 'down',
    'left',
    'down', 'down'
]


def main(commands):
    i = 0
    while True:
        z = input()
        print(json.dumps({"command": commands[i], 'debug': str(z)}))
        i += 1


main(SCRIPT_COLL_3_EXT)
