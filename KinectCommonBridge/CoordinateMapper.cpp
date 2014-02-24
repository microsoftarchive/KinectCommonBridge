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

#include "CoordinateMapper.h"
#include "AutoLock.h"

CoordinateMapper::CoordinateMapper()
    : m_pNuiSensor(nullptr)
    , m_pNuiCoordinateMapper(nullptr)
{
}

CoordinateMapper::~CoordinateMapper()
{
    RemoveDevice();
}

void CoordinateMapper::RemoveDevice()
{
    AutoLock lock(m_nuiLock);

    m_pNuiCoordinateMapper.Release();
    m_pNuiSensor.Release();
}

void CoordinateMapper::AttachDevice( _In_ INuiSensor* pNuiSensor )
{
    AutoLock lock(m_nuiLock);

    if( nullptr == pNuiSensor )
    {
        RemoveDevice();
        return;
    }
    
    if( m_pNuiSensor == pNuiSensor )
    {
        return;
    }

    pNuiSensor->AddRef();
    m_pNuiSensor.Attach( pNuiSensor );
}

HRESULT CoordinateMapper::IsSensorValid()
{
    AutoLock lock( m_nuiLock );

    if( nullptr == m_pNuiSensor )
    {
        return E_NUI_DEVICE_NOT_READY;
    }

    HRESULT hr = S_OK;

    if( nullptr == m_pNuiCoordinateMapper )
    {
        hr = m_pNuiSensor->NuiGetCoordinateMapper( &m_pNuiCoordinateMapper );
    }

    return hr;
}

HRESULT CoordinateMapper::MapColorFrameToDepthFrame(
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution, 
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    DWORD cDepthPoints, _Inout_cap_(cDepthPoints) NUI_DEPTH_IMAGE_POINT *pDepthPoints )
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapColorFrameToDepthFrame(
        eColorType, eColorResolution, 
        eDepthResolution,
        cDepthPixels, pDepthPixels,
        cDepthPoints, pDepthPoints );
}

HRESULT CoordinateMapper::MapColorFrameToSkeletonFrame(
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution, 
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    DWORD cSkeletonPoints, _Inout_cap_(cSkeletonPoints) Vector4 *pSkeletonPoints )
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapColorFrameToSkeletonFrame(
        eColorType, eColorResolution, eDepthResolution,
        cDepthPixels, pDepthPixels, 
        cSkeletonPoints, pSkeletonPoints );
}

HRESULT CoordinateMapper::MapDepthFrameToColorFrame(
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    DWORD cColorPoints, _Inout_cap_(cColorPoints) NUI_COLOR_IMAGE_POINT *pColorPoints)
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapDepthFrameToColorFrame( 
        eDepthResolution, 
        cDepthPixels, pDepthPixels, 
        eColorType, eColorResolution, 
        cColorPoints, pColorPoints );
}

HRESULT CoordinateMapper::MapDepthFrameToSkeletonFrame(
    NUI_IMAGE_RESOLUTION eDepthResolution,
    DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
    DWORD cSkeletonPoints, _Inout_cap_(cSkeletonPoints) Vector4 *pSkeletonPoints)
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapDepthFrameToSkeletonFrame(
        eDepthResolution, 
        cDepthPixels, pDepthPixels, 
        cSkeletonPoints, pSkeletonPoints);
}

HRESULT CoordinateMapper::MapDepthPointToColorPoint(
    NUI_IMAGE_RESOLUTION eDepthResolution, _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint,
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    _Inout_ NUI_COLOR_IMAGE_POINT *pColorPoint )
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapDepthPointToColorPoint(
        eDepthResolution, pDepthPoint, 
        eColorType, eColorResolution, 
        pColorPoint );
}


HRESULT CoordinateMapper::MapDepthPointToSkeletonPoint(
    NUI_IMAGE_RESOLUTION eDepthResolution, _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint,
    _Inout_ Vector4 *pSkeletonPoint)
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapDepthPointToSkeletonPoint(
        eDepthResolution, pDepthPoint,
        pSkeletonPoint );
}

HRESULT CoordinateMapper::MapSkeletonPointToColorPoint(
    _Inout_ Vector4 *pSkeletonPoint,
    NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
    _Inout_ NUI_COLOR_IMAGE_POINT *pColorPoint)
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapSkeletonPointToColorPoint(
        pSkeletonPoint, 
        eColorType, eColorResolution,
        pColorPoint );
}

HRESULT CoordinateMapper::MapSkeletonPointToDepthPoint(
    _Inout_ Vector4 *pSkeletonPoint,
    NUI_IMAGE_RESOLUTION eDepthResolution,
    _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint)
{
    AutoLock lock( m_nuiLock );

    HRESULT hr = IsSensorValid();
    if( FAILED(hr) )
    {
        return hr;
    }

    return m_pNuiCoordinateMapper->MapSkeletonPointToDepthPoint(
        pSkeletonPoint,
        eDepthResolution,
        pDepthPoint );
}

NUI_COLOR_IMAGE_POINT* CoordinateMapper::CreateColorPoints(NUI_IMAGE_RESOLUTION eResolution, _Inout_ DWORD& cPointCount)
{
    DWORD dwWidth, dwHeight;
    NuiImageResolutionToSize(eResolution, dwWidth, dwHeight);

    cPointCount = dwWidth * dwHeight;

    std::unique_ptr<NUI_COLOR_IMAGE_POINT[] > pColorPoints(new (std::nothrow) NUI_COLOR_IMAGE_POINT[cPointCount]);
    if (nullptr == pColorPoints)
    {
        cPointCount = 0;
        return nullptr;
    }

    // release requires caller to delete the array.
    return pColorPoints.release();
}

NUI_DEPTH_IMAGE_PIXEL* CoordinateMapper::CreateDepthPixels(NUI_IMAGE_RESOLUTION eResolution, _Inout_ DWORD& cPixelCount )
{
    DWORD dwWidth, dwHeight;
    NuiImageResolutionToSize(eResolution, dwWidth, dwHeight);

    cPixelCount = dwWidth * dwHeight;

    std::unique_ptr<NUI_DEPTH_IMAGE_PIXEL[] > pDepthPixels(new (std::nothrow) NUI_DEPTH_IMAGE_PIXEL[cPixelCount]);
    if (nullptr == pDepthPixels)
    {
        cPixelCount = 0;
        return nullptr;
    }

    // release requires caller to delete the array.
    return pDepthPixels.release();
}

NUI_DEPTH_IMAGE_POINT* CoordinateMapper::CreateDepthPoints(NUI_IMAGE_RESOLUTION eResolution, _Inout_ DWORD& cPointCount)
{
    DWORD dwWidth, dwHeight;
    NuiImageResolutionToSize(eResolution, dwWidth, dwHeight);

    cPointCount = dwWidth * dwHeight;

    std::unique_ptr<NUI_DEPTH_IMAGE_POINT[] > pDepthPoints(new (std::nothrow) NUI_DEPTH_IMAGE_POINT[cPointCount]);
    if (nullptr == pDepthPoints)
    {
        cPointCount = 0;
        return nullptr;
    }

    // release requires caller to delete the array.
    return pDepthPoints.release();
}

Vector4* CoordinateMapper::CreateSkeletonPoints(NUI_IMAGE_RESOLUTION eResolution, _Inout_ DWORD& cPointCount)
{
    DWORD dwWidth, dwHeight;
    NuiImageResolutionToSize(eResolution, dwWidth, dwHeight);

    cPointCount = dwWidth * dwHeight;

    std::unique_ptr<Vector4[] > pSkeletonPoints(new (std::nothrow) Vector4[cPointCount]);
    if (nullptr == pSkeletonPoints)
    {
        cPointCount = 0;
        return nullptr;
    }

    // release requires caller to delete the array.
    return pSkeletonPoints.release();
}
