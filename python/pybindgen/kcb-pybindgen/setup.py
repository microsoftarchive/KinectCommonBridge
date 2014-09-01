import os
from distutils.core import setup, Extension
from interface import generate_module

LIB = "d:/projects/KinectCB/KinectCommonBridge-v2/Release"
INC = [".", "C:/Program Files/Microsoft SDKs/Kinect/v2.0-PublicPreview1407/inc",
       "c:/Program Files (x86)/Windows Kits/8.0/Include/um", "c:/Program Files (x86)/Windows Kits/8.0/Include/shared",
       "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC/include"]

try:
    os.mkdir("build")
except OSError:
    pass
module_fname = os.path.join("build", "binding-gen.cpp")
with open(module_fname, "wt") as file_:
    print("Generating file {}".format(module_fname))
    generate_module(file_)

# ext = Extension("_KCBv2Lib",
# include_dirs = INC,
# 	library_dirs = [LIB],
# 	sources = ["KCBv2Lib.i", "KCBv2Lib.h"],
# 	swig_opts=['-threads', '-c++'],
# 	language = 'c++',
#     extra_compile_args = ['/Zi', '/EHsc'],
# )


mymodule = Extension('KCBv2Lib',
                     sources=[module_fname, 'KCBv2Lib.cpp', 'KCBv2Frames.cpp', 'KCBv2Sensor.cpp', 'KCBv2List.cpp'],
                     include_dirs=INC,
                     library_dirs=[LIB]
                     #language='c++'
)

setup(name='KCBv2',
      version="0.0",
      description='KCBv2',
      author='Maxim Kostin',
      author_email='dev@maxkostin.ru',
      ext_modules=[mymodule],
)
