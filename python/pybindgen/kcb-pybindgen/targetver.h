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

//// The following macros define the minimum required platform.  The minimum required platform
//// is the earliest version of Windows, Internet Explorer etc. that has the necessary features to run 
//// your application.  The macros work by enabling all features available on platform versions up to and 
//// including the version specified.

//// Modify the following defines if you have to target a platform prior to the ones specified below.
//// Refer to MSDN for the latest info on corresponding values for different platforms.
//#ifndef WINVER                          // Specifies that the minimum required platform is Windows Vista.
//#define WINVER 0x0600           // Change this to the appropriate value to target other versions of Windows.
//#endif

//#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows Vista.
//#define _WIN32_WINNT 0x0600     // Change this to the appropriate value to target other versions of Windows.
//#endif

//#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
//#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
//#endif

//#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
//#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
//#endif


// Including SDKDDKVer.h defines the highest available Windows platform.

// If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
// set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

#include <SDKDDKVer.h>