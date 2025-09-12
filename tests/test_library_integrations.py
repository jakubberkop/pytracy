import time
import unittest
import pytracy

pytracy.enable_tracing(True)
pytracy.set_filtered_out_folders([])

import numpy as np
import pandas as pd

class LibraryIntegrations(unittest.TestCase):

	def test_numpy_integration(self):
		print("ASD")

		a = [j for j in [i for i in range(10)]]

		a = np.random.rand(100, 100)
		b = np.random.rand(100, 100)

		c = np.matmul(a, b)

		d = np.sum(c)

		d = d / (100 * 100)

		q1 = np.quantile(c, 0.25)
		q2 = np.quantile(c, 0.5)
		q3 = np.quantile(c, 0.75)

		std = np.std(c)

		# TODO: verify recorded data

	def test_dataframe_integration(self):


		df = pd.DataFrame({
			'col1': [1, 2],
			'col2': [3, 4]
		})

		df = df + 1

		# Find lowest value in each row
		df.min(axis=1)
	
		# Sum all values in each row
		df.sum(axis=1)

if __name__ == '__main__':
	import time
	time.sleep(4)
	unittest.main()
