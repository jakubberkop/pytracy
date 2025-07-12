import sys

# if True:
# 372
print("Loading tracy")
# import pytracy

# else:

# 513.7266500000806
# def traceit(frame, event, arg):
#     return None
# sys.setprofile(traceit)

# 402
# def on_call(*args):
#     pass
# def on_return(*args):
#     pass
sys.monitoring.use_tool_id(2, "pytracy")
# evs = sys.monitoring.events.PY_START | sys.monitoring.events.PY_RETURN
# sys.monitoring.set_events(2, evs)
# print(sys.monitoring.get_events(2))
# sys.monitoring.register_callback(2, sys.monitoring.events.PY_START, on_call)
# sys.monitoring.register_callback(2, sys.monitoring.events.PY_RETURN, on_return)




import time
import os
import threading

def a(i):
    # pytracy.log("Enter A")
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
    time.sleep(0.1)
    b(i)
    time.sleep(0.1)
    # pytracy.log("Exit A")

def b(i):
    # pytracy.log("Enter B")
    time.sleep(0.1)
    # pytracy.log("Exit B")

# pytracy.enable_tracing(True)
# pytracy.set_filtered_out_folders([])

ON_THREAD = False
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
import numpy
# time.sleep(4)