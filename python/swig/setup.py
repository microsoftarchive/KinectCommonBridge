# -*- coding: utf-8 -*-
import distutils
from distutils.core import setup, Extension

LIB = "d:/projects/KinectCB/KinectCommonBridge-v2/Release"
INC = ["C:/Program Files/Microsoft SDKs/Kinect/v2.0-DevPreview1406/inc", "c:/Program Files (x86)/Windows Kits/8.0/Include/um", "c:/Program Files (x86)/Windows Kits/8.0/Include/shared", "C:/Program Files (x86)/Microsoft Visual Studio 11.0/VC/include"]

ext = Extension("_KCBv2Lib",
	include_dirs = INC,
	library_dirs = [LIB],
	sources = ["KCBv2Lib.i", "KCBv2Lib.h"],
	swig_opts=['-threads', '-c++'],
	language = 'c++',
    extra_compile_args = ['/Zi', '/EHsc'],
)

setup(name = "KCB SWIG test",
     version = "0.007",
     ext_modules = [ext]
);