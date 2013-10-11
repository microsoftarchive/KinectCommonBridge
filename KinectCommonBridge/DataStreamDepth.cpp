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

#include "DataStreamDepth.h"
#include "AutoLock.h"

DataStreamDepth::DataStreamDepth()
    : DataStream()
    , m_imageResolution(NUI_IMAGE_RESOLUTION_640x480)
    , m_imageType(NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX)
    , m_bNearMode(false)
    , m_bAlignToColor(false)
{
}
DataStreamDepth::~DataStreamDepth()
{
}

void DataStreamDepth::Initialize( _In_ INuiSensor* pNuiSensor )
{
    AttachDevice( pNuiSensor );
}
void DataStreamDepth::Initialize( bool bNearMode, NUI_IMAGE_RESOLUTION resolution, _In_ INuiSensor* pNuiSensor )
{
    bool bChanged = false;

    if( m_bNearMode != bNearMode )
    {
        m_bNearMode = bNearMode;
        bChanged = true;
    }

    if( m_imageResolution != resolution )
    {
        m_imageResolution = resolution;
        bChanged = true;
    }
    
    if( bChanged )
    {
        m_started = false;
    }

    // send the sensor to the base class
    Initialize( pNuiSensor );
}

HRESULT DataStreamDepth::StartStream()
{
    return OpenStream();
}

void DataStreamDepth::StopStream()
{
    RemoveDevice();
}

// open the stream with the configuration params
HRESULT DataStreamDepth::OpenStream()
{
    // if there is no device, disable the stream
    if( nullptr == m_pNuiSensor )
    {
        RemoveDevice();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    if( m_started )
    {
        return S_OK;
    }

    AutoLock lock( m_nuiLock );
    m_imageType = HasSkeletalEngine(m_pNuiSensor) ? NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX : NUI_IMAGE_TYPE_DEPTH;

    // Open depth stream
    HRESULT hr = m_pNuiSensor->NuiImageStreamOpen( m_imageType,
                                                m_imageResolution,
                                                0,
                                                2,
                                                GetFrameReadyEvent(),
                                                &m_hStreamHandle);
    if( SUCCEEDED(hr) )
    {
        m_started = true;
    }
    else
    {
        m_started = false;
    }

    return hr;
}

void DataStreamDepth::SetAlignToColor( bool bAlignToColor )
{
    m_bAlignToColor = bAlignToColor;
}

void DataStreamDepth::SetNearMode( bool bNearMode )
{
    m_bNearMode = bNearMode;
    if( nullptr != m_pNuiSensor && INVALID_HANDLE_VALUE != m_hStreamHandle )
    {
        AutoLock lock(m_nuiLock);
        HRESULT hr = m_pNuiSensor->NuiImageStreamSetImageFrameFlags( m_hStreamHandle, (m_bNearMode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0) );
    }
}

void DataStreamDepth::SetImageResolution(NUI_IMAGE_RESOLUTION resolution)
{
    m_imageResolution = resolution;
}

void DataStreamDepth::SetImageType(NUI_IMAGE_TYPE type)
{
    m_imageType = type;
}


void DataStreamDepth::GetFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    if( nullptr == pFrame || ( pFrame->dwStructSize != sizeof(KINECT_IMAGE_FRAME_FORMAT) ) )
    {
        if( nullptr != pFrame )
        { // reset the values to be explicit
            ZeroMemory( pFrame, sizeof(KINECT_IMAGE_FRAME_FORMAT) );
            pFrame->dwStructSize = sizeof(KINECT_IMAGE_FRAME_FORMAT);
        }
        return;
    }

    NuiImageResolutionToSize( m_imageResolution, pFrame->dwWidth, pFrame->dwHeight );
    pFrame->cbBytesPerPixel = sizeof(short);
    pFrame->cbBufferSize = pFrame->dwWidth * pFrame->dwHeight * pFrame->cbBytesPerPixel;

}
HRESULT DataStreamDepth::GetFrameData( ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    if( nullptr == pImageBuffer )
    {
        return E_INVALIDARG;
    }

    HRESULT hr =  ProcessImageFrame( &m_ImageFrame, cbBufferSize, pImageBuffer, liTimeStamp );

    if( SUCCEEDED(hr) && m_bAlignToColor )
    {
        // call coordinate mapping funciton
    }

    return hr;
}
