import time

# import pytracy
# pytracy.enableTracing()


def a(i):
    b(i)

def b(i):
    pass

start = time.time()
iteration_count = 1000000

for i in range(iteration_count):
    a(10)

end = time.time()

print("Time taken: ", end - start, " seconds")
print("Time per iteration (ns): ", (end - start) / iteration_count * 1e9)

