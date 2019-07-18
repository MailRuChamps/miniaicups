import asyncio
import json
import random


async def tcp_echo_client(loop):
    reader, writer = await asyncio.open_connection('127.0.0.1', 8000, loop=loop)

    sid = random.randint(0, 10000)
    writer.write('{}\n'.format(json.dumps({"solution_id": sid})).encode())

    while True:
        data = await reader.readline()
        print(data.decode())

        commands = ['left', 'right', 'up', 'down']
        cmd = random.choice(commands)
        writer.write('{}\n'.format(json.dumps({"command": cmd, 'debug': cmd})).encode())
        await writer.drain()


loop = asyncio.get_event_loop()
loop.run_until_complete(tcp_echo_client(loop))
loop.close()
