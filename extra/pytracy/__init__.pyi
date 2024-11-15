"""
Python Tracy integration https://github.com/wolfpld/tracy.
This module provides a way to trace function calls in Python code.
It can be used get a time profile of the code, or to debug the flow of the code.

Importing this module initializes the default filter_out_folders and calls threading.settrace to set the tracing function.
This means that debugging doesn't work after importing this module.
This undesirable, so it is recommended to only import this module when you want to start tracing.

"""
from typing import List

"""
Starts or stops tracing.

:param enabled: Whether to enable or disable tracing.
"""
def enable_tracing(enabled: bool) -> None: ...

"""
Sets which folders should be ignored when tracing.
Resets previously set_filtered_out_folders.

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