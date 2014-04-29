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
#include "CoordinateMapper.h"

// determine if the handle is valid
KINECT_CB bool APIENTRY KinectIsHandleValid( KCBHANDLE kcbHandle )
{
	return KCB_INVALID_HANDLE != kcbHandle;
}

// for enumerating sensors
KINECT_CB UINT APIENTRY KinectGetPortIDCount()
{
    return SensorManager::GetInstance()->GetSensorCount();
}

KINECT_CB bool APIENTRY KinectGetPortIDByIndex(UINT index, ULONG cchPortID, _Out_cap_(cchPortID) WCHAR* pwcPortID)
{
    return SensorManager::GetInstance()->GetPortIDByIndex( index, cchPortID, pwcPortID );
}

//KinectSensor ctor/dtor
KINECT_CB KCBHANDLE APIENTRY KinectOpenDefaultSensor()
{
    // gets the first available sensor and opens the default streams
    return SensorManager::GetInstance()->OpenDefaultSensor();
}

KINECT_CB KCBHANDLE APIENTRY KinectOpenSensor(_In_z_ const WCHAR* wcPortID)
{
    // get a sensor from the manager with a specific PortID
    return SensorManager::GetInstance()->OpenSensorByPortID( wcPortID );
}

KINECT_CB void APIENTRY KinectCloseSensor(KCBHANDLE kcbHandle)
{
    SensorManager::GetInstance()->CloseSensorHandle( kcbHandle );
}

// get the port id from the sensor
KINECT_CB const WCHAR* APIENTRY KinectGetPortID(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor( kcbHandle, pSensor ) )
    {
        return pSensor->GetPortID();
    }

    return nullptr;
}

// determine the state of the sensor
KINECT_CB KINECT_SENSOR_STATUS APIENTRY KinectGetKinectSensorStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor( kcbHandle, pSensor ) )
    {
        return pSensor->GetKinectSensorStatus();
    }

    return KinectSensorStatusError;
}


// enable a stream
KINECT_CB void APIENTRY KinectEnableIRStream(KCBHANDLE kcbHandle, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame)
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
KINECT_CB void APIENTRY KinectEnableColorStream(KCBHANDLE kcbHandle, NUI_IMAGE_RESOLUTION resolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame)
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
KINECT_CB void APIENTRY KinectEnableDepthStream(KCBHANDLE kcbHandle, bool pNearMode, NUI_IMAGE_RESOLUTION pResolution, _Inout_opt_ KINECT_IMAGE_FRAME_FORMAT* pFrame)
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
KINECT_CB void APIENTRY KinectEnableSkeletonStream(KCBHANDLE kcbHandle, bool bSeatedSkeltons, KINECT_SKELETON_SELECTION_MODE mode, _Inout_opt_ NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothParams)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->EnableSkeletonStream( bSeatedSkeltons, mode, pSmoothParams );
}

// start streams
KINECT_CB HRESULT APIENTRY KinectStartStreams(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartStreams();
}
KINECT_CB HRESULT APIENTRY KinectStartIRStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartColorStream();
}
KINECT_CB HRESULT APIENTRY KinectStartColorStream(KCBHANDLE kcbHandle)
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
KINECT_CB HRESULT APIENTRY KinectStartSkeletonStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->StartSkeletonStream();
}

// pause streams
KINECT_CB void APIENTRY KinectPauseStreams(KCBHANDLE kcbHandle, bool bPause)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseStreams(bPause);
}
KINECT_CB void APIENTRY KinectPauseIRStream(KCBHANDLE kcbHandle, bool bPause)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseColorStream(bPause);

}
KINECT_CB void APIENTRY KinectPauseColorStream(KCBHANDLE kcbHandle, bool bPause)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseColorStream(bPause);

}
KINECT_CB void APIENTRY KinectPauseDepthStream(KCBHANDLE kcbHandle, bool bPause)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseDepthStream(bPause);

}
KINECT_CB void APIENTRY KinectPauseSkeletonStream(KCBHANDLE kcbHandle, bool bPause)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    pSensor->PauseSkeletonStream(bPause);

}

// stop streams
KINECT_CB void APIENTRY KinectStopStreams(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopStreams();
}
KINECT_CB void APIENTRY KinectStopIRStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopColorStream();
}
KINECT_CB void APIENTRY KinectStopColorStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopColorStream();
}
KINECT_CB void APIENTRY KinectStopDepthStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopDepthStream();
}
KINECT_CB void APIENTRY KinectStopSkeletonStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->StopSkeletonStream();
}

// get the status of a stream
KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetIRStreamStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetColorStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetColorStreamStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetColorStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetDepthStreamStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetDepthStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetSkeletonStreamStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->GetSkeletonStreamStatus();
    }
    return KinectStreamStatusError;
}

// check if frame is ready
KINECT_CB bool APIENTRY KinectIsColorFrameReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->ColorFrameReady();
    }
    return false;
}

KINECT_CB bool APIENTRY KinectIsDepthFrameReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->DepthFrameReady();
    }
    return false;
}

KINECT_CB bool APIENTRY KinectIsSkeletonFrameReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->SkeletonFrameReady();
    }
    return false;
}

KINECT_CB bool APIENTRY KinectAnyFrameReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->AnyFrameReady();
    }
    return false;
}

KINECT_CB bool APIENTRY KinectAllFramesReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->AllFramesReady();
    }
    return false;
}

// get frame format structure
KINECT_CB void APIENTRY KinectGetIRFrameFormat(KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return;
    }
    
    pSensor->GetColorFrameFormat( pFrame );
}
KINECT_CB void APIENTRY KinectGetColorFrameFormat(KCBHANDLE kcbHandle, _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame)
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
KINECT_CB HRESULT APIENTRY KinectGetIRFrame(KCBHANDLE kcbHandle, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    
    return pSensor->GetColorFrame( cbBufferSize, pColorBuffer, liTimeStamp );
}
KINECT_CB HRESULT APIENTRY KinectGetColorFrame(KCBHANDLE kcbHandle, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp)
{    
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    
    return pSensor->GetColorFrame( cbBufferSize, pColorBuffer, liTimeStamp );
}
KINECT_CB HRESULT APIENTRY KinectGetDepthFrame(KCBHANDLE kcbHandle, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pDepthBuffer, _Out_opt_ LONGLONG* liTimeStamp)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    
    return pSensor->GetDepthFrame( cbBufferSize, pDepthBuffer, liTimeStamp );
}
KINECT_CB HRESULT APIENTRY KinectGetSkeletonFrame(KCBHANDLE kcbHandle, _Inout_ NUI_SKELETON_FRAME* pSkeletonFrame)
{
    if( nullptr == pSkeletonFrame )
    {
        return E_NUI_BADINDEX;
    }

    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    
    return pSensor->GetSkeletonFrame( *pSkeletonFrame );
}

KINECT_CB HRESULT APIENTRY KinectGetDepthImagePixels(KCBHANDLE kcbHandle, ULONG cbDepthPixels, _Inout_cap_(cbDepthPixels) NUI_DEPTH_IMAGE_PIXEL* pDepthPixels, _Out_opt_ LONGLONG* liTimeStamp)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    return pSensor->GetDepthPixels( cbDepthPixels, pDepthPixels, liTimeStamp );
}

// Coordinate mapping functions
KINECT_CB HRESULT APIENTRY KinectMapColorFrameToDepthFrame( KCBHANDLE kcbHandle, 
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _Inout_cap_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    DWORD cDepthPoints, _Inout_cap_(cDepthPoints) NUI_DEPTH_IMAGE_POINT *pDepthPoints )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapColorFrameToDepthFrame( 
        eColorType, eColorResolution, eDepthResolution,
        cDepthPixels, pDepthPixels,
        cDepthPoints, pDepthPoints );
}

KINECT_CB HRESULT APIENTRY KinectMapColorFrameToSkeletonFrame( KCBHANDLE kcbHandle, 
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _Inout_cap_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    DWORD cSkeletonPoints, _Inout_cap_(cSkeletonPoints) Vector4 *pSkeletonPoints )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapColorFrameToSkeletonFrame( 
        eColorType, eColorResolution, eDepthResolution,
        cDepthPixels, pDepthPixels,
        cSkeletonPoints, pSkeletonPoints );
}

KINECT_CB HRESULT APIENTRY KinectMapDepthFrameToColorFrame( KCBHANDLE kcbHandle, 
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _Inout_cap_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    DWORD cColorPoints, _Inout_cap_(cColorPoints) NUI_COLOR_IMAGE_POINT *pColorPoints )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapDepthFrameToColorFrame(
        eDepthResolution,
        cDepthPixels, pDepthPixels,
        eColorType, eColorResolution,
        cColorPoints, pColorPoints );
}

KINECT_CB HRESULT APIENTRY KinectMapDepthFrameToSkeletonFrame( KCBHANDLE kcbHandle, 
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _Inout_cap_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    DWORD cSkeletonPoints, _Inout_cap_(cSkeletonPoints) Vector4 *pSkeletonPoints )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapDepthFrameToSkeletonFrame(
        eDepthResolution,
        cDepthPixels, pDepthPixels,
        cSkeletonPoints, pSkeletonPoints );
}

KINECT_CB HRESULT APIENTRY KinectMapDepthPointToColorPoint( KCBHANDLE kcbHandle, 
    NUI_IMAGE_RESOLUTION eDepthResolution, _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint,
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    _Inout_ NUI_COLOR_IMAGE_POINT *pColorPoint )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapDepthPointToColorPoint(
        eDepthResolution, pDepthPoint,
        eColorType, eColorResolution,
        pColorPoint );
}

KINECT_CB HRESULT APIENTRY KinectMapDepthPointToSkeletonPoint( KCBHANDLE kcbHandle, 
    NUI_IMAGE_RESOLUTION eDepthResolution,
    _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint,
    _Inout_ Vector4 *pSkeletonPoint )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapDepthPointToSkeletonPoint( eDepthResolution, pDepthPoint, pSkeletonPoint );
}

KINECT_CB HRESULT APIENTRY KinectMapSkeletonPointToColorPoint( KCBHANDLE kcbHandle, 
    _Inout_ Vector4 *pSkeletonPoint,
    NUI_IMAGE_TYPE eColorType,
    NUI_IMAGE_RESOLUTION eColorResolution,
    _Inout_ NUI_COLOR_IMAGE_POINT *pColorPoint )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapSkeletonPointToColorPoint( pSkeletonPoint, eColorType, eColorResolution, pColorPoint );
}

KINECT_CB HRESULT APIENTRY KinectMapSkeletonPointToDepthPoint(KCBHANDLE kcbHandle,
    _Inout_ Vector4 *pSkeletonPoint,
    NUI_IMAGE_RESOLUTION eDepthResolution,
    _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }

    CoordinateMapper& pMapper = pSensor->GetCoordinateMapper();

    return pMapper.MapSkeletonPointToDepthPoint( pSkeletonPoint, eDepthResolution, pDepthPoint );
}

KINECT_CB HRESULT APIENTRY KinectGetColorFrameFromDepthPoints(KCBHANDLE kcbHandle,
    DWORD cDepthPoints, _In_count_(cDepthPoints) NUI_DEPTH_IMAGE_POINT *pDepthPoints,
    ULONG cBufferSize, _Inout_cap_(cBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( !SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return E_NUI_BADINDEX;
    }
    
    return pSensor->GetColorFrameFromDepthPoints( cDepthPoints, pDepthPoints, cBufferSize, pColorBuffer, liTimeStamp );
}

KINECT_CB void APIENTRY KinectEnableAudioStream(KCBHANDLE kcbHandle, _In_opt_ AEC_SYSTEM_MODE* eAECSystemMode, _In_opt_ bool* bGainBounder)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return;
    }

	pSensor->EnableAudioStream(eAECSystemMode, bGainBounder);
}

KINECT_CB HRESULT APIENTRY KinectStartAudioStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_NUI_BADINDEX;
    }

    return pSensor->StartAudioStream();
}

KINECT_CB void APIENTRY KinectPauseAudioStream(KCBHANDLE kcbHandle, bool bPause)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return;
    }
    return pSensor->PauseAudioStream(bPause);
}

KINECT_CB void APIENTRY KinectStopAudioStream(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return;
    }
    pSensor->StopAudioStream();
}

KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetAudioStreamStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return pSensor->GetAudioStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_CB KINECT_STREAM_STATUS APIENTRY KinectGetSpeechStatus(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return pSensor->GetAudioStreamStatus();
    }
    return KinectStreamStatusError;
}

KINECT_CB HRESULT APIENTRY KinectGetAudioSample(KCBHANDLE kcbHandle,
    _Out_ DWORD* cbProduced, _Out_ BYTE** ppbOutputBuffer,
    _Out_ DWORD* dwStatus, _Out_opt_ LONGLONG *llTimeStamp, _Out_opt_ LONGLONG *llTimeLength,
    _Out_opt_ double *beamAngle, _Out_opt_ double *sourceAngle, _Out_opt_ double *sourceConfidence  )
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_NUI_BADINDEX;
    }

    return pSensor->GetAudioSample(cbProduced, ppbOutputBuffer, dwStatus, llTimeStamp, llTimeLength, beamAngle, sourceAngle, sourceConfidence);
}

KINECT_CB HRESULT APIENTRY KinectSetInputVolumeLevel(KCBHANDLE kcbHandle, float fLevelDB)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->SetInputVolumeLevel(fLevelDB);
}

#ifdef KCB_ENABLE_SPEECH
KINECT_CB void APIENTRY KinectEnableSpeech(KCBHANDLE kcbHandle, _In_ const WCHAR* wcGrammarFileName, _In_opt_ KCB_SPEECH_LANGUAGE* sLanguage, _In_opt_ ULONGLONG* ullEventInterest, _In_opt_ bool* bAdaptation)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return;
    }
    pSensor->EnableSpeech(wcGrammarFileName, sLanguage, ullEventInterest, bAdaptation);
}

KINECT_CB HRESULT APIENTRY KinectStartSpeech(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_NUI_BADINDEX;
    }

    return pSensor->StartSpeech();
}

KINECT_CB void APIENTRY KinectStopSpeech(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return;
    }
    pSensor->StopAudioStream();
}

KINECT_CB HRESULT APIENTRY KinectGetSpeechEvent(KCBHANDLE kcbHandle, _In_ SPEVENT* pSPEvent, _In_ ULONG* pulFetched)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_NUI_BADINDEX;
    }
    return pSensor->GetSpeechEvent(pSPEvent, pulFetched);
}

KINECT_CB bool APIENTRY KinectIsSpeechEventReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return pSensor->SpeechEventReady();
    }
    return false;
}
#endif

#ifdef KCB_ENABLE_FT
KINECT_CB HRESULT APIENTRY KinectEnableFaceTracking(KCBHANDLE kcbHandle, bool bNearMode)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_FAIL;
    }

    // enable/ set the stream properties
    return pSensor->EnableFaceTracking(bNearMode);
}

KINECT_CB void APIENTRY KinectDisableFaceTracking(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return;
    }

    // enable/ set the stream properties
    pSensor->DisableFaceTracking();
}

KINECT_CB bool APIENTRY KinectGetColorStreamCameraConfig(KCBHANDLE kcbHandle, FT_CAMERA_CONFIG& config)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return false;
    }

    // get the camera config
    return pSensor->GetColorStreamCameraConfig(config);
}


KINECT_CB bool APIENTRY KinectGetDepthStreamCameraConfig(KCBHANDLE kcbHandle, FT_CAMERA_CONFIG& config)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return false;
    }

    // get the camera config
    return pSensor->GetDepthStreamCameraConfig(config);
}



KINECT_CB bool APIENTRY KinectIsFaceTrackingResultReady(KCBHANDLE kcbHandle)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if( SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor) )
    {
        return pSensor->ColorFrameReady() && pSensor->DepthFrameReady();
    }
    return false;
}


KINECT_CB HRESULT APIENTRY KinectGetFaceTrackingResult(KCBHANDLE kcbHandle, _Out_ IFTResult** ppResult)
{
    std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return E_NUI_BADINDEX;
    }

    return pSensor->GetFaceTrackingResult(ppResult);
}

KINECT_CB HRESULT KinectGetFaceTrackingImage(KCBHANDLE kcbHandle, IFTImage** pImage)
{
	std::shared_ptr<KinectSensor> pSensor = nullptr;
	if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
	{
		return E_NUI_BADINDEX;
	}

	return pSensor->GetFaceTrackingImage(pImage);
	
}

KINECT_CB float KinectGetXCenterFace(KCBHANDLE kcbHandle)
{
	 std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return 0.f;
    }
	return pSensor->GetXCenterFace();
}

KINECT_CB float KinectGetYCenterFace(KCBHANDLE kcbHandle)
{	
	 std::shared_ptr<KinectSensor> pSensor = nullptr;
    if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
    {
        return 0.f;
    }

	return pSensor->GetYCenterFace();
}

KINECT_CB HRESULT KinectGetFaceTracker(KCBHANDLE kcbHandle, IFTFaceTracker** pFaceTracker)
{
	std::shared_ptr<KinectSensor> pSensor = nullptr;
	if (!SensorManager::GetInstance()->GetKinectSensor(kcbHandle, pSensor))
	{
		return E_NUI_BADINDEX;
	}

	return pSensor->GetFaceTracker(pFaceTracker);
}
#endif
