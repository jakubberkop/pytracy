import time
import os
import threading

if False:
    import pytracy
    pytracy.enable_tracing(True)
else:
    import sys
    def traceit(frame, event, arg):
        return traceit
    sys.setprofile(traceit)


def a(i):
    b(i)

def b(i):
    pass

ON_THREAD = False
a(10)

iteration_count = int(os.sys.argv[1])

def func():
    start = time.time()
    for i in range(iteration_count):
        a(10)

    end = time.time()
    print(end - start)

if ON_THREAD:
    t = threading.Thread(target=func)
    t.start()
    t.join()
else:
    func()
