import random
import asyncio

from clients import TcpClient
from game_objects.game import Game
from constants import CONSTS


class GameServer:
    def __init__(self):
        self.clients = []

    async def connection_handler(self, client_reader, client_writer):
        client = TcpClient(client_reader, client_writer)
        is_success = await client.set_solution_id()

        clients_count = len(self.clients)

        if clients_count < CONSTS.CLIENTS_COUNT:
            if is_success:
                clients_count += 1
                print('{} clients connected'.format(clients_count))
                self.clients.append(client)
            else:
                loop.stop()
        else:
            client_writer.close()

        game_future = None
        if clients_count == CONSTS.CLIENTS_COUNT:
            random.shuffle(self.clients)
            game = Game(self.clients)
            game_future = asyncio.ensure_future(game.game_loop_wrapper())

        if game_future:
            done, pending = await asyncio.wait([game_future])
            if not pending:
                loop.stop()

            print('game done')


gs = GameServer()

loop = asyncio.get_event_loop()
loop.run_until_complete(asyncio.start_server(gs.connection_handler, '0.0.0.0', 8000))
try:
    loop.run_forever()
finally:
    loop.close()
