import json
import math

from drawer import Drawer

class Vector(Drawer):
	ARROW_SIZE = 5
	DELTA_ANGLE = (15*math.pi)/180

	def __init__(self, x, y, xv, yv, multiplier=1, color=0xFFFF00, layer=5):
		Drawer.__init__(self, color, layer)
		self.x = x
		self.y = y
		self.xv = xv
		self.yv = yv
		self.multiplier = multiplier

	def update(self, x, y, xv, yv):
		self.move(x, y)
		self.xv, self.yv = xv, yv

	def draw(self, rewind):
		x = self.x + self.xv * self.multiplier
		y = self.y + self.yv * self.multiplier
		l = math.sqrt(self.xv ** 2 + self.yv ** 2)
		angle = math.asin(self.yv / l if l != 0 else 0.0)
		angle2 = math.acos(self.xv / l if l != 0 else 1.0)
		direction = 1 if abs(round(angle, 1)) == abs(round(angle2, 1)) else -1

		size = direction * self.ARROW_SIZE
		angle = direction * angle
		angle_pos = angle + self.DELTA_ANGLE
		angle_neg = angle - self.DELTA_ANGLE
		dx = size * math.cos(angle_pos)
		dy = size * math.sin(angle_pos)
		dx2 = size * math.cos(angle_neg)
		dy2 = size * math.sin(angle_neg)

		rewind.line(self.x, self.y, x, y,
			self.color, self.layer)
		rewind.line(x, y, x - dx, y - dy,
			self.color, self.layer)
		rewind.line(x, y, x - dx2, y - dy2,
			self.color, self.layer)
