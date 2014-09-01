swig -v -c++ -python -cpperraswarn -includeall -E 
-I"C:\Program Files\Microsoft SDKs\Kinect\v2.0-DevPreview1406\inc" 
-I"c:\Program Files (x86)\Windows Kits\8.0\Include\um" 
-I"c:\Program Files (x86)\Windows Kits\8.0\Include\shared" 
-I"C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include" 
-o KCBv2Lib_wrap.cpp KCBv2Lib.i

echo on
swig -v -c++ -python -cpperraswarn -E -I"%KINECTSDK20_DIR%inc" -I"$(WindowsSdkDir)Include\um" -I"$(WindowsSdkDir)Include\shared" -I"C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include" -o KCBv2Lib_wrap.cpp KCBv2Lib.i

echo on
swig -v -c++ -python -cpperraswarn -E -I"$(KINECTSDK20_DIR)inc" -I"$(WindowsSdkDir)Include\um" -I"$(WindowsSdkDir)Include\shared" -I"C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include" -o KCBv2Lib_wrap.cpp KCBv2Lib.i


echo on
swig -v -c++ -python -cpperraswarn -I"$(KINECTSDK20_DIR)inc" -I"$(WindowsSdkDir)Include\um" -I"$(WindowsSdkDir)Include\shared" -I"C:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\include" "%(FullPath)"

swig -v -c++ -python -cpperraswarn -I"C:\Program Files\Microsoft SDKs\Kinect\v2.0-DevPreview1406\inc" -I"c:\Program Files (x86)\Windows Kits\8.0\Include\um" -o KCBv2Lib_wrap.cpp KCBv2Lib.i