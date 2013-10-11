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
    , m_hFrameReadyEvent(nullptr)
    , m_paused(false)
    , m_started(false)
    , m_bPollingMode(true)
{
}

// Dtor
DataStream::~DataStream()
{
    RemoveDevice();
}

// store the SDK version of the sensor object for our stream
void DataStream::AttachDevice( _In_ INuiSensor* pNuiSensor )
{
    // check passed sensor is not null
    if( nullptr == pNuiSensor )
    {
        // a null sensor is a notification we need to reset
        // the sensor until a valid one is ready
        RemoveDevice();

        return;
    }

    // block to ensure the sensor state doesn't change
    AutoLock lock( m_nuiLock );

    // is it one we already have
    if( m_pNuiSensor == pNuiSensor )
    {
        if( !m_started )
        {
            StartStream();
        }
        return;
    }

    // create the event handle
    if( nullptr == m_hFrameReadyEvent )
    {
        m_hFrameReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    }

    // add a reference for this source
    pNuiSensor->AddRef();
    m_pNuiSensor.Attach( pNuiSensor );
}

// removes the sensor when the stream is disabled/closed
void DataStream::RemoveDevice()
{
    AutoLock lock( m_nuiLock );

    m_started = false;
    m_paused = false;

    if( m_hFrameReadyEvent != nullptr )
    {
        CloseHandle(m_hFrameReadyEvent);
        m_hFrameReadyEvent = nullptr;
    }

    // release the COM sensor object
    m_pNuiSensor.Release();
}

// determine if the stream is active based on 
// the status of the sensor
KINECT_STREAM_STATUS DataStream::GetStreamStatus()
{
    AutoLock lock( m_nuiLock );

    if( nullptr != m_pNuiSensor && SUCCEEDED(m_pNuiSensor->NuiStatus()) && m_started )
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
void DataStream::PauseStream( bool bPause )
{
    m_paused = bPause;
}

// get the raw frame of the buffer from Nui
HRESULT DataStream::ProcessImageFrame( _In_ NUI_IMAGE_FRAME *pImageFrame, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    if( nullptr == pImageFrame || nullptr == pImageBuffer )
    {
        return E_INVALIDARG;
    }

    if( nullptr == m_pNuiSensor )
    {
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    // if we haven't started the stream do it now
    if( !m_started )
    {
        hr = StartStream();
        if( FAILED(hr) )
        {
            return hr;
        }
    }        

    AutoLock lock( m_nuiLock );
    hr = m_pNuiSensor->NuiImageStreamGetNextFrame( m_hStreamHandle, 0, pImageFrame );
    if( FAILED(hr) )
    {
        return hr;
    }

    // if paused, release the frame
    if( m_paused )
    {
        goto ReleaseFrame;
    }

    // set timestamp
    if( nullptr != liTimeStamp )
    {
        *liTimeStamp = pImageFrame->liTimeStamp.QuadPart;
    }

    // copy data from the frame
    INuiFrameTexture* pTexture = pImageFrame->pFrameTexture;

    // Lock the frame data so the Kinect knows not to modify it while we are reading it
    NUI_LOCKED_RECT lockedRect;
    pTexture->LockRect( 0, &lockedRect, NULL, 0 );

    // Make sure we've received valid data
    if( lockedRect.Pitch != 0 )
    {
        memcpy_s( pImageBuffer, cbBufferSize, lockedRect.pBits, lockedRect.size );
    }

    // Unlock frame data
    pTexture->UnlockRect(0);

ReleaseFrame:
    m_pNuiSensor->NuiImageStreamReleaseFrame( m_hStreamHandle, pImageFrame );

    return hr;
}

// processing skeletons from Nui
HRESULT DataStream::ProcessSkeletonFrame( _Inout_ NUI_SKELETON_FRAME& skeletonFrame )
{
    AutoLock lock( m_nuiLock );

    if( nullptr == m_pNuiSensor )
    {
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    if( !m_started )
    {
        hr = StartStream(); // this also blocks on m_pNuiSensor, so delay the lock until we call Get
        if( FAILED(hr) )
        {
            return hr;
        }
    }

    return m_pNuiSensor->NuiSkeletonGetNextFrame( 0, &skeletonFrame );
}
