from typing import List
import unittest
import pytracy


class Threads(unittest.TestCase):

	def setUp(self) -> None:
		pytracy.enable_tracing(True)
		return super().setUp()
	
	def tearDown(self) -> None:
		pytracy.enable_tracing(False)
		return super().tearDown()

	def test_threads(self):
		import threading

		def test_function():
			pass

		def worker(_: int):
			test_function()

		threads: List[threading.Thread] = []

		for i in range(10):
			t = threading.Thread(target=worker, args=(i,))
			threads.append(t)

		for t in threads:
			t.start()


		for t in threads:
			test_function()

		for t in threads:
			t.join()

if __name__ == '__main__':
	unittest.main()