#!/usr/bin/env python3

import gzip
import sys
import json
import math

from rewind import RewindClient
from drawer import Drawer
from vector import Vector
from deadline import Deadline
from car import Car
from track import Track

class Visualizer(Drawer):
	def __init__(self):
		self.track = None
		self.player1 = None
		self.player2 = None
		self.deadline = None

	def set_track(self, track):
		self.track = track

	def set_cars(self, car_left, car_right):
		self.player1 = car_left
		self.player2 = car_right

	def set_deadline(self, deadline):
		self.deadline = deadline

	def draw(self, rewind):
		objs = list(filter(lambda obj: obj is not None,
			[self.track, self.player1, self.player2, self.deadline]))
		for obj in objs:
			self.objects.append(obj)

		Drawer.draw(self, rewind)
		rewind.end_frame()

if __name__ == '__main__':
	rewind = RewindClient()
	visualizer = Visualizer()

	with gzip.open(sys.argv[1], 'r') as f:
		log = json.loads(f.read().decode('utf-8'))
		m = None
		for msg in log["visio_info"]:
			visualizer.reset()

			if msg["type"] == "new_match":
				m = Track(msg["params"]["proto_map"], layer=1)
				car_left = Car(msg["params"]["proto_car"])
				car_right = Car(msg["params"]["proto_car"], color=0x0000FF, spawn=-1)
				visualizer.set_track(m)
				visualizer.set_cars(car_left, car_right)
			elif msg["type"] == "end_game":
				break
			else:
				visualizer.set_deadline(Deadline(msg["params"]["deadline_position"]))
				visualizer.player1.update(msg["params"]["cars"]["1"])
				visualizer.player2.update(msg["params"]["cars"]["2"])
			visualizer.draw(rewind)

	rewind.close()

