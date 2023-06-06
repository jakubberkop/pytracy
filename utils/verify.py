import subprocess
import glob
import tempfile

from pathlib import Path

capture_bin_path    = "./bin/capture-release"
csv_export_bin_path = "./bin/csvexport-release"


def run_test_case(test_case_path):

    # Create a temporary directory
    tempdir = Path(tempfile.gettempdir())
    capture_path = tempdir / "capture"

    # start capture and return a process object
    capture_program = subprocess.Popen([capture_bin_path, "-f", "-o", capture_path, "-s", "1"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    # run test case
    subprocess.check_output(["python", test_case_path])

    # stop capture
    capture_program.wait(1)

    # export capture to csv
    output = subprocess.check_output([csv_export_bin_path, capture_path])

    # Verify that the output is correct utf-8 string and return it
    # If the output is not correct explain why

    try:
        output = output.decode("utf-8")
        return True
    except UnicodeDecodeError as e:
        # Try decoding the output without being strict
        # instead, replace the invalid bytes with the unicode replacement character
        output = output.decode("utf-8", "replace")

        # Find all occurrences of the unicode replacement character
        # and print all of them
        for i in range(len(output)):
            if output[i] != "�":
                continue

            # search for the nearest newline character
            start = output.rfind("\n", 0, i)
            end = output.find("\n", i)

            print("Found error near: ", output[start:end])

    return False


if __name__ == "__main__":

    # Find all test cases in the tests folder
    # and run them one by one

    tests = glob.glob("./tests/*.py") 

    for test_case_path in tests:
        print("Running test case: ", test_case_path, "... ", end="", flush=True)
        result = run_test_case(test_case_path)
        print(result and "OK" or "FAIL")

