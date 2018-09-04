from itertools import chain

import math
import pymunk
from pymunk import Vec2d


class Car(object):
    RIGHT_DIRECTION = 0
    LEFT_DIRECTION = 1

    external_id = 0

    FF = 1
    FR = 2
    AWD = 3

    car_body_poly = []
    car_body_mass = 100
    car_body_friction = 0.9
    car_body_elasticity = 0.5

    button_hw = (3, 30)
    button_position = (0, 0)
    button_angle = 0

    max_speed = 300
    max_angular_speed = 2
    torque = 20000000
    drive = FR

    rear_wheel_mass = 60
    rear_wheel_radius = 10
    rear_wheel_position = (0, 0)
    rear_wheel_friction = 1
    rear_wheel_elasticity = 0.8
    rear_wheel_joint = (0, 0)
    rear_wheel_damp_position = (0, 0)
    rear_wheel_damp_length = 20
    rear_wheel_damp_stiffness = 6e4
    rear_wheel_damp_damping = 1e3

    front_wheel_mass = 60
    front_wheel_radius = 10
    front_wheel_position = (0, 0)
    front_wheel_friction = 1
    front_wheel_elasticity = 0.8
    front_wheel_joint = (0, 0)
    front_wheel_damp_position = (0, 0)
    front_wheel_damp_length = 20
    front_wheel_damp_stiffness = 6e4
    front_wheel_damp_damping = 0.9e3

    def __init__(self, car_group, direction, point_query_nearest):
        self.car_group = car_group
        self.button_collision_type = car_group * 10
        self.x_modification = 1 if direction == self.RIGHT_DIRECTION else -1

        self.car_body = self.create_car_body()
        self.car_shape = self.create_car_shape()
        self.button_shape = self.create_button_shape()

        self.car_body.center_of_gravity = Vec2d(self.car_shape.center_of_gravity)

        self.rear_wheel_body, self.rear_wheel_motor, self.rear_wheel_objects = self.create_wheel('rear')
        self.front_wheel_body, self.front_wheel_motor, self.front_wheel_objects = self.create_wheel('front')

        self.motors = []

        if self.rear_wheel_motor:
            self.motors.append(self.rear_wheel_motor)
        if self.front_wheel_motor:
            self.motors.append(self.front_wheel_motor)

        self.point_query_nearest = point_query_nearest

    def create_wheel(self, wheel_side):
        if wheel_side not in ['rear', 'front']:
            raise Exception('Wheel position must be front or rear')
        wheel_objects = []

        wheel_mass = getattr(self, wheel_side + '_wheel_mass')
        wheel_radius = getattr(self, wheel_side + '_wheel_radius')
        wheel_position = getattr(self, wheel_side + '_wheel_position')
        wheel_friction = getattr(self, wheel_side + '_wheel_friction')
        wheel_elasticity = getattr(self, wheel_side + '_wheel_elasticity')
        wheel_damp_position = getattr(self, wheel_side + '_wheel_damp_position')
        wheel_damp_length = getattr(self, wheel_side + '_wheel_damp_length')
        wheel_damp_stiffness = getattr(self, wheel_side + '_wheel_damp_stiffness')
        wheel_damp_damping = getattr(self, wheel_side + '_wheel_damp_damping')

        wheel_body = pymunk.Body(wheel_mass, pymunk.moment_for_circle(wheel_mass, 0, wheel_radius))
        wheel_body.position = (wheel_position[0] * self.x_modification, wheel_position[1])

        wheel_shape = pymunk.Circle(wheel_body, wheel_radius)
        wheel_shape.filter = pymunk.ShapeFilter(group=self.car_group)
        wheel_shape.color = 255, 34, 150
        wheel_shape.friction = wheel_friction
        wheel_shape.elasticity = wheel_elasticity
        wheel_objects.append(wheel_shape)

        wheel_grove = pymunk.GrooveJoint(self.car_body, wheel_body,
                                         (wheel_damp_position[0] * self.x_modification, wheel_damp_position[1]),
                                         (wheel_damp_position[0] * self.x_modification,
                                          wheel_damp_position[1] - wheel_damp_length * 1.5),
                                         (0, 0))
        wheel_objects.append(wheel_grove)

        wheel_damp = pymunk.DampedSpring(wheel_body, self.car_body, anchor_a=(0, 0),
                                         anchor_b=(wheel_damp_position[0] * self.x_modification, wheel_damp_position[1]),
                                         rest_length=wheel_damp_length,
                                         stiffness=wheel_damp_stiffness,
                                         damping=wheel_damp_damping)
        wheel_objects.append(wheel_damp)

        wheel_motor = None
        if (wheel_side == 'rear' and self.drive in [self.AWD, self.FR]) or (wheel_side == 'front' and self.drive in [self.AWD, self.FF]):
            wheel_motor = pymunk.SimpleMotor(wheel_body, self.car_body, 0)

        return wheel_body, wheel_motor, wheel_objects

    def processed_car_body_poly(self):
        return [(x[0] * self.x_modification, x[1]) for x in self.car_body_poly]

    def create_car_body(self):
        return pymunk.Body(self.car_body_mass, pymunk.moment_for_poly(self.car_body_mass, self.processed_car_body_poly()))

    def create_car_shape(self):
        if not self.car_body:
            raise Exception('Create car body before')

        car_shape = pymunk.Poly(self.car_body, self.processed_car_body_poly())
        car_shape.friction = self.car_body_friction
        car_shape.elasticity = self.car_body_elasticity
        car_shape.filter = pymunk.ShapeFilter(group=self.car_group)
        return car_shape

    def create_button_shape(self):
        if not self.car_body:
            raise Exception('Create car body before')

        button_shape = pymunk.Poly(self.car_body, list(map(lambda x: (x[0] * self.x_modification, x[1]), self.get_button_poly())))

        button_shape.color = 23, 230, 230
        button_shape.filter = pymunk.ShapeFilter(group=self.car_group)
        button_shape.sensor = True
        button_shape.collision_type = self.button_collision_type

        return button_shape

    @classmethod
    def get_button_poly(cls):
        x, y = cls.button_position
        h, w = cls.button_hw
        cos = math.cos(cls.button_angle)
        sin = math.sin(cls.button_angle)
        return [
            (x, y),
            ((x + 0 * cos - h * sin), y + +0 * sin + h * cos),
            ((x + w * cos - h * sin), y + +w * sin + h * cos),
            ((x + w * cos - 0 * sin), y + +w * sin + 0 * cos),
        ]

    def get_objects_for_space_at(self, point):
        self.car_body.position = point
        self.front_wheel_body.position = point + (self.front_wheel_position[0] * self.x_modification, self.front_wheel_position[1])
        self.rear_wheel_body.position = point + (self.rear_wheel_position[0] * self.x_modification, self.rear_wheel_position[1])

        return list(chain([self.button_shape, self.car_body, self.car_shape, self.rear_wheel_body, self.front_wheel_body], self.rear_wheel_objects, self.front_wheel_objects, self.motors))

    def go_right(self):
        if self.in_air():
            self.car_body.torque = self.torque

        for motor in self.motors:
            motor.rate = -self.max_speed

    def go_left(self):
        if self.in_air():
            self.car_body.torque = -self.torque

        for motor in self.motors:
            motor.rate = self.max_speed

    def stop(self):
        for motor in self.motors:
            motor.rate = 0

    def in_air(self):
        return not (self.point_query_nearest(self.rear_wheel_body.position, self.rear_wheel_radius + 1, pymunk.ShapeFilter(group=self.car_group))
                or self.point_query_nearest(self.front_wheel_body.position, self.front_wheel_radius + 1, pymunk.ShapeFilter(group=self.car_group)))

    def get_button_collision_type(self):
        return self.button_collision_type

    def fast_dump(self):
        return [(self.car_body.position.x, self.car_body.position.y),
             self.car_body.angle, self.x_modification,
             (self.rear_wheel_body.position.x, self.rear_wheel_body.position.y, self.rear_wheel_body.angle),
             (self.front_wheel_body.position.x, self.front_wheel_body.position.y, self.front_wheel_body.angle)]

    @classmethod
    def proto_dump(cls, visio=False):
        base_car_proto = {
            'car_body_poly': cls.car_body_poly,
            'rear_wheel_radius': cls.rear_wheel_radius,
            'front_wheel_radius': cls.front_wheel_radius,
            'button_poly': cls.get_button_poly(),
            'external_id': cls.external_id,

        }

        if not visio:
            extended_car_proto = {
                'car_body_mass': cls.car_body_mass,
                'car_body_friction': cls.car_body_friction,
                'car_body_elasticity': cls.car_body_elasticity,
                'max_speed': cls.max_speed,
                'max_angular_speed': cls.max_angular_speed,
                'torque': cls.torque,
                'drive': cls.drive,

                'rear_wheel_mass': cls.rear_wheel_mass,
                'rear_wheel_position': cls.rear_wheel_position,
                'rear_wheel_friction': cls.rear_wheel_friction,
                'rear_wheel_elasticity': cls.rear_wheel_elasticity,
                'rear_wheel_joint': cls.rear_wheel_joint,
                'rear_wheel_damp_position': cls.rear_wheel_damp_position,
                'rear_wheel_damp_length': cls.rear_wheel_damp_length,
                'rear_wheel_damp_stiffness': cls.rear_wheel_damp_stiffness,
                'rear_wheel_damp_damping': cls.rear_wheel_damp_damping,

                'front_wheel_mass': cls.front_wheel_mass,
                'front_wheel_position': cls.front_wheel_position,
                'front_wheel_friction': cls.front_wheel_friction,
                'front_wheel_elasticity': cls.front_wheel_elasticity,
                'front_wheel_joint': cls.front_wheel_joint,
                'front_wheel_damp_position': cls.front_wheel_damp_position,
                'front_wheel_damp_length': cls.front_wheel_damp_length,
                'front_wheel_damp_stiffness': cls.front_wheel_damp_stiffness,
                'front_wheel_damp_damping': cls.front_wheel_damp_damping,
            }
            base_car_proto.update(extended_car_proto)

        return base_car_proto
