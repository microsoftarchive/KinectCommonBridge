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

#pragma once

#include "KinectCommonBridgeLib.h"
#include "CriticalSection.h"
#include <memory>

class CoordinateMapper
{
public:
    CoordinateMapper();
    ~CoordinateMapper();

    void AttachDevice( _In_ INuiSensor* pNuiSensor );
    void RemoveDevice();

    HRESULT MapColorFrameToDepthFrame(
         NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
         NUI_IMAGE_RESOLUTION eDepthResolution,
         DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
         DWORD cDepthPoints, _Inout_cap_(cDepthPoints) NUI_DEPTH_IMAGE_POINT *pDepthPoints );

    HRESULT MapColorFrameToSkeletonFrame(
        NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
        NUI_IMAGE_RESOLUTION eDepthResolution,
        DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
        DWORD cSkeletonPoints, _Inout_cap_(cSkeletonPoints) Vector4 *pSkeletonPoints );

    HRESULT MapDepthFrameToColorFrame(
         NUI_IMAGE_RESOLUTION eDepthResolution,
         DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
         NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
         DWORD cColorPoints, _Inout_cap_(cColorPoints) NUI_COLOR_IMAGE_POINT *pColorPoints);

    HRESULT MapDepthFrameToSkeletonFrame(
         NUI_IMAGE_RESOLUTION eDepthResolution,
         DWORD cDepthPixels, _In_count_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL *pDepthPixels,
         DWORD cSkeletonPoints, _Inout_cap_(cSkeletonPoints) Vector4 *pSkeletonPoints);

    HRESULT MapDepthPointToColorPoint(
         NUI_IMAGE_RESOLUTION eDepthResolution, 
         _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint,
         NUI_IMAGE_TYPE eColorType, NUI_IMAGE_RESOLUTION eColorResolution,
         _Inout_ NUI_COLOR_IMAGE_POINT *pColorPoint );

    HRESULT MapDepthPointToSkeletonPoint(
         NUI_IMAGE_RESOLUTION eDepthResolution,
         _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint,
         _Inout_ Vector4 *pSkeletonPoint);

    HRESULT MapSkeletonPointToColorPoint(
        _Inout_ Vector4 *pSkeletonPoint,
         NUI_IMAGE_TYPE eColorType,
         NUI_IMAGE_RESOLUTION eColorResolution,
         _Inout_ NUI_COLOR_IMAGE_POINT *pColorPoint);

    HRESULT MapSkeletonPointToDepthPoint(
        _Inout_ Vector4 *pSkeletonPoint,
         NUI_IMAGE_RESOLUTION eDepthResolution,
         _Inout_ NUI_DEPTH_IMAGE_POINT *pDepthPoint);

    NUI_COLOR_IMAGE_POINT* CreateColorPoints( NUI_IMAGE_RESOLUTION eDepthResolution, _Inout_ DWORD& cColorPoints );
    NUI_DEPTH_IMAGE_PIXEL* CreateDepthPixels( NUI_IMAGE_RESOLUTION eDepthResolution, _Inout_ DWORD& cDepthPixels);
    NUI_DEPTH_IMAGE_POINT* CreateDepthPoints( NUI_IMAGE_RESOLUTION eColorResolution, _Inout_ DWORD& cDepthPoints );
    Vector4* CreateSkeletonPoints(NUI_IMAGE_RESOLUTION eSkeletonResolution, _Inout_ DWORD& cSkeletonPoints);

private:
    HRESULT IsSensorValid();
    HRESULT AllocateColorPoints(NUI_IMAGE_RESOLUTION eDepthResolution);
    HRESULT AllocateDepthPixels(NUI_IMAGE_RESOLUTION eDepthResolution);
    HRESULT AllocateDepthPoints(NUI_IMAGE_RESOLUTION eColorResolution);
    HRESULT AllocateSkeletonPoints(NUI_IMAGE_RESOLUTION eResolution);

private:
    CriticalSection                 m_nuiLock;
    CComPtr<INuiSensor>             m_pNuiSensor;

    CComPtr<INuiCoordinateMapper>   m_pNuiCoordinateMapper;
};
