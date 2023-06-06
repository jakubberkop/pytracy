import pytracy
import threading

pytracy.enableTracing()

def a(i):
    b(i)

def b(i):
    pass

def test_function():
    a(10)

def worker(num):
    pytracy.enableTracing()
    print(f'Worker: {num}')

    for i in range(10_000):
        # print(i)
        test_function()
        i += 1
    return

threads = []

for i in range(10):
    t = threading.Thread(target=worker, args=(i,))
    threads.append(t)

for t in threads:
    t.start()

for t in threads:
    test_function()

for t in threads:
    t.join()

