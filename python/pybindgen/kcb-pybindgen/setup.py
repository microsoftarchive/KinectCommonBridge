#! python3.4

import os
from distutils.core import setup, Extension
from interface import generate_module

# Path to Kinect library (.lib)
LIB = ["c:/Program Files/Microsoft SDKs/Kinect/v2.0-PublicPreview1407/Lib/x86"]

# Kinect includes path
INC_KINECT = ["C:/Program Files/Microsoft SDKs/Kinect/v2.0-PublicPreview1407/inc"]

# Windows includes used by VS 2012
INC_2012 = [
    "C:/Program Files (x86)/Windows Kits/8.0/Include/um",
    "C:/Program Files (x86)/Windows Kits/8.0/Include/shared",
    "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC/include"
]

# Windows includes used by VS 2010
INC_2010 = [
    # "C:/Program Files (x86)/Windows Kits/v7.0a/Include/um",
    "C:/Program Files (x86)/Windows Kits/v7.0a/Include/shared",  # can be commented out, however VS makes use of it
    "C:/Program Files (x86)/Windows Kits/8.1/Include/shared",
    "D:/dev/msvc/2010/VC/include"
]

# Set of includes that is going to be used
INC = ["."] + INC_KINECT + INC_2010

# Set of macro definitions required for proper compilation
MACROS = [('KCBV2_EXPORTS', None), ("KCBV2LIB_EXPORTS", None),  # will make 'KINECT_CB' equal '__declspec(dllexport)'
          ("NDEBUG", None),  # making release build
          ("WIN32", None), ("_WINDOWS", None), ("_USRDLL", None),  # taken from Visual Studio 'cl.exe' execution log
          ("_WINDLL", None), ("_UNICODE", None), ("UNICODE", None)]

# NOT USED. The same set of macro definitions as 'MACROS' but prepared to be provided as compile arguments
EXTRA_COMPILE_ARGS_MACRO = ["/D WIN32", "/D NDEBUG", "/D _WINDOWS", "/D _USRDLL", "/D KCBV2LIB_EXPORTS", "/D _WINDLL",
                            "/D _UNICODE", "/D UNICODE"]

# See:
# - "http://msdn.microsoft.com/en-us/library/fwkeyyhe.aspx"
# - "http://msdn.microsoft.com/en-us/library/610ecb4h.aspx"
EXTRA_COMPILE_ARGS = ["/EHsc",  # catch only c++ exceptions
                      "/Zi",  # generates .pdb file
                      "/nologo",  # suppress extra banners
                      "/W3",  # Warning level >=2
                      "/WX-",  # Don't treat compiler warnings as errors
                      "/Od",   # Disable optimization
                      "/Oy-",  # Disable function pointers omission
                      "/Gm-",  # Disable minimal rebuild
                      "/MD",  # Use multithreaded release runtime library
                      "/GS-",  # Disable buffer security check
                      "/Gy",  # Enables function-level linking
                      "/fp:precise",  # Floating-point calculation mode
                      "/Zc:wchar_t",  # Map 'wchar_t' to '__wchar_t'
                      "/Zc:forScope",  # Dons't use for initializer outside of 'for' scope
                      #"/Fo\"Release\\", "/Fd\"Release\vc100.pdb",
                      "/Gd",  # Specifies __cdecl calling convention
                      "/TP",  # File names are C++ source files, even w/o .cpp or .cxx extension
                      "/analyze-",  # Disables code analysis
                      "/errorReport:none",  # Disable reporting internal compiler errors

                      # linker options
                      "/link",
                      "/OUT:\"D:\projects\KinectCB\KinectCommonBridge-v2-2010\KCBv2Lib\Release\KCBv2.dll\"",
                      "/INCREMENTAL:NO",
                      "/SUBSYSTEM:WINDOWS",
                      "/OPT:REF",
                      "/OPT:ICF",
                      "/LTCG",
                      "/TLBID:1",
                      "/DYNAMICBASE",
                      "/NXCOMPAT",
                      "/MACHINE:X86",
                      "/DLL"
]  # + EXTRA_COMPILE_ARGS_MACRO

# Source files included into compilation process
SRC = ['../../../KCBv2/stdafx.cpp',
       '../../../KCBv2/dllmain.cpp',
       '../../../KCBv2/KinectList.cpp',
       '../../../KCBv2/KCBFrames.cpp',
       '../../../KCBv2/KCBSensor.cpp',
       '../../../KCBv2/KCBv2Lib.cpp']

try:
    os.mkdir("build")
except OSError:
    pass
module_fname = os.path.join("build", "binding-gen.cpp")
with open(module_fname, "wt") as file_:
    print("Generating file {}".format(module_fname))
    generate_module(file_)

kcbmodule = Extension('KCBv2Lib',
                     sources=[module_fname] + SRC,
                     include_dirs=INC,
                     library_dirs=LIB,
                     define_macros=MACROS,
                     extra_compile_args=EXTRA_COMPILE_ARGS
)

setup(name='KCBv2',
      version="0.1",
      description='KCBv2',
      author='Maxim Kostin',
      author_email='dev@maxkostin.ru',
      ext_modules=[kcbmodule],
)
