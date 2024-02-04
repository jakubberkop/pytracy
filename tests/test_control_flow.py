import unittest
import pytracy

class ControlFlow(unittest.TestCase):

	def setUp(self) -> None:
		pytracy.set_tracing_mode(pytracy.TracingMode.All)
		return super().setUp()
	
	def tearDown(self) -> None:
		pytracy.set_tracing_mode(pytracy.TracingMode.Disabled)
		return super().tearDown()

	def test_function_call(self):
		def a(i):
			b(i)

		def b(i):
			pass

		a(10)

		# TODO: verify recorded data

	def test_control_flow(self):
		def aaa(i):
			bb(i)

			try:
				pass_through()
				raiser()
			except:
				pass

			bb(i)

		def bb(i):
			pass

		def pass_through():
			raiser()

		def raiser():
			raise Exception("Test")

		aaa(10)

if __name__ == '__main__':
	unittest.main()