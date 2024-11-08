import time
import os

import pytracy
pytracy.set_tracing_mode(pytracy.TracingMode.All)


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

