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
// KCBv2.cpp : Defines the exported functions for the DLL application.

#include "stdafx.h"
#include "KCBv2Lib.h"
#include "KCBSensor.h"

#include "KinectList.h"

#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <Functiondiscoverykeys_devpkey.h>

static KinectList g_sensors;

KINECT_CB KCBHANDLE APIENTRY KCBOpenDefaultSensor()
{
    HRESULT hr = S_OK;

    KCBHANDLE handle = KCB_INVALID_HANDLE;

    IKinectSensor* pSensor = nullptr;

    IKCBSensor* pKCB = nullptr;

    CHECK_HR(hr = GetDefaultKinectSensor(&pSensor));

    CHECK_HR(hr = KCBSensor::CreateInstance(pSensor, &pKCB));

    handle = g_sensors.Add(pKCB);

done:
    SAFE_RELEASE(pSensor);
    SAFE_RELEASE(pKCB);

    return handle;
}

KINECT_CB HRESULT APIENTRY KCBCloseSensor(_Inout_ KCBHANDLE* pkcbHandle)
{
    if( nullptr == pkcbHandle || KCB_INVALID_HANDLE == *pkcbHandle)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Remove(*pkcbHandle, &pKCB));
    
done:
    (*pkcbHandle) = KCB_INVALID_HANDLE;

    SAFE_RELEASE(pKCB);
    
    return hr;
}

#pragma region Frame Format Functions
HRESULT FindAudioDevice(const WCHAR* wsName, IAudioClient** ppClient)
{
    HRESULT hr = S_OK;

    std::wstring name;
    IMMDeviceEnumerator* pDeviceEnum = nullptr;
    IMMDeviceCollection* pCollection = nullptr;
    IMMDevice* pDevice = nullptr;

    // create the enumerator for WASAPI
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&pDeviceEnum));
    if (CO_E_NOTINITIALIZED == hr)
    {
        CHECK_HR(hr = CoInitializeEx(NULL, COINIT_MULTITHREADED));
        CHECK_HR(hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&pDeviceEnum)));
    }

    // get the colloection of capture endpoints
    CHECK_HR(hr = pDeviceEnum->EnumAudioEndpoints(eCapture, DEVICE_STATEMASK_ALL, &pCollection));

    // find out how many devices are available
    UINT count;
    CHECK_HR(hr = pCollection->GetCount(&count));

    // not expected
    assert(count != 0);

    bool bFound = false;
    IMMDevice* pEndpoint = nullptr;
    IPropertyStore* pProps = nullptr;

    // Each loop until we find the Kinect USB Audio device
    for (ULONG i = 0; i < count; ++i)
    {
        hr = pCollection->Item(i, &pEndpoint);
		if (!SUCCEEDED(hr)) {
			continue;
		}

        // get the property store from the device
        hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
		if (!SUCCEEDED(hr)) {
			continue;
		}

        // Initialize container for property value.
        PROPVARIANT varName;
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);

		if (!SUCCEEDED(hr)) {
			continue;
		}

        // get the name from the property store
        name = varName.pwszVal;
        if (std::string::npos != name.find(wsName))
        {
            pDevice = pEndpoint;
            pEndpoint->AddRef();

            bFound = true;
        }

        PropVariantClear(&varName);

        SAFE_RELEASE(pProps);
        SAFE_RELEASE(pEndpoint);

        if (bFound)
        {
            break;
        }
    }

    CHECK_HR(hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)ppClient));

done:
    SAFE_RELEASE(pDevice);
    SAFE_RELEASE(pCollection);
    SAFE_RELEASE(pDeviceEnum);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetAudioFormat(_In_ KCBHANDLE kcbHandle, _Inout_ WAVEFORMATEX* pAudioFormat)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pAudioFormat)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    IAudioSource* pAudioSource = nullptr;
    IAudioClient* pAudioClient = nullptr;

    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetSource(__uuidof(IAudioSource), reinterpret_cast<void**>(&pAudioSource)));

    CHECK_HR(hr = FindAudioDevice(L"(Xbox NUI Sensor)", &pAudioClient));
    
    WAVEFORMATEX* wfx = { 0 };
    CHECK_HR(hr = pAudioClient->GetMixFormat(&wfx));

    *pAudioFormat = *wfx;

    CoTaskMemFree(wfx);

done:
    SAFE_RELEASE(pAudioClient);
    SAFE_RELEASE(pAudioSource);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetBodyIndexFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pFrameDescription)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IBodyIndexFrameSource* pFrameSource = nullptr;
    IFrameDescription* pDescription = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetSource(__uuidof(IBodyIndexFrameSource), reinterpret_cast<void**>(&pFrameSource)));

    // get the description
    CHECK_HR(hr = pFrameSource->get_FrameDescription(&pDescription));

    CHECK_HR(hr = pDescription->get_Width(&pFrameDescription->width));

    CHECK_HR(hr = pDescription->get_Height(&pFrameDescription->height));

    CHECK_HR(hr = pDescription->get_HorizontalFieldOfView(&pFrameDescription->horizontalFieldOfView));

    CHECK_HR(hr = pDescription->get_VerticalFieldOfView(&pFrameDescription->verticalFieldOfView));

    CHECK_HR(hr = pDescription->get_DiagonalFieldOfView(&pFrameDescription->diagonalFieldOfView));

    CHECK_HR(hr = pDescription->get_LengthInPixels(&pFrameDescription->lengthInPixels));

    CHECK_HR(hr = pDescription->get_BytesPerPixel(&pFrameDescription->bytesPerPixel));

done:
    if(FAILED(hr))
    {
        ZeroMemory(pFrameDescription, sizeof(KCBFrameDescription));
    }
    SAFE_RELEASE(pDescription);
    SAFE_RELEASE(pFrameSource);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetColorFrameDescription(_In_ KCBHANDLE kcbHandle, ColorImageFormat eFormat, _Inout_ KCBFrameDescription* pFrameDescription)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pFrameDescription)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IColorFrameSource* pFrameSource = nullptr;
    IFrameDescription* pDescription = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetSource(__uuidof(IColorFrameSource), reinterpret_cast<void**>(&pFrameSource)));

    // get the description
    CHECK_HR(hr = pFrameSource->CreateFrameDescription(eFormat, &pDescription));

    CHECK_HR(hr = pDescription->get_Width(&pFrameDescription->width));

    CHECK_HR(hr = pDescription->get_Height(&pFrameDescription->height));

    CHECK_HR(hr = pDescription->get_HorizontalFieldOfView(&pFrameDescription->horizontalFieldOfView));

    CHECK_HR(hr = pDescription->get_VerticalFieldOfView(&pFrameDescription->verticalFieldOfView));

    CHECK_HR(hr = pDescription->get_DiagonalFieldOfView(&pFrameDescription->diagonalFieldOfView));

    CHECK_HR(hr = pDescription->get_LengthInPixels(&pFrameDescription->lengthInPixels));

    CHECK_HR(hr = pDescription->get_BytesPerPixel(&pFrameDescription->bytesPerPixel));

done:
    if(FAILED(hr))
    {
        ZeroMemory(pFrameDescription, sizeof(KCBFrameDescription));
    }
    SAFE_RELEASE(pDescription);
    SAFE_RELEASE(pFrameSource);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetDepthFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pFrameDescription)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IDepthFrameSource* pFrameSource = NULL;
    IFrameDescription* pDescription = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetSource(__uuidof(IDepthFrameSource), reinterpret_cast<void**>(&pFrameSource)));

    // get the description
    CHECK_HR(hr = pFrameSource->get_FrameDescription(&pDescription));

    CHECK_HR(hr = pDescription->get_Width(&pFrameDescription->width));

    CHECK_HR(hr = pDescription->get_Height(&pFrameDescription->height));

    CHECK_HR(hr = pDescription->get_HorizontalFieldOfView(&pFrameDescription->horizontalFieldOfView));

    CHECK_HR(hr = pDescription->get_VerticalFieldOfView(&pFrameDescription->verticalFieldOfView));

    CHECK_HR(hr = pDescription->get_DiagonalFieldOfView(&pFrameDescription->diagonalFieldOfView));

    CHECK_HR(hr = pDescription->get_LengthInPixels(&pFrameDescription->lengthInPixels));

    CHECK_HR(hr = pDescription->get_BytesPerPixel(&pFrameDescription->bytesPerPixel));

done:
    if(FAILED(hr))
    {
        ZeroMemory(pFrameDescription, sizeof(KCBFrameDescription));
    }
    SAFE_RELEASE(pDescription);
    SAFE_RELEASE(pFrameSource);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetInfraredFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pFrameDescription)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IInfraredFrameSource* pFrameSource = nullptr;
    IFrameDescription* pDescription = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetSource(__uuidof(IInfraredFrameSource), reinterpret_cast<void**>(&pFrameSource)));

    // get the description
    CHECK_HR(hr = pFrameSource->get_FrameDescription(&pDescription));

    CHECK_HR(hr = pDescription->get_Width(&pFrameDescription->width));

    CHECK_HR(hr = pDescription->get_Height(&pFrameDescription->height));

    CHECK_HR(hr = pDescription->get_HorizontalFieldOfView(&pFrameDescription->horizontalFieldOfView));

    CHECK_HR(hr = pDescription->get_VerticalFieldOfView(&pFrameDescription->verticalFieldOfView));

    CHECK_HR(hr = pDescription->get_DiagonalFieldOfView(&pFrameDescription->diagonalFieldOfView));

    CHECK_HR(hr = pDescription->get_LengthInPixels(&pFrameDescription->lengthInPixels));

    CHECK_HR(hr = pDescription->get_BytesPerPixel(&pFrameDescription->bytesPerPixel));

done:
    if(FAILED(hr))
    {
        ZeroMemory(pFrameDescription, sizeof(KCBFrameDescription));
    }
    SAFE_RELEASE(pDescription);
    SAFE_RELEASE(pFrameSource);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetLongExposureInfraredFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pFrameDescription)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    ILongExposureInfraredFrameSource* pFrameSource = nullptr;
    IFrameDescription* pDescription = nullptr;

    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetSource(__uuidof(ILongExposureInfraredFrameSource), reinterpret_cast<void**>(&pFrameSource)));

    // get the description
    CHECK_HR(hr = pFrameSource->get_FrameDescription(&pDescription));

    CHECK_HR(hr = pDescription->get_Width(&pFrameDescription->width));

    CHECK_HR(hr = pDescription->get_Height(&pFrameDescription->height));

    CHECK_HR(hr = pDescription->get_HorizontalFieldOfView(&pFrameDescription->horizontalFieldOfView));

    CHECK_HR(hr = pDescription->get_VerticalFieldOfView(&pFrameDescription->verticalFieldOfView));

    CHECK_HR(hr = pDescription->get_DiagonalFieldOfView(&pFrameDescription->diagonalFieldOfView));

    CHECK_HR(hr = pDescription->get_LengthInPixels(&pFrameDescription->lengthInPixels));

    CHECK_HR(hr = pDescription->get_BytesPerPixel(&pFrameDescription->bytesPerPixel));

done:
    if(FAILED(hr))
    {
        ZeroMemory(pFrameDescription, sizeof(KCBFrameDescription));
    }
    SAFE_RELEASE(pDescription);
    SAFE_RELEASE(pFrameSource);
    SAFE_RELEASE(pKCB);

    return hr;
}
#pragma endregion

#pragma region Raw Frame Data Functions
KINECT_CB HRESULT APIENTRY KCBGetAudioFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBAudioFrame* pstAudioFrame)
{
    return KCBGetAudioData(kcbHandle, pstAudioFrame->cAudioBufferSize, pstAudioFrame->pAudioBuffer, &pstAudioFrame->ulBytesRead, &pstAudioFrame->fBeamAngle, &pstAudioFrame->fBeamAngleConfidence);
}

KINECT_CB HRESULT APIENTRY KCBGetAudioData(_In_ KCBHANDLE kcbHandle, ULONG cAudioBufferSize, _Inout_cap_(cAudioBufferSize) byte* pbAudioBuffer, _Out_ ULONG* ulBytesRead, _Out_opt_ FLOAT* beamAngle, _Out_opt_ FLOAT* sourceConfidence)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pbAudioBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IAudioBeamFrameReader* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // Get audio buffer
    CHECK_HR(hr = pKCB->GetAudioBuffer(cAudioBufferSize, pbAudioBuffer, ulBytesRead, beamAngle, sourceConfidence));

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetBodyData(_In_ KCBHANDLE kcbHandle, UINT capacity, _Inout_updates_all_(capacity) IBody **bodies, _Out_opt_ LONGLONG* llTimeStamp)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == bodies)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IBodyFrame* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IBodyFrame), reinterpret_cast<void**>(&pFrame)));

    // copy the frame
    CHECK_HR(hr = pFrame->GetAndRefreshBodyData(capacity, bodies));

    // get timestamp
    if(nullptr != llTimeStamp)
    {
        CHECK_HR(hr = pFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(llTimeStamp)));
    }

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetBodyFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBBodyFrame* pstBodyFrame)
{
    return KCBGetBodyData(kcbHandle, pstBodyFrame->Count, pstBodyFrame->Bodies, &pstBodyFrame->TimeStamp);
}

KINECT_CB HRESULT APIENTRY KCBGetBodyIndexFrameBuffer(_In_ KCBHANDLE kcbHandle, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pbBuffer, _Out_opt_ LONGLONG* llTimeStamp)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pbBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IBodyIndexFrame* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IBodyIndexFrame), reinterpret_cast<void**>(&pFrame)));

    // copy the frame
    CHECK_HR(hr = pFrame->CopyFrameDataToArray(cbBufferSize, pbBuffer));

    // get timestamp
    if(nullptr != llTimeStamp)
    {
        CHECK_HR(hr = pFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(llTimeStamp)));
    }

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetBodyIndexFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBBodyIndexFrame* pstBodyIndexFrame)
{
    return KCBGetBodyIndexFrameBuffer(kcbHandle, pstBodyIndexFrame->Size, pstBodyIndexFrame->Buffer, &pstBodyIndexFrame->TimeStamp);
}

KINECT_CB HRESULT APIENTRY KCBGetColorData(_In_ KCBHANDLE kcbHandle, ColorImageFormat eColorFormat, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pbBuffer, _Out_opt_ LONGLONG* llTimeStamp)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == pbBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IColorFrame* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IColorFrame), reinterpret_cast<void**>(&pFrame)));

    // copy the frame
    CHECK_HR(hr = pFrame->CopyConvertedFrameDataToArray(cbBufferSize, pbBuffer, eColorFormat));

    // get timestamp
    if(nullptr != llTimeStamp)
    {
        CHECK_HR(hr = pFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(llTimeStamp)));
    }

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetColorFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBColorFrame* pstColorFrame)
{
    return KCBGetColorData(kcbHandle, pstColorFrame->Format, pstColorFrame->Size, pstColorFrame->Buffer, &pstColorFrame->TimeStamp);
}

KINECT_CB HRESULT APIENTRY KCBGetDepthData(_In_ KCBHANDLE kcbHandle, ULONG cuiBufferSize, _Inout_cap_(cuiBufferSize) UINT16* puiBuffer, _Out_opt_ LONGLONG* llTimeStamp)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == puiBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IDepthFrame* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IDepthFrame), reinterpret_cast<void**>(&pFrame)));

    // copy the frame
    CHECK_HR(hr = pFrame->CopyFrameDataToArray(cuiBufferSize, puiBuffer));

    // get timestamp
    if(nullptr != llTimeStamp)
    {
        CHECK_HR(hr = pFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(llTimeStamp)));
    }

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetDepthFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBDepthFrame* pstDepthFrame)
{
    return KCBGetDepthData(kcbHandle, pstDepthFrame->Size, pstDepthFrame->Buffer, &pstDepthFrame->TimeStamp);
}

KINECT_CB HRESULT APIENTRY KCBGetInfraredData(_In_ KCBHANDLE kcbHandle, ULONG cuiBufferSize, _Inout_cap_(cuiBufferSize) UINT16* puiBuffer, _Out_opt_ LONGLONG* llTimeStamp)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == puiBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IInfraredFrame* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IInfraredFrame), reinterpret_cast<void**>(&pFrame)));

    // copy the frame
    CHECK_HR(hr = pFrame->CopyFrameDataToArray(cuiBufferSize, puiBuffer));

    // get timestamp
    if(nullptr != llTimeStamp)
    {
        CHECK_HR(hr = pFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(llTimeStamp)));
    }

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetInfraredFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBInfraredFrame* pstInfraredFrame)
{
    return KCBGetInfraredData(kcbHandle, pstInfraredFrame->Size, pstInfraredFrame->Buffer, &pstInfraredFrame->TimeStamp);
}

KINECT_CB HRESULT APIENTRY KCBGetLongExposureInfraredData(_In_ KCBHANDLE kcbHandle, ULONG cuiBufferSize, _Inout_cap_(cuiBufferSize) UINT16* puiBuffer, _Out_opt_ LONGLONG* llTimeStamp)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == puiBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    ILongExposureInfraredFrame* pFrame = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(ILongExposureInfraredFrame), reinterpret_cast<void**>(&pFrame)));

    // copy the frame
    CHECK_HR(hr = pFrame->CopyFrameDataToArray(cuiBufferSize, puiBuffer));

    // get timestamp
    if(nullptr != llTimeStamp)
    {
        CHECK_HR(hr = pFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(llTimeStamp)));
    }

done:
    SAFE_RELEASE(pFrame);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetLongExposureInfraredFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBLongExposureInfraredFrame* pstLongExposureInfraredFrame)
{
    return KCBGetLongExposureInfraredData(kcbHandle, pstLongExposureInfraredFrame->Size, pstLongExposureInfraredFrame->Buffer, &pstLongExposureInfraredFrame->TimeStamp);
}

KINECT_CB HRESULT APIENTRY KCBGetAllFrameData(_In_ KCBHANDLE kcbHandle, 
    _Inout_opt_ KCBBodyFrame* pstBodyFrame,
    _Inout_opt_ KCBBodyIndexFrame* pstBodyIndexFrame,
    _Inout_opt_ KCBColorFrame* pstColorFrame,
    _Inout_opt_ KCBDepthFrame* pstDepthFrame,
    _Inout_opt_ KCBInfraredFrame* pstInfraredFrame,
    _Inout_opt_ KCBLongExposureInfraredFrame* pstLongExposureInfraredFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    DWORD dwFlags = 0;

    if(nullptr != pstBodyFrame)
    {
        dwFlags |= FrameSourceTypes_Body;
    }
    if(nullptr != pstBodyIndexFrame)
    {
        dwFlags |= FrameSourceTypes_BodyIndex;
    }
    if(nullptr != pstColorFrame)
    {
        dwFlags |= FrameSourceTypes_Color;
    }
    if(nullptr != pstDepthFrame)
    {
        dwFlags |= FrameSourceTypes_Depth;
    }
    if(nullptr != pstInfraredFrame)
    {
        dwFlags |= FrameSourceTypes_Infrared;
    }
    if(nullptr != pstLongExposureInfraredFrame)
    {
        dwFlags |= FrameSourceTypes_LongExposureInfrared;
    }

    // creates or gets the FrameReader object
    // if the parameters are different from the previous call, this will reset the configuration
    IMultiSourceFrame* pMultiSourceFrame = nullptr;

    IBodyFrameReference* pBodyReference = nullptr;
    IBodyFrame* pBodyFrame = nullptr;

    IBodyIndexFrameReference* pBodyIndexReference = nullptr;
    IBodyIndexFrame* pBodyIndexFrame = nullptr;

    IColorFrameReference* pColorReference = nullptr;
    IColorFrame* pColorFrame = nullptr;

    IDepthFrameReference* pDepthReference = nullptr;
    IDepthFrame* pDepthFrame = nullptr;

    IInfraredFrameReference* pIRReference = nullptr;
    IInfraredFrame* pIRFrame = nullptr;

    ILongExposureInfraredFrameReference* pLIRReference = nullptr;
    ILongExposureInfraredFrame* pLIRFrame = nullptr;

    CHECK_HR(hr = KCBGetIMultiSourceFrame(kcbHandle, dwFlags, &pMultiSourceFrame)); 

    // acquire all the reference frame if avaialble, will check later if they are valid
    if(nullptr != pstBodyFrame)
    {
        CHECK_HR(hr = pMultiSourceFrame->get_BodyFrameReference(&pBodyReference));
        CHECK_HR(hr = pBodyReference->AcquireFrame(&pBodyFrame));
    }

    if(nullptr != pstBodyIndexFrame)
    {
        CHECK_HR(hr = pMultiSourceFrame->get_BodyIndexFrameReference(&pBodyIndexReference));
        CHECK_HR(hr = pBodyIndexReference->AcquireFrame(&pBodyIndexFrame));
    }

    if(nullptr != pstColorFrame && ColorImageFormat_None != pstColorFrame->Format)
    {
        CHECK_HR(hr = pMultiSourceFrame->get_ColorFrameReference(&pColorReference));
        CHECK_HR(hr = pColorReference->AcquireFrame(&pColorFrame));
    }

    if(nullptr != pstDepthFrame)
    {
        CHECK_HR(hr = pMultiSourceFrame->get_DepthFrameReference(&pDepthReference));
        CHECK_HR(hr = pDepthReference->AcquireFrame(&pDepthFrame));
    }

    if(nullptr != pstInfraredFrame)
    {
        CHECK_HR(hr = pMultiSourceFrame->get_InfraredFrameReference(&pIRReference));
        CHECK_HR(hr = pIRReference->AcquireFrame(&pIRFrame));
    }

    if(nullptr != pstLongExposureInfraredFrame)
    {
        CHECK_HR(hr = pMultiSourceFrame->get_LongExposureInfraredFrameReference(&pLIRReference));
        CHECK_HR(hr = pLIRReference->AcquireFrame(&pLIRFrame));
    }


    // get the frame data only if all frames are ready
    if( (nullptr != pstBodyFrame && nullptr == pBodyFrame) || 
        (nullptr != pstBodyIndexFrame && nullptr == pBodyIndexFrame) || 
        (nullptr != pstColorFrame && nullptr == pColorFrame) ||
        (nullptr != pstDepthFrame && nullptr == pDepthFrame) ||
        (nullptr != pstInfraredFrame && nullptr == pIRFrame) || 
        (nullptr != pstLongExposureInfraredFrame && nullptr == pLIRFrame ) )
    {
        hr = E_PENDING;
        goto done;
    }

    if(nullptr != pBodyFrame)
    {
        // copy the frame
        CHECK_HR(hr = pBodyFrame->GetAndRefreshBodyData(pstBodyFrame->Count, pstBodyFrame->Bodies));

        // get timestamp
        CHECK_HR(hr = pBodyFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(&pstBodyFrame->TimeStamp)));
    }
    
    if(nullptr != pBodyIndexFrame)
    {
        // copy the frame
        CHECK_HR(hr = pBodyIndexFrame->CopyFrameDataToArray(pstBodyIndexFrame->Size, pstBodyIndexFrame->Buffer));

        // get timestamp
        CHECK_HR(hr = pBodyIndexFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(&pstBodyIndexFrame->TimeStamp)));
    }

    if(nullptr != pColorFrame)
    {
        // copy the frame
        if(ColorImageFormat_Yuy2 == pstColorFrame->Format)
        {
            CHECK_HR(hr = pColorFrame->CopyRawFrameDataToArray(pstColorFrame->Size, pstColorFrame->Buffer));
        }
        else
        {
            CHECK_HR(hr = pColorFrame->CopyConvertedFrameDataToArray(pstColorFrame->Size, pstColorFrame->Buffer, pstColorFrame->Format));
        }

        // get timestamp
        CHECK_HR(hr = pColorFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(&pstColorFrame->TimeStamp)));
    }

    if(nullptr != pDepthFrame)
    {
        // copy the frame
        CHECK_HR(hr = pDepthFrame->CopyFrameDataToArray(pstDepthFrame->Size, pstDepthFrame->Buffer));

        // get timestamp
        CHECK_HR(hr = pDepthFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(&pstDepthFrame->TimeStamp)));
    }

    if(nullptr != pIRFrame)
    {
        // copy the frame
        CHECK_HR(hr = pIRFrame->CopyFrameDataToArray(pstInfraredFrame->Size, pstInfraredFrame->Buffer));

        // get timestamp
        CHECK_HR(hr = pIRFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(&pstInfraredFrame->TimeStamp)));
    }

    if(nullptr != pLIRFrame)
    {
        // copy the frame
        CHECK_HR(hr = pLIRFrame->CopyFrameDataToArray(pstLongExposureInfraredFrame->Size, pstLongExposureInfraredFrame->Buffer));

        // get timestamp
        CHECK_HR(hr = pLIRFrame->get_RelativeTime(reinterpret_cast<TIMESPAN*>(&pstLongExposureInfraredFrame->TimeStamp)));
    }

done:
    SAFE_RELEASE(pBodyReference);
    SAFE_RELEASE(pBodyFrame);
    SAFE_RELEASE(pBodyIndexReference);
    SAFE_RELEASE(pBodyIndexFrame);
    SAFE_RELEASE(pColorReference);
    SAFE_RELEASE(pColorFrame);
    SAFE_RELEASE(pDepthReference);
    SAFE_RELEASE(pDepthFrame);
    SAFE_RELEASE(pIRReference);
    SAFE_RELEASE(pIRFrame);
    SAFE_RELEASE(pLIRReference);
    SAFE_RELEASE(pLIRFrame);
    SAFE_RELEASE(pMultiSourceFrame);

    return hr;
}
#pragma endregion

#pragma region Coordinate Mapping Functions
KINECT_CB HRESULT APIENTRY KCBMapCameraPointToDepthSpace(_In_ KCBHANDLE kcbHandle,
    CameraSpacePoint cameraPoint, 
    _Out_ DepthSpacePoint* depthPoint)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthPoint)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapCameraPointToDepthSpace(cameraPoint, depthPoint));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapCameraPointToColorSpace(_In_ KCBHANDLE kcbHandle, 
    CameraSpacePoint cameraPoint, 
    _Out_ ColorSpacePoint *colorPoint)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == colorPoint)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapCameraPointToColorSpace(cameraPoint, colorPoint));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapDepthPointToCameraSpace(_In_ KCBHANDLE kcbHandle, 
    DepthSpacePoint depthPoint, UINT16 depth, 
    _Out_ CameraSpacePoint *cameraPoint)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == cameraPoint)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapDepthPointToCameraSpace(depthPoint, depth, cameraPoint));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapDepthPointToColorSpace(_In_ KCBHANDLE kcbHandle, 
    DepthSpacePoint depthPoint, UINT16 depth, 
    _Out_ ColorSpacePoint *colorPoint)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == colorPoint)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapDepthPointToColorSpace(depthPoint, depth, colorPoint));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapCameraPointsToDepthSpace(_In_ KCBHANDLE kcbHandle, 
    UINT cameraPointCount, _In_reads_(cameraPointCount) const CameraSpacePoint *cameraPoints, 
    UINT depthPointCount, 
    _Out_writes_all_(depthPointCount) DepthSpacePoint *depthPoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == cameraPoints || nullptr == depthPoints)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapCameraPointsToDepthSpace( cameraPointCount, cameraPoints, depthPointCount, depthPoints));
    
done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapCameraPointsToColorSpace(_In_ KCBHANDLE kcbHandle,
    UINT cameraPointCount, _In_reads_(cameraPointCount) const CameraSpacePoint *cameraPoints,
    UINT colorPointCount, 
    _Out_writes_all_(colorPointCount) ColorSpacePoint *colorPoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == cameraPoints || nullptr == colorPoints )
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapCameraPointsToColorSpace(cameraPointCount, cameraPoints, colorPointCount, colorPoints));
    
done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapDepthPointsToCameraSpace(_In_ KCBHANDLE kcbHandle, 
    UINT depthPointCount, _In_reads_(depthPointCount) const DepthSpacePoint *depthPoints,
    UINT depthCount, _In_reads_(depthCount) const UINT16 *depths,
    UINT cameraPointCount, 
    _Out_writes_all_(cameraPointCount) CameraSpacePoint *cameraPoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthPoints || nullptr == depths || nullptr == cameraPoints )
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapDepthPointsToCameraSpace(depthPointCount, depthPoints, depthCount, depths, cameraPointCount, cameraPoints));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapDepthPointsToColorSpace(_In_ KCBHANDLE kcbHandle,
    UINT depthPointCount, _In_reads_(depthPointCount) const DepthSpacePoint *depthPoints,
    UINT depthCount, _In_reads_(depthCount) const UINT16 *depths,
    UINT colorPointCount, 
    _Out_writes_all_(colorPointCount) ColorSpacePoint *colorPoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthPoints || nullptr == depths || nullptr == colorPoints)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapDepthPointsToColorSpace(depthPointCount, depthPoints, depthCount, depths, colorPointCount, colorPoints));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapDepthFrameToCameraSpace(_In_ KCBHANDLE kcbHandle, 
    UINT depthPointCount, _In_reads_(depthPointCount) const UINT16 *depthFrameData, 
    UINT cameraPointCount, 
    _Out_writes_all_(cameraPointCount) CameraSpacePoint *cameraSpacePoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthFrameData || nullptr == cameraSpacePoints)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapDepthFrameToCameraSpace(depthPointCount, depthFrameData, cameraPointCount, cameraSpacePoints));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapDepthFrameToColorSpace(_In_ KCBHANDLE kcbHandle,
    UINT depthPointCount, _In_reads_(depthPointCount) const UINT16 *depthFrameData,
    UINT colorPointCount, 
    _Out_writes_all_(colorPointCount) ColorSpacePoint *colorSpacePoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthFrameData || nullptr == colorSpacePoints)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapDepthFrameToColorSpace(depthPointCount, depthFrameData, colorPointCount, colorSpacePoints));
    
done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapColorFrameToDepthSpace(_In_ KCBHANDLE kcbHandle,
    UINT depthDataPointCount, _In_reads_(depthDataPointCount) const UINT16 *depthFrameData,
    UINT depthPointCount, 
    _Out_writes_all_(depthPointCount) DepthSpacePoint *depthSpacePoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthFrameData || nullptr == depthSpacePoints)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapColorFrameToDepthSpace( depthDataPointCount, depthFrameData, depthPointCount, depthSpacePoints));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBMapColorFrameToCameraSpace(_In_ KCBHANDLE kcbHandle,
    UINT depthDataPointCount, _In_reads_(depthDataPointCount)  const UINT16 *depthFrameData,
    UINT cameraPointCount, 
    _Out_writes_all_(cameraPointCount) CameraSpacePoint *cameraSpacePoints)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == depthFrameData || nullptr == cameraSpacePoints)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->MapColorFrameToCameraSpace(depthDataPointCount, depthFrameData, cameraPointCount, cameraSpacePoints));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}

KINECT_CB HRESULT APIENTRY GetDepthFrameToCameraSpaceTable(_In_ KCBHANDLE kcbHandle,
    _Out_  UINT32 *tableEntryCount,
    _Outptr_result_bytebuffer_(*tableEntryCount) PointF **tableEntries)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == tableEntries)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    ICoordinateMapper* pMapper = nullptr;
    CHECK_HR(hr = KCBGetICoordinateMapper(kcbHandle, &pMapper));

    CHECK_HR(hr = pMapper->GetDepthFrameToCameraSpaceTable(tableEntryCount, tableEntries));

done:
    SAFE_RELEASE(pMapper);

    return hr;
}
#pragma endregion

#pragma region Event Functions
KINECT_CB bool APIENTRY KCBIsFrameReady(_In_ KCBHANDLE kcbHandle, FrameSourceTypes eSource)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return false;
    }

    HRESULT hr = S_OK;
    bool bResult = false;

    IKCBSensor* pKCB = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));
    
    KCBSourceType eType = KCBSourceType_Unknown;
    switch (eSource)
    {
    case FrameSourceTypes_Body:
        eType = KCBSourceType_Body;
        break;
    case FrameSourceTypes_BodyIndex:
        eType = KCBSourceType_BodyIndex;
        break;
    case FrameSourceTypes_Color:
        eType = KCBSourceType_Color;
        break;
    case FrameSourceTypes_Depth:
        eType = KCBSourceType_Depth;
        break;
    case FrameSourceTypes_Infrared:
        eType = KCBSourceType_Infrared;
        break;
    case FrameSourceTypes_LongExposureInfrared:
        eType = KCBSourceType_LongExposureInfrared;
        break;
    }

    // check for ready state
    bResult = pKCB->IsFrameReady(eType);

done:
    SAFE_RELEASE(pKCB);

    return bResult;
}

KINECT_CB bool APIENTRY KCBAnyFrameReady(_In_ KCBHANDLE kcbHandle)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return false;
    }

    HRESULT hr = S_OK;
    bool bResult = false;

    IKCBSensor* pKCB = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));
    
    // check for ready state
    bResult = pKCB->AnyFrameReady();

done:
    SAFE_RELEASE(pKCB);

    return bResult;
}

KINECT_CB bool APIENTRY KCBAllFramesReady(_In_ KCBHANDLE kcbHandle)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return false;
    }

    HRESULT hr = S_OK;
    bool bResult = false;

    IKCBSensor* pKCB = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // check for ready state
    bResult = pKCB->AllFramesReady();

done:
    SAFE_RELEASE(pKCB);

    return bResult;
}
#pragma endregion

#pragma region Native Kinect Interfaces
KINECT_CB HRESULT APIENTRY KCBGetIBodyFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IBodyFrame** ppBodyFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppBodyFrame)
    {
        return E_POINTER;
    }

    (*ppBodyFrame) = nullptr;

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IBodyFrame), reinterpret_cast<void**>(ppBodyFrame)));

done:
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetIBodyIndexFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IBodyIndexFrame** ppBodyIndexFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppBodyIndexFrame)
    {
        return E_POINTER;
    }

    (*ppBodyIndexFrame) = nullptr;

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IBodyIndexFrame), reinterpret_cast<void**>(ppBodyIndexFrame)));

done:
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetIColorFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IColorFrame** ppColorFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppColorFrame)
    {
        return E_POINTER;
    }

    (*ppColorFrame) = nullptr;

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IColorFrame), reinterpret_cast<void**>(ppColorFrame)));

done:
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetIDepthFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IDepthFrame** ppDepthFrame )
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppDepthFrame)
    {
        return E_POINTER;
    }

    (*ppDepthFrame) = nullptr;

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IDepthFrame), reinterpret_cast<void**>(ppDepthFrame)));

done:
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetIInfraredFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IInfraredFrame **ppInfraredFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppInfraredFrame)
    {
        return E_POINTER;
    }

    (*ppInfraredFrame) = nullptr;

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(IInfraredFrame), reinterpret_cast<void**>(ppInfraredFrame)));

done:
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetILongExposureInfraredFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ ILongExposureInfraredFrame **ppLongExposureInfraredFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppLongExposureInfraredFrame)
    {
        return E_POINTER;
    }

    (*ppLongExposureInfraredFrame) = nullptr;

    HRESULT hr = S_OK;

    // get the KCBSensor interface
    IKCBSensor* pKCB = nullptr;
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame
    CHECK_HR(hr = pKCB->GetFrame(__uuidof(ILongExposureInfraredFrame), reinterpret_cast<void**>(ppLongExposureInfraredFrame)));

done:
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetIMultiSourceFrame(_In_ KCBHANDLE kcbHandle, DWORD dwFrameSourceTypes, _COM_Outptr_result_maybenull_ IMultiSourceFrame **ppMultiSourceFrame)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    IMultiSourceFrameReader* pReader = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    CHECK_HR(hr = pKCB->OpenMultiSourceReader(dwFrameSourceTypes, &pReader));

    CHECK_HR(hr = pReader->AcquireLatestFrame(ppMultiSourceFrame));

done:
    SAFE_RELEASE(pReader);
    SAFE_RELEASE(pKCB);

    return hr;
}

KINECT_CB HRESULT APIENTRY KCBGetICoordinateMapper(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ ICoordinateMapper** ppCoordinateMapper)
{
    if (KCB_INVALID_HANDLE == kcbHandle)
    {
        return E_INVALIDARG;
    }

    if (nullptr == ppCoordinateMapper)
    {
        return E_POINTER;
    }

    (*ppCoordinateMapper) = nullptr;

    HRESULT hr = S_OK;

    IKCBSensor* pKCB = nullptr;
    ICoordinateMapper* pMapper = nullptr;

    // get the KCBSensor interface
    CHECK_HR(hr = g_sensors.Find(kcbHandle, &pKCB));

    // get the frame source
    CHECK_HR(hr = pKCB->GetKinectInterface(__uuidof(ICoordinateMapper), reinterpret_cast<void**>(&pMapper)));

    // return the interface
    (*ppCoordinateMapper) = pMapper;
    (*ppCoordinateMapper)->AddRef();

done:
    SAFE_RELEASE(pMapper);
    SAFE_RELEASE(pKCB);

    return hr;
}
#pragma endregion
