import os
import threading
import time
import sys

from typing import List

import numpy as np
import pytracy
pytracy.enable_tracing(True)

def test_control_flow():
	try:
		raise ""
	except:
		pass

	def a(i):
		b(i)

	def b(i):
		pass

	a(10)

	def bb(i):
		pass

	def aaa(i):
		bb(i)

		try:
			pass_through()
			raiser()
		except:
			pass

		bb(i)


	def pass_through():
		raiser()

	def raiser():
		raise Exception("Test")

	aaa(10)

def worker2(_: int):
	a = np.random.rand(100, 100)
	b = np.random.rand(100, 100)

	c = np.matmul(a, b)

	d = np.sum(c)

	d = d / (100 * 100)

	q1 = np.quantile(c, 0.25)
	q2 = np.quantile(c, 0.5)
	q3 = np.quantile(c, 0.75)

	std = np.std(c)

def worker(iteration_count: int):
	for i in range(iteration_count):
		# if i % 10 == 0:
		# 	pytracy.set_filtered_out_folders(pytracy.get_filtered_out_folders())


		test_control_flow()
		worker2(iteration_count)

		# Create a new function to test the overhead of creating a new function
		func = lambda: None
		func()


if __name__ == "__main__":
	iteration_count = int(sys.argv[1])

	threads: List[threading.Thread] = []

	a = time.time()

	for i in range(8):
		t = threading.Thread(target=worker, args=(iteration_count, ))
		threads.append(t)

	for t in threads:
		t.start()

	worker(iteration_count)

	for t in threads:
		t.join()

	print(time.time() - a)
