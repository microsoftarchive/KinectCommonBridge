#! python3.4

import os
from distutils.core import setup, Extension
from interface import generate_module

LIB = "d:/projects/KinectCB/KinectCommonBridge-v2/Release"

INC_KINECT = ["C:/Program Files/Microsoft SDKs/Kinect/v2.0-PublicPreview1407/inc"]

INC_2012 = [
    "C:/Program Files (x86)/Windows Kits/8.0/Include/um",
    "C:/Program Files (x86)/Windows Kits/8.0/Include/shared",
    "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC/include"
]

INC_2010 = [
    #"C:/Program Files (x86)/Windows Kits/v7.0a/Include/um",
    #"C:/Program Files (x86)/Windows Kits/v7.0a/Include/shared",
    "C:/Program Files (x86)/Windows Kits/8.1/Include/shared"
    #"D:/dev/msvc/2010/VC/include"
]

#  Set of includes that is going to be used
INC = ["."] + INC_KINECT + INC_2010

MACROS = [# these will make 'KINECT_CB' equal '__declspec(dllexport)'
          ('KCBV2_EXPORTS', None), ("KCBV2LIB_EXPORTS", None),
          # others are taken from Visual Studio 'cl.exe' execution log
          ("WIN32", None), ("NDEBUG", None), ("_WINDOWS", None),
          ("_USRDLL", None), ("_WINDLL", None), ("_UNICODE", None), ("UNICODE", None)]

EXTRA_COMPILE_ARGS_MACRO = ["/D WIN32", "/D NDEBUG", "/D _WINDOWS", "/D _USRDLL", "/D KCBV2LIB_EXPORTS", "/D _WINDLL", "/D _UNICODE", "/D UNICODE"]

EXTRA_COMPILE_ARGS = ["/EHsc", "/Zi", "/nologo", "/W3", "/WX-", "/O2", "/Oi", "/Oy-",
                      "/GL", "/Gm-", "/EHsc", "/MD", "/GS-", "/Gy",
                      "/fp:precise", "/Zc:wchar_t", "/Zc:forScope",
                      #"/Fo\"Release\\", "/Fd\"Release\vc100.pdb",
                      "/Gd", "/TP", "/analyze-", "/errorReport:prompt"
] # + EXTRA_COMPILE_ARGS_MACRO


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
# library_dirs = [LIB],
# sources = ["KCBv2Lib.i", "KCBv2Lib.h"],
# 	swig_opts=['-threads', '-c++'],
# 	language = 'c++',
#     extra_compile_args = ['/Zi', '/EHsc'],
# )


mymodule = Extension('KCBv2Lib',
                     sources=[module_fname,  'stdafx.cpp', 'dllmain.cpp', 'KinectList.cpp', 'KCBFrames.cpp', 'KCBSensor.cpp', 'KCBv2Lib.cpp'],
                     include_dirs=INC,
                     library_dirs=[LIB],
                     define_macros=MACROS,
                     extra_compile_args=EXTRA_COMPILE_ARGS
                     #language='c++'
)

setup(name='KCBv2',
      version="0.0",
      description='KCBv2',
      author='Maxim Kostin',
      author_email='dev@maxkostin.ru',
      ext_modules=[mymodule],
)
