import math

def rotate_point(point, angle):
	ln = math.sqrt(point[0] ** 2 + point[1] ** 2)
	ang = math.acos(point[0] / ln)
	return [
		ln * math.cos(ang + angle),
		ln * math.sin(ang + angle)
	]

def rotate_line(line, angle):
	return [
		rotate_point(line[0], angle),
		rotate_point(line[1], angle)
	]
