from typing import Callable
from enum import Enum

class TracingMode(Enum):
	NONE = 0
	ONLY_SELECTED_FUNCTIONS = 1
	FULL = 2

def set_tracing_mode(mode: Enum) -> None: ...

def trace_function(func: Callable) -> Callable: ...
