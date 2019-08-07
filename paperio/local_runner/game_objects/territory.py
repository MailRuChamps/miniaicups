from helpers import in_polygon, batch_draw_territory, get_neighboring, get_vert_and_horiz
from constants import CONSTS
import networkx as nx


class Territory:
    def __init__(self, x, y, color):
        self.color = color
        self.points = {(x, y), *get_neighboring((x, y))}
        # fro scripted_coll_8
        # self.points = {(x, y), *get_neighboring((x, y)), (x - WIDTH, y - 3 * WIDTH), (x, y - 4 * WIDTH), (x + WIDTH, y - 3 * WIDTH)}
        self.changed = True

    def draw(self):
        batch_draw_territory(self.points, self.color, self.changed)
        self.changed = False

    def get_boundary(self):
        boundary = []
        for point in self.points:
            if any([neighboring not in self.points for neighboring in get_neighboring(point)]):
                boundary.append(point)
        return boundary

    def _get_start_points(self, point, boundary):
        res = []
        for neighbor in [point, *get_neighboring(point)]:
            if neighbor in boundary:
                res.append(neighbor)
        return res

    def _capture(self, boundary):
        poligon_x_arr = [x for x, _ in boundary]
        poligon_y_arr = [y for _, y in boundary]

        max_x = max(poligon_x_arr)
        max_y = max(poligon_y_arr)
        min_x = min(poligon_x_arr)
        min_y = min(poligon_y_arr)

        captured = []
        x = max_x
        while x > min_x:
            y = max_y
            while y > min_y:
                if (x, y) not in self.points and in_polygon(x, y, poligon_x_arr, poligon_y_arr):
                    captured.append((x, y))
                y -= CONSTS.WIDTH
            x -= CONSTS.WIDTH
        return captured

    def is_siblings(self, p1, p2):
        return p2 in get_vert_and_horiz(p1)

    def get_voids_between_lines_and_territory(self, lines):
        boundary = self.get_boundary()
        graph = self.get_graph(boundary)
        voids = []
        for i_lp1, lp1 in enumerate(lines):
            for point in get_neighboring(lp1):
                if point in boundary:
                    prev = None
                    for lp2 in lines[:i_lp1 + 1]:
                        start_points = self._get_start_points(lp2, boundary)
                        for start_point in start_points:
                            if prev and (self.is_siblings(prev, start_point) or prev == start_point):
                                prev = start_point
                                continue
                            end_index = boundary.index(point)
                            start_index = boundary.index(start_point)

                            try:
                                path = nx.shortest_path(graph, end_index, start_index, weight='weight')
                            except (nx.NetworkXNoPath, nx.NodeNotFound):
                                continue

                            if len(path) > 1 and path[0] == path[-1]:
                                path = path[1:]

                            path = [boundary[index] for index in path]
                            lines_path = lines[lines.index(lp2):i_lp1 + 1]

                            voids.append(lines_path + path)
                            prev = start_point
        return voids

    def capture_voids_between_lines(self, lines):
        captured = []
        for index, cur in enumerate(lines):
            for point in get_neighboring(cur):
                if point in lines:
                    end_index = lines.index(point)
                    path = lines[index:end_index + 1]
                    if len(path) >= 8:
                        captured.extend(self._capture(path))
        return captured

    def capture(self, lines):
        captured = set()
        if len(lines) > 1:
            if lines[-1] in self.points:
                voids = self.get_voids_between_lines_and_territory(lines)

                captured.update(self.capture_voids_between_lines(lines))

                for line in lines:
                    if line not in self.points:
                        captured.add(line)

                for void in voids:
                    captured.update(self._capture(void))
        if len(captured) > 0:
            self.changed = True
        return captured

    def remove_points(self, points):
        removed = []
        for point in points:
            if point in self.points:
                self.points.discard(point)
                removed.append(point)

        if len(removed) > 0:
            self.changed = True
        return removed

    def get_siblings(self, point, boundary):
        return [sibling for sibling in get_neighboring(point) if sibling in boundary]

    def get_graph(self, boundary):
        graph = nx.Graph()
        for index, point in enumerate(boundary):
            siblings = self.get_siblings(point, boundary)
            for sibling in siblings:
                graph.add_edge(index, boundary.index(sibling), weight=1)
        return graph

    def split(self, line, direction, player):
        removed = []
        l_point = line[0]

        if any([point in self.points for point in line]):
            for point in list(self.points):
                if direction in [CONSTS.UP, CONSTS.DOWN]:
                    if player.x < l_point[0]:
                        if point[0] >= l_point[0]:
                            removed.append(point)
                            self.points.discard(point)
                    else:
                        if point[0] <= l_point[0]:
                            removed.append(point)
                            self.points.discard(point)

                if direction in [CONSTS.LEFT, CONSTS.RIGHT]:
                    if player.y < l_point[1]:
                        if point[1] >= l_point[1]:
                            removed.append(point)
                            self.points.discard(point)
                    else:
                        if point[1] <= l_point[1]:
                            removed.append(point)
                            self.points.discard(point)

        if len(removed) > 0:
            self.changed = True
        return removed
