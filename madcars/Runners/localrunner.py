from itertools import product

import pyglet
import pymunk.pyglet_util
import argparse

from asyncio import events

from mechanic.game import Game
from mechanic.strategy import KeyboardClient, FileClient

scale_factor = 0.7;

window = pyglet.window.Window(1200 * scale_factor, 800 * scale_factor, vsync=False)
draw_options = pymunk.pyglet_util.DrawOptions()
_ = pyglet.clock.ClockDisplay(interval=0.016)


parser = argparse.ArgumentParser(description='LocalRunner for MadCars')

parser.add_argument('-f', '--fp', type=str, nargs='?',
                    help='Path to executable with strategy for first player', default='keyboard')
parser.add_argument('--fpl', type=str, nargs='?', help='Path to log for first player')

parser.add_argument('-s', '--sp', type=str, nargs='?',
                    help='Path to executable with strategy for second player', default='keyboard')
parser.add_argument('--spl', type=str, nargs='?', help='Path to log for second player')


maps = ['PillMap', 'PillHubbleMap', 'PillHillMap', 'PillCarcassMap', 'IslandMap', 'IslandHoleMap']
cars = ['Buggy', 'Bus', 'SquareWheelsBuggy']
games = [','.join(t) for t in product(maps, cars)]


parser.add_argument('-m', '--matches', nargs='+', help='List of pairs(map, car) for games', default=games)

args = parser.parse_args()

first_player = args.fp
second_player = args.sp

if args.fp == 'keyboard':
    fc = KeyboardClient(window)
else:
    fc = FileClient(args.fp.split(), args.fpl)

if args.sp == 'keyboard':
    sc = KeyboardClient(window)
else:
    sc = FileClient(args.sp.split(), args.spl)

game = Game([fc, sc], args.matches, extended_save=False)

loop = events.new_event_loop()
events.set_event_loop(loop)


@window.event
def on_draw():
    pyglet.gl.glClearColor(255,255,255,255)
    
    pyglet.gl.glPushMatrix()
    pyglet.gl.glScalef(scale_factor, scale_factor, scale_factor)
    window.clear()
    game.draw(draw_options)
    game.tick()
    if not game.game_complete:
        future_message = loop.run_until_complete(game.tick())
    else:
        winner = game.get_winner()
        if winner:
            pyglet.text.Label("Player {} win".format(winner.id), font_name='Times New Roman',
                          font_size=36,
                          color=(255, 0, 0, 255),
                          x=600, y=500,
                          anchor_x='center', anchor_y='center').draw()
        else:
            pyglet.text.Label("Draw", font_name='Times New Roman',
                              font_size=36,
                              color=(255, 0, 0, 255),
                              x=600, y=500,
                              anchor_x='center', anchor_y='center').draw()
                
    pyglet.gl.glPopMatrix()


pyglet.app.run()
