from __future__ import annotations
import contextlib
import dis
import random
import sys
from types import CodeType

import numpy as np

if sys.version_info < (3, 12):
    raise RuntimeError("Python 3.12 or higher required")

"""
    get_events(tool_id, /)

    get_local_events(tool_id, code, /)

    get_tool(tool_id, /)

    register_callback(tool_id, event, func, /)

    restart_events()

    set_events(tool_id, event_set, /)

    set_local_events(tool_id, code, event_set, /)

    use_tool_id(tool_id, name, /)
    
events = namespace(PY_START=1,
          PY_RESUME=2,
          PY_RETURN=4,
          PY_YIELD=8,
          CALL=16,
          LINE=32,
          INSTRUCTION=64,
          JUMP=128,
          BRANCH=256,
          STOP_ITERATION=512,
          RAISE=1024,
          EXCEPTION_HANDLED=2048,
          PY_UNWIND=4096,
          PY_THROW=8192,
          RERAISE=16384,
          C_RETURN=32768,
          C_RAISE=65536,
          NO_EVENTS=0)
"""

log = []

def fn(code: CodeType, instruction_offset: int, callable: object, arg0: object | sys.monitoring.MISSING) -> ...:
    log.append((code, instruction_offset, callable, arg0))







def somtimes_branch():
    if random.randint(0, 1):
        print("not branching", file=sys.stderr)
        return False
    else:
        print("branching", file=sys.stderr)
        return True


@contextlib.contextmanager
def monitor(tool_id: int):
    sys.monitoring.use_tool_id(tool_id, "monitor_me")
    events = [sys.monitoring.events.CALL]
    for event in events:
        sys.monitoring.set_events(tool_id, event)
        sys.monitoring.register_callback(tool_id, event, fn)
    try:
        yield
    finally:
        sys.monitoring.reset_events()

        sys.monitoring.free_tool_id(tool_id)


def main():
    with monitor(0):
        if somtimes_branch():
            a = np.random.randn(1000, 1000)
            b = np.random.randn(1000, 1000)
            c = a @ b
            print("matrix multiplication done", file=sys.stderr)
        else:
            with open("/dev/urandom", "rb") as f:
                f.read(1024 * 1024 * 50)
            print("read 50MB from /dev/urandom", file=sys.stderr)


if __name__ == "__main__":
    main()
    print(log)
    breakpoint()