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

#include "DataStreamColor.h"
#include "AutoLock.h"

DataStreamColor::DataStreamColor()
    : DataStream()
    , m_imageType( NUI_IMAGE_TYPE_COLOR )
    , m_imageResolution( NUI_IMAGE_RESOLUTION_640x480 )
{
}
DataStreamColor::~DataStreamColor()
{
}

void DataStreamColor::Initialize( _In_ INuiSensor* pNuiSensor )
{
    AttachDevice( pNuiSensor );
}

void DataStreamColor::Initialize( NUI_IMAGE_TYPE type, NUI_IMAGE_RESOLUTION resolution, _In_ INuiSensor* pNuiSensor )
{
    bool bChanged = false;

    if( m_imageType != type )
    {
        m_imageType = type;
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
HRESULT DataStreamColor::StartStream()
{
    return OpenStream();
}

void DataStreamColor::StopStream()
{
    RemoveDevice();
}

// Color
HRESULT DataStreamColor::OpenStream()
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

    // Open color stream
    AutoLock lock( m_nuiLock );
    HRESULT hr = m_pNuiSensor->NuiImageStreamOpen( m_imageType,
                                                m_imageResolution,
                                                0,
                                                2,
                                                GetFrameReadyEvent(),
                                                &m_hStreamHandle );
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
void DataStreamColor::SetImageType( NUI_IMAGE_TYPE type )
{
    // only color image types accepted
    switch (type)
    {
    case NUI_IMAGE_TYPE_COLOR:
    case NUI_IMAGE_TYPE_COLOR_YUV:
    case NUI_IMAGE_TYPE_COLOR_INFRARED:
    case NUI_IMAGE_TYPE_COLOR_RAW_BAYER:
        m_imageType = type;
        break;
    default:
        break;
    }
}
void DataStreamColor::SetImageResolution( NUI_IMAGE_RESOLUTION resolution )
{
    switch (resolution)
    {
    case NUI_IMAGE_RESOLUTION_640x480:
    case NUI_IMAGE_RESOLUTION_1280x960:
        m_imageResolution = resolution;
        break;

    default:
        break;
    }
}

void DataStreamColor::GetFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    if( nullptr == pFrame || ( pFrame->dwStructSize != sizeof(KINECT_IMAGE_FRAME_FORMAT) ) )
    {
        if( nullptr != pFrame )
        {
            // these shouldn't be incorrect
            assert( pFrame->dwStructSize == sizeof(KINECT_IMAGE_FRAME_FORMAT) );

            // reset the values to be explicit
            ZeroMemory( pFrame, sizeof(KINECT_IMAGE_FRAME_FORMAT) );
            pFrame->dwStructSize = sizeof(KINECT_IMAGE_FRAME_FORMAT);
        }
        return;
    }

    // don't need a sensor to determine the size based on the ImageResolution
    NuiImageResolutionToSize( m_imageResolution, pFrame->dwWidth, pFrame->dwHeight );

    // based on the image type set byptes per pixel
    switch( m_imageType )
    {
    case NUI_IMAGE_TYPE_COLOR_RAW_BAYER:
        pFrame->cbBytesPerPixel = 1;
        break;
    case NUI_IMAGE_TYPE_COLOR_INFRARED:
        pFrame->cbBytesPerPixel = 2;
        break;
    default: // RGB
        pFrame->cbBytesPerPixel = 4;
    }

    // set the size of the buffer
    pFrame->cbBufferSize = pFrame->dwWidth * pFrame->dwHeight * pFrame->cbBytesPerPixel;
}

HRESULT DataStreamColor::GetFrameData( ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    if( nullptr == pImageBuffer )
    {
        return E_INVALIDARG;
    }

    HRESULT hr = ProcessImageFrame( &m_ImageFrame, cbBufferSize, pImageBuffer, liTimeStamp );
    
    if( FAILED(hr) )
    {
        return hr;
    }

    // post process frame?

    return hr;
}
