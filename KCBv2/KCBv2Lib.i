 /* KCBv2Lib.i */
 %module KCBv2Lib
 
 #ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
 #endif

 %{
	//#include <wceatl.h>
	#include "stdafx.h"
	#include <windows.h>
	#include <objbase.h>
	#include <Kinect.h>
 %}

 %include <windows.i>
 
 //%include <wceatl.h>
 %include "stdafx.h"
 %include <windows.h>
 %include <objbase.h>
 %include <Kinect.h>

 #ifndef __KCB_HANDLE__
 #define __KCB_HANDLE__
	static const int KCB_INVALID_HANDLE = 0xffffffff;
 #endif
 
 %inline %{
	typedef int KCBHANDLE;

	extern __declspec(dllexport) KCBHANDLE KCBOpenDefaultSensor();
	extern HRESULT KCBCloseSensor(KCBHANDLE* kcbHandle);	
 %}
 
 extern __declspec(dllexport) KCBHANDLE KCBOpenDefaultSensor();
 extern __declspec(dllexport) HRESULT KCBCloseSensor(KCBHANDLE* kcbHandle);
