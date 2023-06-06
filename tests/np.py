import numpy as np
import pytracy
import time

pytracy.enableTracing()

pytracy.enableTracing()

a = [j for j in [i for i in range(10)]]

# def bbbb():
#     return

# def aaa():
#     # Create a 100x100 matrix of random numbers
a = np.random.rand(100, 100)
b = np.random.rand(100, 100)

# # # Multiply the matrices
c = np.matmul(a, b)

# # Sum the elements of the matrix
d = np.sum(c)

# # Calculate the average of the elements of the matrix
d = d / (100 * 100)

# Calculate the quartiles of the elements of the matrix
q1 = np.quantile(c, 0.25)
q2 = np.quantile(c, 0.5)
q3 = np.quantile(c, 0.75)

#     # # Calculate the standard deviation of the elements of the matrix
#     # std = np.std(c)

#     # bbbb()

#     # Print everything
#     # print("a: ", a)
#     # print("b: ", b)
#     # print("c: ", c)
#     # print("d: ", d)
#     # print("q1: ", q1)
#     # print("q2: ", q2)
#     # print("q3: ", q3)
#     # print("std: ", std)

# # for i in range(1):
# aaa()

time.sleep(1)