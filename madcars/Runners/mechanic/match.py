import asyncio
from functools import partial

import os
from pymunk import Vec2d

from mechanic.constants import TICKS_TO_DEADLINE, REST_TICKS
from mechanic.game_objects.base_car import Car
from mechanic.game_objects.deadline import DeadLine


class Match:
    def __init__(self, map, car, players, space):
        self.ticks_to_deadline = TICKS_TO_DEADLINE
        self.map = map
        self.car = car
        self.players = players
        self.map_objects = map(space).get_objects_for_space()
        self.cars_objects = []
        self.deadline = DeadLine(DeadLine.ASC, 1800, 800, space)

        self.map_objects.append(self.deadline.get_object_for_space())
        self.dead_players = set()

        self.is_rest = False
        self.rest_counter = 0

        self.match_log = []

        for index, player in enumerate(self.players):
            if (index + 1) % 2:
                c = car(player.get_game_id(), Car.RIGHT_DIRECTION, space.point_query_nearest)
                self.cars_objects.extend(c.get_objects_for_space_at(Vec2d(300, 300)))
            else:
                c = car(player.get_game_id(), Car.LEFT_DIRECTION, space.point_query_nearest)
                self.cars_objects.extend(c.get_objects_for_space_at(Vec2d(900, 300)))

            co = space.add_wildcard_collision_handler(c.get_button_collision_type())
            co.begin = partial(self.lose_callback, player)
            player.set_car(c)

        self.send_new_match_message()

    @asyncio.coroutine
    def apply_turn_wrapper(self, player, game_tick):
        if not self.is_rest:
            yield from player.apply_turn(game_tick)

    @asyncio.coroutine
    def tick(self, game_tick):
        self.send_tick(game_tick)
        futures = []
        for p in self.players:
            futures.append(asyncio.ensure_future(self.apply_turn_wrapper(p, game_tick)))
        if futures:
            yield from asyncio.wait(futures)

        if self.ticks_to_deadline < 1:
            self.deadline.move()
        else:
            self.ticks_to_deadline -= 1

        if not self.is_rest and self.smbd_die():
            self.rest_counter = REST_TICKS
            self.is_rest = True

        if self.rest_counter > 0:
            self.rest_counter -= 1

    def get_objects_for_space(self):
        return self.map_objects + self.cars_objects

    def get_players_lives(self, player=None):
        if player:
            myself = player.lives
            enemy = None
            for p in self.players:
                if player != p:
                    enemy = p.lives
                    break
            return myself, enemy
        else:
            return {p.id: p.lives for p in self.players}

    def get_players_car(self, player=None):
        if player:
            my = player.car.fast_dump()
            enemy = None

            for p in self.players:
                if p != player:
                    enemy = p.car.fast_dump()
                    break
            return my, enemy
        else:
            return {p.id: p.car.fast_dump()for p in self.players}

    def send_new_match_message(self):
        proto_map = self.map.get_proto()
        proto_car = self.car.proto_dump()

        self.match_log.append({
            'type': 'new_match',
            'params': {
                'lives': self.get_players_lives(),
                'proto_map': proto_map,
                'proto_car': proto_car
            }
        })

        for p in self.players:
            my_lives, enemy_lives = self.get_players_lives(p)
            p.send_message('new_match', {
                'my_lives': my_lives,
                'enemy_lives': enemy_lives,
                'proto_map': proto_map,
                'proto_car': proto_car,
            })

    def send_tick(self, game_tick):
        self.match_log.append({
            'type': 'tick',
            'params': {
                'cars': self.get_players_car(),
                'deadline_position': self.deadline.get_position(),
                'tick_num': game_tick
            }
        })

        if not self.is_rest:
            for p in self.players:
                my_car, enemy_car = self.get_players_car(p)
                p.send_message('tick', {
                    'my_car': my_car,
                    'enemy_car': enemy_car,
                    'deadline_position': self.deadline.get_position()
                })

    def lose_callback(self, player, arbiter, space, _):
        if not self.is_rest:
            self.dead_players.add(player)
        return False

    def smbd_die(self):
        return bool(self.dead_players)

    def is_match_ended(self):
        return self.rest_counter == 0 and bool(self.dead_players) and self.is_rest

    def end_match(self):
        for p in self.dead_players:
            p.die()
        return self.match_log
