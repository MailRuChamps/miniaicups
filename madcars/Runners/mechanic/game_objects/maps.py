import math
import pymunk
from pymunk import Vec2d


class Map(object):
    external_id = 0

    base_arcs = []
    base_segments = []

    additional_arcs = []
    additional_segments = []

    segment_friction = 1
    segment_elasticity = 0
    segment_height = 10

    cars_start_position = []

    max_width = 1200
    max_height = 800

    def __init__(self, space):
        self.objects = []
        self.create_box(space)

        for fp, sp, h in self._get_segments_points():
            segment = pymunk.Segment(space.static_body, fp, sp, h)
            segment.friction = self.segment_friction
            segment.elasticity = self.segment_elasticity
            self.objects.append(segment)

    def create_box(self, space):
        bo = self.segment_height - 1  # box offset
        left = pymunk.Segment(space.static_body, (-bo, -bo), (-bo, self.max_height + bo), self.segment_height)
        left.sensor = True

        top = pymunk.Segment(space.static_body, (-bo, self.max_height + bo),
                             (self.max_width + bo, self.max_height + bo), 10)
        top.sensor = True

        right = pymunk.Segment(space.static_body, (self.max_width + bo, self.max_height + bo),
                               (self.max_width + bo, -bo), 10)
        right.sensor = True

        bottom = pymunk.Segment(space.static_body, (self.max_width + bo, -bo), (-bo, -bo), 10)
        bottom.sensor = True

        self.objects.extend([left, top, right, bottom])

    @classmethod
    def _get_segments_points(cls):
        points = []
        for fp, sp in cls.base_segments + cls.additional_segments:
            points.append([fp, sp, cls.segment_height])

        for c, r, a, b, sc in cls.base_arcs + cls.additional_arcs:
            rad_pre_seg = (b - a) / sc
            for i in range(sc):
                fpoint_rad = a + rad_pre_seg * i
                spoint_rad = a + rad_pre_seg * (i + 1)
                fpoint = Vec2d(c) + Vec2d(r * math.cos(fpoint_rad), r * math.sin(fpoint_rad))
                spoint = Vec2d(c) + Vec2d(r * math.cos(spoint_rad), r * math.sin(spoint_rad))
                points.append([tuple(fpoint), tuple(spoint), cls.segment_height])

        return points

    def get_objects_for_space(self):
        return self.objects

    def get_cars_start_position(self):
        return self.cars_start_position

    @classmethod
    def get_proto(cls):
        return {
            'external_id': cls.external_id,
            'segments': cls._get_segments_points()
        }


class PillMap(Map):
    external_id = 1

    base_arcs = [
        ((300, 400), 300, math.pi/2, math.pi * 3/2, 30),
        ((900, 400), 300, math.pi/2, -math.pi / 2,  30),
    ]

    base_segments = [
        ((300, 100), (900, 100)),
        ((300, 700), (900, 700))
    ]


class PillHubbleMap(PillMap):
    external_id = 2

    additional_arcs = [
        ((600, -150), 300, math.pi/3.2, math.pi/1.45, 30)
    ]


class PillHillMap(PillMap):
    external_id = 3

    additional_arcs = [
        ((300, 300), 200, -math.pi / 2, -math.pi / 6, 30),
        ((900, 300), 200, math.pi * 3 / 2, math.pi * 7 / 6, 30),
    ]

    additional_segments = [
        ((465, 195), (735, 195))
    ]


class PillCarcassMap(PillMap):
    external_id = 4

    additional_segments = [
        ((300, 400), (900, 400))
    ]


class IslandMap(Map):
    external_id = 5

    base_segments = [
        ((100, 100), (1100, 100)),
    ]


class IslandHoleMap(Map):
    external_id = 6

    base_segments = [
        ((10, 400), (50, 200)),
        ((50, 200), (300, 200)),

        ((380, 150), (820, 150)),
        ((900, 200), (1150, 200)),
        ((1150, 200), (1190, 400))
    ]

    base_arcs = [
        ((300, 100), 100, math.pi / 6, math.pi / 2, 30),
        ((900, 100), 100, math.pi / 2, math.pi * 5 / 6, 30),
    ]