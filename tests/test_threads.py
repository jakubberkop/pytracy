import pytracy
pytracy.enable_tracing(True)
# pytracy.set_filtered_out_folders([])
from typing import List
import unittest

def on_call(*args):
	print("CALL:  ", args)

def on_return(*args):
	print("RETURN:", args)

# import sys
# sys.monitoring.use_tool_id(2, "pytracy")
# evs = sys.monitoring.events.PY_START | sys.monitoring.events.PY_RETURN
# sys.monitoring.set_events(2, evs)

# print(sys.monitoring.get_events(2))

# sys.monitoring.register_callback(2, sys.monitoring.events.PY_START, on_call)
# sys.monitoring.register_callback(2, sys.monitoring.events.PY_RETURN, on_return)



class Threads(unittest.TestCase):

	def setUp(self) -> None:
		return super().setUp()
	
	def tearDown(self) -> None:
		pytracy.enable_tracing(False)
		return super().tearDown()

	def test_threads(self):
		import threading

		def test_function():
			pass

		def worker(_: int):
			for _ in range(2):
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