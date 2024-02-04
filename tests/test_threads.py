import unittest
import pytracy

class Threads(unittest.TestCase):

	def setUp(self) -> None:
		pytracy.set_tracing_mode(pytracy.TracingMode.All)
		return super().setUp()
	
	def tearDown(self) -> None:
		pytracy.set_tracing_mode(pytracy.TracingMode.Disabled)
		return super().tearDown()

	def test_threads(self):
		import threading

		def test_function():
			pass

		def worker(num):
			test_function()

		threads = []

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