# pytracy

PyTracy is a Python package that enables you to profile your code using the Tracy profiler, a real-time, nanosecond-resolution tool designed for games and applications. With minimal performance overhead, PyTracy integrates seamlessly with your Python code and visualizes profiling results in the Tracy profiler interface.

## Features
- Low overhead: Tracy profiler introduces only 273ns additional overhead per function call, compared to ~62ns for untraced functions. (See the [Performance](#performance) section for more details.)
- Easy integration: PyTracy integrates seamlessly with your Python code, requiring minimal setup to start profiling.
- Multithreading support: Works with multithreaded applications, including full support for free-threaded Python in version 3.13.

## Installation 

Download the pytracy package using the following command:
`pip install pytracy`

You will also need the Tracy profiler to connect to the profiled application. Download it from the  [Tracy Github releases page](https://github.com/wolfpld/tracy/releases/tag/v0.11.1). 
- For Windows, an executable is provided by the author.
- For Linux, you need to build it from the source. Refer to Tracy documentation for build instructions

## Usage
1. Enable profiling in your Python code:
```(python)
import pytracy
pytracy.enable_tracing(True)
```
2. Run your Python script as usual.
3. Open the Tracy profiler and click "Connect" to start profiling.

## Limitations:
- No support for functions run with multiprocessing.

## Performance
PyTracy code impacts the function call and return time, as it has to send those events to the Tracy profiler.
To measure that acurately, the we have to compare the time taken by an empty function and a function with the Tracy profiler enabled. This has been done in the utils/testPer.py.

We have measured 3 cases:
- No profiling, just the function call and return
- Tracy profiling enabled
- Profiling enabled, but the profiler callback is an empty function

The third scenario is used to show the overhead of the Python tracing API itself. The results are as follows:

| Case | Time taken per 20 000 000 calls (s) | Time taken per call (ns) |
| --- | --- | --- |
| No Profiling | 6,721 | 62 |
| Tracy profiling enabled | 1,247 | 336 |
| Empty profiling function | 6,339 | 317 |

The overhead of the Tracy profiler is 273ns per function call (336ns - 62ns), where most of the overhead is due to the Python tracing API itself. The overhead of the Python tracing API is 255ns per function call (317ns - 62ns).

## Goals for 0.3.0 release
- Add Mac support
- Shutdown and startup during program execution
- Allow runtime switching of tracing modes.
- Easy integration with python logging

## Goals for 0.4.0 release
- Offline mode - save trace to file
- Preinitialize all of the code objects, to avoid additional runtime overhead


