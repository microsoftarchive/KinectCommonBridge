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

#include "KinectSensor.h"
#include "DataStreamColor.h"
#include "DataStreamDepth.h"
#include "AutoLock.h"

/// <summary>
/// Check whether the specified sensor is available.
/// </summary>
bool KinectSensor::IsSensorConflict( _In_ INuiSensor* pNuiSensor )
{
    assert( nullptr != pNuiSensor );

    // Because we can still open a sensor even if it is occupied by other process,
    // we have to explicitly initialize it to check if we can operate it.
    HRESULT hr = pNuiSensor->NuiInitialize( NUI_INITIALIZE_FLAG_USES_COLOR );
    if( SUCCEEDED(hr) )
    {
        pNuiSensor->NuiShutdown();
        return false;
    }

    return true;
}

/// <summary>
/// Return the current status of the sensor
/// </summary>
void KinectSensor::NuiSensorStatus( _In_ INuiSensor* pNuiSensor, _Out_ HRESULT& hr, _Out_ KINECT_SENSOR_STATUS& curStatus, bool bCheckConflict)
{
    if( nullptr == pNuiSensor )
    {
        curStatus = KinectSensorStatusNone;
        hr = E_NUI_DEVICE_NOT_CONNECTED;

        return;
    }
    else
    {
        hr = pNuiSensor->NuiStatus();
    }

    switch( hr )
    {
    case S_OK:
        curStatus = KinectSensorStatusStarted;

        if( bCheckConflict )
        {
            // Even if the input sensor is started, we still need to check if it is
            // in use by other process.
            if( IsSensorConflict(pNuiSensor) )
            {
                // other app owns this sensor
                curStatus = KinectSensorStatusConflict;
                hr = E_NUI_ALREADY_INITIALIZED;
            }
        }
        break;
    case S_NUI_INITIALIZING:
        curStatus = KinectSensorStatusInitializing;
        break;
    case E_NUI_NOTGENUINE:
        curStatus = KinectSensorStatusNotGenuine;
        break;
    case E_NUI_INSUFFICIENTBANDWIDTH:
        curStatus = KinectSensorStatusInsufficientBandwidth;
        break;
    case E_NUI_NOTSUPPORTED:
        curStatus = KinectSensorStatusNotSupported;
        break;
    case E_NUI_NOTPOWERED:
        curStatus = KinectSensorStatusNotPowered;
        break;
    default:
        curStatus = KinectSensorStatusError;
    }
}

// Ctor
KinectSensor::KinectSensor( _In_z_ const WCHAR* wcPortID )
    : m_wsPortID(wcPortID)
    , m_pNuiSensor(nullptr)
    , m_bSelected(false)
    , m_bInitialized(false)
    , m_hrLast(S_OK)
    , m_eStatus(KinectSensorStatusNone)
    , m_pColorStream(nullptr)
    , m_pDepthStream(nullptr)
    , m_pSkeletonStream(nullptr)
{
}

// Dtor
KinectSensor::~KinectSensor()
{
    Close();
}

// initialization
HRESULT KinectSensor::Open()
{
    return SelectSensor( true );
}

// closes down all streams the restores to default configuration
void KinectSensor::Close()
{
    SelectSensor( false );
}

// setup or shutdown the sensor
HRESULT KinectSensor::SelectSensor( bool bSelected )
{
    HRESULT hr = S_OK;


    if( bSelected != m_bSelected )
    {
        // reset to a default state
        ResetDevice();

        if( bSelected )
        {
            // enable default streams
            EnableColorStream();
            EnableDepthStream();
        }
        else
        {
            // remove the streams
            m_pColorStream.reset();
            m_pDepthStream.reset();
            m_pSkeletonStream.reset();

            // reset the state
            m_hrLast = S_OK;
            m_eStatus = KinectSensorStatusNone;

            // Release the sensor
            m_pNuiSensor.Release();
        }

    }

    m_bSelected = bSelected;

    // returns the state of the sensor to make a decision on being used
    return hr;
}

// used to notify object to release of create the sensor
void KinectSensor::NuiStatusNotification( _In_z_ const WCHAR* wcPortID, HRESULT hrStatus )
{
    AutoLock lock( m_nuiLock );

    if( E_NUI_NOTCONNECTED == hrStatus )
    {
        ResetDevice();

        // release the instance of the sensor
        m_pNuiSensor.Release();
    }
    else
    {
        if( 0 != m_wsPortID.compare(wcPortID) )
        {
            ResetDevice();
            m_wsPortID = wcPortID;
        }

        // notifaction callback, check the state
        // if there is a non-recovery error, reset
        HRESULT hr = GetNUISensorStatus();
        if( FAILED(hr) && KinectSensorStatusError == m_eStatus )
        {
            ResetDevice();

            // release the instance of the sensor
            m_pNuiSensor.Release();
            
            return;
        }
    }

}

// create the instance of the nui sensor
HRESULT KinectSensor::CreateNuiDevice()
{
    assert( m_wsPortID.size() != 0 );

    // check if we can use it
    CComPtr<INuiSensor> pNuiSensor;
    HRESULT hr = NuiCreateSensorById( m_wsPortID.c_str(), &pNuiSensor );
    if( FAILED(hr) )
    {
        return hr;
    }

    // is this one we already have
    if( m_pNuiSensor == pNuiSensor )
    {
        return S_OK;
    }

    assert( nullptr == m_pNuiSensor );

    // if the device is not in use by another process
    if( !IsSensorConflict(pNuiSensor) )
    {
        // start clean
        ResetDevice();

        m_pNuiSensor = pNuiSensor; // auto refcount with CComPtr
    }
    
    // update internal state and return
    return GetNUISensorStatus(true);
}

// updates the instance of the sensor and initializes the stream
HRESULT KinectSensor::UpdateSensor()
{
    AutoLock lock( m_nuiLock );

    // only try to create and initialize for Opened/Selected sensors
    if( !m_bSelected )
    {
        return E_NUI_DEVICE_NOT_READY;
    }

    if( m_bInitialized )
    {
        return GetNUISensorStatus();
    }

    // if the sensor isn't ready, try to create it now
    HRESULT hr = S_OK;
    if( !IsAvailable() )
    {
        hr = CreateNuiDevice();
        if( FAILED(hr) )
        {
            return hr;
        }
    }

    // set the parameters based on the streams that were created
    DWORD dwInitFlags = 0;
    if( ( (nullptr != m_pColorStream) || (nullptr != m_pDepthStream) || (nullptr != m_pSkeletonStream) ) )
    {
        if( (nullptr != m_pColorStream) )
        {
            dwInitFlags |= NUI_INITIALIZE_FLAG_USES_COLOR;
        }
        if( (nullptr != m_pSkeletonStream)  )
        {
            dwInitFlags |= NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX;
            dwInitFlags |= NUI_INITIALIZE_FLAG_USES_SKELETON;
        }
        else if( (nullptr != m_pDepthStream)  )
        {
            dwInitFlags |= NUI_INITIALIZE_FLAG_USES_DEPTH;
        }
    }

    // initialize the sensor
    {
        hr = m_pNuiSensor->NuiInitialize( dwInitFlags );
        if( SUCCEEDED(hr) )
        {
            m_bInitialized = true;
        }
        else
        {
            ResetDevice();
            return hr;
        }

        // if we initialized the sensor, pass it along to the enabled streams
        if( m_bInitialized )
        {
            if( nullptr != m_pColorStream )
            {
                m_pColorStream->Initialize( m_pNuiSensor );
            }
            if( nullptr != m_pDepthStream )
            {
                m_pDepthStream->Initialize( m_pNuiSensor );
            }
            if( nullptr != m_pSkeletonStream )
            {
                m_pSkeletonStream->Initialize( m_pNuiSensor );
            }
        }
    }
    return hr;
}

// stops all streams and releases the sensor
void KinectSensor::ResetDevice()
{
    // stop the current streams
    StopStreams();

    // shutdown the sensor if in use
    if( nullptr != m_pNuiSensor )
    {
        AutoLock lock( m_nuiLock );

        // if it is started, shutdown
        HRESULT hr = m_pNuiSensor->NuiStatus();
        if( SUCCEEDED(hr) )
        {
            m_pNuiSensor->NuiShutdown(); 
        }
    }

    m_bInitialized = false;
}

// is the instance of the sensor ready
bool KinectSensor::IsAvailable()
{
    if( SUCCEEDED( GetNUISensorStatus(true) ) && ( m_eStatus == KinectSensorStatusStarted ) )
    {
        return true;
    }
    
    return false;
}

// checks the state of the wrapper
HRESULT KinectSensor::GetNUISensorStatus( bool bCheckInUse )
{
    KINECT_SENSOR_STATUS status;
    HRESULT hr;

    {    // get the status from the device
        NuiSensorStatus( m_pNuiSensor, hr, status, bCheckInUse );    
    }

    if( hr != m_hrLast || status != m_eStatus )
    {
        m_eStatus = status;
        m_hrLast = hr;
    }

    return m_hrLast;
}

// default configuration for color stream
void KinectSensor::EnableColorStream()
{
    EnableColorStream( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480 );
}
// full configuartion for color stream
void KinectSensor::EnableColorStream( NUI_IMAGE_TYPE type, NUI_IMAGE_RESOLUTION resolution )
{
    if( nullptr == m_pColorStream )
    {
        // in the event the constructor fails any call to the stream
        // will fail, caller should call 
        // GetXXXStreamStatus to get the state of the stream
        m_pColorStream.reset( new (std::nothrow) DataStreamColor() );
        if( nullptr == m_pColorStream )
        {
            return;
        }
    }
    m_pColorStream->Initialize( type, resolution, (m_bInitialized ? m_pNuiSensor : nullptr) );
}
// default configuration for depth stream
void KinectSensor::EnableDepthStream()
{ 
    EnableDepthStream( false, NUI_IMAGE_RESOLUTION_640x480 );
}
// full configuration for depth stream
void KinectSensor::EnableDepthStream( _In_opt_ bool bNearMode, _In_opt_ NUI_IMAGE_RESOLUTION resolution )
{
    if( nullptr == m_pDepthStream )
    {
        m_pDepthStream.reset( new (std::nothrow) DataStreamDepth() ); 
        if( nullptr == m_pDepthStream )
        {
            return;
        }
    }

    m_pDepthStream->Initialize( bNearMode, resolution, (m_bInitialized ? m_pNuiSensor : nullptr) );
}
// default skeleton stream configuration
void KinectSensor::EnableSkeletonStream()
{ 
    EnableSkeletonStream( false, SkeletonSelectionModeDefault, nullptr );
}
// full configuration of the skeleton stream
void KinectSensor::EnableSkeletonStream( _In_opt_ bool bSeated, _In_opt_ KINECT_SKELETON_SELECTION_MODE mode, _Out_opt_ const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothParams )
{
    if( nullptr == m_pSkeletonStream )
    {
        m_pSkeletonStream.reset( new (std::nothrow) DataStreamSkeleton() );
        if( nullptr == m_pSkeletonStream )
        {
            return;
        }

        // first time created we have to reset the NuiSensor to enable skeleton stream
        if( m_bInitialized )
        {
            ResetDevice();
        }
    }

    m_pSkeletonStream->Initialize( bSeated, mode, (m_bInitialized ? m_pNuiSensor : nullptr), pSmoothParams );
}

// start the color stream
HRESULT KinectSensor::StartColorStream()
{
    // since this can be call publically
    // we will ensure the stream is configured and ready
    if( nullptr == m_pColorStream )
    {
        EnableColorStream();
    }

    // previous call failed, must be memory issue
    if( nullptr == m_pColorStream )
    {
        return E_OUTOFMEMORY;
    }

    // be sure the sensor is initialized before starting stream
    HRESULT hr = UpdateSensor();
    if( FAILED(hr) )
    {
        ResetDevice();
        return hr;
    }

    return m_pColorStream->StartStream();
}
// start the depth stream
HRESULT KinectSensor::StartDepthStream()
{
    // since this can be call publically
    // we will ensure the stream is configured and ready
    if( nullptr == m_pDepthStream )
    {
        EnableDepthStream();
    }

    // previous call failed, must be memory issue
    if( nullptr == m_pDepthStream )
    {
        return E_OUTOFMEMORY; 
    }

    // be sure the sensor is initialized before starting stream
    HRESULT hr = UpdateSensor();
    if( FAILED(hr) )
    {
        ResetDevice();
        return hr;
    }

    return m_pDepthStream->StartStream();
}
// start the skeleton stream
HRESULT KinectSensor::StartSkeletonStream()
{
    // since this can be call publically
    // we will ensure the stream is configured and ready
    if( nullptr == m_pSkeletonStream )
    {
        // enable a stream
        EnableSkeletonStream();
    }

    // previous call failed, must be memory issue
    if( nullptr == m_pSkeletonStream )
    {
        return E_OUTOFMEMORY; 
    }

    // be sure the sensor is initialized before starting stream
    HRESULT hr = UpdateSensor();
    if( FAILED(hr) )
    {
        ResetDevice();
        return hr;
    }

    return m_pSkeletonStream->StartStream();
}

// pause the color stream
void KinectSensor::PauseColorStream( bool bPause ) 
{
    if( nullptr != m_pColorStream )
    {
        m_pColorStream->PauseStream( bPause );
    }
}
// pause the depth stream
void KinectSensor::PauseDepthStream( bool bPause ) 
{
    if( nullptr != m_pDepthStream )
    {
        m_pDepthStream->PauseStream( bPause );
    }
}
// pause the skeleton stream
void KinectSensor::PauseSkeletonStream( bool bPause ) 
{
    if( nullptr != m_pSkeletonStream )
    {
        m_pSkeletonStream->PauseStream( bPause );
    }
}

// disables the color stream
void KinectSensor::StopColorStream()
{
    if( nullptr != m_pColorStream )
    {
        m_pColorStream->StopStream();
    }
}
// disables the depth stream
void KinectSensor::StopDepthStream()
{
    if( nullptr != m_pDepthStream )
    {
        m_pDepthStream->StopStream();
    }
}
// disables the skeleton stream
void KinectSensor::StopSkeletonStream()
{
    if( nullptr != m_pSkeletonStream )
    {
        m_pSkeletonStream->StopStream();
    }
}

// open the streams that are enabled
HRESULT KinectSensor::StartStreams()
{
    HRESULT hr = E_NUI_DEVICE_NOT_READY;

    if( false == m_bInitialized )
    {
        // ensure everything is initialized 
        hr = UpdateSensor();
        if( FAILED(hr) )
        {
            return false;
        }
    }

    // only call start on the streams that are configured
    if( nullptr != m_pColorStream )
    {
        hr = StartColorStream();
        if( FAILED(hr) )
        {
            return hr;
        }
    }
    if( nullptr != m_pDepthStream )
    {
        hr = StartDepthStream();
        if( FAILED(hr) )
        {
            return hr;
        }
    }
    if( nullptr != m_pSkeletonStream )
    {
        hr = StartSkeletonStream();
        if( FAILED(hr) )
        {
            return hr;
        }
    }

    return hr;
}
// stop the streams that are enabled
void KinectSensor::StopStreams()
{
    StopColorStream();
    StopDepthStream();
    StopSkeletonStream();
}
// pause the streams that are enabled
void KinectSensor::PauseStreams( bool bPause )
{
    PauseColorStream( bPause );
    PauseDepthStream( bPause );
    PauseSkeletonStream( bPause );
}

// get status of the color stream
KINECT_STREAM_STATUS KinectSensor::GetColorStreamStatus()
{
    if( nullptr == m_pColorStream )
    {
        return KinectStreamStatusError; // handle the out of memory or not initialized yet scenario
    }

    return m_pColorStream->GetStreamStatus();
}
// get status of the depth stream
KINECT_STREAM_STATUS KinectSensor::GetDepthStreamStatus()
{
    if( nullptr == m_pDepthStream )
    {
        return KinectStreamStatusError; // handle the out of memory or not initialized yet scenario
    }

    return m_pDepthStream->GetStreamStatus();
}
// get status of the skeleton stream
KINECT_STREAM_STATUS KinectSensor::GetSkeletonStreamStatus()
{
    if( nullptr == m_pSkeletonStream )
    {
        return KinectStreamStatusError; // handle the out of memory or not initialized yet scenario
    }

    return m_pSkeletonStream->GetStreamStatus();
}

// get the color frame data structure from the sensor
void KinectSensor::GetColorFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    // color frame data requested, be sure it is configured
    if( nullptr == m_pColorStream )
    {
        EnableColorStream();
    }

    // get the frame format data
    if( nullptr != m_pColorStream )
    {
        m_pColorStream->GetFrameFormat( pFrame );
    }
}
// get the depth frame data structure from the sensor
void KinectSensor::GetDepthFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    // depth frame data requested, be sure it is configured
    if( nullptr == m_pDepthStream )
    {
        EnableDepthStream();
    }

    // get the frame format data
    if( nullptr != m_pDepthStream )
    {
        m_pDepthStream->GetFrameFormat( pFrame );
    }
}

// get the color frame data from the stream
HRESULT KinectSensor::GetColorFrame( ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    // is the buffer valid
    if( nullptr == pColorBuffer )
    {
        return E_INVALIDARG;
    }

    // be sure the color stream is running
    HRESULT hr = StartColorStream();
    if( FAILED(hr) )
    {
        return hr;
    }

    // grab the frame
    return m_pColorStream->GetFrameData( cbBufferSize, pColorBuffer, liTimeStamp );
}
// get the depth frame data from the stream
HRESULT KinectSensor::GetDepthFrame( ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pDepthBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    // is the buffer valid
    if( nullptr == pDepthBuffer )
    {
        return E_INVALIDARG;
    }

    // be sure the depth stream is running
    HRESULT hr = StartDepthStream();
    if( FAILED(hr) )
    {
        return hr;
    }

    // grab the frame
    return m_pDepthStream->GetFrameData( cbBufferSize, pDepthBuffer, liTimeStamp );
}
// get the skeleton frame data from the stream
HRESULT KinectSensor::GetSkeletonFrame( _Inout_ NUI_SKELETON_FRAME& skeletonFrame )
{
    HRESULT hr = StartSkeletonStream();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pSkeletonStream->GetFrameData( skeletonFrame );
}

// check the frame status before getting the frame
// not required, but may improve perf
bool KinectSensor::ColorFrameReady()
{
    // Ensure the streams are started
    HRESULT hr = StartStreams();
    if( FAILED(hr) )
    {
        return false;
    }

    // check if the color stream event has been set
    if( nullptr != m_pColorStream )
    {
        DWORD dwResult = WaitForSingleObject( m_pColorStream->GetFrameReadyEvent(), 0 );
        if( WAIT_OBJECT_0  == dwResult )
        {
            return true;
        }
    }
    return false;
}
bool KinectSensor::DepthFrameReady()
{
    // Ensure the streams are started
    HRESULT hr = StartStreams();
    if( FAILED(hr) )
    {
        return false;
    }

    // check if the depth stream event has been set
    if( nullptr != m_pDepthStream )
    {
        DWORD dwResult = WaitForSingleObject( m_pDepthStream->GetFrameReadyEvent(), 0 );
        if( WAIT_OBJECT_0  == dwResult )
        {
            return true;
        }
    }
    return false;
}
bool KinectSensor::SkeletonFrameReady()
{
    // Ensure the streams are started
    HRESULT hr = StartStreams();
    if( FAILED(hr) )
    {
        return false;
    }

    // check if the skeleton stream event has been set
    if( nullptr != m_pSkeletonStream )
    {
        DWORD dwResult = WaitForSingleObject( m_pSkeletonStream->GetFrameReadyEvent(), 0 );
        if( WAIT_OBJECT_0  == dwResult )
        {
            return true;
        }
    }
    return false;
}

// check if any frame is ready
bool KinectSensor::AnyFrameReady()
{
    // Ensure the streams are started
    HRESULT hr = StartStreams();
    if( FAILED(hr) )
    {
        return false;
    }

    // create a list of events to check
    std::vector<HANDLE> events;
    GetWaitEvents(events);

    if (events.empty())
    {
        return false;
    }

    // if any frame is ready we return true
    DWORD dwResult = WaitForMultipleObjects((DWORD)events.size(), events.data(), false, 0);
    if( dwResult == WAIT_OBJECT_0 )
    {
        return true;
    }

    return false;
}

// help with sync between color/depth frames
// not guarnteed to be exact because of the time it takes to execute the copy
bool KinectSensor::AllFramesReady()
{
    // Ensure the streams are started
    HRESULT hr = StartStreams();
    if( FAILED(hr) )
    {
        return false;
    }

    std::vector<HANDLE> events;
    GetWaitEvents( events );

    if( events.empty() )
    {
        return false;
    }

    // if all frame events as set, return true
    DWORD dwResult = WaitForMultipleObjects((DWORD)events.size(), events.data(), true, 0);
    if( dwResult == WAIT_OBJECT_0 )
    {
        return true;
    }

    return false;
}

// add events for enabled streams
void KinectSensor::GetWaitEvents( _Inout_ std::vector<HANDLE>& events )
{
    if( nullptr != m_pColorStream && KinectStreamStatusEnabled == m_pColorStream->GetStreamStatus() )
    {
        events.push_back( m_pColorStream->GetFrameReadyEvent() );
    }
    if( nullptr != m_pDepthStream && KinectStreamStatusEnabled == m_pDepthStream->GetStreamStatus() )
    {
        events.push_back( m_pDepthStream->GetFrameReadyEvent() );
    }
    if( nullptr != m_pSkeletonStream && KinectStreamStatusEnabled == m_pSkeletonStream->GetStreamStatus() )
    {
        events.push_back( m_pSkeletonStream->GetFrameReadyEvent() );
    }
}
