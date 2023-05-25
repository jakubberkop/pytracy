print("Before import")

import pytracy

import time
import numpy as np

def a(i):
	b(i)

def b(i):
	for j in range(i):
		c(j)

def c(i):
	pass

i = 0

while True:
	i += 1
	# print(i)
	a(10)

	# Do numpy stuff
	np.random.rand(100, 100)


	# time.sleep(0.1)

# for i in range(100):
# 	a(100)