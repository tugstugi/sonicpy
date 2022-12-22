import glob
from setuptools import setup, Extension

VERSION = '0.1.2'

pysonic_lib = Extension(name='sonic',
                        define_macros=[('VERSION', '"%s"' % VERSION)],
                        sources=[f for f in glob.glob('src/*.c')] + ['external/sonic/sonic.c'])

setup(name='sonicpy',
      version=VERSION,
      license='Apache',
      author="Erdene-Ochir Tuguldur",
      author_email='tugstugi@gmail.com',
      description='Sonic Python library',
      url='https://github.com/tugstugi/pysonic',
      ext_modules=[pysonic_lib],
      include_dirs=['external/sonic/']
      )
