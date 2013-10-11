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

#include "KinectCommonBridgeLib.h"
#include "SensorManager.h"

bool APIENTRY KinectIsHandleValid( KCBHANDLE kcbHandle )
{
    if( KCB_INVALID_HANDLE != kcbHandle )
    {
        return true;
    }

    return false;
}

// for enumerating sensors
UINT APIENTRY KinectGetPortIDCount()
{
    return SensorManager::GetInstance()->GetSensorCount();
}

bool APIENTRY KinectGetPortIDByIndex( UINT index, ULONG cchPortID, _Out_cap_(cchPortID) WCHAR* pwcPortID )
{
    return SensorManager::GetInstance()->GetPortIDByIndex( index, cchPortID, pwcPortID );
}

//KinectSensor ctor/dtor
KCBHANDLE APIENTRY KinectOpenDefaultSensor()
{
    // gets the first available sensor and opens the default streams
    return SensorManager::GetInstance()->OpenDefaultSensor();
}

KCBHANDLE APIENTRY KinectOpenSensor( _In_z_ const WCHAR* wcPortID )
{
    // get a sensor from the manager with a specific PortID
    return SensorManager::GetInstance()->OpenSensorByPortID( wcPortID );
}

void APIENTRY KinectCloseSensor( KCBHANDLE kcbHandle )
{
    SensorManager::GetInstance()->CloseSensorHandle( kcbHandle );
}

// get the port id from the sensor
const WCHAR* APIENTRY KinectGetPortID( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor( kcbHandle, pSensor ) )
    {
        return pSensor->GetPortID();
    }

    return nullptr;
}

// determine the state of the sensor
KINECT_SENSOR_STATUS APIENTRY KinectGetKinectSensorStatus( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor( kcbHandle, pSensor ) )
    {
        return pSensor->GetKinectSensorStatus();
    }

    return KinectSensorStatusError;
}


// enable a stream
void APIENTRY KinectEnableIRStream( KCBHANDLE kcbHandle, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }

    // enable/ set the stream properties
    pSensor->EnableColorStream( NUI_IMAGE_TYPE_COLOR_INFRARED, resolution );

    // get the frame format for this stream
    if( nullptr != pFrame )
    {
        pSensor->GetColorFrameFormat(pFrame);
    }
}
void APIENTRY KinectEnableColorStream( KCBHANDLE kcbHandle, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    // enable/ set the stream properties
    pSensor->EnableColorStream( NUI_IMAGE_TYPE_COLOR, resolution );

    // get the frame format for this stream
    if( nullptr != pFrame )
    {
        pSensor->GetColorFrameFormat(pFrame);
    }
}
void APIENTRY KinectEnableDepthStream( KCBHANDLE kcbHandle, bool pNearMode, NUI_IMAGE_RESOLUTION pResolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    // enable/ set the stream properties
    pSensor->EnableDepthStream( pNearMode, pResolution );

    // get the frame format for this stream
    if( nullptr != pFrame )
    {
        pSensor->GetDepthFrameFormat(pFrame);
    }
}
void APIENTRY KinectEnableSkeletonStream( KCBHANDLE kcbHandle, bool bSeatedSkeltons, KINECT_SKELETON_SELECTION_MODE mode, _Inout_opt_ const NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothParams )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->EnableSkeletonStream( bSeatedSkeltons, mode, pSmoothParams );
}

// start streams
HRESULT APIENTRY KinectStartStreams( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartStreams();
}
HRESULT APIENTRY KinectStartIRStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartColorStream();
}
HRESULT APIENTRY KinectStartColorStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartColorStream();
}
HRESULT APIENTRY KinectStartDepthStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartDepthStream();
}
HRESULT APIENTRY KinectStartSkeletonStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartSkeletonStream();
}

// pause streams
void APIENTRY KinectPauseStreams( KCBHANDLE kcbHandle, bool bPause )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseStreams(bPause);
}
void APIENTRY KinectPauseIRStream( KCBHANDLE kcbHandle, bool bPause )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseColorStream(bPause);

}
void APIENTRY KinectPauseColorStream( KCBHANDLE kcbHandle, bool bPause )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseColorStream(bPause);

}
void APIENTRY KinectPauseDepthStream( KCBHANDLE kcbHandle, bool bPause )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseDepthStream(bPause);

}
void APIENTRY KinectPauseSkeletonStream( KCBHANDLE kcbHandle, bool bPause )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseSkeletonStream(bPause);

}

// stop streams
void APIENTRY KinectStopStreams( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopStreams();
}
void APIENTRY KinectStopIRStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopColorStream();
}
void APIENTRY KinectStopColorStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopColorStream();
}
void APIENTRY KinectStopDepthStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopDepthStream();
}
void APIENTRY KinectStopSkeletonStream( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopSkeletonStream();
}

// get the status of a stream
KINECT_STREAM_STATUS APIENTRY KinectGetIRStreamStatus( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetColorStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_STREAM_STATUS APIENTRY KinectGetColorStreamStatus( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetColorStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_STREAM_STATUS APIENTRY KinectGetDepthStreamStatus( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetDepthStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_STREAM_STATUS APIENTRY KinectGetSkeletonStreamStatus( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetSkeletonStreamStatus();
    }
    return KinectStreamStatusError;
}

// check if frame is ready
bool APIENTRY KinectIsColorFrameReady( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->ColorFrameReady();
    }
    return false;
}

bool APIENTRY KinectIsDepthFrameReady( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->DepthFrameReady();
    }
    return false;
}

bool APIENTRY KinectIsSkeletonFrameReady( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->SkeletonFrameReady();
    }
    return false;
}

bool APIENTRY KinectAnyFrameReady( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->AnyFrameReady();
    }
    return false;
}

bool APIENTRY KinectAllFramesReady( KCBHANDLE kcbHandle )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->AllFramesReady();
    }
    return false;
}

// get frame format structure
void APIENTRY KinectGetIRFrameFormat( KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->GetColorFrameFormat( pFrame );
}
void APIENTRY KinectGetColorFrameFormat( KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->GetColorFrameFormat( pFrame );
}
void APIENTRY KinectGetDepthFrameFormat( KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->GetDepthFrameFormat( pFrame );
}

// get the actual frame data
HRESULT APIENTRY KinectGetIRFrame( KCBHANDLE kcbHandle, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NOT_VALID_STATE;
    }
    
    return pSensor->GetColorFrame( cbBufferSize, pColorBuffer, liTimeStamp );
}
HRESULT APIENTRY KinectGetColorFrame( KCBHANDLE kcbHandle, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{    
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NOT_VALID_STATE;
    }
    
    return pSensor->GetColorFrame( cbBufferSize, pColorBuffer, liTimeStamp );
}
HRESULT APIENTRY KinectGetDepthFrame( KCBHANDLE kcbHandle, ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pDepthBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NOT_VALID_STATE;
    }
    
    return pSensor->GetDepthFrame( cbBufferSize, pDepthBuffer, liTimeStamp );
}
HRESULT APIENTRY KinectGetSkeletonFrame( KCBHANDLE kcbHandle, _Inout_ NUI_SKELETON_FRAME* pSkeletonFrame )
{
    if( nullptr == pSkeletonFrame )
    {
        return E_INVALIDARG;
    }

    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NOT_VALID_STATE;
    }
    
    return pSensor->GetSkeletonFrame( *pSkeletonFrame );
}
