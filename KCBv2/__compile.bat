REM Setting up environment...

call d:\dev\msvc\2010\VC\vcvarsall.bat

REM Compiling...

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe"  /c /I"c:\Program Files (x86)\Windows Kits\8.1\Include\shared\\" /I"C:\Program Files\Microsoft SDKs\Kinect\v2.0-PublicPreview1407\inc" /Zi /nologo /W3 /WX- /Od /Oy- /D WIN32 /D NDEBUG /D _WINDOWS /D _USRDLL /D KCBV2LIB_EXPORTS /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Yc"StdAfx.h" /Fp"Release\KCBv2.pch" /Fo"Release\\" /Fd"Release\vc100.pdb" /Gd /TP /analyze- /errorReport:none dllmain.cpp

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe"  /c /I"c:\Program Files (x86)\Windows Kits\8.1\Include\shared\\" /I"C:\Program Files\Microsoft SDKs\Kinect\v2.0-PublicPreview1407\inc" /Zi /nologo /W3 /WX- /O2 /Oi /Oy- /GL /D WIN32 /D NDEBUG /D _WINDOWS /D _USRDLL /D KCBV2LIB_EXPORTS /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Yc"StdAfx.h" /Fp"Release\KCBv2.pch" /Fo"Release\\" /Fd"Release\vc100.pdb" /Gd /TP /analyze- /errorReport:none stdafx.cpp

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\CL.exe"  "/Ic:\Program Files (x86)\Windows Kits\8.1\Include\shared\\" "/IC:\Program Files\Microsoft SDKs\Kinect\v2.0-PublicPreview1407\inc"    /Zi /nologo /W3 /WX- /O2 /Oi /Oy- /GL /D WIN32 /D NDEBUG /D _WINDOWS /D _USRDLL /D KCBV2LIB_EXPORTS /D _WINDLL /D _UNICODE /D UNICODE /Gm- /EHsc /MD /GS /Gy /fp:precise /Zc:wchar_t /Zc:forScope /Fo"Release\\" /Fd"Release\vc100.pdb" /Gd /TP /analyze- /errorReport:none KCBFrames.cpp KCBSensor.cpp KCBv2Lib.cpp KinectList.cpp

REM Linking...

call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\bin\link.exe"  /ERRORREPORT:PROMPT /OUT:"D:\projects\KinectCB\KinectCommonBridge-python\KCBv2\Release\KCBv2.dll" /INCREMENTAL:NO /NOLOGO "/LIBPATH:c:\Program Files\Microsoft SDKs\Kinect\v2.0-PublicPreview1407\Lib\x86"inect20.lib" kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /MANIFEST /ManifestFile:"Release\KCBv2.dll.intermediate.manifest" /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /DEBUG /PDB:"D:\projects\KinectCB\KinectCommonBridge-python\KCBv2\Release\KCBv2.pdb" /SUBSYSTEM:WINDOWS /OPT:REF /OPT:ICF /LTCG /TLBID:1 /DYNAMICBASE /NXCOMPAT /IMPLIB:"D:\projects\KinectCB\KinectCommonBridge-python\KCBv2\Release\KCBv2.lib" /MACHINE:X86 /DLL Release\dllmain.obj Release\KCBFrames.obj Release\KCBSensor.obj Release\KCBv2Lib.obj Release\KinectList.obj Release\stdafx.obj