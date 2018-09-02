import math
import pymunk

from mechanic.game_objects.base_car import Car


class Buggy(Car):
    external_id = 1

    car_body_poly = [
        (0, 6),
        (0, 25),
        (33, 42),
        (85, 42),
        (150, 20),
        (150, 0),
        (20, 0)
    ]
    car_body_mass = 200

    button_position = (40, 42)
    button_hw = (1, 38)

    max_speed = 70
    torque = 14000000

    drive = Car.FR

    rear_wheel_mass = 50
    rear_wheel_position = (29, -5)
    rear_wheel_joint = (150, 0)
    rear_wheel_damp_position = (29, 20)
    rear_wheel_damp_stiffness = 5e4
    rear_wheel_damp_damping = 3e3
    rear_wheel_damp_length = 25
    rear_wheel_radius = 12

    front_wheel_mass = 5
    front_wheel_position = (122, -5)
    front_wheel_joint = (0, 6)
    front_wheel_damp_position = (122, 20)
    front_wheel_damp_length = 25
    front_wheel_radius = 12


class Bus(Car):
    external_id = 2

    car_body_poly = [
        (0, 6),
        (8, 62),
        (136, 62),
        (153, 32),
        (153, 5),
        (110, 0),
        (23, 0)
    ]
    car_body_mass = 700

    button_position = (137, 59)
    button_angle = -math.atan(3/1.7)
    button_hw = (1, 28)

    max_speed = 45
    torque = 35000000

    drive = Car.AWD

    rear_wheel_radius = 13
    rear_wheel_position = (38, -5)
    rear_wheel_friction = 0.9
    rear_wheel_joint = (153, 5)
    rear_wheel_damp_position = (38, 30)
    rear_wheel_damp_length = 35
    rear_wheel_damp_stiffness = 10e4
    rear_wheel_damp_damping = 6e3

    front_wheel_radius = 13
    front_wheel_position = (125, -5)
    front_wheel_joint = (0, 6)
    front_wheel_damp_position = (125, 30)
    front_wheel_damp_length = 35
    front_wheel_damp_stiffness = 10e4
    front_wheel_damp_damping = 6e3


class SquareWheelsBuggy(Buggy):
    external_id = 3

    max_speed = 50

    drive = Buggy.AWD

    car_body_mass = 230
    rear_wheel_mass = 10
    rear_wheel_damp_stiffness = 10e4
    rear_wheel_damp_damping = .9e3

    front_wheel_mass = 10

    def create_wheel(self, wheel_side):
        if wheel_side not in ['rear', 'front']:
            raise Exception('Wheel position must be front or rear')
        wheel_objects = []

        wheel_mass = getattr(self, wheel_side + '_wheel_mass')
        wheel_radius = getattr(self, wheel_side + '_wheel_radius')
        wheel_position = getattr(self, wheel_side + '_wheel_position')
        wheel_friction = getattr(self, wheel_side + '_wheel_friction')
        wheel_elasticity = getattr(self, wheel_side + '_wheel_elasticity')
        wheel_joint = getattr(self, wheel_side + '_wheel_joint')
        wheel_damp_position = getattr(self, wheel_side + '_wheel_damp_position')
        wheel_damp_length = getattr(self, wheel_side + '_wheel_damp_length')
        wheel_damp_stiffness = getattr(self, wheel_side + '_wheel_damp_stiffness')
        wheel_damp_damping = getattr(self, wheel_side + '_wheel_damp_damping')

        wheel_body = pymunk.Body(wheel_mass, pymunk.moment_for_box(wheel_mass, (wheel_radius * 2, wheel_radius * 2)))
        wheel_body.position = (wheel_position[0] * self.x_modification, wheel_position[1])

        wheel_shape = pymunk.Poly.create_box(wheel_body, (wheel_radius * 2, wheel_radius * 2))
        wheel_shape.filter = pymunk.ShapeFilter(group=self.car_group)
        wheel_shape.color = 255, 34, 150
        wheel_shape.friction = wheel_friction
        wheel_shape.elasticity = wheel_elasticity
        wheel_objects.append(wheel_shape)

        wheel_joint = pymunk.PinJoint(wheel_body, self.car_body, anchor_b=(wheel_joint[0] * self.x_modification, wheel_joint[1]))
        wheel_objects.append(wheel_joint)

        wheel_damp = pymunk.DampedSpring(wheel_body, self.car_body, anchor_a=(0, 0),
                                         anchor_b=(wheel_damp_position[0] * self.x_modification, wheel_damp_position[1]),
                                         rest_length=wheel_damp_length,
                                         stiffness=wheel_damp_stiffness,
                                         damping=wheel_damp_damping)
        wheel_objects.append(wheel_damp)

        wheel_stop = pymunk.Poly(self.car_body, [(0, 0), (0, 1),(wheel_radius * 2 * self.x_modification, 1), (wheel_radius * 2 * self.x_modification, 0)],
                                 transform=pymunk.Transform(tx=wheel_damp_position[0] * self.x_modification - wheel_radius * self.x_modification, ty=wheel_damp_position[1]))
        wheel_objects.append(wheel_stop)

        wheel_stop.color = 0, 255, 0

        wheel_motor = None
        if (wheel_side == 'rear' and self.drive in [self.AWD, self.FR]) or (wheel_side == 'front' and self.drive in [self.AWD, self.FF]):
            wheel_motor = pymunk.SimpleMotor(wheel_body, self.car_body, 0)

        return wheel_body, wheel_motor, wheel_objects

    @classmethod
    def proto_dump(cls, visio=False):
        proto = super().proto_dump(visio)
        proto['squared_wheels'] = True
        return proto
