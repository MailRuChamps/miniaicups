import os
import random


def toint(value, default=None):
    try:
        return int(value)
    except (TypeError, ValueError):
        return default


TICKS_TO_DEADLINE = toint(os.getenv('TICKS_TO_DEADLINE'), 600)
MATCHES = os.getenv('MATCHES', ' '.join(['PillMap,Buggy'] * 9)).split()
MAX_EXECUTION_TIME = toint(os.getenv('MAX_EXECUTION_TIME'), 120)
REQUEST_MAX_TIME = toint(os.getenv('REQUEST_MAX_TIME'), 5)
MAX_TICK_COUNT = toint(os.getenv('MAX_TICK_COUNT'), 20000)
SEED = toint(os.getenv('SEED'), random.randint(0, 2**128))
MATCHES_COUNT = toint(os.getenv('MATCHES_COUNT'), 9)
REST_TICKS = toint(os.getenv('REST_TICKS'), 90)

random.seed(SEED)
