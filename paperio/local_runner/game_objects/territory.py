from helpers import in_polygon, batch_draw_territory, get_neighboring, get_vert_and_horiz
from constants import WIDTH, LEFT, RIGHT, UP, DOWN
import networkx as nx


class Territory:
    def __init__(self, x, y, color):
        self.color = color
        self.points = {(x, y), *get_neighboring((x, y))}
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

    def get_nearest_boundary(self, point, boundary):
        for neighbor in [point, *get_neighboring(point)]:
            if neighbor in boundary:
                return neighbor

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
                    self.points.add((x, y))
                    captured.append((x, y))
                y -= WIDTH
            x -= WIDTH
        return captured

    def get_voids_between_lines_and_territory(self, lines):
        boundary = self.get_boundary()
        voids = []
        for cur in lines:
            for point in get_neighboring(cur):
                if point in boundary:
                    start_point = self.get_nearest_boundary(lines[0], boundary)
                    if start_point:
                        end_index = boundary.index(point)
                        start_index = boundary.index(start_point)

                        try:
                            path = self.get_path(start_index, end_index, boundary)
                        except (nx.NetworkXNoPath, nx.NodeNotFound):
                            continue

                        if len(path) > 1 and path[0] == path[-1]:
                            path = path[1:]

                        path = [boundary[index] for index in path]
                        lines_path = lines[:lines.index(cur) + 1]

                        voids.append(lines_path + path)
        return voids

    def capture_voids_between_lines(self, lines):
        captured = []
        for index, cur in enumerate(lines):
            for point in get_neighboring(cur):
                if point in lines:
                    end_index = lines.index(point)
                    path = lines[index:end_index]
                    if len(path) >= 8:
                        captured.extend(self._capture(path))
        return captured

    def capture(self, lines):
        captured = []
        if len(lines) > 1:
            if lines[-1] in self.points:
                voids = self.get_voids_between_lines_and_territory(lines)

                captured.extend(self.capture_voids_between_lines(lines))

                for line in lines:
                    if line not in self.points:
                        self.points.add(line)
                        captured.append(line)

                for void in voids:
                    captured.extend(self._capture(void))
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
        return [sibling for sibling in get_vert_and_horiz(point) if sibling in boundary]

    def get_path(self, start_index, end_index, boundary):
        graph = nx.Graph()
        for index, point in enumerate(boundary):
            siblings = self.get_siblings(point, boundary)
            for sibling in siblings:
                graph.add_edge(index, boundary.index(sibling), weight=1)

        return nx.shortest_path(graph, end_index, start_index, weight='weight')

    def split(self, line, direction, player):
        removed = []
        l_point = line[0]

        if any([point in self.points for point in line]):
            for point in list(self.points):
                if direction in [UP, DOWN]:
                    if player.x < l_point[0]:
                        if point[0] >= l_point[0]:
                            removed.append(point)
                            self.points.discard(point)
                    else:
                        if point[0] <= l_point[0]:
                            removed.append(point)
                            self.points.discard(point)

                if direction in [LEFT, RIGHT]:
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
