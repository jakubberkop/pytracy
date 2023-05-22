from distutils.core import setup, Extension

ext_modules = [Extension(
  'PyTracy',
  ['pyTracy.c'],
  include_dirs=["/home/m/Desktop/tracy/public/tracy"],
  library_dirs=['lib'],
  libraries=["TracyClient"]
  )
]

setup(name = 'PyTracy', version = '1.0',
  ext_modules=ext_modules,
)