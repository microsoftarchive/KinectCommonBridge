#! python3.4

# Do "py setup.py install" to make KCBv2Lib module available, uncomment otherwise
#sys.path.insert(0, "build/lib.win32-3.4/KCBv2Lib")
from KCBv2Lib import *

KCB_INVALID_HANDLE = -1

kcb_handle = KCB_INVALID_HANDLE;
while(kcb_handle == KCB_INVALID_HANDLE):
    print("Connecting...")
    kcb_handle = KCBOpenDefaultSensor()

print(kcb_handle)

