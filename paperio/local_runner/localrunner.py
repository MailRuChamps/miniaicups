from asyncio import events
import argparse
import os

import pyglet
from pyglet.gl import *
from pyglet.window import key

from helpers import TERRITORY_CACHE, load_image
from clients import KeyboardClient, SimplePythonClient, FileClient
from constants import LR_CLIENTS_MAX_COUNT, MAX_TICK_COUNT
from game_objects.scene import Scene
from game_objects.game import LocalGame

from constants import WINDOW_WIDTH, WINDOW_HEIGHT

loop = events.new_event_loop()
events.set_event_loop(loop)

parser = argparse.ArgumentParser(description='LocalRunner for paperio')

for i in range(1, LR_CLIENTS_MAX_COUNT + 1):
    parser.add_argument('-p{}'.format(i), '--player{}'.format(i), type=str, nargs='?',
                        help='Path to executable with strategy for player {}'.format(i))
    parser.add_argument('--p{}l'.format(i), type=str, nargs='?', help='Path to log for player {}'.format(i))

parser.add_argument('-t', '--timeout', type=str, nargs='?', help='off/on timeout', default='on')
parser.add_argument('-s', '--scale', type=int, nargs='?', help='window scale (%)', default=100)

args = parser.parse_args()

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

if len(clients) == 0:
    clients.append(KeyboardClient(scene.window))


class Runner:
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
        Runner.load_sprites()
        Runner.game = LocalGame(clients, scene, args.timeout == 'on')
        Runner.game.send_game_start()
        pyglet.clock.schedule_interval(Runner.game_loop_wrapper, 1 / 200)


Runner.run_game()
pyglet.app.run()
