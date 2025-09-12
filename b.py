import sys
import time

# Event tracking storage
events_log = []
call_stack = {}

def log_event(event_name, code, offset, extra_info=""):
    """Helper to log all events with timestamps"""
    timestamp = time.perf_counter()
    function_name = code.co_name if hasattr(code, 'co_name') else str(code)
    events_log.append({
        'time': timestamp,
        'event': event_name,
        'function': function_name,
        'offset': offset,
        'extra': extra_info
    })
    global deep
    print(f"[{deep} {event_name:15} | {function_name:15} | offset: {offset:3} | {extra_info}")

# Tool ID for profiler
TOOL_ID = sys.monitoring.PROFILER_ID

# Define callbacks for all interesting events with matching signatures
deep = 0


def call_callback(code, offset, callable_obj, arg):
    call_stack[id(code)] = time.perf_counter()
    log_event("CALL", code, offset, f"callable: {callable_obj}")

def yield_callback(code, offset, value):
    global deep
    deep -= 1
    log_event("YIELD", code, offset, f"value: {value}")

def resume_callback(code, offset):
    global deep
    deep += 1
    log_event("PY_RESUME", code, offset)

def py_return_callback(code, offset, v):
    global deep
    deep += -1
    log_event("PY_RETURN", code, offset, v)

def py_start_callback(code, offset):
    global deep
    deep += 1
    log_event("PY_START", code, offset)

def py_unwind_callback(code, offset, exc):
    global deep
    deep -= 1
    log_event("PY_UNWIND", code, offset, f"exc: {type(exc).__name__}")

def stop_iteration_callback(code, offset, value):
    log_event("STOP_ITERATION", code, offset, f"value: {value}")

def exception_callback(code, offset, exception):
    log_event("EXCEPTION", code, offset, f"exception: {type(exception).__name__}: {exception}")


def py_throw_callback(code, offset, exc):
    log_event("PY_THROW", code, offset, f"exc: {type(exc).__name__}")


def c_return_callback(code, offset, retval, G):
    # global deep
    # deep -= 1
    log_event("C_RETURN", code, offset, f"retval: {retval}")

def c_raise_callback(code, offset, exc, v):
    log_event("C_RAISE", code, offset, f"exception: {type(exc).__name__} {v}")

def py_raise_callback(code, offset, value):
    log_event("RAISE", code, offset, f"value: {value}")

def py_reraise_callback(code, offset, value):
    log_event("RERAISE", code, offset, f"value: {value}")

# Register callbacks for all these events

#     BRANCH: int
CALL: int
C_RAISE: int
C_RETURN: int
EXCEPTION_HANDLED: int
INSTRUCTION: int
JUMP: int
LINE: int
NO_EVENTS: int
PY_RESUME: int
PY_RETURN: int
PY_START: int
PY_THROW: int
PY_UNWIND: int
PY_YIELD: int
RAISE: int
RERAISE: int
STOP_ITERATION: int

event_callbacks = {
    # sys.monitoring.events.CALL: call_callback,
    sys.monitoring.events.PY_YIELD: yield_callback, ###
    sys.monitoring.events.PY_RESUME: resume_callback, ###
    sys.monitoring.events.STOP_ITERATION: stop_iteration_callback,
    # sys.monitoring.events.EXCEPTION_HANDLED: lambda c, o: log_event("EXCEPTION_HANDLED", c, o),
    # sys.monitoring.events.EXCEPTION: exception_callback,
    sys.monitoring.events.PY_RETURN: py_return_callback, ###
    sys.monitoring.events.PY_START: py_start_callback, ###
    sys.monitoring.events.PY_THROW: py_throw_callback,
    sys.monitoring.events.PY_UNWIND: py_unwind_callback, ###
    # sys.monitoring.events.C_RETURN: c_return_callback,
    # sys.monitoring.events.C_RAISE: c_raise_callback,
    # sys.monitoring.events.RAISE: py_raise_callback,
    sys.monitoring.events.RERAISE: py_reraise_callback,
}

# Use the profiler tool ID and register callbacks
# sys.monitoring.use_tool_id(TOOL_ID, "profiler_all_events")
# for event, callback in event_callbacks.items():
#     sys.monitoring.register_callback(TOOL_ID, event, callback)

# # Enable all events at once
# event_mask = 0
# for event in event_callbacks:
#     event_mask |= event

# sys.monitoring.set_events(TOOL_ID, event_mask)

import atexit

def cleanup_function():
    print("This runs before actual interpreter exit (_python_exit).")

atexit.register(cleanup_function)


print("="*80)
print("MONITORING STARTED - All sys.monitoring events will be logged")
print("="*80)

# Test functions same as before

def regular_function(x):
    time.sleep(0.001)
    return x * 2

def generator_function(x):
    yield x
    yield x * 2
    yield x * 3
    return "generator finished"

def g():
    return exception_function()

def exception_function():
    raise ValueError("Test exception")

# print("\n1. Regular function call:")
# result = regular_function(10)
# print(f"Result: {result}")

# # print("\n2. Generator function call:")
# gen = generator_function(10)
# try:
#     while True:
#         val = next(gen)
#         # print(f"Yielded: {val}")
# except StopIteration as e:
#     print("FUCK YOU")

# for g in gen:
#     print(f"Yielded: {g}")

print("\n3. Exception function call:")
try:
    g()
except ValueError as e:
    print(f"Caught exception: {e}")

# Clean up: disable monitoring and free tool ID
sys.monitoring.set_events(TOOL_ID, 0)
sys.monitoring.free_tool_id(TOOL_ID)
