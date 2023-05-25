import os
import shutil

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install
from subprocess import check_call

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

		os.makedirs(self.build_temp)

		# Change directory to the location where Tracy should be cloned
		os.chdir(self.build_temp)

		# Clone Tracy repository using Git
		check_call(['git', 'clone', 'https://github.com/wolfpld/tracy'])

		# Change directory to Tracy repository
		os.chdir('tracy')

		# Run cmake so that it generates shared libraries
		# Compile with flags -fPIC -Wl and --no-undefined
		check_call(['cmake', '.', '-DCMAKE_CXX_FLAGS=-fPIC'])

		# Run make to build Tracy
		check_call(['make'])

		self.include_dirs.append(os.path.join(cwd, self.build_temp, 'tracy', 'public', 'tracy'))
		# This build results in tracy/libTracyClient.a, add this to the library_dirs
		self.library_dirs.append(os.path.join(cwd, self.build_temp, 'tracy'))		

		# Change directory back to the python package
		os.chdir(cwd)

		super().run()

class CustomInstall(install):
	def run(self):
		super().run()


# Define the custom extension module
extension = Extension(
	'pytracy',
	['src/pyTracy.cpp'],
	include_dirs=["tracy/public/tracy"],
	libraries=['TracyClient'],
	# Add -lm to link with math library
	extra_link_args=['-lm', "-Wno-undef"],

)

setup(name = 'pytracy',
	version = '0.0.1-5',
	cmdclass={
	'build_ext': CMakeBuildExt,
	'install': CustomInstall,
	},
	ext_modules=[extension],
	# copy the shared library to the python package
	# data_files=[('shared', ['libTracyClient.so'])],
)