@echo off
@echo off

echo./*
echo. * Check VC++ environment...
echo. */
echo.

set FOUND_VC=0

if defined VS120COMNTOOLS (
    set VSTOOLS="%VS120COMNTOOLS%"
    set VC_VER=120
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
echo. * Building Nuget Version Update Utility...
echo. */
echo.


call %VSVARS%

if %FOUND_VC%==1 (


    msbuild updatenugetversion/updatenugetversion.sln /p:Configuration="Release" /p:Platform="Win32" /t:Clean,Build
 
) else (
    echo Script error.
    goto ERROR
)


