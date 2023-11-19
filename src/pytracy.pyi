from typing import Callable
from enum import Enum

class TracingMode(Enum):
	Disabled = 0
	MarkedFunctions = 1
	All = 2

def set_tracing_mode(mode: Enum) -> None: ...
def trace_function(func: Callable) -> Callable: ...
