import os
import json
import gzip
import random

from helpers import is_intersect
from constants import WIDTH, WINDOW_HEIGHT, WINDOW_WIDTH, PLAYER_COLORS, MAX_TICK_COUNT, BONUS_CHANCE, \
    BONUSES_MAX_COUNT, X_CELLS_COUNT, Y_CELLS_COUNT, SPEED, NEUTRAL_TERRITORY_SCORE, ENEMY_TERRITORY_SCORE, \
    LINE_KILL_SCORE, SAW_KILL_SCORE, AVAILABLE_BONUSES, SAW_SCORE
from game_objects.player import Player
from game_objects.bonuses import Nitro, Slowdown, Bonus, Saw


class Game:
    border_color = (144, 163, 174, 255)
    available_bonuses = [b for b in [Nitro, Slowdown, Saw] if b.visio_name in AVAILABLE_BONUSES]

    RESULT_LOCATION = os.environ.get('GAME_LOG_LOCATION', './result')

    BASE_DIR = os.path.dirname(RESULT_LOCATION)

    VISIO_LOCATION = os.path.join(BASE_DIR, 'visio.gz')
    SCORES_LOCATION = os.path.join(BASE_DIR, 'scores.json')
    DEBUG_LOCATION = os.path.join(BASE_DIR, '{}')

    def get_busy_points(self):
        players_points = {(p.x, p.y) for p in self.players}
        bonuses_points = {(b.x, b.y) for b in self.bonuses}
        lines_poins = set()
        for player in self.players:
            lines_poins |= {i for i in player.lines}

        return players_points | bonuses_points | lines_poins

    def generate_bonus(self):
        if len(self.available_bonuses) > 0:
            if random.randint(1, BONUS_CHANCE) == 1 and len(self.bonuses) < BONUSES_MAX_COUNT:
                coors = Bonus.generate_coordinates(self.players, self.get_busy_points())
                bonus = random.choice(self.available_bonuses)(coors)
                self.bonuses.append(bonus)

    def get_coordinates(self, clients_count):
        dx = round(X_CELLS_COUNT / 6) * WIDTH
        dy = round(Y_CELLS_COUNT / 6) * WIDTH

        if clients_count == 1:
            coors = [(3 * dx, 3 * dy)]
        elif clients_count == 2:
            coors = [
                (2 * dx, 3 * dy),
                (4 * dx, 3 * dy),
            ]
        elif clients_count <= 4:
            coors = [
                (2 * dx, 2 * dy),
                (2 * dx, 4 * dy),
                (4 * dx, 2 * dy),
                (4 * dx, 4 * dy),
            ]
        else:
            x = round(X_CELLS_COUNT / 5) * WIDTH
            y = (WINDOW_HEIGHT + WINDOW_WIDTH - 4 * x) / 3
            b = (WINDOW_WIDTH - 2 * x) / 2
            a = y - b

            coors = [
                (x, x + a),
                (x, x + a + y  + WIDTH),

                (round(WINDOW_WIDTH / 2), WINDOW_HEIGHT - x + WIDTH),
                (round(WINDOW_WIDTH / 2), x),

                (WINDOW_WIDTH - x + WIDTH, x + a),
                (WINDOW_WIDTH - x + WIDTH, x + a + y  + WIDTH),
            ]

        coors = [(round(x / WIDTH) * WIDTH - round(WIDTH / 2),  round(y / WIDTH) * WIDTH  - round(WIDTH / 2))  for x, y in coors]
        yield from coors

    def __init__(self, clients):
        players = []
        coordinates = self.get_coordinates(len(clients))
        for index, client in enumerate(clients):
            players.append(Player(index + 1, *next(coordinates), 'Player {}'.format(index + 1), PLAYER_COLORS[index], client))

        self.players = players
        self.losers = []
        self.bonuses = []
        self.game_log = []
        self.tick = 1

    def check_loss(self, player, players):
        is_loss = False

        if player.y < 0 + round(WIDTH / 2):
            is_loss = True

        if player.y > WINDOW_HEIGHT - round(WIDTH / 2):
            is_loss = True

        if player.x < 0 + round(WIDTH / 2):
            is_loss = True

        if player.x > WINDOW_WIDTH - round(WIDTH / 2):
            is_loss = True

        for p in players:
            if (p.x, p.y) in player.lines:
                if p != player:
                    p.score += LINE_KILL_SCORE
                is_loss = True

        for p in players:
            if is_intersect((player.x, player.y), (p.x, p.y)) and p != player:
                if len(player.lines) >= len(p.lines):
                    is_loss = True

        if len(player.territory.points) == 0:
            is_loss = True

        return is_loss

    def send_game_start(self):
        start_message = {
            'x_cells_count': X_CELLS_COUNT,
            'y_cells_count': Y_CELLS_COUNT,
            'speed': SPEED,
            'width': WIDTH
        }
        self.game_log.append({'type': 'start_game', **start_message})
        for player in self.players:
            player.send_message('start_game', start_message)

    def send_game_end(self):
        self.game_log.append({
            'type': 'end_game'
        })
        for player in self.players:
            player.send_message('end_game', {})

    def send_game_tick(self):
        self.game_log.append({
            'type': 'tick',
            'players': self.get_players_states(),
            'bonuses': self.get_bonuses_states(),
            'tick_num': self.tick,
            'saw': Saw.log
        })

        for player in self.players:
            if (player.x - round(WIDTH / 2)) % WIDTH == 0 and (player.y - round(WIDTH / 2)) % WIDTH == 0:
                player.send_message('tick', {
                    'players': self.get_players_states(player),
                    'bonuses': self.get_bonuses_states(),
                    'tick_num': self.tick,
                })

        Saw.log = []

    async def game_loop_wrapper(self, *args, **kwargs):
        self.send_game_start()
        while True:
            is_game_over = await self.game_loop(*args, **kwargs)
            print('tick: {}'.format(self.tick))
            if is_game_over or self.tick >= MAX_TICK_COUNT:
                self.send_game_end()
                self.game_save()
                break

    def get_players_states(self, player=None):
        states = {p.id: p.get_state() for p in self.players}

        if player:
            states['i'] = states.pop(player.id)

        return states

    def get_bonuses_states(self):
        return [b.get_state() for b in self.bonuses]

    async def game_loop(self, *args, **kwargs):
        self.send_game_tick()

        for player in self.players:
            if (player.x - round(WIDTH / 2)) % WIDTH == 0 and (player.y - round(WIDTH / 2)) % WIDTH == 0:
                command = await player.get_command(self.tick)
                if command:
                    player.change_direction(command)

        for player in self.players:
            player.move()

        for index, player in enumerate(self.players):
            is_loss = self.check_loss(player, self.players)
            if is_loss:
                self.losers.append(self.players[index])

        for player in self.players:
            player.remove_saw_bonus()

            if (player.x - round(WIDTH / 2)) % WIDTH == 0 and (player.y - round(WIDTH / 2)) % WIDTH == 0:
                player.update_lines()

                captured = player.territory.capture(player.lines)
                if len(captured) > 0:
                    player.lines.clear()
                    player.score += NEUTRAL_TERRITORY_SCORE * len(captured)

                player.tick_action()

                for bonus in self.bonuses[:]:
                    if bonus.is_ate(player, captured):
                        bonus.apply(player)
                        self.bonuses.remove(bonus)

                        if isinstance(bonus, Saw):
                            line = player.get_direction_line()
                            Saw.append_line(line)
                            for p in self.players:
                                if p != player:
                                    if any([is_intersect((p.x, p.y), point) for point in line]):
                                        self.losers.append(p)
                                        Saw.log.append({
                                            'player': player.id,
                                            'loser': p.id,
                                            'killed': True
                                        })
                                        player.score += SAW_KILL_SCORE
                                    else:
                                        removed = p.territory.split(line, player.direction, p)
                                        if len(removed) > 0:
                                            player.score += SAW_SCORE
                                            Saw.append_territory(removed, p.territory.color)
                                            Saw.log.append({
                                                'player': player.id,
                                                'loser': p.id,
                                                'points': removed,
                                                'killed': False
                                            })
                for p in self.players:
                    if p != player:
                        removed = p.territory.remove_points(captured)
                        player.score += (ENEMY_TERRITORY_SCORE - NEUTRAL_TERRITORY_SCORE) * len(removed)

        for player in self.losers:
            if player in self.players:
                self.players.remove(player)

        self.generate_bonus()

        self.tick += 1
        return len(self.players) == 0

    def save_scores(self):
        d = {p.client.get_solution_id(): p.score for p in self.losers}

        with open(self.SCORES_LOCATION, 'w') as f:
            f.write(json.dumps(d))

        return {
            "filename": os.path.basename(self.SCORES_LOCATION),
            "location": self.SCORES_LOCATION,
            "is_private": False
        }

    def get_players_external_id(self):
        return {p.id: p.client.get_solution_id() for p in self.losers + self.players}

    def save_visio(self):
        d = {
            'config': self.get_players_external_id(),
            'visio_info': self.game_log
        }
        with gzip.open(self.VISIO_LOCATION, 'wb') as f:
            f.write(json.dumps(d).encode())
        return {
            "filename": os.path.basename(self.VISIO_LOCATION),
            "location": self.VISIO_LOCATION,
            "is_private": False
        }

    def save_debug(self):
        return [
            p.save_log(self.DEBUG_LOCATION) for p in self.losers
        ]

    def game_save(self):
        result = {
            "scores": self.save_scores(),
            "debug": self.save_debug(),
            "visio": self.save_visio()
        }

        with open(self.RESULT_LOCATION, 'w') as f:
            f.write(json.dumps(result))


class LocalGame(Game):
    border_color = (144, 163, 174, 255)

    def __init__(self, clients, scene, timeout):
        super().__init__(clients)
        self.scene = scene
        self.timeout = timeout

    def show_bonuses(self):
        for player in self.players:
            if len(player.bonuses) > 0:
                for bonus in player.bonuses:
                    label = '{} - {} - {}'.format(player.name, bonus.name, bonus.get_remaining_ticks())
                    self.scene.append_label_to_leaderboard(label, player.color)

    def show_losers(self):
        for player in self.losers:
            label = '{} выбыл, результат: {}'.format(player.name, player.score)
            self.scene.append_label_to_leaderboard(label, player.color)

    def show_score(self):
        for player in self.players:
            label = '{} результат: {}'.format(player.name, player.score)
            self.scene.append_label_to_leaderboard(label, player.color)

    def draw_bonuses(self):
        for bonus in self.bonuses:
            bonus.draw()

    def draw(self):
        for player in self.players:
            player.territory.draw()

        Saw.draw_lines()
        Saw.draw_territories()

        for player in self.players:
            player.draw_lines()

        for player in self.players:
            player.draw_position()

        if len(self.players) == 0:
            self.scene.show_game_over()
        elif self.timeout and self.tick >= MAX_TICK_COUNT:
            self.scene.show_game_over(timeout=True)

        self.draw_bonuses()

        self.scene.draw_leaderboard()
        self.show_losers()
        self.show_score()
        self.show_bonuses()
        self.scene.reset_leaderboard()

    async def game_loop(self, *args, **kwargs):
        self.scene.clear()
        self.draw()
        return await super().game_loop(*args, **kwargs)
