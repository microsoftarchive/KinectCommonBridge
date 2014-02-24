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

#include <ppl.h>

DataStreamDepth::DataStreamDepth()
    : DataStream()
    , m_imageResolution(NUI_IMAGE_RESOLUTION_INVALID)
    , m_imageType(NUI_IMAGE_TYPE_DEPTH)
    , m_bNearMode(false)
    , m_cDepthBuffer(0)
    , m_pDepthBuffer(nullptr)
    , m_cDepthPixels(0)
    , m_pDepthPixels(nullptr)
{
}
DataStreamDepth::~DataStreamDepth()
{
    StopStream();
}

void DataStreamDepth::Initialize(_In_ INuiSensor* pNuiSensor)
{
    DataStream::Initialize(pNuiSensor);
}

void DataStreamDepth::Initialize( bool bNearMode, NUI_IMAGE_RESOLUTION resolution, _In_ INuiSensor* pNuiSensor )
{
    AutoLock lock(m_nuiLock);

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

#ifdef KCB_ENABLE_FT
    SetCameraConfig();
#endif
    
    // send the sensor to the base class
    Initialize( pNuiSensor );
}

HRESULT DataStreamDepth::StartStream()
{
    AutoLock lock( m_nuiLock );

    return OpenStream();
}

void DataStreamDepth::StopStream()
{
    AutoLock lock( m_nuiLock );

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

void DataStreamDepth::SetNearMode( bool bNearMode )
{
    AutoLock lock(m_nuiLock);

    m_bNearMode = bNearMode;
    if( nullptr != m_pNuiSensor && INVALID_HANDLE_VALUE != m_hStreamHandle )
    {
        AutoLock lock(m_nuiLock);
        HRESULT hr = m_pNuiSensor->NuiImageStreamSetImageFrameFlags( m_hStreamHandle, (m_bNearMode ? NUI_IMAGE_STREAM_FLAG_ENABLE_NEAR_MODE : 0) );
    }
}

void DataStreamDepth::GetFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame )
{
    AutoLock lock(m_nuiLock);

    if (nullptr == pFrame || (pFrame->dwStructSize != sizeof(KINECT_IMAGE_FRAME_FORMAT)))
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
HRESULT DataStreamDepth::GetFrameData( ULONG cBufferSize, _Inout_cap_(cBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    AutoLock lock( m_nuiLock );

    if( nullptr == pImageBuffer )
    {
        return E_INVALIDARG;
    }

    m_cDepthBuffer = cBufferSize;
    m_pDepthBuffer = pImageBuffer;

    if( 0 != m_cDepthPixels )
    {
        m_cDepthPixels = 0;
        m_pDepthPixels = nullptr;
    }

    return ProcessImageFrame( liTimeStamp );
}

void DataStreamDepth::CopyData( _In_ void* pImageFrame )
{
    NUI_IMAGE_FRAME* pFrame = reinterpret_cast<NUI_IMAGE_FRAME*>(pImageFrame);
    if( nullptr == pFrame )
    {
        return;
    }

    if( nullptr != m_pDepthPixels )
    {
        CopyPixelData( pFrame );
    }
    else
    {
        CopyRawData( pFrame );
    }
}

void DataStreamDepth::CopyRawData( _In_ NUI_IMAGE_FRAME *pImageFrame )
{
    // copy data from the frame
    INuiFrameTexture* pTexture = pImageFrame->pFrameTexture;

    // Lock the frame data so the Kinect knows not to modify it while we are reading it
    NUI_LOCKED_RECT lockedRect;
    pTexture->LockRect( 0, &lockedRect, NULL, 0 );

    // Make sure we've received valid data
    if( lockedRect.Pitch != 0 )
    {
        memcpy_s( m_pDepthBuffer, m_cDepthBuffer, lockedRect.pBits, lockedRect.size );
    }

    // Unlock frame data
    pTexture->UnlockRect(0);
}

void DataStreamDepth::CopyPixelData( _In_ NUI_IMAGE_FRAME *pImageFrame )
{
    BOOL nearMode;
    CComPtr<INuiFrameTexture> pTexture;

    // Get the depth image pixel texture
    HRESULT hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture( m_hStreamHandle, pImageFrame, &nearMode, &pTexture );
    if (FAILED(hr))
    {
        return;
    }

    NUI_LOCKED_RECT lockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &lockedRect, NULL, 0);

    // Make sure we've received valid data
    if( lockedRect.Pitch != 0 )
    {
        const NUI_DEPTH_IMAGE_PIXEL* pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(lockedRect.pBits);
        
        const size_t sizeOfShort = sizeof(short);
        Concurrency::parallel_for(size_t(0), size_t(m_cDepthPixels), [&](size_t index)
        {
            m_pDepthPixels[index] = pBufferRun[index];

            // if we also want the raw depth buffer we can copy that as well
            if( nullptr != m_pDepthBuffer )
            {
                short packed = m_pDepthPixels[index].depth << NUI_IMAGE_PLAYER_INDEX_SHIFT | m_pDepthPixels[index].playerIndex;
                m_pDepthBuffer[index * sizeOfShort] = packed & 0xff;
                m_pDepthBuffer[index * sizeOfShort + 1] = packed >> 8 & 0xff;
            }
        } );
    }

    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    // release the pixel texture frame
    pTexture.Release();
}

HRESULT DataStreamDepth::GetDepthImagePixels( ULONG cbDepthPixels, _Inout_cap_(cbDepthPixels) NUI_DEPTH_IMAGE_PIXEL* pDepthPixelBuffer, _Out_opt_ LONGLONG* liTimeStamp )
{
    AutoLock lock( m_nuiLock );

    if( nullptr == pDepthPixelBuffer )
    {
        return E_INVALIDARG;
    }

    if( 0 != m_cDepthBuffer || nullptr != m_pDepthBuffer )
    {
        m_cDepthBuffer = 0;
        m_pDepthBuffer = nullptr;
    }

    m_cDepthPixels = cbDepthPixels;
    m_pDepthPixels = pDepthPixelBuffer;

    return ProcessImageFrame( liTimeStamp );
}

#ifdef KCB_ENABLE_FT
void DataStreamDepth::SetCameraConfig()
{
    DWORD width = 0;
    DWORD height = 0;

    NuiImageResolutionToSize(m_imageResolution, width, height);



    FLOAT focalLength = 0.f;

    if (width == 80 && height == 60)
    {
        focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS / 4.f;
    }
    else if (width == 320 && height == 240)
    {
        focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS;
    }
    else if (width == 640 && height == 480)
    {
        focalLength = NUI_CAMERA_DEPTH_NOMINAL_FOCAL_LENGTH_IN_PIXELS * 2.f;
    }

    m_cameraConfig.FocalLength = focalLength;
    m_cameraConfig.Width = width;
    m_cameraConfig.Height = height;
}
#endif