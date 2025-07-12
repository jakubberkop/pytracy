import sys
import os
evs = sys.monitoring.events
# Separate tracking for Python and C-level events to minimize mismatches
count_py = 0
call_stack_py = []
count_c = 0
call_stack_c = []
unmatched_returns = 0


# Increment count based on callable type
def call(code, instruction_offset, callable_obj, arg0):
    global count_py, call_stack_py, count_c, call_stack_c
    # Treat callable_obj without __code__ as C-level; otherwise Python. If callable_obj is None, assume Python-level.
    if callable_obj is not None and not hasattr(callable_obj, '__code__'):
        # C-level function
        count_c += 1
        call_stack_c.append((code, instruction_offset))
    else:
        # Python-level function (includes callable_obj is None)
        count_py += 1
        call_stack_py.append((code, instruction_offset))


def py_return(code, instruction_offset, retval):
    global count_py, call_stack_py, unmatched_returns
    if call_stack_py:
        count_py -= 1
        call_stack_py.pop()
    else:
        unmatched_returns += 1
        print(f"Unmatched PY_RETURN: stack is empty")
        print(f"File: {code.co_filename}")
        print(f"Unmatched event: PY_RETURN for {getattr(retval, '__name__', 'unknown')}")

def c_return(code, instruction_offset, callable_obj, arg0):
    global count_c, call_stack_c, unmatched_returns
    callable_name = 'unknown' if callable_obj is None else getattr(callable_obj, '__name__', 'unknown')
    if call_stack_c:
        count_c -= 1
        call_stack_c.pop()
    else:
        unmatched_returns += 1
        print(f"Unmatched C_RETURN: stack is empty, callable: {callable_name}")
        print(f"File: {code.co_filename}")
        if 'pytracy' in code.co_filename or 'numpy' in code.co_filename:
            print(f"Unmatched C_RETURN: stack is empty, callable: {callable_name}")
            print(f"File: {code.co_filename}")

def py_unwind(code, instruction_offset, exc):
    global count_py, call_stack_py, unmatched_returns
    if call_stack_py:
        count_py -= 1
        call_stack_py.pop()
    else:
        unmatched_returns += 1
        if 'pytracy' in code.co_filename or 'numpy' in code.co_filename:
            print(f"Unmatched PY_UNWIND: stack is empty")
            print(f"File: {code.co_filename}")
            print(f"Unmatched event: PY_UNWIND for {getattr(exc, '__name__', 'unknown')}")

def c_raise(code, instruction_offset, callable_obj, arg0):
    global count_c, call_stack_c, unmatched_returns
    if call_stack_c:
        count_c -= 1
        call_stack_c.pop()
    else:
        unmatched_returns += 1
        if 'pytracy' in code.co_filename or 'numpy' in code.co_filename:
            print(f"Unmatched C_RAISE: stack is empty")
            print(f"File: {code.co_filename}")
            print(f"Unmatched event: C_RAISE for {callable_obj.__name__ if hasattr(callable_obj, '__name__') else 'unknown'}")

def raise_event(code, instruction_offset, exc):
    global count_py, call_stack_py, unmatched_returns
    if call_stack_py:
        count_py -= 1
        call_stack_py.pop()
    else:
        unmatched_returns += 1
        if 'pytracy' in code.co_filename or 'numpy' in code.co_filename:
            print(f"Unmatched RAISE: stack is empty")
            print(f"File: {code.co_filename}")
            print(f"Unmatched event: RAISE for {exc.__name__ if hasattr(exc, '__name__') else 'unknown'}")

def reraise_event(code, instruction_offset, exc):
    global count_py, call_stack_py, unmatched_returns
    if call_stack_py:
        count_py -= 1
        call_stack_py.pop()
    else:
        unmatched_returns += 1
        if 'pytracy' in code.co_filename or 'numpy' in code.co_filename:
            print(f"Unmatched RERAISE: stack is empty")
            print(f"File: {code.co_filename}")
            print(f"Unmatched event: RERAISE for {exc.__name__ if hasattr(exc, '__name__') else 'unknown'}")


TOOL_ID = 3
sys.monitoring.use_tool_id(TOOL_ID, "TEST")
sys.monitoring.register_callback(TOOL_ID, evs.CALL, call)
sys.monitoring.register_callback(TOOL_ID, evs.C_RETURN, c_return)
sys.monitoring.register_callback(TOOL_ID, evs.PY_RETURN, py_return)
sys.monitoring.register_callback(TOOL_ID, evs.PY_UNWIND, py_unwind)
sys.monitoring.register_callback(TOOL_ID, evs.C_RAISE, c_raise)
sys.monitoring.register_callback(TOOL_ID, evs.RAISE, raise_event)
sys.monitoring.register_callback(TOOL_ID, evs.RERAISE, reraise_event)

# Monitor calls, returns, and all exception-related events, including C-level, to track everything
sys.monitoring.set_events(TOOL_ID, evs.CALL | evs.C_RETURN | evs.PY_RETURN | evs.PY_UNWIND | evs.RAISE | evs.RERAISE | evs.C_RAISE)

import time
start_import_time = time.time()
try:
    import numpy as np
except Exception as e:
    print(f"Error importing numpy: {e}")
import_time = time.time() - start_import_time
print(f"Numpy import took {import_time:.4f} seconds or failed")

sys.monitoring.free_tool_id(TOOL_ID)
sys.monitoring.register_callback(TOOL_ID, evs.CALL, None)
sys.monitoring.register_callback(TOOL_ID, evs.C_RETURN, None)
sys.monitoring.register_callback(TOOL_ID, evs.PY_RETURN, None)
sys.monitoring.register_callback(TOOL_ID, evs.PY_UNWIND, None)
sys.monitoring.register_callback(TOOL_ID, evs.C_RAISE, None)
sys.monitoring.register_callback(TOOL_ID, evs.RAISE, None)
sys.monitoring.register_callback(TOOL_ID, evs.RERAISE, None)



print(f"Final count (Python): {count_py}")
print(f"Final count (C): {count_c}")
print(f"Stack depth (Python): {len(call_stack_py)}")
print(f"Stack depth (C): {len(call_stack_c)}")
print(f"Unmatched returns: {unmatched_returns}")
