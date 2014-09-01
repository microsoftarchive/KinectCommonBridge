REM echo Visual Studio 2010 installed
REM SET VS90COMNTOOLS=%VS100COMNTOOLS%

REM echo Visual Studio 2012 installed (Visual Studio Version 11)
REM SET VS90COMNTOOLS=%VS110COMNTOOLS%

echo Visual Studio 2013 installed (Visual Studio Version 12)
SET VS90COMNTOOLS = %VS120COMNTOOLS%

echo _building_ module with 'distutils'
python setup.py build

REM echo _installing_ module with 'distutils'
REM python setup.py install

REM swig.exe -python -cpperraswarn -I"C:\Program Files\Microsoft SDKs\Kinect\v2.0-DevPreview1406\inc" -I"c:\Program Files (x86)\Microsoft SDKs\Windows\v5.0\Include" -o KCBv2Lib-SWIG_wrap.c KCBv2Lib-SWIG.i

