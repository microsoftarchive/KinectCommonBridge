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
#include "KCBv2Lib.h"

// initializing frame buffers
KINECT_CB HRESULT APIENTRY KCBCreateBodyFrame(_Out_ KCBBodyFrame** ppBodyFrame)
{
    if(nullptr == ppBodyFrame)
    {
        return E_POINTER;
    }

    // do we need to create the buffer
    KCBBodyFrame* pFrame = new KCBBodyFrame();

    pFrame->Count = BODY_COUNT;
    pFrame->Bodies = new IBody* [BODY_COUNT]();
    pFrame->TimeStamp = 0;

    *ppBodyFrame = pFrame;

    return S_OK;
}

KINECT_CB HRESULT APIENTRY KCBReleaseBodyFrame(_Inout_ KCBBodyFrame** pBodyFrame)
{
    if(nullptr != pBodyFrame || nullptr != *pBodyFrame )
    {
        return E_INVALIDARG;
    }

    KCBBodyFrame* pFrame = *pBodyFrame;

    for(UINT i=0; i < pFrame->Count; ++i)
    {
        SafeArrayDelete(pFrame->Bodies[i]);
    }
    SafeArrayDelete(pFrame->Bodies);

    SafeDelete(pFrame);

    pBodyFrame = nullptr;

    return S_OK;
}


KINECT_CB HRESULT APIENTRY KCBCreateBodyIndexFrame(KCBFrameDescription frameDescription, _Out_ KCBBodyIndexFrame** ppBodyIndexFrame)
{
    if(nullptr == ppBodyIndexFrame)
    {
        return E_POINTER;
    }

    // create the new buffer
    KCBBodyIndexFrame* pFrame = new KCBBodyIndexFrame();
    pFrame->Size = frameDescription.lengthInPixels;
    pFrame->Buffer = new (std::nothrow) BYTE[pFrame->Size];
    pFrame->TimeStamp = 0;

    *ppBodyIndexFrame = pFrame;

    return S_OK;
}

KINECT_CB HRESULT APIENTRY KCBReleaseBodyIndexFrame(_Inout_ KCBBodyIndexFrame** pBodyIndexFrame)
{
    if(nullptr != pBodyIndexFrame || nullptr != *pBodyIndexFrame)
    {
        return E_INVALIDARG;
    }
    
    KCBBodyIndexFrame* pFrame = *pBodyIndexFrame;

    SafeArrayDelete(pFrame->Buffer);
    SafeDelete(pFrame);

    pBodyIndexFrame = nullptr;

    return S_OK;
}


KINECT_CB HRESULT APIENTRY KCBCreateColorFrame(ColorImageFormat colorFormat, KCBFrameDescription frameDescription, _Out_ KCBColorFrame** ppColorFrame)
{
    if(nullptr == ppColorFrame)
    {
        return E_POINTER;
    }

    if(ColorImageFormat_None == colorFormat)
    {
        return E_INVALIDARG;
    }

    KCBColorFrame* pFrame = new KCBColorFrame();

    // create the new buffer
    pFrame->Format = colorFormat;
    pFrame->Size = frameDescription.lengthInPixels * frameDescription.bytesPerPixel;
    pFrame->Buffer = new (std::nothrow) BYTE[pFrame->Size];

	*ppColorFrame = pFrame;

    return S_OK;
}

KINECT_CB HRESULT APIENTRY KCBReleaseColorFrame(_Inout_ KCBColorFrame** pColorFrame)
{
    if(nullptr != pColorFrame || nullptr != *pColorFrame)
    {
        return E_INVALIDARG;
    }

    KCBColorFrame* pFrame = *pColorFrame;

    SafeArrayDelete(pFrame->Buffer);
    SafeDelete(pFrame);
    pColorFrame = nullptr;

    return S_OK;
}


KINECT_CB HRESULT APIENTRY KCBCreateDepthFrame(KCBFrameDescription frameDescription, _Out_ KCBDepthFrame** ppDepthFrame)
{
    if(nullptr == ppDepthFrame)
    {
        return E_POINTER;
    }

    // create the new buffer
    KCBDepthFrame* pFrame = new KCBDepthFrame();
    pFrame->Size = frameDescription.lengthInPixels;
    pFrame->Buffer = new (std::nothrow) UINT16[pFrame->Size];
    pFrame->TimeStamp = 0;

    *ppDepthFrame = pFrame;

    return S_OK;
}

KINECT_CB HRESULT APIENTRY KCBReleaseDepthFrame(_Inout_ KCBDepthFrame** pDepthFrame)
{
    if(nullptr != pDepthFrame || nullptr != *pDepthFrame)
    {
        return E_INVALIDARG;
    }

    KCBDepthFrame* pFrame = *pDepthFrame;
    SafeArrayDelete(pFrame->Buffer);
    SafeDelete(pFrame);
    pDepthFrame = nullptr;

    return S_OK;
}


KINECT_CB HRESULT APIENTRY KCBCreateInfraredFrame(KCBFrameDescription frameDescription, _Out_ KCBInfraredFrame** ppInfraredFrame)
{
    if(nullptr == ppInfraredFrame)
    {
        return E_POINTER;
    }

    // create the new buffer
    KCBInfraredFrame* pFrame = new KCBInfraredFrame();
    pFrame->Size = frameDescription.lengthInPixels;
    pFrame->Buffer = new (std::nothrow) UINT16[pFrame->Size];
    pFrame->TimeStamp = 0;

    *ppInfraredFrame = pFrame;

    return S_OK;
}

KINECT_CB HRESULT APIENTRY KCBReleaseInfraredFrame(_Inout_ KCBInfraredFrame** pInfraredFrame)
{
    if(nullptr != pInfraredFrame || nullptr != *pInfraredFrame)
    {
        return E_INVALIDARG;
    }

    KCBDepthFrame* pFrame = *pInfraredFrame;
    SafeArrayDelete(pFrame->Buffer);
    SafeDelete(pFrame);
    pInfraredFrame = nullptr;

    return S_OK;
}


KINECT_CB HRESULT APIENTRY KCBCreateLongExposureInfraredFrame(KCBFrameDescription frameDescription, _Out_ KCBLongExposureInfraredFrame** ppLongExposureInfraredFrame)
{
    if(nullptr == ppLongExposureInfraredFrame)
    {
        return E_POINTER;
    }

    // create the new buffer
    KCBLongExposureInfraredFrame* pFrame = new KCBLongExposureInfraredFrame();
    pFrame->Size = frameDescription.lengthInPixels;
    pFrame->Buffer = new (std::nothrow) UINT16[pFrame->Size];
    pFrame->TimeStamp = 0;

    *ppLongExposureInfraredFrame = pFrame;

    return S_OK;
}

KINECT_CB HRESULT APIENTRY KCBReleaseLongExposureInfraredFrame(_Inout_ KCBLongExposureInfraredFrame** pLongExposureInfraredFrame)
{
    if(nullptr != pLongExposureInfraredFrame || nullptr != *pLongExposureInfraredFrame)
    {
        return E_INVALIDARG;
    }

    KCBDepthFrame* pFrame = *pLongExposureInfraredFrame;
    SafeArrayDelete(pFrame->Buffer);
    SafeDelete(pFrame);
    pLongExposureInfraredFrame = nullptr;

    return S_OK;
}
