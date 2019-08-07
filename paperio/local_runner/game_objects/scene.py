import pyglet

from constants import CONSTS
from helpers import draw_quadrilateral, draw_line


class Grid:
    def __init__(self, color):
        self.batch = pyglet.graphics.Batch()

        y = CONSTS.WIDTH
        while (y < CONSTS.WINDOW_HEIGHT):
            self.batch.add(2, pyglet.gl.GL_LINES, None,
                      ('v2i', (0, y, CONSTS.WINDOW_WIDTH, y)),
                      ('c4B', 2 * color))
            y += CONSTS.WIDTH

        x = CONSTS.WIDTH
        while (x < CONSTS.WINDOW_WIDTH):
            self.batch.add(2, pyglet.gl.GL_LINES, None,
                      ('v2i', (x, 0, x, CONSTS.WINDOW_HEIGHT)),
                      ('c4B', 2 * color))
            x += CONSTS.WIDTH

    def draw(self):
        self.batch.draw()


class Scene:
    background_color = (220 / 255, 240 / 255, 244 / 255, 1)
    border_color = (144, 163, 174, 255)
    grid_color = (144, 163, 174, 64)
    border_width = 2
    game_over_label_color = (95, 99, 104, 255)

    leaderboard_color = (255, 255, 255, 128)
    leaderboard_width = 320
    leaderboard_height = 240

    leaderboard_rows_count = 0
    labels_buffer = []

    def __init__(self, scale):
        self.window = pyglet.window.Window(height=int(CONSTS.WINDOW_HEIGHT * scale / 100),
                                           width=int(CONSTS.WINDOW_WIDTH * scale / 100),
                                           resizable=True)
        pyglet.options['debug_gl'] = False
        pyglet.gl.glClearColor(*self.background_color)
        pyglet.gl.glEnable(pyglet.gl.GL_BLEND)
        pyglet.gl.glBlendFunc(pyglet.gl.GL_SRC_ALPHA, pyglet.gl.GL_ONE_MINUS_SRC_ALPHA)

        self.grid = Grid(self.grid_color)
        self.game_over_label = pyglet.text.Label('GAME OVER', font_name='Times New Roman',
                                                 font_size=30,
                                                 color=self.game_over_label_color,
                                                 x=CONSTS.WINDOW_WIDTH / 2, y=CONSTS.WINDOW_HEIGHT / 2,
                                                 anchor_x='center', anchor_y='center')

    def clear(self):
        self.window.clear()
        self.draw_grid()

    def append_label_to_leaderboard(self, label, color):
        if len(self.labels_buffer) > self.leaderboard_rows_count:
            self.labels_buffer[self.leaderboard_rows_count].text = label
            self.labels_buffer[self.leaderboard_rows_count].color = color
        else:
            self.labels_buffer.append(
                pyglet.text.Label(label,
                                  font_name='Times New Roman',
                                  font_size=16,
                                  color=color,
                                  x=CONSTS.WINDOW_WIDTH - self.leaderboard_width + 20,
                                  y=CONSTS.WINDOW_HEIGHT - 20 - CONSTS.WIDTH / 2 - 30 * self.leaderboard_rows_count,
                                  anchor_x='left', anchor_y='center')
            )
        self.leaderboard_rows_count += 1

    def reset_leaderboard(self):
        self.leaderboard_rows_count = 0

    def show_game_over(self, timeout=False):
        self.game_over_label.text = 'TIMEOUT' if timeout else 'GAME OVER'
        self.game_over_label.draw()

    def draw_grid(self):
        self.grid.draw()

    def draw_border(self):
        draw_line((0, 0), (0, CONSTS.WINDOW_HEIGHT), self.border_color, self.border_width)
        draw_line((0, CONSTS.WINDOW_HEIGHT), (CONSTS.WINDOW_WIDTH, CONSTS.WINDOW_HEIGHT), self.border_color, self.border_width)
        draw_line((CONSTS.WINDOW_WIDTH, CONSTS.WINDOW_HEIGHT), (CONSTS.WINDOW_WIDTH, 0), self.border_color, self.border_width)
        draw_line((CONSTS.WINDOW_WIDTH, 0), (0, 0), self.border_color, self.border_width)

    def draw_leaderboard(self):
        draw_quadrilateral((CONSTS.WINDOW_WIDTH - self.leaderboard_width, CONSTS.WINDOW_HEIGHT - self.leaderboard_height,
                            CONSTS.WINDOW_WIDTH, CONSTS.WINDOW_HEIGHT - self.leaderboard_height,
                            CONSTS.WINDOW_WIDTH, CONSTS.WINDOW_HEIGHT,
                            CONSTS.WINDOW_WIDTH - self.leaderboard_width, CONSTS.WINDOW_HEIGHT),
                           self.leaderboard_color)
        for label in self.labels_buffer[:self.leaderboard_rows_count]:
            label.draw()
        self.reset_leaderboard()
