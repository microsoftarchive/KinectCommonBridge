/***********************************************************************************************************
Copyright © Microsoft Open Technologies, Inc.
All Rights Reserved
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
except in compliance with the License. You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0

THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR
CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

See the Apache 2 License for the specific language governing permissions and limitations under the License.
***********************************************************************************************************/
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>
#include <objbase.h>

#include <Kinect.h>                         // will need $(KINECTSDK20_DIR)inc; in the includes directory for project

#ifdef WIN32
    #ifdef KCBV2_EXPORTS
        #define KINECT_CB __declspec(dllexport)
    #else
        #define KINECT_CB __declspec(dllimport)
        #pragma comment (lib, "KCBv2")      // add $(KINECTSDK20_DIR)lib\x64 or x86 to the library directory for project
    #endif // DLL_EXPORTS
#endif //_WIN32

#ifndef __KCB_HANDLE__
#define __KCB_HANDLE__

typedef int KCBHANDLE;
static const int KCB_INVALID_HANDLE = 0xffffffff;

#endif

extern "C"
{
    KINECT_CB KCBHANDLE APIENTRY KCBOpenDefaultSensor();
    KINECT_CB HRESULT APIENTRY KCBCloseSensor(_Inout_ KCBHANDLE* kcbHandle);
}
