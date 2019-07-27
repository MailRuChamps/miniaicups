#!/usr/bin/env python3
from asyncio import events
import argparse
import os
import gzip
import json
import sys

import pyglet
from pyglet.gl import *
from pyglet.window import key

from helpers import TERRITORY_CACHE, load_image
from clients import Client, KeyboardClient, SimplePythonClient, FileClient
from constants import LR_CLIENTS_MAX_COUNT, MAX_TICK_COUNT, WINDOW_WIDTH, WINDOW_HEIGHT, WIDTH
from game_objects.scene import Scene
from game_objects.game import LocalGame, Game
from game_objects.bonuses import Bonus


loop = events.new_event_loop()
events.set_event_loop(loop)

parser = argparse.ArgumentParser(description='LocalRunner for paperio')

for i in range(1, LR_CLIENTS_MAX_COUNT + 1):
    parser.add_argument('-p{}'.format(i), '--player{}'.format(i), type=str, nargs='?',
                        help='Path to executable with strategy for player {}'.format(i))
    parser.add_argument('--p{}l'.format(i), type=str, nargs='?', help='Path to log for player {}'.format(i))

parser.add_argument('-t', '--timeout', type=str, nargs='?', help='off/on timeout', default='on')
parser.add_argument('-s', '--scale', type=int, nargs='?', help='window scale (%%)', default=100)
parser.add_argument('--replay', help='Replay visio.gz (no gui)')
parser.add_argument('--no-gui', help='Disable default gui', action='store_true')
parser.add_argument('-r', '--rewind-viewer', help='RewindViewer', action='store_true')

args = parser.parse_args()

if args.rewind_viewer:
    from RewindClient import RewindClient
    rewind_client = RewindClient()

    org_append_tick_to_game_log = Game.append_tick_to_game_log
    def append_tick_to_game_log(game: Game):
        W2 = WIDTH//2
        TERRITORY_LAYER = 1
        BONUS_LAYER = 2
        GRID_LAYER = 3
        TRAIL_LAYER = 4
        PLAYER_LAYER = 5
        DIRECTION_DELTA = {
            'left': (-1, 0),
            'right': (1, 0),
            'up': (0, 1),
            'down': (0, -1),
        }
        SPRITES = {
            'n': ((-W2+3, -W2+3, W2-3, W2-3), (0, W2-3, W2-3, W2-3), (W2-3, 0, W2-3, W2-3)),
            's': ((-W2+3, W2-3, W2-3, -W2+3), (0, -W2+3, W2-3, -W2+3), (W2-3, 0, W2-3, -W2+3)),
            'saw': ((-W2+3, 0, 0, W2-3), (0, W2-3, W2-3, 0), (W2-3, 0, 0, -W2+3), (0, -W2+3, -W2+3, 0)),
        }

        def color2rv(c):
            return c[2] | (c[1] << 8) | (c[0] << 16) | (c[3] << 24)

        rc = rewind_client
        for x in range(32):
            rc.line(x*WIDTH, 0, x*WIDTH, 31*WIDTH, 0xffffff, GRID_LAYER)
        for y in range(32):
            rc.line(0, y*WIDTH, 31*WIDTH, y*WIDTH, 0xffffff, GRID_LAYER)
        for p in game.players:
            for (x, y) in p.territory.points:
                rc.rectangle(x-W2, y-W2, x+W2, y+W2, color2rv(p.territory.color), TERRITORY_LAYER)
            for (x, y) in p.lines:
                rc.circle(x, y, int(0.1*WIDTH), color2rv(p.line_color), TRAIL_LAYER)
            rc.circle(p.x, p.y, int(0.2*WIDTH), 0, PLAYER_LAYER)
            rc.circle(p.x, p.y, int(0.2*WIDTH)-2, color2rv(p.color), PLAYER_LAYER)
            dx, dy = DIRECTION_DELTA.get(p.direction, (0, 0))
            rc.line(p.x, p.y, p.x+dx*int(0.2*WIDTH), p.y+dy*int(0.2*WIDTH), 0, PLAYER_LAYER)
        message = [f'Tick {game.tick}']
        for p in sorted(game.players + game.losers, key=lambda p: p.id):
            message += (
                '#{} @{} score {:3}'.format(p.id, '(dead    )' if p in game.losers else f'({p.x:3}, {p.y:3})', p.score),
                '' if not p.bonuses else '  bonus: {}'.format(', '.join(f'{b.visio_name} ({b.get_remaining_ticks()} cells)' for b in p.bonuses)),
            )
            pos = int(31.5*WIDTH), int((31.5-p.id)*WIDTH)
            rc.circle(*pos, int(0.4*WIDTH), color2rv(p.color), PLAYER_LAYER)
            rc.popup(*pos, int(0.4*WIDTH), p.name)
        for b in game.bonuses:
            for dx1, dy1, dx2, dy2 in SPRITES[b.visio_name]:
                rc.line(b.x+dx1, b.y+dy1, b.x+dx2, b.y+dy2, 0, BONUS_LAYER)
            rc.circle(b.x, b.y, int(0.1*WIDTH), 0, BONUS_LAYER)
            rc.popup(b.x, b.y, int(0.4*WIDTH), b.visio_name)
        rc.message('\n'.join(message))
        rc.end_frame()
        org_append_tick_to_game_log(game)
    Game.append_tick_to_game_log = append_tick_to_game_log

if args.replay:
    args.no_gui = True
    visio = json.load(gzip.open(args.replay))
    start_game = visio['visio_info'][0]
    assert(start_game['type'] == 'start_game')
    # FIXME: load WIDTH, SPEED, etc from `start_game`

    BONUS_CLASSES = {bc.visio_name: bc for bc in Bonus.__subclasses__()}
    org_send_game_tick = Game.send_game_tick

    def send_game_tick(self: Game):
        try:
            self.bonuses = []
            for b in visio['visio_info'][game.tick]['bonuses']:
                bb = BONUS_CLASSES[b['type']](b['position'])
                bb.active_ticks = b['active_ticks']
                self.bonuses.append(bb)
        except:
            pass
        org_send_game_tick(self)
    Game.send_game_tick = send_game_tick

    class ReplayClient(Client):
        def __init__(self, id):
            self.id = id

        async def get_command(self):
            try:
                direction = visio['visio_info'][game.tick+1]['players'][self.id]['direction']
            except:
                direction = 'left'
            return {"command": direction}

        def get_solution_id(self):
            return visio['config'][self.id]
    clients = [ReplayClient(id) for id in visio['config'].keys()]
else:
    if not args.no_gui:
        scene = Scene(args.scale)
    clients = []
    for i in range(1, LR_CLIENTS_MAX_COUNT + 1):
        arg = getattr(args, 'player{}'.format(i))
        if arg:
            if arg == 'keyboard':
                client = KeyboardClient(scene.window)
            elif arg == 'simple_bot':
                client = SimplePythonClient()
            else:
                client = FileClient(arg.split(), getattr(args, 'p{}l'.format(i)))

            clients.append(client)

if args.no_gui:
    game = Game(clients)
    loop.run_until_complete(game.game_loop_wrapper())
    if args.replay:
        for a, b in zip(visio['visio_info'], game.game_log):
            if a != json.loads(json.dumps(b)):  # json roundtrip to convert tuples to lists and int dict keys to strings
                print("Replay '{}' failed on tick {}".format(args.replay, a.get("tick_num", None)))
                sys.exit(1)
        else:
            print("OK")
    sys.exit(0)

if len(clients) == 0:
    clients.append(KeyboardClient(scene.window))


class Runner:
    @staticmethod
    def game_over_loop(dt):
        Runner.game.scene.clear()
        Runner.game.draw()

    @staticmethod
    def game_loop_wrapper(dt):
        is_game_over = loop.run_until_complete(Runner.game.game_loop())
        if is_game_over or (args.timeout == 'on' and Runner.game.tick >= MAX_TICK_COUNT):
            loop.run_until_complete(Runner.game.game_loop())
            Runner.game.send_game_end()
            Runner.game.game_save()
            Runner.stop_game()

    @staticmethod
    @scene.window.event
    def on_key_release(symbol, modifiers):
        if symbol == key.R:
            Runner.stop_game()
            TERRITORY_CACHE.clear()
            Runner.run_game()

    @staticmethod
    @scene.window.event
    def on_resize(width, height):
        (actual_width, actual_height) = scene.window.get_viewport_size()
        glViewport(0, 0, actual_width, actual_height)
        glMatrixMode(gl.GL_PROJECTION)
        glLoadIdentity()

        factScale = max(WINDOW_WIDTH / actual_width, WINDOW_HEIGHT / actual_height)
        xMargin = (actual_width * factScale - WINDOW_WIDTH) / 2
        yMargin = (actual_height * factScale - WINDOW_HEIGHT) / 2
        glOrtho(-xMargin, WINDOW_WIDTH + xMargin, -yMargin, WINDOW_HEIGHT + yMargin, -1, 1)
        glMatrixMode(gl.GL_MODELVIEW)
        return pyglet.event.EVENT_HANDLED

    @staticmethod
    def stop_game():
        pyglet.clock.schedule_interval(Runner.game_over_loop, 1 / 200)
        pyglet.clock.unschedule(Runner.game_loop_wrapper)

    @staticmethod
    def load_sprites():
        base_dir = os.path.dirname(os.path.realpath(__file__))
        absolute_path = os.path.join(base_dir, 'sprites')
        sprites = os.listdir(absolute_path)
        for sprite in sprites:
            if sprite.endswith('png'):
                load_image('sprites/{}'.format(sprite))

    @staticmethod
    def run_game():
        pyglet.clock.unschedule(Runner.game_over_loop)
        Runner.load_sprites()
        Runner.game = LocalGame(clients, scene, args.timeout == 'on')
        Runner.game.send_game_start()
        pyglet.clock.schedule_interval(Runner.game_loop_wrapper, 1 / 200)


Runner.run_game()
pyglet.app.run()
