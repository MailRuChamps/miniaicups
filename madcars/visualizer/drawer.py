class Drawer:
	def __init__(self, color=0x000000, layer=3):
		self.reset()
		self.x, self.y = 0, 0
		self.color, self.layer = color, layer

	def reset(self):
		self.objects = []

	def move(self, x, y):
		self.x, self.y = x, y

	def add(self, drawer):
		self.objects.append(drawer)

	def draw(self, rewind):
		for obj in self.objects:
			obj.draw(rewind)
