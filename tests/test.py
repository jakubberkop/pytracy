print("Before import")

import pytracy

def enabler():
	print("In enabler")
	pytracy.enableTracing()

import time

enabler()

def aaa(i):
	bb(i)

	try:
		pass_through()
		raiser()
	except:
		pass

	bb(i)

def bb(i):
	pass

def pass_through():
	raiser()

def raiser():
	raise Exception("Test")


aaa(10)

time.sleep(1)