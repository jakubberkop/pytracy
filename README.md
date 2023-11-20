# pytracy


## Installation 

Download tracy profiler
https://github.com/wolfpld/tracy/releases/tag/v0.10

Download pytracy package
`pip install pytracy`

## Usage
There are two ways to use pytracy


### 1. Measure marked functions
```
import pytracy

# Set tracing mode
pytracy.set_tracing_mode(pytracy.TracingMode.MarkedFunctions)

# Mark function you want to measure
@pytracy.mark_function
def function_you_want_to_measure():
	pass
```
Then use function_you_want_to_measure() as usual

### 2. Measure all functions - very slow (python limitation)

```
import pytracy
pytracy.set_tracing_mode(pytracy.TracingMode.All)
```

### Start measurement

Then start the program you want to measure.
Start Tracy profiler, program you want to measure should be shown in the list, if not set its ip address manually