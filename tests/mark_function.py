import pytracy
pytracy.set_tracing_mode(pytracy.TracingMode.MarkedFunctions)

def func():
	return None

@pytracy.mark_function
def func_args(a, b):
	return a, b

@pytracy.mark_function
def func_kwargs(a=1, b=2):
	return a, b

assert func() == None
assert func_args(1, 2) == (1, 2)
assert func_kwargs(a=1, b=2) == (1, 2)
assert func_kwargs(b=2, a=1) == (1, 2)