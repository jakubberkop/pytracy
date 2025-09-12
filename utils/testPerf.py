import time
import os
import threading
import pytracy

pytracy.enable_tracing(True)
pytracy.set_filtered_out_folders([])


def a(i):
    pass
    pass
    pass
    pass
    pass
    pass
    pass
    pass
    pass
    pass
    pass
    b(i)

def b(i):
    pass

ON_THREAD = True
a(10)

iteration_count = int(os.sys.argv[1])

def func():
    start = time.perf_counter()
    for i in range(iteration_count):
        a(10)

    end = time.perf_counter()
    dur = end - start
    iter_dur_s = dur / iteration_count
    iter_dur_ms = iter_dur_s  * 1000
    iter_dur_us = iter_dur_ms * 1000
    iter_dur_ns = iter_dur_us * 1000
    # print(iter_dur_ns)

        
if ON_THREAD:
    t = threading.Thread(target=func)
    t.start()
    t.join()
else:
    func()
