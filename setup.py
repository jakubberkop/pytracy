import glob
import os
import shutil
import sys

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install
from subprocess import check_call, run
from pathlib import Path

debug = False

class CMakeBuildExt(build_ext):
	def run(self):
		# Ensure that cmake is installed
		try:
			check_call(['cmake', '--version'])
		except OSError:
			raise RuntimeError(
				"cmake is not installed. Please install cmake and try again."
			)
		
		# current directory
		cwd = os.getcwd()

		# Do a clean build every time
		# if os.path.exists(self.build_temp):
		# 	shutil.rmtree(self.build_temp)

		print("Building Tracy...")

		os.makedirs(self.build_temp, exist_ok=True)

		# Change directory to the location where Tracy should be cloned
		os.chdir(self.build_temp)

		# Clone Tracy repository using Git
		check_call(['git', 'clone', 'https://github.com/wolfpld/tracy'])

		# # Change directory to Tracy repository
		os.chdir('tracy')

		run(['git', 'checkout', 'v0.11.1'])

		# Run cmake so that it generates shared libraries
		# Compile with flags -fPIC -Wl and --no-undefined
		if os.name == "posix":
			cxx_flags = "-fPIC"
			check_call(['cmake', '.', f'-DCMAKE_CXX_FLAGS={cxx_flags}', '-DCMAKE_BUILD_TYPE=Release'])
		elif os.name == "nt":
			check_call(['cmake', '.', '-DCMAKE_BUILD_TYPE=Release', 'TRACY_STATIC=ON'])
		else:
			raise Exception("Unsupported OS")


		check_call(['cmake', "."])

		# Run make to build Tracy
		check_call(['cmake', '--build', '.', '--config', 'Release'])

		self.include_dirs.append(os.path.join(cwd, self.build_temp, 'tracy', 'public', 'tracy'))

		if os.name == 'nt':
			print("Searching in ", Path.cwd().absolute())
			tracy_lib_path = glob.glob("**/TracyClient.lib", recursive=True)
			print(tracy_lib_path)
			assert len(tracy_lib_path) == 1

			tracy_lib_dir = Path(tracy_lib_path[0]).parent.resolve()

			# Print to stderr
			print(f"Found TracyClient.lib in {tracy_lib_dir}", file=sys.stderr)

			self.library_dirs.append(str(tracy_lib_dir))
		else:
			self.library_dirs.append(os.path.join(cwd, self.build_temp, 'tracy'))

		print("Building capture module...")

		# if os.name == "nt":
		# 	check_call(["cmd", "/C", "vcpkg\\install_vcpkg_dependencies.bat"])
		# 	check_call(["MSBuild.exe", "./capture/build/win32/capture.sln", "-p:Configuration=Release"])
		# elif os.name == "posix":
		# 	print("TODO: Build capture module for Linux")

		# Change directory back to the python package
		os.chdir(cwd)

		super().run()

if os.name == 'nt':
	extra_compile_args = ["/std:c++17"]
	extra_link_args = []
else:
	extra_compile_args = ["-std=c++17"]
	extra_link_args = ["-Wno-undef", "-ldl", "-lm"]

# if debug:
# 	if os.name == "nt":
# 		extra_compile_args.extend(["/Od", "/Zi", "/DEBUG", "/Yd"])
# 		extra_link_args.extend(["/DEBUG", "/Zi"])
# 	# Linux 
# 	elif os.name == "posix":
# 		extra_compile_args.extend(["-O0", "-g3"])
# 		extra_link_args.extend(["-O0", "-g3"])

# 		extra_compile_args.extend(["-fsanitize=address", "-fsanitize=undefined"])
# 		extra_link_args.extend(["-fsanitize=address", "-fsanitize=undefined"])
# 	else:
# 		raise Exception("Unsupported OS")

# Define the custom extension module
extension = Extension(
	'pytracy',
	['src/pyTracy.cpp'],
	include_dirs=[
		"tracy/public/tracy",
		"pybind11/include",
		"src"
	],
	libraries=['TracyClient'],
	extra_compile_args=extra_compile_args,
	extra_link_args=extra_link_args,
)

setup(name = 'pytracy',
	version = '0.0.2rc3',
	cmdclass={
	'build_ext': CMakeBuildExt,
	},
	ext_modules=[extension],
	packages=[],
)