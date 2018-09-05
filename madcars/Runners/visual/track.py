import math
from drawer import Drawer

tracks_cache = {} # { track_id: [points] }

class Track(Drawer):
    def __init__(self, proto_map, color=0x000000, layer=1):
        Drawer.__init__(self, color, layer)
        global tracks_cache
        if proto_map["external_id"] not in tracks_cache:
            points = []
            for (start, end, width) in proto_map["segments"]:
                bigdx = end[0] - start[0]
                bigdy = end[1] - start[1]
                segment_len = math.sqrt((bigdx) ** 2 + (bigdy) ** 2)
                gamma = math.acos(bigdx / segment_len)
                dy = math.cos(gamma) * width
                dx = math.sin(gamma) * width
                points.append([
                        [start[0] - dx, start[1] - dy],
                        [end[0] - dx, end[1] - dy]
                    ])
                points.append([
                        [start[0] + dx, start[1] + dy],
                        [end[0] + dx, end[1] + dy]
                    ])
            tracks_cache[proto_map["external_id"]] = points
        self.points = tracks_cache[proto_map["external_id"]]

    def draw(self, rewind):
        for (start, end) in self.points:
            rewind.line(start[0], start[1], end[0], end[1], 0x0000FF, self.layer)
