import random

from helpers import draw_square_with_image, get_random_coordinates, batch_draw
from constants import WIDTH


class Bonus:
    image_path = None
    color = None
    name = None
    visio_name = None

    def __init__(self, point):
        x, y = point
        self.x = x
        self.y = y
        self.tick = 0
        self.active_ticks = self.generate_active_ticks()

    @staticmethod
    def generate_active_ticks():
        return random.choice([i * 10 for i in range(1, 6)])

    @staticmethod
    def is_available_point(x, y, players, busy_points):
        for p in players:
            if (p.x - 2 * WIDTH <= x <= p.x + 2 * WIDTH) and (p.y - 2 * WIDTH <= y <= p.y + 2 * WIDTH):
                return False
        return (x, y) not in busy_points

    @staticmethod
    def generate_coordinates(players, busy_points):
        x, y = get_random_coordinates()
        while not Bonus.is_available_point(x, y, players, busy_points):
            x, y = get_random_coordinates()
        return x, y

    def draw(self):
        draw_square_with_image((self.x, self.y), self.color, self.image_path, self.active_ticks)

    def is_ate(self, player, captured):
        return (self.x, self.y) == (player.x, player.y) or (self.x, self.y) in captured

    def get_remaining_ticks(self):
        return self.active_ticks - self.tick

    def cancel(self, player):
        pass

    def get_state(self):
        return {'type': self.visio_name, 'position': (self.x, self.y)}


class Nitro(Bonus):
    color = (255, 249, 221, 255)
    image_path = 'sprites/flash.png'
    name = 'Нитро'
    visio_name = 'n'

    def apply(self, player):
        b = [b for b in player.bonuses if type(b) == type(self)]
        if len(b) > 0:
            b[0].active_ticks += self.active_ticks
        else:
            player.bonuses.append(self)

            while player.speed < WIDTH:
                player.speed += 1
                if WIDTH % player.speed == 0:
                    break

    def cancel(self, player):
        while player.speed > 1:
            player.speed -= 1
            if WIDTH % player.speed == 0:
                break


class Slowdown(Bonus):
    color = (234, 249, 255, 255)
    image_path = 'sprites/explorer.png'
    name = 'Замедление'
    visio_name = 's'

    def apply(self, player):
        b = [b for b in player.bonuses if type(b) == type(self)]
        if len(b) > 0:
            b[0].active_ticks += self.active_ticks
        else:
            player.bonuses.append(self)

            while player.speed > 1:
                player.speed -= 1
                if WIDTH % player.speed == 0:
                    break

    def cancel(self, player):
        while player.speed < WIDTH:
            player.speed += 1
            if WIDTH % player.speed == 0:
                break


class Saw(Bonus):
    color = (226, 228, 226, 255)
    image_path = 'sprites/saw.png'
    name = 'Пила'
    visio_name = 'saw'
    lines = []
    opacity_step = 10
    line_color = (189, 236, 246)
    territories = []
    log = []

    def apply(self, player):
        self.active_ticks = 0
        player.bonuses.append(self)

    def cancel(self, player):
        pass

    @staticmethod
    def append_line(line):
        Saw.lines.append([255, line])

    @staticmethod
    def append_territory(territory, color):
        Saw.territories.append([color, territory])

    @staticmethod
    def draw_territories():
        for pair in Saw.territories[:]:
            pair[0] = [*pair[0][:3], pair[0][3] - Saw.opacity_step]
            if pair[0][3] <= 0:
                Saw.territories.remove(pair)
            else:
                batch_draw(pair[1], pair[0])

    @staticmethod
    def draw_lines():
        for pair in Saw.lines[:]:
            pair[0] -= Saw.opacity_step
            if pair[0] <= 0:
                Saw.lines.remove(pair)
            else:
                batch_draw(pair[1], (*Saw.line_color, pair[0]))
