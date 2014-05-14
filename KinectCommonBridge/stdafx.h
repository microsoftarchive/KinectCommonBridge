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

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

// For speech APIs
// NOTE: To ensure that application compiles and links against correct SAPI versions (from Microsoft Speech
//       SDK), VC++ include and library paths should be configured to list appropriate paths within Microsoft
//       Speech SDK installation directory before listing the default system include and library directories,
//       which might contain a version of SAPI that is not appropriate for use together with Kinect sensor.
#include <sapi.h>
#include <sphelper.h>

#include "ComSmartPtr.h"
//#include <atlbase.h>

#include <objbase.h>
#include <NuiApi.h> // check for Include Dir: $(KINECTSDK10_DIR)inc;$(KINECT_TOOLKIT_DIR)inc;

#include <assert.h>

#include <memory>
#include <new>
#include <xstring>
#include <map>
#include <vector>
#include <regex>

// For configuring DMO properties
#include <dmo.h>    // IMediaBuffer
#include <mmreg.h>  // WAVEFORMATEX
#include <propsys.h>
#include <uuids.h>
#include <wmcodecdsp.h>

#ifndef KCB_AUDIOFMT
#define KCB_AUDIOFMT
// the audio format required for the DMO
static const WAVEFORMATEX KINECT_WAVEFORMATEX = { WAVE_FORMAT_PCM, 1, 16000, 32000, 2, 16, 0 };
#endif

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else 
#include <guiddef.h>
#endif
DEFINE_GUID(CLSID_ExpectedRecognizer, 0x495648e7, 0xf7ab, 0x4267, 0x8e, 0x0f, 0xca, 0xfb, 0x7a, 0x33, 0xc1, 0x60);

