import subprocess
import tqdm

iteration_count = 10000

def get_result():
    output = subprocess.check_output(f"python ./utils/testPerf.py {iteration_count}", shell=True)
    result = float(output)

    return result / iteration_count * 1e9

# run 10 times and calculate statistics

results = []

# for i in tqdm.tqdm(range(10)):
    # results.append(get_result())

results = [get_result() for i in range(10)]

average = sum(results) / len(results)
dev = (sum([(x - sum(results) / len(results)) ** 2 for x in results]) / len(results)) ** 0.5

print(f"Average {average:.2f} ns, dev {dev:.2f} ns")
