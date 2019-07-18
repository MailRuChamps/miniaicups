import os
import json


def toint(value, default=None):
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


def parse_json(value, default=None):
    try:
        return json.loads(value)
    except (TypeError, json.decoder.JSONDecodeError):
        return default


LEFT = 'left'
RIGHT = 'right'
UP = 'up'
DOWN = 'down'

SPEED = toint(os.getenv('SPEED'), 5)
WIDTH = toint(os.getenv('WIDTH'), 30) # должно делиться на 2
BONUS_CHANCE = toint(os.getenv('BONUS_CHANCE'), 500) # 1 из BONUS_CHANCE
BONUSES_MAX_COUNT = toint(os.getenv('BONUSES_MAX_COUNT'), 3)
Y_CELLS_COUNT = toint(os.getenv('Y_CELLS_COUNT'), 31)
X_CELLS_COUNT = toint(os.getenv('X_CELLS_COUNT'), 31)

NEUTRAL_TERRITORY_SCORE = toint(os.getenv('NEUTRAL_TERRITORY_SCORE'), 1)
ENEMY_TERRITORY_SCORE = toint(os.getenv('ENEMY_TERRITORY_SCORE'), 5)
SAW_SCORE = toint(os.getenv('SAW_SCORE'), 30)
LINE_KILL_SCORE = toint(os.getenv('LINE_KILL_SCORE'), 50)
SAW_KILL_SCORE = toint(os.getenv('SAW_KILL_SCORE'), 150)

LR_CLIENTS_MAX_COUNT = toint(os.getenv('LR_CLIENTS_MAX_COUNT'), 6)

MAX_EXECUTION_TIME = toint(os.getenv('MAX_EXECUTION_TIME'), 120)
REQUEST_MAX_TIME = toint(os.getenv('REQUEST_MAX_TIME'), 5)
MAX_TICK_COUNT = toint(os.getenv('MAX_TICK_COUNT'), 1500)
CLIENTS_COUNT = toint(os.getenv('CLIENTS_COUNT'), 2)
AVAILABLE_BONUSES = parse_json(os.getenv('AVAILABLE_BONUSES'), ['n', 's', 'saw'])

PLAYER_COLORS = [
    (90, 159, 153, 255),
    (216, 27, 96, 255),
    (96, 125, 139, 255),
    (245, 124, 0, 255),
    (92, 107, 192, 255),
    (141, 110, 99, 255)
]

WINDOW_HEIGHT = Y_CELLS_COUNT * WIDTH
WINDOW_WIDTH = X_CELLS_COUNT * WIDTH
