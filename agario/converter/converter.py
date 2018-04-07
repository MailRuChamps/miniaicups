from collections import defaultdict
import math
import copy
import json
import sys

if len(sys.argv) < 3:
    print("Format:\npython native_log_converter.py [-f] <path_to_gcode.log> <path_to_json.log>")
    print("Flags:")
    print("-f : Log full snapshots for each tick (default it delta)")
    sys.exit(0)

GCODE_LOG_FILENAME = sys.argv[-2]
JSON_LOG_FILENAME = sys.argv[-1]
SAVE_TICKS_FULL = sys.argv[1] == '-f'

DYNAMIC_PARAMS_MARK = "# Dynamic params "

DEFAULT_CONFIG = {
    'GAME_WIDTH': 990,
    'GAME_HEIGHT': 990,
    'GAME_TICK': 7500,
    'FOOD_RADIUS': 2.5,
    'VIRUS_MASS': 40,
    'PLAYER_MASS': 40,
    'PLAYER_RADIUS': 2 * math.sqrt(40),
    'EJECT_MASS': 15,
    'EJECT_RADIUS': 4
}


def parse_number(string):
    try:
        return int(string)
    except (ValueError, TypeError):
        pass

    try:
        return float(string)
    except (ValueError, TypeError):
        return -1


def parse_config(cfg_line):
    config_list = cfg_line.split()
    return {c[0]: parse_number(c[1]) for c in map(lambda x: x.split('='), config_list)}


log_file = open(GCODE_LOG_FILENAME, 'r')

_ = log_file.readline()
config_line = log_file.readline()
z = log_file.readlines()[7:]

config = DEFAULT_CONFIG
config.update(parse_config(config_line.replace(DYNAMIC_PARAMS_MARK, '')))

TICKS = {}
TICKS_DELTA = {}

FOODS = {}
PLAYERS = {}
EJECTS = {}
VIRUSES = {}
SCORES = {}
SOLUTIONS = {}
COMMANDS = {}

TICK_NUM = 0

updated_food = defaultdict(dict)
updated_eject = defaultdict(dict)
updated_viruses = defaultdict(dict)
updated_players = defaultdict(dict)
new_scores = {}

deleted_food = []
deleted_players = []
deleted_viruses = []
deleted_eject = []


def parse_line(f_line: str, param: str, ignore: list = None):
    result = {}
    spl = f_line.split()
    entity_id = spl[0].replace(param, '')

    for s_param in spl[1:]:
        key = s_param[0:1]
        value = s_param[1:]
        if (ignore is None) or (key not in ignore):
            value = parse_number(value)
        result[key.lower()] = value

    return entity_id, result


def flush_snapshot():
    TICKS[TICK_NUM] = {
        'p': copy.deepcopy(PLAYERS),
        'f': copy.deepcopy(FOODS),
        'v': copy.deepcopy(VIRUSES),
        'e': copy.deepcopy(EJECTS),
        's': copy.deepcopy(SCORES),
        'c': copy.deepcopy(COMMANDS)
    }

    TICKS_DELTA[TICK_NUM] = {
        'p': copy.deepcopy(updated_players),
        'f': copy.deepcopy(updated_food),
        'v': copy.deepcopy(updated_viruses),
        'e': copy.deepcopy(updated_eject),
        's': copy.deepcopy(new_scores),

        'df': copy.deepcopy(deleted_food),
        'dp': copy.deepcopy(deleted_players),
        'dv': copy.deepcopy(deleted_viruses),
        'de': copy.deepcopy(deleted_eject),
        'c': copy.deepcopy(COMMANDS)
    }


def save_json(log_filename):
    json_filename = log_filename
    o_json = {
        'config': config
    }

    if SAVE_TICKS_FULL:
        o_json['ticks'] = TICKS
    else:
        o_json['ticks_delta'] = TICKS_DELTA
    with open(json_filename, 'w') as fp:
        json.dump(o_json, fp)


map_entities = {
    'AF': (FOODS, updated_food),
    'AV': (VIRUSES, updated_viruses),
    'AE': (EJECTS, updated_eject),
    'AP': (PLAYERS, updated_players),
}

# skeleton_food = ['id', 'x', 'y']
# skeleton_player = ['id', 'x', 'y', 'radius', 'mass', 'color', ]


for line in z:
    line = line.strip()
    if line == "":
        continue

    if line.startswith('T'):
        flush_snapshot()

        TICK_NUM = int(line[1:])

        updated_food.clear()
        updated_viruses.clear()
        updated_eject.clear()
        updated_players.clear()
        new_scores = {}
        COMMANDS = {}

        deleted_food.clear()
        deleted_players.clear()
        deleted_viruses.clear()
        deleted_eject.clear()

    elif line[0:2] in map_entities.keys():
        e_key = line[0:2]
        p_value = map_entities[e_key]
        id_, res = parse_line(line, e_key)
        if e_key != 'AP':
            id_ = int(id_)

        if id_ not in p_value[0]:
            p_value[0][id_] = res
            p_value[1][id_] = res

    elif line[0:1] == 'C':
        sp = line.split()
        COMMANDS[sp[0][1:]] = {
            'x': parse_number(sp[1][1:]),
            'y': parse_number(sp[2][1:]),
            's': sp[-1] == 'S',
            'e': sp[-1] == 'E'
        }

    elif line[0:1] == 'P':
        sp = line.split()
        pid = sp[0][1:2]
        score = parse_number(sp[1][1:])
        SCORES[pid] = score
        new_scores[pid] = score

    elif line[0:2] == 'OI':
        sp = line.split()
        id_ = (sp[0][2:])
        SOLUTIONS[id_] = parse_number(sp[1][1:])

    elif line[0:2] == 'KP':
        sp = line.split()
        id_ = (sp[0][2:])
        PLAYERS.pop(id_)
        deleted_players.append(id_)

    elif line[0:2] == 'KV':
        sp = line.split()
        id_ = int(sp[0][2:])
        VIRUSES.pop(id_)
        deleted_viruses.append(id_)

    elif line[0:2] == 'KE':
        sp = line.split()
        id_ = int(sp[0][2:])
        EJECTS.pop(id_)
        deleted_eject.append(id_)

    elif line[0:1] == '+':
        subj = line[1:2]

        if subj == 'P':
            id_, res = parse_line(line, '+P', ['I'])

            # rename player
            if 'i' in res.keys():
                new_id = str(res['i'])
                PLAYERS[new_id] = PLAYERS[id_]
                PLAYERS.pop(id_)
                updated_players[new_id] = updated_players[id_]
                updated_players.pop(id_)
                id_ = new_id
                res.pop('i')

            updated_players[id_].update(res)
            PLAYERS[id_].update(res)

        elif subj == 'E':
            id_, res = parse_line(line, '+E')
            id_ = int(id_)
            updated_eject[id_].update(res)
            EJECTS[id_].update(res)

        elif subj == 'V':
            id_, res = parse_line(line, '+V')
            id_ = int(id_)
            updated_viruses[id_].update(res)
            VIRUSES[id_].update(res)

    elif line.startswith('KF'):
        id_ = int(line.replace('KF', '').strip())
        FOODS.pop(id_)
        deleted_food.append(parse_number(id_))

    else:
        print("Unknown sequence:", line)

flush_snapshot()
save_json(JSON_LOG_FILENAME)

print("Done.")
