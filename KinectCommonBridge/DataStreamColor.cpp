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

#include <ppl.h>

DataStreamColor::DataStreamColor()
    : DataStream()
    , m_imageType( NUI_IMAGE_TYPE_COLOR )
    , m_imageResolution( NUI_IMAGE_RESOLUTION_INVALID )
    , m_dwWidth(0)
    , m_dwHeight(0) 
    , m_cBufferSize(0)
    , m_pImageBuffer(nullptr)
    , m_cDepthPoints(0)
    , m_pDepthPoints(nullptr)
{

}
DataStreamColor::~DataStreamColor()
{
    StopStream();
}

void DataStreamColor::Initialize(_In_ INuiSensor* pNuiSensor)
{
    DataStream::Initialize(pNuiSensor);
}

void DataStreamColor::Initialize( NUI_IMAGE_TYPE type, NUI_IMAGE_RESOLUTION resolution, _In_ INuiSensor* pNuiSensor )
{
    AutoLock lock( m_nuiLock );

    bool bChanged = false;

    if( m_imageType != type )
    {
        SetImageType(type);
        bChanged = true;
    }

    if( m_imageResolution != resolution )
    {
        SetImageResolution(resolution);
        bChanged = true;
    }

    if( bChanged )
    {
        m_started = false;
    }

#ifdef KCB_ENABLE_FT
    SetCameraConfig();
#endif

    // send the sensor to the base class
    Initialize( pNuiSensor );
}
HRESULT DataStreamColor::StartStream()
{
    AutoLock lock( m_nuiLock );

    return OpenStream();
}

void DataStreamColor::StopStream()
{
    AutoLock lock( m_nuiLock );

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
#ifdef KCB_ENABLE_FT
    SetCameraConfig();
#endif
}
void DataStreamColor::SetImageResolution( NUI_IMAGE_RESOLUTION resolution )
{
    NuiImageResolutionToSize( m_imageResolution, m_dwWidth, m_dwHeight );

    switch (resolution)
    {
    case NUI_IMAGE_RESOLUTION_640x480:
    case NUI_IMAGE_RESOLUTION_1280x960:
        m_imageResolution = resolution;
        break;

    default:
        break;
    }

#ifdef KCB_ENABLE_FT
    SetCameraConfig();
#endif
}
void DataStreamColor::GetFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    AutoLock lock( m_nuiLock );

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

HRESULT DataStreamColor::GetFrameData( ULONG cBufferSize, _Out_cap_(cBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    AutoLock lock( m_nuiLock );

    if( nullptr == pImageBuffer )
    {
        return E_INVALIDARG;
    }

    m_cBufferSize = cBufferSize;
    m_pImageBuffer = pImageBuffer;

    if (nullptr != m_pDepthPoints)
    {
        m_cDepthPoints = 0;
        m_pDepthPoints = nullptr;
    }

    return ProcessImageFrame( liTimeStamp );
}

HRESULT DataStreamColor::GetColorAlignedToDepth(
    ULONG cbDepthPoints, _In_count_(cbDepthPoints) const NUI_DEPTH_IMAGE_POINT* pDepthPoints, 
    ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    AutoLock lock( m_nuiLock );

    if( nullptr == pImageBuffer || nullptr == pDepthPoints )
    {
        return E_INVALIDARG;
    }

    m_cBufferSize = cbBufferSize;
    m_pImageBuffer = pImageBuffer;

    m_cDepthPoints = cbDepthPoints;
    m_pDepthPoints = pDepthPoints;

    return ProcessImageFrame( liTimeStamp );
}

void DataStreamColor::CopyData( _In_ void* pImageFrame )
{
    NUI_IMAGE_FRAME* pFrame = reinterpret_cast<NUI_IMAGE_FRAME*>(pImageFrame); 
    if( nullptr == pFrame )
    {
        return;
    }

    if (nullptr != m_pDepthPoints)
    {
        CopyColorToDepth(pFrame);
    }
    else
    {
        // copy data from the frame
        INuiFrameTexture* pTexture = pFrame->pFrameTexture;

        // Lock the frame data so the Kinect knows not to modify it while we are reading it
        NUI_LOCKED_RECT lockedRect;
        pTexture->LockRect( 0, &lockedRect, NULL, 0 );

        // Make sure we've received valid data
        if (lockedRect.Pitch != 0)
        {
            memcpy_s( m_pImageBuffer, m_cBufferSize, lockedRect.pBits, lockedRect.size );
        }

        // Unlock frame data
        pTexture->UnlockRect(0);
    }
}

void DataStreamColor::CopyColorToDepth(_In_ NUI_IMAGE_FRAME *pImageFrame)
{
    // copy data from the frame
    INuiFrameTexture* pTexture = pImageFrame->pFrameTexture;

    // Lock the frame data so the Kinect knows not to modify it while we are reading it
    NUI_LOCKED_RECT lockedRect;
    pTexture->LockRect(0, &lockedRect, NULL, 0);

    // Make sure we've received valid data
    if (lockedRect.Pitch != 0)
    {
        size_t bpp = 4;  // 4bpp - BGR32
        if (NUI_IMAGE_TYPE_COLOR_INFRARED == m_imageType)
        {
            bpp = 2;
        }
        else if (NUI_IMAGE_TYPE_COLOR_RAW_BAYER == m_imageType)
        {
            bpp = 1;
        }

        // total width for a row of pixels
        size_t dwByteWidthTotal = m_dwWidth * bpp;

        Concurrency::parallel_for(size_t(0), size_t(m_cDepthPoints), [&](size_t index)
            //for( size_t index = 0; index < m_cbDepthPoints; ++index )
        {
            NUI_DEPTH_IMAGE_POINT depthPoint = m_pDepthPoints[index];

            size_t imageBufferOffset = depthPoint.y * dwByteWidthTotal + (depthPoint.x * bpp);
            size_t colorBufferOffset = index * bpp;

            if (imageBufferOffset + bpp <= (size_t) m_cBufferSize && colorBufferOffset + bpp <= (size_t) lockedRect.size)
            {
                // for the pixels that are mapped, map the depth point with the correct color value
                for (size_t i = 0; i < bpp; ++i)
                {
                    m_pImageBuffer[imageBufferOffset + i] = lockedRect.pBits[colorBufferOffset + i];
                }
            }
        });
    }

    // Unlock frame data
    pTexture->UnlockRect(0);
}

#ifdef KCB_ENABLE_FT

void DataStreamColor::SetCameraConfig()
{

    DWORD m_dwWidth = 0;
    DWORD m_dwHeight = 0;

    NuiImageResolutionToSize(m_imageResolution, m_dwWidth, m_dwHeight);

    FLOAT focalLength = 0.f;

    if (m_dwWidth == 640 && m_dwHeight == 480)
    {
        focalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
    }
    else if (m_dwWidth == 1280 && m_dwHeight == 960)
    {
        focalLength = NUI_CAMERA_COLOR_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
    }

    m_cameraConfig.FocalLength = focalLength;
    m_cameraConfig.Width = m_dwWidth;
    m_cameraConfig.Height = m_dwHeight;
}

#endif
