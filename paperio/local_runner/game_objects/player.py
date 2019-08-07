from copy import copy
from game_objects.territory import Territory
from game_objects.bonuses import Saw
from constants import CONSTS
from helpers import batch_draw, draw_square


class Player:
    direction = None

    def __init__(self, id, x, y, name, color, client):
        self.id = id
        self.x = x
        self.y = y
        self.color = [i - 25 if i >= 25 else i for i in color[:-1]] + [color[-1]]
        self.line_color = list(color[:-1]) + [160]
        self.territory = Territory(x, y, color)
        self.lines = []
        self.bonuses = []
        self.name = name
        self.score = 0
        self.tick_score = 0

        self.debug_log = []
        self.client = client
        self.is_disconnected = False
        self.speed = CONSTS.SPEED

    def change_direction(self, command):
        if command == CONSTS.UP and self.direction != CONSTS.DOWN:
            self.direction = CONSTS.UP

        if command == CONSTS.DOWN and self.direction != CONSTS.UP:
            self.direction = CONSTS.DOWN

        if command == CONSTS.LEFT and self.direction != CONSTS.RIGHT:
            self.direction = CONSTS.LEFT

        if command == CONSTS.RIGHT and self.direction != CONSTS.LEFT:
            self.direction = CONSTS.RIGHT

    def move(self):
        if self.direction == CONSTS.UP:
            self.y += self.speed

        if self.direction == CONSTS.DOWN:
            self.y -= self.speed

        if self.direction == CONSTS.LEFT:
            self.x -= self.speed

        if self.direction == CONSTS.RIGHT:
            self.x += self.speed

    def draw_lines(self):
        batch_draw(self.lines, self.line_color)

    def draw_position(self):
        draw_square((self.x, self.y), self.color)

    def update_lines(self):
        if (self.x, self.y) not in self.territory.points or len(self.lines) > 0:
            self.lines.append((self.x, self.y))

    def send_message(self, t, d):
        if self.is_disconnected:
            return
        try:
            self.client.send_message(t, d)
        except Exception as e:
            print('write exception', self.client.get_solution_id(), e)
            self.is_disconnected = True
            self.client.close()

    def remove_saw_bonus(self):
        for bonus in self.bonuses[:]:
            if isinstance(bonus, Saw):
                bonus.cancel(self)
                self.bonuses.remove(bonus)

    def tick_action(self):
        for bonus in self.bonuses[:]:
            bonus.tick += 1

            if bonus.tick >= bonus.active_ticks:
                bonus.cancel(self)
                self.bonuses.remove(bonus)

    def get_bonuses_state(self):
        return [{'type': b.visio_name, 'ticks': b.get_remaining_ticks()} for b in self.bonuses]

    def get_state(self):
        return {
            'score': self.score,
            'direction': self.direction,
            'territory': list(self.territory.points),
            'lines': copy(self.lines),
            'position': (self.x, self.y),
            'bonuses': self.get_bonuses_state()
        }

    def get_state_for_event(self):
        return {
            'id': self.id,
            'direction': self.direction,
            'lines_length': len(self.lines),
            'position': (self.x, self.y),
        }

    async def get_command(self, tick):
        if self.is_disconnected:
            return

        try:
            client_answer = await self.client.get_command()
            if client_answer:
                debug_info = client_answer.get('debug')
                error_info = client_answer.get('error')
                if debug_info:
                    self.debug_log.append({
                        'tick': tick,
                        'message': debug_info[:200]
                    })

                if error_info:
                    self.debug_log.append({
                        'tick': tick,
                        'message': error_info[:200]
                    })

                return client_answer.get('command')
        except Exception as e:
            args = e.args
            if len(args) > 0:
                self.debug_log.append({'tick': tick, 'message': args[0]})
            else:
                self.debug_log.append({'tick': tick, 'message': str(e)})
            print('read exception', self.client.get_solution_id(), e)
            self.is_disconnected = True
            self.client.close()

    def save_log(self, path):
        return self.client.save_log_to_disk(self.debug_log, path)

    def _get_line(self, dx, dy):
        x, y = self.x, self.y
        points = []
        while 0 < x < CONSTS.WINDOW_WIDTH and 0 < y < CONSTS.WINDOW_HEIGHT:
            x += dx
            y += dy
            points.append((x, y))

        return points

    def get_direction_line(self):
        if self.direction == CONSTS.UP:
            return self._get_line(0, CONSTS.WIDTH)

        if self.direction == CONSTS.DOWN:
            return self._get_line(0, -CONSTS.WIDTH)

        if self.direction == CONSTS.LEFT:
            return self._get_line(-CONSTS.WIDTH, 0)

        if self.direction == CONSTS.RIGHT:
            return self._get_line(CONSTS.WIDTH, 0)

    def diff_position(self, direction, x, y, val):
        if direction == CONSTS.UP:
            return x, y - val

        if direction == CONSTS.DOWN:
            return x, y + val

        if direction == CONSTS.LEFT:
            return x + val, y

        if direction == CONSTS.RIGHT:
            return x - val, y

    def get_position(self):
        if self.direction is None:
            return self.x, self.y

        x, y = self.x, self.y
        while not ((x - round(CONSTS.WIDTH / 2)) % CONSTS.WIDTH == 0 and (y - round(CONSTS.WIDTH / 2)) % CONSTS.WIDTH == 0):
            x, y = self.diff_position(self.direction, x, y, self.speed)

        return (x, y), (x, y) != (self.x, self.y)

    def get_prev_position(self):
        if self.direction is None:
            return self.x, self.y
        return self.diff_position(self.direction, self.x, self.y, CONSTS.WIDTH)

    def is_ate(self, players_to_captured):
        for p, captured in players_to_captured.items():
            position, is_move = self.get_position()
            if self != p and position in captured and \
                    (is_move or self.get_prev_position() in captured):
                return True, p
        return False, None
