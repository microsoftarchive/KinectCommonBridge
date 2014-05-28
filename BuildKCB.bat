@echo off
@echo off

echo./*
echo. * Check VC++ environment...
echo. */
echo.

set FOUND_VC=0

if defined VS110COMNTOOLS (
    set VSTOOLS="%VS110COMNTOOLS%"
    set VC_VER=110
    set FOUND_VC=1
)

set VSTOOLS=%VSTOOLS:"=%
set "VSTOOLS=%VSTOOLS:\=/%"
set VSVARS="%VSTOOLS%vsvars32.bat"

if not defined VSVARS (
    echo Can't find VC2012 or VC2013 installed!

    goto ERROR
)

echo./*
echo. * Building libraries...
echo. */
echo.

call %VSVARS%

if %FOUND_VC%==1 (
    msbuild KinectCommonBridge.sln /p:Configuration="Debug" /p:Platform="Win32" /t:Clean,Build
    msbuild KinectCommonBridge.sln /p:Configuration="Release" /p:Platform="Win32" /t:Clean,Build
    msbuild KinectCommonBridge.sln /p:Configuration="Debug" /p:Platform="x64" /t:Clean,Build
    msbuild KinectCommonBridge.sln /p:Configuration="Release" /p:Platform="x64" /t:Clean,Build

) else (
    echo Script error.
    goto ERROR
)


 md KinectCommonBridge\OutDir

 md OutDir\Win32
 md OutDir\Win32\Debug
 md OutDir\Win32\Release

 md OutDir\x64
 md OutDir\x64\Debug
 md OutDir\x64\Release


rem Win32 Post Build Event 

xcopy "Out\Win32\Debug\FaceTrackLib.dll"	"OutDir\Win32\Debug" /eiycq
xcopy "Out\Win32\Debug\FaceTrackData.dll"	"OutDir\Win32\Debug" /eiycq
xcopy "Out\Win32\Release\FaceTrackLib.dll"	"OutDir\Win32\Release" /eiycq
xcopy "Out\Win32\Release\FaceTrackData.dll"	"OutDir\Win32\Release" /eiycq

 
rem x64 Post Build Event 

xcopy "Out\x64\Debug\FaceTrackLib.dll"							"OutDir\x64\Debug" /eiycq
xcopy "Out\x64\Debug\FaceTrackData.dll"							"OutDir\x64\Debug" /eiycq
xcopy "Out\x64\Release\FaceTrackLib.dll"						"OutDir\x64\Release" /eiycq
xcopy "Out\x64\Release\FaceTrackData.dll"	"OutDir\x64\Release" /eiycq

goto EOF

 
:ERROR

pause
 

:EOF