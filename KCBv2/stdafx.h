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

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>

#include <objbase.h>
#pragma comment(lib, "ole32.lib")

#include <assert.h>

// Kinect v2
#include <Kinect.h>
#pragma comment(lib, "kinect20")

#include <winerror.h>

static const int KCB_DEFAULT_HANDLE = 0x0000e000;

#ifndef __KCB_HANDLE__
#define __KCB_HANDLE__

typedef int KCBHANDLE;
static const int KCB_INVALID_HANDLE = 0xffffffff;

#endif

#ifndef __KCB_SOURCE_TYPES__
#define __KCB_SOURCE_TYPES__

typedef enum _KCBSourceType
{
    KCBSourceType_Audio = 0,
    KCBSourceType_Body,
    KCBSourceType_BodyIndex,
    KCBSourceType_Color,
    KCBSourceType_Depth,
    KCBSourceType_Infrared,
    KCBSourceType_LongExposureInfrared,
    KCBSourceType_Count,
    KCBSourceType_Unknown
} KCBSourceType;

#endif

#ifndef IF_FAILED_GOTO
#define IF_FAILED_GOTO(hr, label) if (FAILED(hr)) { goto label; }
#endif

#ifndef CHECK_HR
#define CHECK_HR(hr) IF_FAILED_GOTO(hr, done)
#endif

// Safe release for interfaces
#ifndef SAFE_RELEASE
template <class T>
inline void SAFE_RELEASE(T*& p)
{
    if (nullptr != p)
    {
        (void)p->Release();
        p = nullptr;
    }
}
#endif

template<typename T> 
inline void SafeDelete(T*& pObject) 
{  
    delete pObject;  
    pObject = nullptr;
}

template<typename T> 
inline void SafeArrayDelete(T*& pArray) 
{  
    delete [] pArray;  
    pArray = nullptr;
}

#include <iostream>
#include <sstream>
#define DEBUG_PRINT( s )                        \
{                                               \
    std::wostringstream os_;                     \
    os_ << s;                                   \
    OutputDebugString( os_.str().c_str() );     \
}
