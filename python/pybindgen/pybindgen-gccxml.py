#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import FileCodeSink
from pybindgen.gccxmlparser import ModuleParser

KCB_HEADERS = ["D:/projects/KinectCB/KinectCommonBridge-v2-python/KCBv2/KCBv2Lib.h"]

def my_module_gen():
    module_parser = ModuleParser('KCBv2Lib', '::')
    module = module_parser.parse(
        header_files=KCB_HEADERS,
        include_paths=['C:/Program Files/Microsoft SDKs/Kinect/v2.0-PublicPreview1407/inc'])
    #pygen_sink=[FileCodeSink('sink.py')]
    #module = module_parser.parse(sys.argv[1:])
    #module.add_include('"d:\projects\KinectCB\KinectCommonBridge-python\KCBv2\KCBv2Lib.h"')

    pybindgen.write_preamble(FileCodeSink(sys.stdout))
    module.generate(FileCodeSink(sys.stdout))

if __name__ == '__main__':
    my_module_gen()