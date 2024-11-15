import time
import os

import pytracy
pytracy.enable_tracing(True)


def a(i):
    b(i)

def b(i):
    pass

start = time.time()
iteration_count = int(os.sys.argv[1])

for i in range(iteration_count):
    a(10)

end = time.time()

print(end - start)

