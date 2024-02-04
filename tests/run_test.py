import argparse
import pathlib
import time
from typing import List
import unittest
import subprocess
import os

class CustomTestRunner(): # unittest.TextTestRunner):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.root_dir = pathlib.Path(__file__).parent.parent

        if os.name == "nt":
            self.capture_path = self.root_dir / "tracy/capture/build/win32/x64/Release/capture.exe"
            self.csv_export_path = self.root_dir / "tracy/csvexport/build/win32/x64/Release/csvexport.exe"
        else:
            assert NotImplementedError("Only Windows is supported")

        # Assert that the capture executable exists
        assert self.capture_path.exists(), f"Capture executable not found at {self.capture_path}"

        self.capture_result_path = self.root_dir / "capture_result.tracy"

    def start_capture(self):
        # Start the capture process
        self.capture_process = subprocess.Popen([self.capture_path.resolve(), "-f", "-o", self.capture_result_path.resolve(), "-s", "1" ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

    def wait_for_capture_and_verify(self):
        stdout, stderr = self.capture_process.communicate()
        capture_exit_code = self.capture_process.wait()

        if capture_exit_code != 0:
            print("Capture process failed with exit code: ", capture_exit_code)
            print("stdout: ", stdout)
            print("stderr: ", stderr)
            return False

        # Export the capture to a csv file
        csv_output = subprocess.check_output([self.csv_export_path.resolve(), self.capture_result_path.resolve()])

        # Verify that the output is correct utf-8 string and return it
        # If the output is not correct explain why
        try:
            output = csv_output.decode("utf-8")
            return True
        except UnicodeDecodeError as e:
            # Try decoding the output without being strict
            # instead, replace the invalid bytes with the unicode replacement character
            output = csv_output.decode("utf-8", "replace")

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


    def run(self, test_suites: List[pathlib.Path]):
        for test_suite in test_suites:
            self.start_capture()
            test_file_name = test_suite.name

            test_runner_process = subprocess.Popen(["py", __file__, "--test-suite", test_file_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            stdout, stderr = test_runner_process.communicate()
            print(stdout)
            print(stderr)

            exit_code = test_runner_process.wait()

            if exit_code != 0:
                print(f"Test suite {test_file_name} failed")

            capture_result = self.wait_for_capture_and_verify()

            if not capture_result:
                print(f"Capture failed for {test_file_name}")



def run_test_suite(test_suite):
    dir = pathlib.Path(test_suite).parent
    file = pathlib.Path(test_suite).name

    print(f"Running test suite: {test_suite}")
    test_suite = unittest.TestLoader().discover(dir, pattern=file)
    unittest.TextTestRunner().run(test_suite)
    # print("waiting for 5 seconds")
    time.sleep(1)
    print("Test suite finished")

if __name__ == "__main__":
    args = argparse.ArgumentParser()
    args.add_argument("--test-suite", type=str, help="The test suite to run")
    args = args.parse_args()

    if args.test_suite:
        run_test_suite(args.test_suite)
    else:
        # test_suite = unittest.TestLoader().discover(".", pattern="test_*.py")
        import glob
        test_suites = [pathlib.Path(path) for path in glob.glob("test_*.py")]
        CustomTestRunner().run(test_suites)

        # CustomTestRunner().run(test_suite)