import pyglet

from constants import WINDOW_HEIGHT, WINDOW_WIDTH, WIDTH
from helpers import draw_quadrilateral, draw_line


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
    game_over_label = pyglet.text.Label('GAME OVER', font_name='Times New Roman',
                                        font_size=30,
                                        color=game_over_label_color,
                                        x=WINDOW_WIDTH / 2, y=WINDOW_HEIGHT / 2,
                                        anchor_x='center', anchor_y='center')

    def __init__(self, scale):
        self.window = pyglet.window.Window(height=int(WINDOW_HEIGHT * scale / 100),
                                           width=int(WINDOW_WIDTH * scale / 100),
                                           resizable=True)
        pyglet.options['debug_gl'] = False
        pyglet.gl.glClearColor(*self.background_color)
        pyglet.gl.glEnable(pyglet.gl.GL_BLEND)
        pyglet.gl.glBlendFunc(pyglet.gl.GL_SRC_ALPHA, pyglet.gl.GL_ONE_MINUS_SRC_ALPHA)

    def clear(self):
        self.window.clear()

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
                                  x=WINDOW_WIDTH - self.leaderboard_width + 20,
                                  y=WINDOW_HEIGHT - 20 - WIDTH / 2 - 30 * self.leaderboard_rows_count,
                                  anchor_x='left', anchor_y='center')
            )
        self.leaderboard_rows_count += 1

    def reset_leaderboard(self):
        self.leaderboard_rows_count = 0

    def show_game_over(self, timeout=False):
        self.game_over_label.text = 'TIMEOUT' if timeout else 'GAME OVER'
        self.game_over_label.draw()

    def draw_grid(self):
        y = WIDTH
        while (y < WINDOW_HEIGHT):
            draw_line((0, y), (WINDOW_WIDTH, y), self.grid_color, 2)
            y += WIDTH

        x = WIDTH
        while (x < WINDOW_WIDTH):
            draw_line((x, 0), (x, WINDOW_HEIGHT), self.grid_color, 2)
            x += WIDTH

    def draw_border(self):
        draw_line((0, 0), (0, WINDOW_HEIGHT), self.border_color, self.border_width)
        draw_line((0, WINDOW_HEIGHT), (WINDOW_WIDTH, WINDOW_HEIGHT), self.border_color, self.border_width)
        draw_line((WINDOW_WIDTH, WINDOW_HEIGHT), (WINDOW_WIDTH, 0), self.border_color, self.border_width)
        draw_line((WINDOW_WIDTH, 0), (0, 0), self.border_color, self.border_width)

    def draw_leaderboard(self):
        draw_quadrilateral((WINDOW_WIDTH - self.leaderboard_width, WINDOW_HEIGHT - self.leaderboard_height,
                            WINDOW_WIDTH, WINDOW_HEIGHT - self.leaderboard_height,
                            WINDOW_WIDTH, WINDOW_HEIGHT,
                            WINDOW_WIDTH - self.leaderboard_width, WINDOW_HEIGHT),
                           self.leaderboard_color)
        for label in self.labels_buffer[:self.leaderboard_rows_count]:
            label.draw()
        self.reset_leaderboard()
