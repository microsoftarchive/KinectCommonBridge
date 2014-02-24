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

#include "stdafx.h"

#include "DataStream.h"
#include "AutoLock.h"

// this is to handle all interaction with the INuiSensor object for all streams
// to ensure single point of interaction with the sensor
// Ctor
DataStream::DataStream()
    : m_pNuiSensor(nullptr)
    , m_hStreamHandle(INVALID_HANDLE_VALUE)
    , m_hFrameReadyEvent(INVALID_HANDLE_VALUE)
    , m_paused(false)
    , m_started(false)
    , m_bPollingMode(true)
{
#ifdef KCB_ENABLE_FT
    m_cameraConfig.Width = 0;
    m_cameraConfig.Height = 0;
    m_cameraConfig.FocalLength = 0.0;
#endif
}

// Dtor
DataStream::~DataStream()
{
    RemoveDevice();

}

// store the SDK version of the sensor object for our stream
void DataStream::Initialize(_In_ INuiSensor* pNuiSensor)
{
    // block to ensure the sensor state doesn't change
    AutoLock lock(m_nuiLock);

    // check passed sensor is not null
    if (nullptr == pNuiSensor)
    {
        // a null sensor is a notification we need to reset
        // the sensor until a valid one is ready
        RemoveDevice();

        return;
    }

    // is it one we already have
    if (m_pNuiSensor == pNuiSensor)
    {
        if (!m_started)
        {
            StartStream();
        }
        return;
    }

    // create the event handle
    if (INVALID_HANDLE_VALUE == m_hFrameReadyEvent)
    {
        m_hFrameReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    // add a reference for this source
    pNuiSensor->AddRef();
    m_pNuiSensor.Attach(pNuiSensor);
}

// removes the sensor when the stream is disabled/closed
void DataStream::RemoveDevice()
{
    AutoLock lock(m_nuiLock);

    m_started = false;
    m_paused = false;

    if (INVALID_HANDLE_VALUE != m_hFrameReadyEvent)
    {
        CloseHandle(m_hFrameReadyEvent);
        m_hFrameReadyEvent = INVALID_HANDLE_VALUE;
    }

    // release the COM sensor object
    m_pNuiSensor.Release();
}

// determine if the stream is active based on 
// the status of the sensor
KINECT_STREAM_STATUS DataStream::GetStreamStatus()
{
    AutoLock lock(m_nuiLock);

    if (nullptr != m_pNuiSensor && SUCCEEDED(m_pNuiSensor->NuiStatus()) && m_started)
    {
        return KinectStreamStatusEnabled;
    }

    return KinectStreamStatusDisabled;
}

// for the owner to check for frame is ready for this stream
HANDLE DataStream::GetFrameReadyEvent()
{
    return m_hFrameReadyEvent;
}

// toggle pause and get the status of paused
void DataStream::PauseStream(bool bPause)
{
    m_paused = bPause;
}

// get the raw frame of the buffer from Nui
HRESULT DataStream::ProcessImageFrame(_Out_opt_ LONGLONG* liTimeStamp)
{
    AutoLock lock(m_nuiLock);
    if (nullptr == m_pNuiSensor)
    {
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;
    // if we haven't started the stream do it now
    if (!m_started)
    {
        hr = StartStream();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_hStreamHandle, 0, &m_ImageFrame);
    if (FAILED(hr))
    {
        return hr;
    }

    // set timestamp
    if (nullptr != liTimeStamp)
    {
        *liTimeStamp = m_ImageFrame.liTimeStamp.QuadPart;
    }

    // if paused, release the frame
    if (m_paused)
    {
        goto ReleaseFrame;
    }

    CopyData(&m_ImageFrame);

ReleaseFrame:
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_hStreamHandle, &m_ImageFrame);

    return hr;
}

// processing skeletons from Nui
HRESULT DataStream::ProcessSkeletonFrame(_Inout_ NUI_SKELETON_FRAME& skeletonFrame)
{
    AutoLock lock(m_nuiLock);

    if (nullptr == m_pNuiSensor)
    {
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    if (!m_started)
    {
        hr = StartStream(); // this also blocks on m_pNuiSensor, so delay the lock until we call Get
        if (FAILED(hr))
        {
            return hr;
        }
    }

    hr = m_pNuiSensor->NuiSkeletonGetNextFrame(0, &skeletonFrame);

    return hr;
}
