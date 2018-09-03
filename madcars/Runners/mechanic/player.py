import asyncio


class Player(object):
    def __init__(self, id, client, lives):
        self.client = client
        self.id = id
        self.car = None
        self.lives = lives
        self.debug_log = []
        self.is_disconnected = False

    def set_car(self, car):
        self.car = car

    @asyncio.coroutine
    def apply_turn(self, tick):
        if self.is_disconnected:
            return
        try:
            turn = yield from self.client.get_command()
            if turn:
                debug_info = turn.get('debug')
                error_info = turn.get('error')
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

                turn = turn.get('command')
                if turn == 'stop':
                    self.car.stop()
                elif turn == 'left':
                    self.car.go_left()
                elif turn == 'right':
                    self.car.go_right()
        except Exception as e:
            args = e.args
            if len(args) > 0:
                self.debug_log.append({'tick': tick, 'message': args[0]})
            else:
                self.debug_log.append({'tick': tick, 'message': str(e)})
            print('read exception', self.client.get_solution_id(), e)
            self.is_disconnected = True
            self.client.close()

    def send_message(self, t, d):
        if self.is_disconnected:
            return
        try:
            self.client.send_message(t, d)
        except Exception as e:
            print('write exception', self.client.get_solution_id(), e)
            self.is_disconnected = True
            self.client.close()

    def get_car(self):
        return self.car

    def get_game_id(self):
        return self.id

    def remove(self):
        self.client.close()

    def die(self):
        self.lives -= 1

    def is_alive(self):
        return self.lives > 0

    def save_log(self, path):
        return self.client.save_log_to_disk(self.debug_log, path)

    def get_solution_id(self):
        return self.client.get_solution_id()

    def get_lives(self):
        return self.lives

