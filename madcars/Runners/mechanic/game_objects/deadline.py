import pymunk


class DeadLine:
    ASC = 0
    DESC = 1

    def __init__(self, type, max_length, max_height):
        self.type = type
        self.line_body = pymunk.Body(0, 0, pymunk.Body.KINEMATIC)
        self.line = pymunk.Poly(self.line_body, [(0, 2), (max_length, 2), (max_length, -max_height), (0, -max_height)])
        self.line.sensor = True
        self.line.body.position = (0, 10 if self.type == self.ASC else max_height - 10)

    def move(self):
        position = self.line.body.position
        if self.type == self.ASC:
            self.line.body.position = (position.x, position.y + 0.5)
        else:
            self.line.body.position = (position.x, position.y - 0.5)

    def get_object_for_space(self):
        return self.line

    def get_position(self):
        return self.line.body.position.y
