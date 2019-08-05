import json

# death by intersect
SCRIPT_COLL_1 = [
    'up',
    'left', 'left', 'left', 'left', 'left', 'left',
    'down', 'down', 'down',
    'right',
]

# empty hole
SCRIPT_COLL_2 = [
    'up',
    'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down', 'down', 'down',
    'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'up',
]

# paint death
SCRIPT_COLL_3 = [
    'up', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
]

# paint death ext
SCRIPT_COLL_3_EXT = [
    'down', 'down', 'down', 'down', 'right', 'up', 'up', 'up',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'right', 'right',
]

# paint inside
SCRIPT_COLL_4 = [
    'up',
    'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
    'down', 'left', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'up', 'right',
]

# intersect attempt
SCRIPT_COLL_5 = [
    'up', 'left', 'down', 'down', 'right', 'right',
    'up', 'up', 'left', 'left', 'down', 'down', 'down', 'right', 'right', 'right',
    'up', 'up', 'up', 'left', 'left', 'left', 'down', 'down', 'down', 'right', 'right', 'right',
    'up', 'up', 'up', 'left', 'up', 'left', 'left', 'left', 'left', 'left', 'left', 'left',
    'down', 'down', 'down', 'right', 'right', 'right', 'right', 'right', 'right', 'right', 'right',
]

# paint vs intersect
SCRIPT_COLL_6 = [
    'down', 'down', 'left', 'left', 'left', 'up', 'up', 'up', 'right', 'right',
    'down', 'down', 'left', 'left', 'left', 'left', 'up', 'up', 'up', 'up', 'right', 'right', 'down', 'down', 'left',
]

SCRIPT_COLL_7 = [
    'up', 'up', 'up',
    'right', 'right', 'right', 'right', 'right',
    'down', 'down', 'down', 'down', 'down', 'down', 'down',
    'left', 'left','left', 'left', 'left', 'left',
    'up', 'up', 'up', 'up',
    'right',
]


def main(commands):
    i = 0
    while True:
        z = input()
        print(json.dumps({"command": commands[i], 'debug': str(z)}))
        i += 1


main(SCRIPT_COLL_3_EXT)
