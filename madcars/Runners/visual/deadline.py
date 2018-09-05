from drawer import Drawer

class Deadline(Drawer):
	def __init__(self, level, layer=1):
		Drawer.__init__(self, layer)
		self.level = level

	def draw(self, rewind):
		rewind.line(0, self.level, 1200, self.level,
			self.color, self.layer)
