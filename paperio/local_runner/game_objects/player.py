from copy import copy
from game_objects.territory import Territory
from game_objects.bonuses import Saw
from constants import UP, DOWN, LEFT, RIGHT, SPEED, WINDOW_HEIGHT, WINDOW_WIDTH, WIDTH
from helpers import batch_draw, draw_square


class Player:
    speed = SPEED
    direction = LEFT

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

        self.debug_log = []
        self.client = client
        self.is_disconnected = False

    def change_direction(self, command):
        if command == UP and self.direction != DOWN:
            self.direction = UP

        if command == DOWN and self.direction != UP:
            self.direction = DOWN

        if command == LEFT and self.direction != RIGHT:
            self.direction = LEFT

        if command == RIGHT and self.direction != LEFT:
            self.direction = RIGHT

    def move(self):
        if self.direction == UP:
            self.y += self.speed

        if self.direction == DOWN:
            self.y -= self.speed

        if self.direction == LEFT:
            self.x -= self.speed

        if self.direction == RIGHT:
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
        while 0 < x < WINDOW_WIDTH and 0 < y < WINDOW_HEIGHT:
            x += dx
            y += dy
            points.append((x, y))

        return points

    def get_direction_line(self):
        if self.direction == UP:
            return self._get_line(0, WIDTH)

        if self.direction == DOWN:
            return self._get_line(0, -WIDTH)

        if self.direction == LEFT:
            return self._get_line(-WIDTH, 0)

        if self.direction == RIGHT:
            return self._get_line(WIDTH, 0)
