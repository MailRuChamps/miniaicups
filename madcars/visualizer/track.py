from pymunk.vec2d import Vec2d

from drawer import Drawer

tracks_cache = {} # { track_id: [Segment] }

class Segment(Drawer):
    def __init__(self, start, end, width, color=0x000000, layer=1):
        Drawer.__init__(self, color, layer)
        self.width = width
        self.start, self.end = start, end
        self.vector = Vec2d(end[0] - start[0], end[1] - start[1])
        self.normal = self.vector.perpendicular_normal()

    def draw(self, rewind):
        dx = self.normal.x * self.width
        dy = self.normal.y * self.width

        rewind.line(self.start[0] + dx, self.start[1] + dy,
            self.end[0] + dx, self.end[1] + dy,
            color=self.color, layer=self.layer)
        rewind.line(self.start[0] - dx, self.start[1] - dy,
            self.end[0] - dx, self.end[1] - dy,
            color=self.color, layer=self.layer)

class Track(Drawer):
    def __init__(self, proto_map, color=0x000000, layer=1):
        Drawer.__init__(self, color, layer)
        global tracks_cache
        if proto_map["external_id"] not in tracks_cache:
            segments = []
            for (start, end, width) in proto_map["segments"]:
                segments.append(Segment(start, end, width))
            tracks_cache[proto_map["external_id"]] = segments
        self.objects = tracks_cache[proto_map["external_id"]]

    def draw(self, rewind):
        Drawer.draw(self, rewind)
