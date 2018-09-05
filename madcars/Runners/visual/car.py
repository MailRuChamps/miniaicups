import math
from functools import reduce

from drawer import Drawer
from vector import Vector
from transforms import *

bodies_cache = {} # { car_id: [lines]}

class Wheel(Drawer):
	def __init__(self, pos, radius, color=0xFF0000, layer=2):
		Drawer.__init__(self, color, layer)
		self.move(pos[0], pos[1])
		self.radius = radius
		self.angle = 0

	def update(self, data):
		self.move(data[0], data[1])
		self.angle = data[-1]

	def draw(self, rewind):
		rewind.circle(self.x, self.y, self.radius,
			0x000000, self.layer)
		rewind.circle(self.x, self.y, self.radius - 5,
			self.color, self.layer)
		rewind.line(
			self.x, self.y,
			self.x + math.cos(self.angle) * self.radius,
			self.y + math.sin(self.angle) * self.radius,
			0x000000, self.layer)

class Body(Drawer):
	def __init__(self, car_id, points, color=0x00FF00, layer=2, spawn=1):
		Drawer.__init__(self, color, layer)
		global bodies_cache
		if car_id not in bodies_cache:
			lines = []
			for i in range(len(points)):
				lines.append([points[i-1], points[i]])
			bodies_cache[car_id] = lines
		self.lines = bodies_cache[car_id]
		self.spawn = spawn
		self.angle = 0

	def center(self):
		ln = len(self.lines) - 1
		psum = reduce(lambda acc, nxt: [[acc[0][0] + nxt[0][0], acc[0][1] + nxt[0][1]]],
			self.lines[1:], [[0, 0]])[0]

		return rotate_point([psum[0] * self.spawn / ln, psum[1] / ln], self.angle)

	def update(self, x, y, angle, spawn):
		x0, y0 = self.x, self.y
		self.move(x, y)
		self.angle, self.spawn = angle, spawn

	def draw(self, rewind):
		Drawer.draw(self, rewind)
		for (start, end) in list(map(lambda line: rotate_line(line, self.angle * self.spawn), self.lines)):
			rewind.line(
				self.x + start[0] * self.spawn, self.y + start[1],
				self.x + end[0] * self.spawn, self.y + end[1],
				self.color, self.layer)

class Car(Drawer):
	def __init__(self, proto_car, spawn=1, color=0x00FF00, layer=2):
		Drawer.__init__(self, color, layer)
		self.proto_car = proto_car
		self.spawn = spawn
		self.angle = 0
		self.body = Body(proto_car["external_id"], proto_car["car_body_poly"], color=self.color, spawn=self.spawn)
		self.button = Body(proto_car["external_id"]+20, proto_car["button_poly"], color=0xFF0000, spawn=self.spawn)
		self.front_wheel = Wheel(proto_car["front_wheel_position"],
			proto_car["front_wheel_radius"])
		self.rear_wheel = Wheel(proto_car["rear_wheel_position"],
			proto_car["rear_wheel_radius"], color=0x960303)
		self.move_vector = Vector(0, 0, 0, 0, multiplier=10)

	def update(self, data):
		x0, y0 = self.x, self.y
		self.move(data[0][0], data[0][1])
		self.angle = data[1]
		self.spawn = data[2]
		self.body.update(self.x, self.y, self.angle, self.spawn)
		self.button.update(self.x, self.y, self.angle, self.spawn)
		self.front_wheel.update(data[4])
		self.rear_wheel.update(data[3])

		c = self.body.center()
		self.move_vector.update(self.x + c[0], self.y + c[1], self.x - x0, self.y - y0)

	def draw(self, rewind):
		self.reset()
		self.objects = [self.body, self.button, self.front_wheel, self.rear_wheel, self.move_vector]
		Drawer.draw(self, rewind)

		c = self.body.center()
		rewind.circle(self.x + c[0], self.y + c[1], 5, self.color, self.layer)
