import sys

from pybindgen import FileCodeSink
from pybindgen.gccxmlparser import ModuleParser

def my_module_gen():
    module_parser = ModuleParser('KCBv2Lib', '::')
    module_parser.parse(sys.argv[1], includes=['"KCBv2Lib.h"'], pygen_sink=FileCodeSink(sys.stdout))

if __name__ == '__main__':
    my_module_gen()