import unittest
import pytracy

class MarkedFunctions(unittest.TestCase):

	def setUp(self) -> None:
		pytracy.set_tracing_mode(pytracy.TracingMode.MarkedFunctions)
		return super().setUp()

	def tearDown(self) -> None:
		pytracy.set_tracing_mode(pytracy.TracingMode.Disabled)
		return super().tearDown()

	def test_mark_function(self):

		@pytracy.mark_function
		def func():
			return None

		@pytracy.mark_function
		def func_args(a, b):
			return a, b

		@pytracy.mark_function
		def func_kwargs(a=1, b=2):
			return a, b

		self.assertIsNone(func())
		self.assertTupleEqual(func_args(1, 2),  (1, 2))
		self.assertTupleEqual(func_kwargs(a=1, b=2),  (1, 2))
		self.assertTupleEqual(func_kwargs(b=2, a=1),  (1, 2))

if __name__ == '__main__':
	unittest.main()
