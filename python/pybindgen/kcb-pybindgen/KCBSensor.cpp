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
#include "KCBSensor.h"
#include <memory>

inline HRESULT OpenSensor(_In_ IKinectSensor* pSensor)
{
    HRESULT hr = S_OK;

    BOOLEAN bOpen = FALSE;
    CHECK_HR(hr = pSensor->get_IsOpen(&bOpen));
    if(!bOpen)
    {
        CHECK_HR(hr = pSensor->Open()); // will always return S_OK
    }

done:
    return hr;
}

HRESULT KCBSensor::CreateInstance(_In_ IKinectSensor* pSensor, _COM_Outptr_result_maybenull_ IKCBSensor** ppKCBSensor)
{
    if(nullptr == pSensor || nullptr == ppKCBSensor)
    {
        return E_POINTER;
    }

    (*ppKCBSensor) = nullptr;

    HRESULT hr = S_OK;

    KCBSensor* pKCBSensor = nullptr;

    pKCBSensor = new (std::nothrow) KCBSensor();
    if(nullptr == pKCBSensor)
    {
        CHECK_HR(hr = E_OUTOFMEMORY);
    }

    CHECK_HR(hr = pKCBSensor->Initialize(pSensor));

    (*ppKCBSensor) = pKCBSensor;
    (*ppKCBSensor)->AddRef();

done:
    SAFE_RELEASE(pKCBSensor);

    return hr;
}

KCBSensor::KCBSensor(void)
: m_refCount(1)

, m_pKinectSensor(nullptr)
, m_pCoordinateMapper(nullptr)

, m_eFrameSourceTypes(FrameSourceTypes_None)
, m_pMultiSourceFrameReader(nullptr)

, m_pBodyFrameSource(nullptr)
, m_pBodyIndexFrameSource(nullptr)
, m_pColorFrameSource(nullptr)
, m_pColorFrameDescription(nullptr)
, m_pDepthFrameSource(nullptr)
, m_pInfraredFrameSource(nullptr)
, m_pLongExposureInfraredFrameSource(nullptr)
, m_pAudioSource(nullptr)

, m_pBodyFrameReader(nullptr)
, m_pBodyIndexFrameReader(nullptr)
, m_pColorFrameReader(nullptr)
, m_pDepthFrameReader(nullptr)
, m_pInfraredFrameReader(nullptr)
, m_pLongExposureInfraredFrameReader(nullptr)
, m_pAudioBeam(nullptr)
, m_pAudioStream(nullptr)

, m_hMultiSourceFrameEvent(NULL)
{
    // reset multisource reader handles
    for(int i = 0; i < 6; ++i)
    {
        m_hFrameEvents[i] = NULL;
    }
}

KCBSensor::~KCBSensor(void)
{
    Shutdown();
}

HRESULT KCBSensor::Initialize(_In_ IKinectSensor* pSensor)
{
    if(nullptr == pSensor)
    {
        return E_PENDING;
    }

    AutoLock lock(m_lock);

    HRESULT hr = S_OK;
    
    if(nullptr != m_pKinectSensor)
    {
        Shutdown();
    }

    m_pKinectSensor = pSensor;
    m_pKinectSensor->AddRef();
    
    return hr;
}

void KCBSensor::Shutdown()
{
    AutoLock lock(m_lock);

    HRESULT hr = S_OK;

    // close the sensor
    if(nullptr != m_pKinectSensor)
    {
        hr = m_pKinectSensor->Close();
    }

    // body reader
    if(nullptr != m_pBodyFrameReader) 
    {
        hr = m_pBodyFrameReader->UnsubscribeFrameArrived(m_hFrameEvents[KCBSourceType_Body]); 

        m_hFrameEvents[KCBSourceType_Body] = NULL;
        SAFE_RELEASE(m_pBodyFrameReader);
    }

    // body index reader
    if(nullptr != m_pBodyIndexFrameReader)
    {
        hr = m_pBodyIndexFrameReader->UnsubscribeFrameArrived(m_hFrameEvents[KCBSourceType_BodyIndex]); 
        
        m_hFrameEvents[KCBSourceType_BodyIndex] = NULL;
        SAFE_RELEASE(m_pBodyIndexFrameReader);
    }

    // color reader
    if(nullptr != m_pColorFrameReader)
    {
        if(nullptr != m_pColorFrameDescription)
        {
            SAFE_RELEASE(m_pColorFrameDescription);
        }

        hr = m_pColorFrameReader->UnsubscribeFrameArrived(m_hFrameEvents[KCBSourceType_Color]); 

        m_hFrameEvents[KCBSourceType_Color] = NULL;
        SAFE_RELEASE(m_pColorFrameReader);
    }

    // depth reader
    if(nullptr != m_pBodyIndexFrameReader)
    {
        hr = m_pDepthFrameReader->UnsubscribeFrameArrived(m_hFrameEvents[KCBSourceType_Depth]); 

        m_hFrameEvents[KCBSourceType_Depth] = NULL;
        SAFE_RELEASE(m_pDepthFrameReader);
    }

    // infrared reader
    if(nullptr != m_pInfraredFrameReader)
    {
        hr = m_pInfraredFrameReader->UnsubscribeFrameArrived(m_hFrameEvents[KCBSourceType_Infrared]); 

        m_hFrameEvents[KCBSourceType_Infrared] = NULL;
        SAFE_RELEASE(m_pInfraredFrameReader);
    }

    // long ir
    if(nullptr != m_pLongExposureInfraredFrameReader)
    {
        hr = m_pLongExposureInfraredFrameReader->UnsubscribeFrameArrived(m_hFrameEvents[KCBSourceType_LongExposureInfrared]); 

        m_hFrameEvents[KCBSourceType_LongExposureInfrared] = NULL;
        SAFE_RELEASE(m_pLongExposureInfraredFrameReader);
    }

    // audio
    if (nullptr != m_pAudioStream)
    {
        SAFE_RELEASE(m_pAudioStream);
    }

    if (nullptr != m_pAudioBeam)
    {
        SAFE_RELEASE(m_pAudioBeam);
    }

    // multisource reader
    if(nullptr != m_pMultiSourceFrameReader)
    {
        m_eFrameSourceTypes = FrameSourceTypes_None;

        hr = m_pMultiSourceFrameReader->UnsubscribeMultiSourceFrameArrived(m_hMultiSourceFrameEvent);

        m_hMultiSourceFrameEvent = NULL;
        SAFE_RELEASE(m_pMultiSourceFrameReader);
    }

    // cleanup
    SAFE_RELEASE(m_pAudioSource);
    SAFE_RELEASE(m_pBodyFrameSource);
    SAFE_RELEASE(m_pBodyIndexFrameSource);
    SAFE_RELEASE(m_pColorFrameSource);
    SAFE_RELEASE(m_pDepthFrameSource);
    SAFE_RELEASE(m_pInfraredFrameSource);
    SAFE_RELEASE(m_pLongExposureInfraredFrameSource);

    SAFE_RELEASE(m_pCoordinateMapper);

    SAFE_RELEASE(m_pKinectSensor);
}


// IUnknown methods
IFACEMETHODIMP KCBSensor::QueryInterface(REFIID riid, _COM_Outptr_ void** ppvObject)
{
    AutoLock lock(m_lock);

    if (nullptr == ppvObject)
    {
        return E_POINTER;
    }

    HRESULT hr = E_NOINTERFACE;

    (*ppvObject) = nullptr;

    if (riid == IID_IUnknown ||
        riid ==  __uuidof(IKCBSensor))
    {
        (*ppvObject) = static_cast<IKCBSensor *>(this);
        AddRef();

        hr = S_OK;
    }

    return hr;
}

IFACEMETHODIMP_(ULONG) KCBSensor::AddRef()
{
    return InterlockedIncrement(&m_refCount);
}

IFACEMETHODIMP_(ULONG) KCBSensor::Release()
{
    long cRef = InterlockedDecrement(&m_refCount);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}


// IKCBSensor
HRESULT KCBSensor::GetKinectInterface(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject)
{
    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    HRESULT hr = E_NOINTERFACE;

    // ICoordinateMapper
    if(__uuidof(ICoordinateMapper) == riid)
    {
        // if it wasn't created yet, create one from the sensor
        if( nullptr == m_pCoordinateMapper)
        {
            CHECK_HR(hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper));
        }

        assert(nullptr != m_pCoordinateMapper);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pCoordinateMapper;
            m_pCoordinateMapper->AddRef(); // caller must release its copy
        }

        hr = S_OK;
    }

done:
    return hr;
}
//
//HRESULT KCBSensor::GetStatus(_Out_ KinectStatus* peStatus)
//{
//    AutoLock lock(m_lock);
//
//    if(nullptr == peStatus)
//    {
//        return E_POINTER;
//    }
//
//    assert( nullptr != m_pKinectSensor );
//
//    return m_pKinectSensor->get_Status(peStatus);
//}


// wrapper around for returning the various 
// frame sources for the Kinect
// riid = _uuidof(IKinectInterface)
// ppvObject = generic pointer to the object
// return = S_OK if found
HRESULT KCBSensor::GetSource(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject)
{
    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    HRESULT hr = S_OK;

    CHECK_HR(hr = OpenSensor(m_pKinectSensor));

    hr = E_NOINTERFACE;

    if(__uuidof(IBodyFrameSource) == riid)
    {
        if(nullptr == m_pBodyFrameSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_BodyFrameSource(&m_pBodyFrameSource));
        }

        assert(nullptr != m_pBodyFrameSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pBodyFrameSource;
            m_pBodyFrameSource->AddRef(); // caller must release its copy
        }

        hr = S_OK;
    }
    else if(__uuidof(IBodyIndexFrameSource) == riid)
    {
        if(nullptr == m_pBodyIndexFrameSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_BodyIndexFrameSource(&m_pBodyIndexFrameSource));
        }

        assert(nullptr != m_pBodyIndexFrameSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pBodyIndexFrameSource;
            m_pBodyIndexFrameSource->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IColorFrameSource) == riid)
    {
        if(nullptr == m_pColorFrameSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_ColorFrameSource(&m_pColorFrameSource));
        }
        
        assert(nullptr != m_pColorFrameSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pColorFrameSource;
            m_pColorFrameSource->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IDepthFrameSource) == riid)
    {
        if(nullptr == m_pDepthFrameSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_DepthFrameSource(&m_pDepthFrameSource));
        }

        assert(nullptr != m_pDepthFrameSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pDepthFrameSource;
            m_pDepthFrameSource->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IInfraredFrameSource) == riid)
    {
        if(nullptr == m_pInfraredFrameSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_InfraredFrameSource(&m_pInfraredFrameSource));
        }

        assert(nullptr != m_pInfraredFrameSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pInfraredFrameSource;
            m_pInfraredFrameSource->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(ILongExposureInfraredFrameSource) == riid)
    {
        if(nullptr == m_pLongExposureInfraredFrameSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_LongExposureInfraredFrameSource(&m_pLongExposureInfraredFrameSource));
        }

        assert(nullptr != m_pLongExposureInfraredFrameSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pLongExposureInfraredFrameSource;
            m_pLongExposureInfraredFrameSource->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IAudioSource) == riid)
    {
        if(nullptr == m_pAudioSource)
        {
            CHECK_HR(hr = m_pKinectSensor->get_AudioSource(&m_pAudioSource));
        }

        assert(nullptr != m_pAudioSource);

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pAudioSource;
            m_pAudioSource->AddRef();
        }

        hr = S_OK;
    }

done:
    return hr;
}

HRESULT KCBSensor::OpenReader(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject)
{
    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    HRESULT hr = E_NOINTERFACE;

    if(__uuidof(IBodyFrameReader) == riid)
    {
        if(nullptr == m_pBodyFrameReader)
        {
            if(nullptr == m_pBodyFrameSource)
            {
                IBodyFrameSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(IBodyFrameSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }
            CHECK_HR(hr = m_pBodyFrameSource->OpenReader(&m_pBodyFrameReader));
        }

        assert(nullptr != m_pBodyFrameReader);

        // create event handle
        if(NULL == m_hFrameEvents[KCBSourceType_Body])
        {
            CHECK_HR(hr = m_pBodyFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_Body]));
        }

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pBodyFrameReader;
            m_pBodyFrameReader->AddRef();
        }        

        hr = S_OK;
    }
    else if(__uuidof(IBodyIndexFrameReader) == riid)
    {
        if(nullptr == m_pBodyIndexFrameReader)
        {
            if(nullptr == m_pBodyIndexFrameSource)
            {
                IBodyIndexFrameSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(IBodyIndexFrameSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }
            CHECK_HR(hr = m_pBodyIndexFrameSource->OpenReader(&m_pBodyIndexFrameReader));
        }

        assert(nullptr != m_pBodyIndexFrameReader);

        // create event handle
        if(NULL == m_hFrameEvents[KCBSourceType_BodyIndex])
        {
            CHECK_HR(hr = m_pBodyIndexFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_BodyIndex]));
        }

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pBodyIndexFrameReader;
            m_pBodyIndexFrameReader->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IColorFrameReader) == riid)
    {
        if(nullptr == m_pColorFrameReader)
        {
            if(nullptr == m_pColorFrameSource)
            {
                IColorFrameSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(IColorFrameSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }
            CHECK_HR(hr = m_pColorFrameSource->OpenReader(&m_pColorFrameReader));
        }

        assert(nullptr != m_pColorFrameReader);

        // create event handle
        if (NULL == m_hFrameEvents[KCBSourceType_Color])
        {
            CHECK_HR(hr = m_pColorFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_Color]));
        }

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pColorFrameReader;
            m_pColorFrameReader->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IDepthFrameReader) == riid)
    {
        if(nullptr == m_pDepthFrameReader)
        {
            if(nullptr == m_pDepthFrameSource)
            {
                IDepthFrameSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(IDepthFrameSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }
            CHECK_HR(hr = m_pDepthFrameSource->OpenReader(&m_pDepthFrameReader));
        }

        assert(nullptr != m_pDepthFrameReader);

        // create event handle
        if (NULL == m_hFrameEvents[KCBSourceType_Depth])
        {
            CHECK_HR(hr = m_pDepthFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_Depth]));
        }

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pDepthFrameReader;
            m_pDepthFrameReader->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IInfraredFrameReader) == riid)
    {
        if(nullptr == m_pInfraredFrameReader)
        {
            if(nullptr == m_pInfraredFrameSource)
            {
                IInfraredFrameSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(IInfraredFrameSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }
            CHECK_HR(hr = m_pInfraredFrameSource->OpenReader(&m_pInfraredFrameReader));
        }
        
        assert(nullptr != m_pInfraredFrameReader);

        // create event handle
        if (NULL == m_hFrameEvents[KCBSourceType_Infrared])
        {
            CHECK_HR(hr = m_pInfraredFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_Infrared]));
        }

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pInfraredFrameReader;
            m_pInfraredFrameReader->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(ILongExposureInfraredFrameReader) == riid)
    {
        if(nullptr == m_pLongExposureInfraredFrameReader)
        {
            if(nullptr == m_pLongExposureInfraredFrameSource)
            {
                ILongExposureInfraredFrameSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(ILongExposureInfraredFrameSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }
            CHECK_HR(hr = m_pLongExposureInfraredFrameSource->OpenReader(&m_pLongExposureInfraredFrameReader));
        }

        assert(nullptr != m_pLongExposureInfraredFrameReader);

        // create event handle
        if (NULL == m_hFrameEvents[KCBSourceType_LongExposureInfrared])
        {
            CHECK_HR(hr = m_pLongExposureInfraredFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_LongExposureInfrared]));
        }

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pLongExposureInfraredFrameReader;
            m_pLongExposureInfraredFrameReader->AddRef();
        }

        hr = S_OK;
    }
    else if(__uuidof(IAudioBeam) == riid)
    {
        if (nullptr == m_pAudioBeam)
        {
            if(nullptr == m_pAudioSource)
            {
                IAudioSource* pSource = nullptr;
                CHECK_HR(hr = GetSource(__uuidof(IAudioSource), reinterpret_cast<void**>(&pSource)));
                SAFE_RELEASE(pSource);
            }

            IAudioBeamList* pAudioBeamList = nullptr;
            CHECK_HR(hr = m_pAudioSource->get_AudioBeams(&pAudioBeamList));

            hr = pAudioBeamList->OpenAudioBeam(0, &m_pAudioBeam);
            SAFE_RELEASE(pAudioBeamList);

            CHECK_HR(hr);

            CHECK_HR(hr = m_pAudioBeam->OpenInputStream(&m_pAudioStream));
        }
        
        assert(nullptr != m_pAudioBeam);

        // create event handle
        //if(NULL == m_hFrameEvents[KCBSourceType_Audio])
        //{
        //    CHECK_HR(hr = m_pAudioBeamFrameReader->SubscribeFrameArrived(&m_hFrameEvents[KCBSourceType_Audio]));
        //}

        if(nullptr != ppvObject)
        {
            (*ppvObject) = m_pAudioStream;
            m_pAudioStream->AddRef();
        }

        hr = S_OK;
    }

done:
    return hr;
}

HRESULT KCBSensor::OpenMultiSourceReader(DWORD eFrameSourceTypes, _COM_Outptr_opt_result_maybenull_ IMultiSourceFrameReader** ppvObject)
{
    if(FrameSourceTypes_None == eFrameSourceTypes)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    HRESULT hr = E_NOINTERFACE;

    CHECK_HR(hr = OpenSensor(m_pKinectSensor));

    if(nullptr == m_pMultiSourceFrameReader || m_eFrameSourceTypes != eFrameSourceTypes)
    {
        // reset the reader if the streams are different
        if( m_eFrameSourceTypes != eFrameSourceTypes )
        {
            SAFE_RELEASE(m_pMultiSourceFrameReader);
        }

        CHECK_HR(hr = m_pKinectSensor->OpenMultiSourceFrameReader(eFrameSourceTypes, &m_pMultiSourceFrameReader));

        m_eFrameSourceTypes = eFrameSourceTypes;
    }

    // create event handle
    if(NULL == m_hMultiSourceFrameEvent)
    {
        CHECK_HR(hr = m_pMultiSourceFrameReader->SubscribeMultiSourceFrameArrived(&m_hMultiSourceFrameEvent));
    }

    if(nullptr != ppvObject)
    {
        (*ppvObject) = m_pMultiSourceFrameReader;
        m_pMultiSourceFrameReader->AddRef();
    }

    hr = S_OK;

done:
    return hr;
}

HRESULT KCBSensor::CreateColorFrameDescription(ColorImageFormat eFormat, _COM_Outptr_result_maybenull_ IFrameDescription** ppvObject)
{
    if(nullptr == ppvObject)
    {
        return E_POINTER;
    }

    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    (*ppvObject) = nullptr;

    HRESULT hr = E_NOINTERFACE;

    if(nullptr == m_pColorFrameDescription)
    {
        if(nullptr == m_pColorFrameSource)
        {
            IColorFrameSource* pSource = nullptr;
            CHECK_HR(hr = GetSource(__uuidof(IColorFrameSource), reinterpret_cast<void**>(&pSource)));
            SAFE_RELEASE(pSource);
        }
    }

    CHECK_HR(hr = m_pColorFrameSource->CreateFrameDescription(eFormat, &m_pColorFrameDescription));

done:
    return hr;
}

HRESULT KCBSensor::GetFrame(REFIID riid, _COM_Outptr_result_maybenull_ void** ppvObject)
{
    if(nullptr == ppvObject)
    {
        return E_POINTER;
    }

    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    (*ppvObject) = nullptr;

    HRESULT hr = E_NOINTERFACE;

    if(__uuidof(IBodyFrame) == riid)
    {
        if(nullptr == m_pBodyFrameReader)
        {
            IBodyFrameReader* pReader = nullptr;
            CHECK_HR(hr = OpenReader(__uuidof(IBodyFrameReader), reinterpret_cast<void**>(&pReader)));
            SAFE_RELEASE(pReader);
        }

        assert(nullptr != m_pBodyFrameReader);

        IBodyFrame* pFrame = nullptr;
        CHECK_HR(hr = m_pBodyFrameReader->AcquireLatestFrame(&pFrame));

        (*ppvObject) = pFrame;
    }
    else if(__uuidof(IBodyIndexFrame) == riid)
    {
        if(nullptr == m_pBodyIndexFrameReader)
        {
            IBodyIndexFrameReader* pReader = nullptr;
            CHECK_HR(hr = OpenReader(__uuidof(IBodyIndexFrameReader), reinterpret_cast<void**>(&pReader)));
            SAFE_RELEASE(pReader);
        }

        assert(nullptr != m_pBodyIndexFrameReader);

        IBodyIndexFrame* pFrame = nullptr;
        CHECK_HR(hr = m_pBodyIndexFrameReader->AcquireLatestFrame(&pFrame));

        (*ppvObject) = pFrame;
    }
    else if(__uuidof(IColorFrame) == riid)
    {
        if(nullptr == m_pColorFrameReader)
        {
            IColorFrameReader* pReader = nullptr;
            CHECK_HR(hr = OpenReader(__uuidof(IColorFrameReader), reinterpret_cast<void**>(&pReader)));
            SAFE_RELEASE(pReader);
        }

        assert(nullptr != m_pColorFrameReader);

        IColorFrame* pFrame = nullptr;
        CHECK_HR(hr = m_pColorFrameReader->AcquireLatestFrame(&pFrame));

        (*ppvObject) = pFrame;
    }
    else if(__uuidof(IDepthFrame) == riid)
    {
        if(nullptr == m_pDepthFrameReader)
        {
            IDepthFrameReader* pReader = nullptr;
            CHECK_HR(hr = OpenReader(__uuidof(IDepthFrameReader), reinterpret_cast<void**>(&pReader)));
            SAFE_RELEASE(pReader);
        }

        assert(nullptr != m_pDepthFrameReader);

        IDepthFrame* pFrame = nullptr;
        CHECK_HR(hr = m_pDepthFrameReader->AcquireLatestFrame(&pFrame));

        (*ppvObject) = pFrame;
    }
    else if(__uuidof(IInfraredFrame) == riid)
    {
        if(nullptr == m_pInfraredFrameReader)
        {
            IInfraredFrameReader* pReader = nullptr;
            CHECK_HR(hr = OpenReader(__uuidof(IInfraredFrameReader), reinterpret_cast<void**>(&pReader)));
            SAFE_RELEASE(pReader);
        }

        assert(nullptr != m_pInfraredFrameReader);

        IInfraredFrame* pFrame = nullptr;
        CHECK_HR(hr = m_pInfraredFrameReader->AcquireLatestFrame(&pFrame));

        (*ppvObject) = pFrame;
    }
    else if(__uuidof(ILongExposureInfraredFrame) == riid)
    {
        if(nullptr == m_pLongExposureInfraredFrameReader)
        {
            ILongExposureInfraredFrameReader* pReader = nullptr;
            CHECK_HR(hr = OpenReader(__uuidof(ILongExposureInfraredFrameReader), reinterpret_cast<void**>(&pReader)));
            SAFE_RELEASE(pReader);
        }

        assert(nullptr != m_pLongExposureInfraredFrameReader);

        ILongExposureInfraredFrame* pFrame = nullptr;
        CHECK_HR(hr = m_pLongExposureInfraredFrameReader->AcquireLatestFrame(&pFrame));

        (*ppvObject) = pFrame;
    }

done:
    return hr;
}

HRESULT KCBSensor::GetAudioBuffer(ULONG cb, _Out_writes_bytes_to_(cb, *pcbRead) byte *cbBuffer, _Inout_ ULONG *pcbRead, _Out_opt_ FLOAT* beamAngle, _Out_opt_ FLOAT* sourceConfidence)
{
    if (nullptr == cbBuffer || nullptr == pcbRead)
    {
        return E_POINTER;
    }

    AutoLock lock(m_lock);

    assert(nullptr != m_pKinectSensor);

    HRESULT hr = E_NOINTERFACE;

    if (nullptr == m_pAudioStream)
    {
        IAudioBeam* pAudioBeam = nullptr;
        CHECK_HR(hr = OpenReader(__uuidof(IAudioBeam), reinterpret_cast<void**>(&pAudioBeam)));
        SAFE_RELEASE(pAudioBeam);
    }

    CHECK_HR(hr = m_pAudioStream->Read((void *)cbBuffer, sizeof(cb), pcbRead));

    if (*pcbRead > 0)
    {
        // Get most recent audio beam angle and confidence
        if (nullptr != beamAngle)
        {
            m_pAudioBeam->get_BeamAngle(beamAngle);
        }
        if (nullptr != sourceConfidence)
        {
            m_pAudioBeam->get_BeamAngleConfidence(sourceConfidence);
        }
    }

done:
    return hr;
}

bool KCBSensor::IsFrameReady(KCBSourceType eSource = KCBSourceType_Unknown)
{
    AutoLock lock(m_lock);

    HRESULT hr = S_OK;
    bool bIsReady = false;
    
    // ensure the reader you want is opened
    switch (eSource)
    {
    //case KCBSourceType_Audio:
    //    CHECK_HR(hr = OpenReader(__uuidof(IAudioBeamFrameReader), nullptr));
    //    break;
    case KCBSourceType_Body:
        CHECK_HR(hr = OpenReader(__uuidof(IBodyFrameReader), nullptr));
        break;
    case KCBSourceType_BodyIndex:
        CHECK_HR(hr = OpenReader(__uuidof(IBodyIndexFrameReader), nullptr));
        break;
    case KCBSourceType_Color:
        CHECK_HR(hr = OpenReader(__uuidof(IColorFrameReader), nullptr));
        break;
    case KCBSourceType_Depth:
        CHECK_HR(hr = OpenReader(__uuidof(IDepthFrameReader), nullptr));
        break;
    case KCBSourceType_Infrared:
        CHECK_HR(hr = OpenReader(__uuidof(IInfraredFrameReader), nullptr));
        break;
    case KCBSourceType_LongExposureInfrared:
        CHECK_HR(hr = OpenReader(__uuidof(ILongExposureInfraredFrameReader), nullptr));
        break;
    }

    // check if the event has been set
    if (eSource != KCBSourceType_Unknown && NULL != m_hFrameEvents[eSource])
    {
        DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(m_hFrameEvents[eSource]), 0, FALSE);
        if (WAIT_OBJECT_0 == dwResult)
        {
            bIsReady = true;
        }
    }

done:
    if(FAILED(hr))
    {
        bIsReady = false;
    }

    return bIsReady;
}

// check if any frame is ready
bool KCBSensor::AnyFrameReady()
{
    AutoLock lock(m_lock);

    // create a list of events to check
    std::vector<HANDLE> events;
    GetWaitEvents(events);

    if (events.empty())
    {
        return false;
    }

    // if any frame is ready we return true
    DWORD dwResult = WaitForMultipleObjectsEx((DWORD) events.size(), events.data(), false, 0, FALSE);
    if (dwResult == WAIT_OBJECT_0)
    {
        return true;
    }

    return false;
}

// help with sync between color/depth frames
// not guarnteed to be exact because of the time it takes to execute the copy
bool KCBSensor::AllFramesReady()
{
    AutoLock lock(m_lock);

    std::vector<HANDLE> events;
    GetWaitEvents(events);

    if (events.empty())
    {
        return false;
    }

    // if all frame events as set, return true
    DWORD dwResult = WaitForMultipleObjectsEx((DWORD) events.size(), events.data(), true, 0, false);
    if (dwResult == WAIT_OBJECT_0)
    {
        return true;
    }

    return false;
}

bool KCBSensor::MultiFrameReady()
{
    if(FrameSourceTypes_None == m_eFrameSourceTypes  || nullptr == m_pMultiSourceFrameReader)
    {
        return false;
    }

    AutoLock lock(m_lock);

    bool bIsReady = false;

    // check if the event has been set
    DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(m_hMultiSourceFrameEvent), 0, false);
    if (WAIT_OBJECT_0 == dwResult)
    {
        bIsReady = true;
    }

    return bIsReady;
}
// add events for enabled streams
void KCBSensor::GetWaitEvents(_Inout_ std::vector<HANDLE>& events)
{
    for(int i = 0; i < KCBSourceType_Count; ++i)
    {
        if (NULL != m_hFrameEvents[i])
        {
            events.push_back(reinterpret_cast<HANDLE>(m_hFrameEvents[i]));
        }
    }
}
