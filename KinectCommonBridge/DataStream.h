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

#include "KinectCommonBridgeLib.h"
#include "CriticalSection.h"

// abstract class to represent streams from the KinectSensor
// the Sensor will manage which instance of the sensor to use
class DataStream
{
public:
    DataStream();
    virtual ~DataStream();

    // start opens the stream
    virtual HRESULT StartStream() = 0;

    // while paused, stream should drop the frames until unpaused
    virtual void PauseStream( bool bPause );

    // stop the stream
    virtual void StopStream() = 0;

    // check the state of the sensor
    virtual KINECT_STREAM_STATUS GetStreamStatus();

    // returns the handle to the frame ready event 
    virtual HANDLE GetFrameReadyEvent();

protected:
    // initialize with an instance of the sensor
    virtual void Initialize( _In_ INuiSensor* pNuiSensor ) = 0;

    // manage the sensor that will be used by the stream
    virtual void AttachDevice( _In_ INuiSensor* pNuiSensor );
    virtual void RemoveDevice();

    // methods to get the frame data from the sensor
    virtual HRESULT ProcessImageFrame( _In_ NUI_IMAGE_FRAME *pImageFrame, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp );
    virtual HRESULT ProcessSkeletonFrame( _Inout_ NUI_SKELETON_FRAME& skeletonFrame );

protected:
    CriticalSection            m_nuiLock;
    CComPtr<INuiSensor>        m_pNuiSensor;
    HANDLE                    m_hStreamHandle;
    HANDLE                    m_hFrameReadyEvent;

    bool m_paused;
    bool m_started;
    bool m_bPollingMode;
};
