import asyncio
import gzip
import json
import random
from collections import defaultdict

import math
import os
from itertools import product

import pymunkoptions

from mechanic.constants import MAX_TICK_COUNT

pymunkoptions.options["debug"] = False
import pymunk

from mechanic.game_objects.cars import Buggy, Bus, SquareWheelsBuggy
from mechanic.game_objects.maps import PillMap, PillHubbleMap, PillHillMap, PillCarcassMap, IslandMap, IslandHoleMap
from mechanic.match import Match
from mechanic.player import Player


class Game(object):
    CARS_MAP = {
        'Buggy': Buggy,
        'Bus': Bus,
        'SquareWheelsBuggy': SquareWheelsBuggy,

    }

    MAPS_MAP = {
        'PillMap': PillMap,
        'PillHubbleMap': PillHubbleMap,
        'PillHillMap': PillHillMap,
        'PillCarcassMap': PillCarcassMap,
        'IslandMap': IslandMap,
        'IslandHoleMap': IslandHoleMap,
    }

    RESULT_LOCATION = os.environ.get('GAME_LOG_LOCATION', './result')

    BASE_DIR = os.path.dirname(RESULT_LOCATION)

    VISIO_LOCATION = os.path.join(BASE_DIR, 'visio.gz')
    SCORES_LOCATION = os.path.join(BASE_DIR, 'scores.json')
    DEBUG_LOCATION = os.path.join(BASE_DIR, '{}')

    def __init__(self, clients, games_list, extended_save=True):
        self.game_complete = False
        self.extended_save = extended_save

        self.max_match_count = math.ceil(len(games_list)/2)
        self.all_players = [Player(index + 1, client, self.max_match_count) for index, client in enumerate(clients)]

        self.space = pymunk.Space()
        self.space.gravity = (0.0, -700)
        self.space.damping = 0.85
        self.scores = defaultdict(int)
        self.matches = self.parse_games(games_list)
        self.current_match = None
        self.tick_num = 0

        self.game_log = []

        self.next_match()

    @classmethod
    def parse_games(cls, games_list):
        for g in games_list:
            m, c = g.split(',', maxsplit=1)
            yield (cls.MAPS_MAP.get(m, PillMap), cls.CARS_MAP.get(c, Buggy))

    def clear_space(self):
        self.space.remove(self.space.shapes)
        self.space.remove(self.space.bodies)
        self.space.remove(self.space.constraints)

    def end_game(self):
        self.game_complete = True
        self.game_save()

    def get_winner(self):
        winner = sorted(self.all_players, key=lambda x: x.lives, reverse=True)
        if winner:
            return winner[0]
        return False

    def next_match(self):
        map, car = next(self.matches)
        self.clear_space()
        match = Match(map, car, self.all_players, self.space)
        self.space.add(match.get_objects_for_space())
        self.current_match = match

    @asyncio.coroutine
    def game_loop(self):
        for i in range(MAX_TICK_COUNT):
            if i % 2000 == 0:
                print('tick {}'.format(i))
            is_game_continue = yield from self.tick()
            if is_game_continue == 'end_game':
                break

    @asyncio.coroutine
    def tick(self):
        yield from self.current_match.tick(self.tick_num)
        self.space.step(0.016)

        if self.current_match.is_match_ended():
            self.game_log.extend(self.current_match.end_match())

            if not all([p.is_alive() for p in self.all_players]):
                self.game_log.append({
                    'type': "end_game",
                    "params": {p.id: p.get_lives() for p in self.all_players}
                })
                self.end_game()

                return 'end_game'
            self.next_match()

        self.tick_num += 1

    def draw(self, draw_options):
        self.space.debug_draw(draw_options)

    def get_players_external_id(self):
        return {p.id: p.get_solution_id() for p in self.all_players}

    def save_visio(self):
        d = {
            'config': self.get_players_external_id(),
            'visio_info': self.game_log
        }
        with gzip.open(self.VISIO_LOCATION, 'wb') as f:
            f.write(json.dumps(d).encode())
        return {
            "filename": os.path.basename(self.VISIO_LOCATION),
            "location": self.VISIO_LOCATION,
            "is_private": False
        }

    def save_scores(self):
        d = {p.get_solution_id(): p.get_lives() for p in self.all_players}

        with open(self.SCORES_LOCATION, 'w') as f:
            f.write(json.dumps(d))

        return {
            "filename": os.path.basename(self.SCORES_LOCATION),
            "location": self.SCORES_LOCATION,
            "is_private": False
        }

    def save_debug(self):
        return [
            p.save_log(self.DEBUG_LOCATION) for p in self.all_players
        ]

    def game_save(self):
        if self.extended_save:
            result = {
                "scores": self.save_scores(),
                "debug": self.save_debug(),
                "visio": self.save_visio()
            }

            with open(self.RESULT_LOCATION, 'w') as f:
                f.write(json.dumps(result))
        else:
            self.save_debug()
            self.save_visio()

    @classmethod
    def generate_matches(cls, count):
        available_matches = product(sorted(cls.MAPS_MAP.keys()), sorted(cls.CARS_MAP.keys()))
        return random.sample([','.join(x) for x in available_matches], count)
