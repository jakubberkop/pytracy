# How to use

## Installation 
`pip install pytracy`

`import pytracy`

## Usage
There are to ways to use pytracy

### Measure marked functions
```
# Set tracing mode
pytracy.set_tracing_mode(pytracy.TracingMode.MarkedFunctions)

# Mark function you want to measure
@pytracy.mark_function
def function_you_want_to_measure():
	pass
```
Then use function_you_want_to_measure() as usual

### Measure all functions - very slow (python limitation)

`pytracy.set_tracing_mode(pytracy.TracingMode.All)`

Then start the program you want to measure.
Start Tracy profiler, program you want to measure should be shown in the list, if not set its ip address manually