from functools import reduce

from pymunk.vec2d import Vec2d

from drawer import Drawer
from arrow import Arrow

class Wheel(Drawer):
	TIRE_WIDTH = 5

	def __init__(self, pos, radius, color=0xFF0000, layer=2):
		Drawer.__init__(self, color, layer)
		self.move(pos[0], pos[1])
		self.radius = radius
		self.vector = Vec2d.unit()
		self.move_vector = Arrow(0, 0, multiplier=5, color=0xFF00FF)

	def update(self, data):
		x0, y0 = self.x, self.y
		self.move(data[0], data[1])
		self.vector.angle = data[-1]
		self.move_vector.update(self.x, self.y, self.x - x0, self.y - y0)

	def draw(self, rewind):
		rewind.circle(self.x, self.y, self.radius,
			0x000000, self.layer)
		rewind.circle(self.x, self.y, self.radius - self.TIRE_WIDTH,
			self.color, self.layer)
		rewind.line(
			self.x, self.y,
			self.x + self.vector.x * self.radius,
			self.y + self.vector.y * self.radius,
			0x000000, self.layer)
		self.move_vector.draw(rewind)

class Body(Drawer):
	CENTER_RADIUS = 2

	def __init__(self, points, color=0x00FF00, layer=2, spawn=1):
		Drawer.__init__(self, color, layer)
		self.points = points
		self.spawn, self.vector = spawn, Vec2d.unit()

	def center(self):
		ln = len(self.points)
		psum = reduce(lambda acc, nxt: [acc[0] + nxt[0], acc[1] + nxt[1]],
			self.points, [0, 0])

		v = Vec2d(*[psum[0] / ln, psum[1] / ln])
		v.angle += self.spawn * self.vector.angle
		return [v.x, self.spawn * v.y]

	def update(self, x, y, angle, spawn):
		self.move(x, y)
		self.vector.angle, self.spawn = self.spawn * angle, spawn
		self.vector.x *= self.spawn

	def draw(self, rewind):
		Drawer.draw(self, rewind)

		c = self.center()
		rewind.circle(self.x + c[0], self.y + c[1], self.CENTER_RADIUS,
			self.color, self.layer)

		x = Arrow(self.vector.x, self.vector.y, multiplier=10, color=0x666666)
		x.move(self.x, self.y) 
		x.draw(rewind)
		v0 = Vec2d(self.points[-1][0], self.points[-1][1] * self.spawn)
		v0.angle += self.vector.angle
		for (x, y) in self.points:
			v = Vec2d(x, self.spawn * y)
			v.angle += self.vector.angle
			rewind.line(self.x + v0.x,
				self.y + v0.y,
				self.x + v.x,
				self.y + v.y,
				self.color, self.layer)
			v0 = v
		rewind.circle(self.x, self.y, 1, 0x000000)

class Car(Drawer):
	def __init__(self, proto_car, spawn=1, color=0x00FF00, layer=2):
		Drawer.__init__(self, color, layer)
		self.proto_car = proto_car
		self.spawn, self.vector = spawn, Vec2d.unit()
		self.body = Body(proto_car["car_body_poly"],
			color=self.color, spawn=self.spawn)
		self.button = Body(proto_car["button_poly"],
			color=0xFF0000, spawn=self.spawn)
		self.front_wheel = Wheel(proto_car["front_wheel_position"],
			proto_car["front_wheel_radius"])
		self.rear_wheel = Wheel(proto_car["rear_wheel_position"],
			proto_car["rear_wheel_radius"], color=0x960303)
		self.move_vector = Arrow(0, 0, multiplier=10)

	def update(self, data):
		x0, y0 = self.x, self.y
		self.move(data[0][0], data[0][1])
		self.vector.angle = data[1]
		self.spawn = data[2]
		self.body.update(self.x, self.y, self.vector.angle, self.spawn)
		self.button.update(self.x, self.y, self.vector.angle, self.spawn)
		self.front_wheel.update(data[4])
		self.rear_wheel.update(data[3])

		c = self.body.center()
		self.move_vector.update(self.x + c[0], self.y + c[1], self.x - x0, self.y - y0)

	def draw(self, rewind):
		self.objects = [self.body, self.button, self.front_wheel, self.rear_wheel, self.move_vector]
		rewind.circle(self.x, self.y, 2, 0xFF00FF)
		Drawer.draw(self, rewind)
