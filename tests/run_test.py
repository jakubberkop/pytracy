import argparse
import csv
import dataclasses
import glob
import os
import subprocess
import time
import unittest

from pathlib import Path
from typing import Callable, Dict, List, Optional

root_dir = Path(__file__).parent.parent
csv_export_path = root_dir / "tracy/csvexport/build/win32/x64/Release/csvexport.exe"

@dataclasses.dataclass
class FunctionData:
    name: str
    src_file: str
    src_line: int
    total_ns: int
    total_perc: float
    counts: int
    mean_ns: int
    min_ns: int
    max_ns: int
    std_ns: int


class TracyReport:
    def __init__(self, file: Path):
        self.file = file

    def parse(self) -> bool:
        # Export the capture to a csv file
        csv_output: bytes = subprocess.check_output([csv_export_path.resolve(), self.file.resolve()])

        if not self.verify_output_is_utf8(csv_output):
            print("Capture output is not valid utf-8")
            return False

        # Parse the csv_output
        self.function_data: Dict[str, FunctionData] = {}
        for row in csv.reader(csv_output.decode("utf-8").splitlines()[1:]):
            data = FunctionData(*row)
            self.function_data[data.name] = data

        return True

    def verify_output_is_utf8(self, output: bytes) -> bool:
        # Verify that the output is correct utf-8 string and return it
        # If the output is not correct explain why
        try:
            output.decode("utf-8")
            return True
        except UnicodeDecodeError:
            # Try decoding the output without being strict
            # instead, replace the invalid bytes with the unicode replacement character
            output_str = output.decode("utf-8", "replace")

            # Find all occurrences of the unicode replacement character
            # and print all of them
            for i in range(len(output_str)):
                if output_str[i] != "�":
                    continue

                # search for the nearest newline character
                start = output_str.rfind("\n", 0, i)
                end = output_str.find("\n", i)

                print("Found error near: ", output_str[start:end])

        return False

    def number_of_calls_to_function(self, function_name: str) -> int:
        if function_name not in self.function_data:
            return 0

        return int(self.function_data[function_name].counts)

class CustomTestRunner():

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        if os.name == "nt":
            self.capture_path = root_dir / "tracy/capture/build/win32/x64/Release/capture.exe"
        else:
            assert NotImplementedError("Only Windows is supported")

        # Assert that the capture executable exists
        assert self.capture_path.exists(), f"Capture executable not found at {self.capture_path}"

        self.capture_result_path = root_dir / "capture_result.tracy"

        self.verifications: Dict[str, Callable[[TracyReport], bool]] = {
            "test_control_flow.py": self.verify_control_flow,
            "test_library_integrations.py": self.verify_library_integrations,
            "test_mark_function.py": self.verify_mark_function,
            "test_threads.py": self.verify_threads,
        }

    def start_capture(self):
        # Start the capture process
        self.capture_process = subprocess.Popen([self.capture_path.resolve(), "-f", "-o", self.capture_result_path.resolve(), "-s", "1" ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)


    def wait_for_capture_and_verify(self) -> Optional[TracyReport]:
        stdout, stderr = self.capture_process.communicate()
        capture_exit_code = self.capture_process.wait()

        if capture_exit_code != 0:
            print("Capture process failed with exit code: ", capture_exit_code)
            print("stdout: ", stdout)
            print("stderr: ", stderr)
            return None

        self.result = TracyReport(self.capture_result_path)

        if not self.result.parse():
            print("Failed to parse capture result")
            return None

        return self.result

    def run(self, test_suites: List[Path]):

        for test_suite in test_suites:
            self.start_capture()
            test_file_name = test_suite.name

            print(f"Started  {test_file_name}")

            test_runner_process = subprocess.Popen(["py", __file__, "--test-suite", test_file_name], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
            stdout, stderr = test_runner_process.communicate()

            if "Ran 0 tests" in stdout:
                print(f"Test suite {test_file_name} did not run any tests")
                print(stdout)
                print(stderr)
                continue

            exit_code = test_runner_process.wait()

            if exit_code != 0:
                print(f"Test suite {test_file_name} failed")

            capture_result = self.wait_for_capture_and_verify()

            if not capture_result:
                print(f"Capture failed for {test_file_name}")

            if test_file_name in self.verifications:
                self.verifications[test_file_name](capture_result)
            else:
                print(f"No verification for test suite {test_file_name}")

            print(f"Finished {test_file_name}")

    def check_folders(self, capture_result: TracyReport, expected_folders: List[Path]) -> bool:
        return_value = True

        for function in capture_result.function_data.values():
            scr_file_path = Path(function.src_file).parent

            if scr_file_path not in expected_folders:
                print(f"Function {function.name} is in unexpected folder {scr_file_path}")
                return_value = False

        return return_value

    def check_function_counts(self, capture_result: TracyReport, expected_counts: Dict[str, int]) -> bool:
        for k, v in expected_counts.items():
            if capture_result.number_of_calls_to_function(k) != v:
                print(f"Expected {v} calls to {k}, got {capture_result.number_of_calls_to_function(k)}")
                return False

        return True

    def verify_control_flow(self, capture_result: TracyReport):
        expected_counts = {
            "a": 1,
            "b": 1,
            "aaa": 1,
            "bb": 2,
            "pass_through": 1,
            "raiser": 1,
        }

        return self.check_function_counts(capture_result, expected_counts) \
           and self.check_folders(capture_result, [root_dir / "tests"])

    def verify_library_integrations(self, capture_result: TracyReport):
        expected_counts = {}
        return self.check_function_counts(capture_result, expected_counts) \
           and self.check_folders(capture_result, [root_dir / "tests"])

    def verify_mark_function(self, capture_result: TracyReport):
        expected_counts = {
            "func": 1,
            "func_args": 1,
            "func_kwargs": 2,
        }

        if len(capture_result.function_data) != 3:
            print("Error in verify_mark_function: ")
            print("Expected 3 functions to be recorded, got ", len(capture_result.function_data))
            return False

        return self.check_function_counts(capture_result, expected_counts) \
           and self.check_folders(capture_result, [root_dir / "tests"])

    def verify_threads(self, capture_result: TracyReport):
        expected_counts = {
            "worker": 10,
            "test_function": 20,
        }
        return self.check_function_counts(capture_result, expected_counts) \
           and self.check_folders(capture_result, [root_dir / "tests"])

def run_test_suite(test_suite: Path):
    dir = Path(test_suite).parent
    file = Path(test_suite).name

    test_suite = unittest.TestLoader().discover(dir, pattern=file)
    unittest.TextTestRunner().run(test_suite)
    time.sleep(0.5)


if __name__ == "__main__":
    args = argparse.ArgumentParser()
    args.add_argument("--test-suite", type=Path, help="The test suite to run")
    args = args.parse_args()

    if args.test_suite:
        run_test_suite(args.test_suite)
    else:
        # test_suite = unittest.TestLoader().discover(".", pattern="test_*.py")
        import glob
        test_suites = [Path(path) for path in glob.glob("test_*.py")]
        CustomTestRunner().run(test_suites)

        # CustomTestRunner().run(test_suite)