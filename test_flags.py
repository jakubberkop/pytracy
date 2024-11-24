import os
import shutil
import subprocess
import itertools
import time
from typing import List

import tqdm
import pandas as pd

iteration_count = 100

def get_result():
    time.sleep(1)
    output = subprocess.check_output(f"python3.13t ./utils/test_flags_perf.py {iteration_count}", shell=True)
    result = float(output)
    return result

    # return result / iteration_count # * 1e9

def run_perf(case):
	os.environ["SPECIAL_ARGS"] = " ".join(case)
	subprocess.check_output("python3.13t -m pip uninstall pytracy -y", shell=True)
	# subprocess.check_output("python3.13t setup.py install", shell=True)
	shutil.rmtree("build/temp.win-amd64-cpython-313t/Release/src", ignore_errors=True)

	shutil.rmtree("build/lib.linux-x86_64-cpython-313t", ignore_errors=True)
	shutil.rmtree("build/temp.linux-x86_64-cpython-313t/src", ignore_errors=True)


	res = subprocess.run("python3.13t setup.py install --force", shell=True)
	if res.returncode != 0:
		print("Failed to install")
		raise Exception("Failed to install")

	results = [get_result() for i in range(5)]
	return results

	# average = sum(results) / len(results)
	# dev = (sum([(x - sum(results) / len(results)) ** 2 for x in results]) / len(results)) ** 0.5


def run():
	# subprocess.run("taskkill /IM python3.13t /F", shell=True)
	subprocess.check_output("python3.13t -m pip uninstall pytracy -y", shell=True)
	subprocess.check_output("python3.13t setup.py install", shell=True)
	time.sleep(1)
	return run_perf()

# PYTRACY_FILTERING_OLD
# PYTRACY_FILTERING_ATOMIC
# PYTRACY_FILTERING_GENERATIONS
# PYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION

OPTIONS = [
	[ "-DPYTRACY_CONSTANT_FUNCTION_DATA", "-DPYTRACY_CONSTANT_FUNCTION_DATA_DISABLED"],
	[ "-DPYTRACY_FILTERING_OLD", "-DPYTRACY_FILTERING_ATOMIC", "-DPYTRACY_FILTERING_GENERATIONS" ],
	[ "-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION", "-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION_DISABLED" ],
]

OPTIONS2 = [
	[ "-DPYTRACY_CONSTANT_FUNCTION_DATA_DISABLED"],
	[ "-DPYTRACY_FILTERING_OLD", "-DPYTRACY_FILTERING_ATOMIC", "-DPYTRACY_FILTERING_GENERATIONS" ],
	[ "-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION"],
]

OPTIONS = OPTIONS2

cases = list(itertools.product(*OPTIONS))

if os.path.exists("results.txt"):
	os.remove("results.txt")

INTERESTING = [
	"-DPYTRACY_CONSTANT_FUNCTION_DATA",
	"-DPYTRACY_FILTERING_OLD",
	"-DPYTRACY_FILTERING_ATOMIC",
	"-DPYTRACY_FILTERING_GENERATIONS",
	"-DPYTRACY_DEDUPLICATE_FUNCTION_DATA_CREATION",
]

rows = []

# with open("results2.txt", "w+") as f:
import random


random.shuffle(cases)

def case_to_folder_name(case):
	return "__".join([c[1:] for c in case])

def build_cast(cast: List[str]):
	os.environ["SPECIAL_ARGS"] = " ".join(case)

	output_dir = "wheels/" + case_to_folder_name(cast)
	os.makedirs(output_dir, exist_ok=True)

	# subprocess.run("taskkill /IM git.exe /F", shell=True)
	# if os.path.exists("build"):
	# 	shutil.rmtree("build")

	# Remove the build\temp.win-amd64-cpython-313t\Release folder, excluding the tracy folder
	# shutil.rmtree("build/temp.win-amd64-cpython-313t/Release/src", ignore_errors=True)
	# ls = os.listdir("build/temp.win-amd64-cpython-313t/Release")
	# for f in ls:
	# 	if f != "tracy":
	# 		shutil.rmtree(f"build/temp.win-amd64-cpython-313t/Release/{f}", ignore_errors=True)
	# shutil.rmtree(f"build/lib.win-amd64-cpython-313t")

	os.environ["SPECIAL_ARGS"] = " ".join(case)
	subprocess.check_output("python3.13t -m pip uninstall pytracy -y", shell=True)
	# subprocess.check_output("python3.13t setup.py install", shell=True)
	shutil.rmtree("build/temp.win-amd64-cpython-313t/Release/src", ignore_errors=True)

	shutil.rmtree("build/lib.linux-x86_64-cpython-313t", ignore_errors=True)
	shutil.rmtree("build/temp.linux-x86_64-cpython-313t/src", ignore_errors=True)

	shutil.rmtree(f"dist", ignore_errors=True)

	subprocess.run("python3.13t setup.py bdist_wheel", shell=True, check=True)

	# copy the wheel to the output directory from dist/ folder
	ls = os.listdir("dist")
	for f in ls:
		shutil.copy(f"dist/{f}", output_dir)

# for case in tqdm.tqdm(cases):
# 	build_cast(case)


cases = cases * 3
for case in tqdm.tqdm(cases):


	
	# os.environ["SPECIAL_ARGS"] = " ".join(case)

	# Add a row to the dataframe

	print(f"Running with {case}")
	print("Folder name:", case_to_folder_name(case))
	# build_cast(case)
	for res in run_perf(case):
		# add a row to the dataframe
		row = {}

		for c in case:
			if c in INTERESTING:
				row[c] = 1
			else:
				row[c] = 0

		row["time"] = res

		rows.append(row)


	print(f"Running with {case}")
	# res = run()
	print(res)

	df = pd.DataFrame(rows)
	# Save the dataframe to a CSV file
	df.to_csv("results7.csv")
	del df


	# f.write(f"Case: {case}\n")
	# f.flush()
	# f.write(res)
	# f.flush()


