import glob
import os
import shutil
import sys
import sysconfig

from pathlib import Path
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install
from subprocess import check_call, run

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

		if not os.path.exists("tracy"):
			# Clone Tracy repository using Git
			check_call(['git', 'clone', 'https://github.com/wolfpld/tracy'])

		# Change directory to Tracy repository
		os.chdir('tracy')

		run(['git', 'checkout', 'v0.11.1'])

		# Run cmake so that it generates shared libraries
		# Compile with flags -fPIC -Wl and --no-undefined
		if os.name == "posix":
			cxx_flags = "-fPIC"
		elif os.name == "nt":
			cxx_flags = ""
		else:
			raise Exception("Unsupported OS")

		build_type = "Debug" if self.debug else "Release"

		check_call(['cmake', '.', f'-DCMAKE_CXX_FLAGS={cxx_flags}', f'-DCMAKE_BUILD_TYPE={build_type}', '-DTRACY_STATIC=ON'])

		# # Run make to build Tracy
		check_call(['cmake', '--build', '.', '--config', build_type])

		self.include_dirs.append(os.path.join(cwd, self.build_temp, 'tracy', 'public', 'tracy'))

		if os.name == 'nt':
			print("Searching in ", Path.cwd().absolute())
			tracy_lib_path = glob.glob(f"**/TracyClient.lib", recursive=True)
			tracy_lib_path = [x for x in tracy_lib_path if build_type.lower() in str(x).lower()]

			print(tracy_lib_path)
			assert len(tracy_lib_path) == 1

			tracy_lib_dir = Path(tracy_lib_path[0]).parent.resolve()

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

		if os.name == 'nt':
			extra_compile_args = ["/std:c++20"]
			extra_link_args = []
		else:
			extra_compile_args = ["-std=c++20"]
			extra_link_args = ["-Wno-undef", "-ldl", "-lm"]

		# Disable GIL if it is disabled in Python
		if sysconfig.get_config_var('Py_GIL_DISABLED'):
			extra_compile_args.append('-DPy_GIL_DISABLED=1')

		if os.getenv("SPECIAL_ARGS") is not None:
			extra_compile_args.extend(os.getenv("SPECIAL_ARGS").split(" "))
		print("Extra compile args: ", extra_compile_args)


		if self.debug:
			if os.name == "nt":
				extra_compile_args.extend(["/Od", "/Zi", "/DEBUG", "/Yd"])
				extra_link_args.extend(["/DEBUG", "/Zi"])
			# Linux rm
			elif os.name == "posix":
				extra_compile_args.extend(["-O0", "-g3"])
				extra_link_args.extend(["-O0", "-g3"])

				extra_compile_args.extend(["-fsanitize=address", "-fsanitize=undefined"])
				extra_link_args.extend(["-fsanitize=address", "-fsanitize=undefined"])
			else:
				raise Exception("Unsupported OS")

		if self.extensions is not None:
			for extension in self.extensions:
				extension.extra_compile_args.extend(extra_compile_args)
				extension.extra_link_args.extend(extra_link_args)

		super().run()


# Define the custom extension module
extension = Extension(
	'pytracy',
	['src/pyTracy.cpp'],
	include_dirs=[
		"tracy/public/tracy",
		"pybind11/include",
		"src"
	],
	libraries=['TracyClient']
)

setup(name = 'pytracy',
	version = '0.0.2',
	cmdclass={
	'build_ext': CMakeBuildExt,
	},
	ext_modules=[extension],
	packages=[],
)