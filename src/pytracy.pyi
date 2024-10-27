"""
Python Tracy integration https://github.com/wolfpld/tracy.
This module provides a way to trace function calls in Python code.
It can be used get a time profile of the code, or to debug the flow of the code.

Importing this module initializes the default filter_out_folders and calls threading.settrace to set the tracing function.
This means that debugging doesn't work after importing this module.
This undesirable, so it is recommended to only import this module when you want to start tracing.

Current limitations:
- Debugging doesn't work after importing this module.
- High overhead when using *TracingMode.All* mode. This is the limitation of python tracing api.
- No visibility into functions run using multiprocessing.
- set_tracing_mode doesn't work across multiple threads.

"""
from typing import Any, Callable, List
from enum import Enum

"""
Specifies different tracing modes.
- Disabled: No tracing is done.
- MarkedFunctions: Only functions decorated with mark_function are traced. Very low overhead.
- All: All functions are traced. High overhead, comparable to running with debugger attached.
"""
class TracingMode(Enum):
	Disabled = 0
	MarkedFunctions = 1
	All = 2

"""
Enables or disables tracing of function calls.
:param mode: The tracing mode to set.

"""
def set_tracing_mode(mode: TracingMode) -> None: ...

"""
Decorates a function to be traced, only active when the tracing mode is set to MarkedFunctions.

Example:
```
@mark_function
def my_function():
	pass

set_tracing_mode(TracingMode.MarkedFunctions)

my_function()
```

"""
def mark_function(func: Callable[..., Any]) -> Callable[..., Any] : ...

"""
Sets which folders should be ignored when tracing.
Resets previously set filtered_out_folders.

:param stdlib: Whether to ignore standard library folders.
:param third_party: Whether to ignore third party folders.
:param user: Whether to ignore user folders.

By default, stdlib and third_party folders are ignored.

"""
def set_filtering_mode(stdlib: bool, third_party: bool, user: bool) -> None: ...

"""
Get current list of folders that are ignored when tracing.
"""
def get_filtered_out_folders() -> List[str]: ...

"""
Manually sets which folders should be ignored when tracing.
"""
def set_filtered_out_folders(files: List[str]) -> None: ...