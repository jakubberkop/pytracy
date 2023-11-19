import os
import shutil

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install
from subprocess import check_call, run

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
		if os.path.exists(self.build_temp):
			shutil.rmtree(self.build_temp)

		print("Building Tracy...")

		os.makedirs(self.build_temp, exist_ok=True)

		# Change directory to the location where Tracy should be cloned
		os.chdir(self.build_temp)

		# Clone Tracy repository using Git
		check_call(['git', 'clone', 'https://github.com/wolfpld/tracy'])

		# # Change directory to Tracy repository
		os.chdir('tracy')

		run(['git', 'checkout', 'v0.10'])

		# Run cmake so that it generates shared libraries
		# Compile with flags -fPIC -Wl and --no-undefined
		if os.name == "posix":
			cxx_flags = "-fPIC"
		elif os.name == "nt":
			cxx_flags = ""
		else:
			raise Exception("Unsupported OS")

		check_call(['cmake', '.', f'-DCMAKE_CXX_FLAGS={cxx_flags}', '-DCMAKE_BUILD_TYPE=Release'])

		# Run make to build Tracy
		check_call(['cmake', '--build', '.', '--config', 'Release'])

		self.include_dirs.append(os.path.join(cwd, self.build_temp, 'tracy', 'public', 'tracy'))
		
		if os.name == 'nt':
			self.library_dirs.append(os.path.join(cwd, self.build_temp, 'tracy', 'Release'))
		else:
			self.library_dirs.append(os.path.join(cwd, self.build_temp, 'tracy'))

		# Change directory back to the python package
		os.chdir(cwd)

		super().run()

class CustomInstall(install):
	def run(self):
		super().run()

if os.name == 'nt':
	extra_compile_args = []
	extra_link_args = []
else:
	extra_compile_args = []
	extra_link_args = ["-Wno-undef", "-ldl", "-lm"]

if debug:
	if os.name == "nt":
		extra_compile_args.extend(["/Od", "/Zi", "/DEBUG", "/Yd"])
		extra_link_args.extend(["/DEBUG", "/Zi"])
	# Linux 
	elif os.name == "posix":
		extra_compile_args.extend(["-O0", "-g3"])
		extra_link_args.extend(["-O0", "-g3"])

		extra_compile_args.extend(["-fsanitize=address", "-fsanitize=undefined"])
		extra_link_args.extend(["-fsanitize=address", "-fsanitize=undefined"])
	else:
		raise Exception("Unsupported OS")

# Define the custom extension module
extension = Extension(
	'pytracy',
	['src/pyTracy.cpp'],
	include_dirs=["tracy/public/tracy"],
	libraries=['TracyClient'],

	extra_compile_args=extra_compile_args,
	extra_link_args=extra_link_args,
)

setup(name = 'pytracy',
	version = '0.0.1-8',
	cmdclass={
	'build_ext': CMakeBuildExt,
	'install': CustomInstall,
	},
	ext_modules=[extension],
)