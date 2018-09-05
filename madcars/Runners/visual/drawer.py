class Drawer:
	def __init__(self, color=0x000000, layer=3):
		self.objects = []
		self.x = 0
		self.y = 0

		self.layer = layer
		self.color = color

	def reset(self):
		self.objects = []

	def move(self, x, y):
		self.x, self.y = x, y

	def add_object(self, obj):
		self.objects.append(obj)

	def draw(self, rewind):
		for obj in self.objects:
			obj.draw(rewind)
