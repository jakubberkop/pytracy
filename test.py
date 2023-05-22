print("Before import")

import PyTracy

import time

def a(i):
	# time.sleep(0.1)
	i -= 1
	if i == 0:
		return
	else:
		a(i)

i = 0

while True:
	i += 1
	print(i)
	a(10)

	time.sleep(0.1)

# for i in range(100):
# 	a(100)