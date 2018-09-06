import json
from math import pi

from pymunk.vec2d import Vec2d

from drawer import Drawer

class Arrow(Drawer):
	ARROW_SIZE = 5
	DELTA_ANGLE = pi/12

	def __init__(self, xv, yv, multiplier=1, color=0xFFFF00, layer=5):
		Drawer.__init__(self, color, layer)
		self.vector, self.multiplier = Vec2d(xv, yv), multiplier

	def update(self, x, y, xv, yv):
		self.move(x, y)
		self.vector.x, self.vector.y = xv, yv

	def draw(self, rewind):
		x = self.x + self.vector.x * self.multiplier
		y = self.y + self.vector.y * self.multiplier

		vpos, vneg = Vec2d.unit(), Vec2d.unit()
		vpos.angle = self.vector.angle + self.DELTA_ANGLE 
		vneg.angle = self.vector.angle - self.DELTA_ANGLE

		rewind.line(self.x, self.y, x, y,
			self.color, self.layer)
		rewind.line(x, y,
			x - self.ARROW_SIZE * vpos.x,
			y - self.ARROW_SIZE * vpos.y,
			self.color, self.layer)
		rewind.line(x, y,
			x - self.ARROW_SIZE * vneg.x,
			y - self.ARROW_SIZE * vneg.y,
			self.color, self.layer)
