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

#define WIN32_LEAN_AND_MEAN        // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <objbase.h>
#include <NuiApi.h>                // be sure to add Include Dir: $(KINECTSDK10_DIR)inc;$(KINECT_TOOLKIT_DIR)inc;

#ifdef _WIN32
    #ifdef DLL_EXPORTS
        #define KINECT_CB __declspec(dllexport)
    #else
        #define KINECT_CB __declspec(dllimport)
        #pragma comment (lib, "KinectCommonBridge.lib") // be sure to add the .dll/.lib folder to your Linker path
    #endif // DLL_EXPORTS
#endif //_WIN32

typedef int KCBHANDLE;
#define KCB_INVALID_HANDLE    0xffffffff
#define KINECT_MAX_PORTID_LENGTH    50

// statuses that the KinectSensor wrapper uses to determine state
typedef enum _KinectSensorStatus
{
    // This NuiSensorChooser has a connected and started sensor.
    KinectSensorStatusNone                        = 0,
    /// <summary>
    /// This NuiSensorChooser has a connected and started sensor.
    /// </summary>
    KinectSensorStatusStarted                    = 1,
    /// <summary>
    /// The available sensor is not powered.  If it receives power we
    /// will try to use it automatically.
    /// </summary>
    KinectSensorStatusNotPowered                = 2,
    /// <summary>
    /// There is not enough bandwidth on the USB controller available
    /// for this sensor. Can recover in some cases
    /// </summary>
    KinectSensorStatusInsufficientBandwidth        = 3,
    /// <summary>
    /// Available sensor is in use by another application.
    /// Will recover once the other application releases its sensor
    /// </summary>
    KinectSensorStatusConflict                    = 4,
     /// <summary>
    /// Don't have a sensor yet, a sensor is initializing, you may not get it
    /// Can't trust the state of the sensor yet
    /// </summary>
   KinectSensorStatusInitializing                = 5,
    /// <summary>
    /// Available sensor is not genuine.
    /// </summary>
    KinectSensorStatusNotGenuine                = 6,
    /// <summary>
    /// Available sensor is not supported
    /// </summary>
    KinectSensorStatusNotSupported                = 7,
    /// <summary>
    /// Available sensor has an error
    /// </summary>
    KinectSensorStatusError                        = 8,
} KINECT_SENSOR_STATUS;

typedef enum _KinectStreamStatus
{
    KinectStreamStatusError        = 0,
    KinectStreamStatusEnabled    = 1,
    KinectStreamStatusDisabled    = 2
} KINECT_STREAM_STATUS;

// Skeleton Selection mode
typedef enum _KINECT_SKELETON_SELECTION_MODE
{
    SkeletonSelectionModeDefault    = 0,
    SkeletonSelectionModeClosest1    = 1,
    SkeletonSelectionModeClosest2    = 2,
    SkeletonSelectionModeSticky1    = 3,
    SkeletonSelectionModeSticky2    = 4,
    SkeletonSelectionModeActive1    = 5,
    SkeletonSelectionModeActive2    = 6,
} KINECT_SKELETON_SELECTION_MODE;

// Structure for the frame data for depth/color
// take note of cbBytesPerPixel 
typedef struct _KinectImageFrameFormat
{   
    DWORD dwStructSize;
    DWORD dwHeight;
    DWORD dwWidth;
    ULONG cbBytesPerPixel;
    ULONG cbBufferSize;
} KINECT_IMAGE_FRAME_FORMAT;


// exported api's
extern "C"
{
    // For enumerating the sensors to find the PortID
    KINECT_CB UINT APIENTRY KinectGetPortIDCount();
    KINECT_CB bool APIENTRY KinectGetPortIDByIndex( UINT index, ULONG cchPortID, _Out_cap_(cchPortID) WCHAR* pwcPortID );
    
    // Create instance of the Kinect Sensor
    // The device we be selected and the default streams initialized
    // Color & Depth - 640x480 / with PlayerIndex / Near Mode off
    //
    // In the event the sensor is not connected, you can connect it afterwards
    // but Initialization/Startup will occur after the Notifaction from Nui 
    // this may cause a delay in the update loop, but should be expected given the 
    // sensor is getting plugged in
    KINECT_CB KCBHANDLE APIENTRY KinectOpenDefaultSensor();
    KINECT_CB KCBHANDLE APIENTRY KinectOpenSensor( _In_z_ const WCHAR* wcPortID );

    // determine if the handle is valid
    KINECT_CB bool APIENTRY KinectIsHandleValid( KCBHANDLE kcbHandle );

    // close down resources for the sensor
    // handle will still be valid, but you will have to call "Open" again
    KINECT_CB void APIENTRY KinectCloseSensor( KCBHANDLE kcbHandle );

    // Sensor properties
    // gets the connection id the senosr is using
    KINECT_CB const WCHAR* APIENTRY KinectGetPortID( KCBHANDLE kcbHandle );
    // internal state of the wrapper
    KINECT_CB KINECT_SENSOR_STATUS APIENTRY KinectGetKinectSensorStatus( KCBHANDLE kcbHandle );

    // enable a specific stream, by default color and depth are enabled for you
    // enable will start the stream if the sensor is available
    KINECT_CB void APIENTRY KinectEnableIRStream( KCBHANDLE kcbHandle, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    KINECT_CB void APIENTRY KinectEnableColorStream( KCBHANDLE kcbHandle, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    KINECT_CB void APIENTRY KinectEnableDepthStream( KCBHANDLE kcbHandle, bool bNearMode, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    KINECT_CB void APIENTRY KinectEnableSkeletonStream( KCBHANDLE kcbHandle, bool bSeatedSkeltons, KINECT_SKELETON_SELECTION_MODE mode, _Inout_opt_ const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothParams );

    // start streams
    KINECT_CB HRESULT APIENTRY KinectStartStreams( KCBHANDLE kcbHandle );
    KINECT_CB HRESULT APIENTRY KinectStartIRStream( KCBHANDLE kcbHandle );
    KINECT_CB HRESULT APIENTRY KinectStartColorStream( KCBHANDLE kcbHandle );
    KINECT_CB HRESULT APIENTRY KinectStartDepthStream( KCBHANDLE kcbHandle );
    KINECT_CB HRESULT APIENTRY KinectStartSkeletonStream( KCBHANDLE kcbHandle );

    // pause streams
    // bPause = true will pause
    KINECT_CB void APIENTRY KinectPauseStreams( KCBHANDLE kcbHandle, bool bPause );
    KINECT_CB void APIENTRY KinectPauseIRStream( KCBHANDLE kcbHandle, bool bPause );
    KINECT_CB void APIENTRY KinectPauseColorStream( KCBHANDLE kcbHandle, bool bPause );
    KINECT_CB void APIENTRY KinectPauseDepthStream( KCBHANDLE kcbHandle, bool bPause );
    KINECT_CB void APIENTRY KinectPauseSkeletonStream( KCBHANDLE kcbHandle, bool bPause );

    // stop streams
    KINECT_CB void APIENTRY KinectStopStreams( KCBHANDLE kcbHandle );
    KINECT_CB void APIENTRY KinectStopIRStream( KCBHANDLE kcbHandle );
    KINECT_CB void APIENTRY KinectStopColorStream( KCBHANDLE kcbHandle );
    KINECT_CB void APIENTRY KinectStopDepthStream( KCBHANDLE kcbHandle );
    KINECT_CB void APIENTRY KinectStopSkeletonStream( KCBHANDLE kcbHandle );

    // get the status of a stream
    KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetIRStreamStatus( KCBHANDLE kcbHandle );
    KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetColorStreamStatus( KCBHANDLE kcbHandle );
    KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetDepthStreamStatus( KCBHANDLE kcbHandle );
    KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetSkeletonStreamStatus( KCBHANDLE kcbHandle );


    // check if frame is ready
    KINECT_CB bool APIENTRY KinectIsColorFrameReady( KCBHANDLE kcbHandle );
    KINECT_CB bool APIENTRY KinectIsDepthFrameReady( KCBHANDLE kcbHandle );
    KINECT_CB bool APIENTRY KinectIsSkeletonFrameReady( KCBHANDLE kcbHandle );
    KINECT_CB bool APIENTRY KinectAnyFrameReady( KCBHANDLE kcbHandle );
    KINECT_CB bool APIENTRY KinectAllFramesReady( KCBHANDLE kcbHandle );
    
    
    // Get the frame structure for color/depth stream
    // this will provide the height, width, bytes per pixel and buffer size for the specified stream
    // pFrame - should already be allocated by the caller
    KINECT_CB void APIENTRY KinectGetIRFrameFormat( KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    KINECT_CB void APIENTRY KinectGetColorFrameFormat( KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    KINECT_CB void APIENTRY KinectGetDepthFrameFormat( KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    

    // Get the data frame from a stream
    // Return: status of the call from the Kinect for Windows
    // cbBufferSize - size of the buffer that you have allocated, KINECT_IMAGE_FRAME_FORMAT::cbBufferSize
    // pBuffer - byte buffer that frame will be copied to
    // liTimeStamp - (optional) reference for the timestamp of the frame
    KINECT_CB HRESULT APIENTRY KinectGetIRFrame( KCBHANDLE kcbHandle, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp );
    KINECT_CB HRESULT APIENTRY KinectGetColorFrame( KCBHANDLE kcbHandle, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp );
    KINECT_CB HRESULT APIENTRY KinectGetDepthFrame( KCBHANDLE kcbHandle, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pDepthBuffer, _Out_opt_ LONGLONG* liTimeStamp );
    
    // pSkeletons - reference to the allocated NUI_SKELETON_FRAME structure allocated by the caller
    KINECT_CB HRESULT APIENTRY KinectGetSkeletonFrame( KCBHANDLE kcbHandle, _Inout_ NUI_SKELETON_FRAME* pSkeletons );
};

