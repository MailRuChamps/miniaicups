from copy import copy
from game_objects.territory import Territory
from game_objects.bonuses import Saw
from constants import UP, DOWN, LEFT, RIGHT, SPEED, WINDOW_HEIGHT, WINDOW_WIDTH, WIDTH
from helpers import batch_draw, draw_square, is_cell_center, get_inversed_direction, get_incremented_pos_by_direction


class Player:
    speed = SPEED
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
        
    def change_direction(self, command):
        inversed_direction = get_inversed_direction(self.direction)

        if command != inversed_direction and command != None:
            self.direction = command

    def move(self):
        self.x, self.y = get_incremented_pos_by_direction(self.x, self.y, self.direction, self.speed)

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

    def on_center_bonuses_tick(self):
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
            'territory': list(sorted(self.territory.points)),
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

    def get_forward_center_position(self):
        if self.direction is None:
            return self.x, self.y
            
        if self.is_in_cell_center():
            return self.x, self.y
            
        x, y = self.x, self.y
        while not is_cell_center(x, y):
            x, y = get_incremented_pos_by_direction(x, y, self.direction, self.speed)

        return (x, y)

    def get_backward_center_position(self):
        if self.direction is None:
            return self.x, self.y
            
        inversed_direction = get_inversed_direction(self.direction)
        
        if self.is_in_cell_center():
            return get_incremented_pos_by_direction(self.x, self.y, inversed_direction, WIDTH)
            
        x, y = self.x, self.y
        while not is_cell_center(x, y):
            x, y = get_incremented_pos_by_direction(x, y, inversed_direction, self.speed)

        return (x, y)

    def is_head_pawned(self, players_to_captured):
        for p, captured in players_to_captured.items():
            if self != p:
                if self.is_in_cell_center():
                    if self.get_backward_center_position() in captured and self.get_forward_center_position() in captured:
                        return True, p
                else:
                    if self.get_backward_center_position() in captured:
                        return True, p
        return False, None
        
    def is_in_cell_center(self):
        return is_cell_center(self.x, self.y)
        
    def pos(self):
        return (self.x, self.y)
