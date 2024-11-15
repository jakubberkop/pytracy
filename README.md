# pytracy

## Installation 

Download the pytracy package using the following command:
`pip install pytracy`

To connect to a profiled application, you will need the Tracy profiler. Follow the steps below:

Download Tracy profiler from [Tracy Github](https://github.com/wolfpld/tracy/releases/tag/v0.10). 
- For Windows, an executable is provided by the author.
- For Linux, you need to build it from the source (refer to the documentation for build instructions).
- Use the provided script for Ubuntu to install dependencies and build the Tracy Profiler - scripts\build_tracy_ubuntu.sh TODO: Should this be included?

## Usage
1. Enable the profiler
```(python)
import pytracy
pytracy.enable_tracing(True)
```
2. Start your script
3. Start Tracy profiler and click connect

## Limitations:
- Relatively high overhead. Enabling PyTracy currently average ~380ns per function call, compared to ~80ns for an untraced function call. The theoretical limit is an overhead of ~250ns per function call. This has been measured by giving the python profiler an empty function as a profiler callback.
- set_tracing_mode doesn't work for already started threads. It has to be called before other threads are started.
- No visibility into functions run using multiprocessing.

## Goals for 0.3.0 release
- Support freethreaded 3.13+ python versions.
- Shutdown and startup during program execution
- Changing of the tracing mode during runtime
- Easy integration with python logging

## Goals for 0.4.0 release
- Offline mode - save trace to file
- Preinitialize all of the code objects, using gc module

## Performance measurements
`py .\utils\perf.py`

Base				    		Average	80.00 ns	dev 45.87 ns
python empty profile_function	Average 246.06 ns	dev 47.37 ns
PyTracy Debug(19587f466)		Average 3390.63 ns	dev 375.23 ns
PyTracy Release(19587f466)		Average 1441.24 ns	dev 66.48 ns
PyTracy Release(c6c8c803e)		Average 950.58 ns	dev 113.11 ns
PyTracy Release(a42aa86b5)		Average 760.63 ns	dev 49.53 ns
