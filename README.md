# pytracy

## Installation 

Download the pytracy package using the following command:
`pip install pytracy`

To connect to a profiled application, you will need the Tracy profiler. Follow the steps below:

Download Tracy profiler from [Tracy Github](https://github.com/wolfpld/tracy/releases/tag/v0.10). 
- For Windows, an executable is provided by the author.
- For Linux, you need to build it from the source (refer to the documentation for build instructions).
- Use the provided script for Ubuntu to install dependencies and build the Tracy Profiler - scripts\build_tracy_ubuntu.sh

## Usage
1. Enable the profiler
```(python)
import pytracy
pytracy.set_tracing_mode(pytracy.TracingMode.All)
```
2. Start your script
3. Start Tracy profiler and click connect

## Limitations:
- Debugging is not possible when using *TracingMode.All* mode. This is the limitation of python tracing api.
- High overhead when using *TracingMode.All* mode. This is the limitation of python tracing api.
- Limited support for multithreading. Tracing mode changes are local to the thread. TODO
- set_tracing_mode doesn't work for already started threads. It to be called before other threads are started.
- No visibility into functions run using multiprocessing.

# Mark function you want to measure
@pytracy.mark_function
def function_you_want_to_measure():
	pass

function_you_want_to_measure()
```

2. Measure all functions - very slow (python limitation)

```(python)
import pytracy
pytracy.set_tracing_mode(pytracy.TracingMode.All)
```

### Start measurement

Then start the program you want to measure.
Start Tracy profiler, program you want to measure should be shown in the list, if not set its ip address manually and click connect.