import os
import asyncio
import datetime
import gzip
import json
import random
from subprocess import Popen, PIPE

import pyglet
from constants import LEFT, RIGHT, UP, DOWN, MAX_EXECUTION_TIME, REQUEST_MAX_TIME


class Client(object):
    def get_command(self):
        pass

    def close(self):
        pass

    def send_message(self, t, d):
        pass

    def save_log_to_disk(self, log, path):
        pass

    def get_solution_id(self):
        return random.randint(11000, 12000)


class KeyboardClient(Client):
    @property
    def KEY_COMMAND_MAP(self):
        return {
            pyglet.window.key.MOTION_LEFT: LEFT,
            pyglet.window.key.MOTION_RIGHT: RIGHT,
            pyglet.window.key.MOTION_DOWN: DOWN,
            pyglet.window.key.MOTION_UP: UP,
        }

    def __init__(self, window):
        self.last_pressed_button = pyglet.window.key.MOTION_LEFT

        @window.event
        def on_key_press(symbol, _):
            self.last_pressed_button = symbol

    async def get_command(self):
        return {'command': self.KEY_COMMAND_MAP.get(self.last_pressed_button, None)}

    def save_log_to_disk(self, log, path):
        pass


class KeyboardClient2(KeyboardClient):
    @property
    def KEY_COMMAND_MAP(self):
        return {
            pyglet.window.key.A: LEFT,
            pyglet.window.key.D: RIGHT,
            pyglet.window.key.S: DOWN,
            pyglet.window.key.W: UP,
        }

    def __init__(self, window):
        self.last_pressed_button = pyglet.window.key.A

        @window.event
        def on_key_release(symbol, _):
            self.last_pressed_button = symbol


class SimplePythonClient(Client):
    def __init__(self):
        self.command = None
        self.tick = 0
        self.next_change = None
        self.next_dir = 0

        self.width = None
        self.x_cells_count = None
        self.y_cells_count = None
        self.lines = []
        self.position = None

    def change_command(self):
        commands = [LEFT, DOWN, RIGHT, UP]
        command = commands[self.next_dir % 4]
        self.next_dir += 1
        self.command = command

    def get_next_point(self):
        x, y = self.position

        if self.command == UP:
            return x, y + self.width

        if self.command == DOWN:
            return x, y - self.width

        if self.command == LEFT:
            return x - self.width, y

        if self.command == RIGHT:
            return x + self.width, y

    def is_border(self, point):
        x, y = point
        return x < round(self.width / 2) or \
               x > self.x_cells_count * self.width + round(self.width / 2) or \
               y < round(self.width / 2) or \
               y > self.y_cells_count * self.width + round(self.width / 2)

    def is_empty_next_point(self):
        if not self.position:
            return True
        next_point = self.get_next_point()
        return next_point not in self.lines and not self.is_border(next_point)

    async def get_command(self):
        if not self.next_change or self.next_change == 0 or not self.is_empty_next_point():
            self.next_change = random.randint(1, 4)
            self.change_command()
            attempts = 0
            while not self.is_empty_next_point() and attempts < 3:
                self.change_command()
                attempts += 1

        self.tick += 1
        self.next_change -= 1
        return {'command': self.command}

    def save_log_to_disk(self, log, path):
        pass

    def send_message(self, t, d):
        if t == 'start_game':
            self.width = d['width']
            self.x_cells_count = d['x_cells_count']
            self.y_cells_count = d['y_cells_count']

        if t == 'tick':
            p_data = d['players']['i']
            self.lines = p_data['lines']
            self.position = p_data['position']

        if t == 'end_game':
            pass


class TcpClient(Client):
    EXECUTION_LIMIT = datetime.timedelta(seconds=MAX_EXECUTION_TIME)

    def __init__(self, reader, writer):
        self.reader = reader
        self.writer = writer
        self.execution_time = datetime.timedelta()
        self.solution_id = None

    def save_log_to_disk(self, log, path):
        location = path.format(str(self.solution_id) + '.gz')

        with gzip.open(location, 'wb') as f:
            f.write(json.dumps(log).encode())

        return {
            'filename': os.path.basename(location),
            'is_private': True,
            'location': location
        }

    async def set_solution_id(self):
        hello_json = await asyncio.wait_for(self.reader.readline(), timeout=REQUEST_MAX_TIME)
        try:
            self.solution_id = json.loads(hello_json.decode('utf-8')).get('solution_id')
        except ValueError:
            pass

        return bool(self.solution_id)

    def send_message(self, t, d):
        msg = {
            'type': t,
            'params': d
        }
        msg_bytes = '{}\n'.format(json.dumps(msg)).encode()
        self.writer.write(msg_bytes)

    async def get_command(self):
        try:
            before = datetime.datetime.now()
            z = await asyncio.wait_for(self.reader.readline(), timeout=REQUEST_MAX_TIME)
            if not z:
                raise ConnectionError('Connection closed')
            self.execution_time += (datetime.datetime.now() - before)
            if self.execution_time > self.EXECUTION_LIMIT:
                raise Exception('sum timeout error')
        except asyncio.TimeoutError:
            raise asyncio.TimeoutError('read timeout error')
        try:
            z = json.loads(z.decode())
        except ValueError:
            z = {'debug': 'cant pars json'}

        return z

    def close(self):
        self.writer.close()

    def get_solution_id(self):
        return self.solution_id


class FileClient(Client):
    def __init__(self, path_to_script, path_to_log=None):
        self.process = Popen(path_to_script, stdout=PIPE, stdin=PIPE)
        self.last_message = None
        if path_to_log is None:
            base_dir = os.getcwd()
            now = datetime.datetime.now().strftime('%Y_%m_%d-%H-%M-%S.log.gz')
            self.path_to_log = os.path.join(base_dir, now)
        else:
            self.path_to_log = path_to_log

    def send_message(self, t, d):
        msg = {
            'type': t,
            'params': d
        }
        msg_bytes = '{}\n'.format(json.dumps(msg)).encode()

        self.process.stdin.write(msg_bytes)
        self.process.stdin.flush()

    async def get_command(self):
        try:
            line = self.process.stdout.readline().decode('utf-8')
            state = json.loads(line)
            return state
        except Exception as e:
            return {'debug': str(e)}

    def save_log_to_disk(self, log, _):
        with gzip.open(self.path_to_log, 'w') as f:
            f.write(json.dumps(log).encode())

        return {
            'filename': os.path.basename(self.path_to_log),
            'is_private': True,
            'location': self.path_to_log
        }
