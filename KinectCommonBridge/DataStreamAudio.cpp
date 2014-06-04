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

#include "DataStreamAudio.h"
#include "AutoLock.h"
#include "MediaBuffer.h"

#include <mmdeviceapi.h>
#include <endpointvolume.h> 
#include <functiondiscoverykeys_devpkey.h>
#include <Audioclient.h>

#include <sstream>

DataStreamAudio::DataStreamAudio()
: m_eAECSystemMode(OPTIBEAM_ARRAY_AND_AEC)
, m_bOverrideFeatureMode(false) 
, m_lAES(0)					// MFPKEY_WMAAECMA_FEATR_AES Property
, m_bGainBounder(true)		// MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER
, m_pOutputBuffer(nullptr)
, m_pNuiAudioSource(nullptr)

, m_pKinectAudioStream(nullptr)

#ifdef KCB_ENABLE_SPEECH
, m_bAdaptation(true)
, m_wcGrammarFileName(L"")
, m_sLanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US))
, m_ullEventInterest(SPFEI(SPEI_RECOGNITION))
, m_pSpeechStream(nullptr)
, m_pSpeechRecognizer(nullptr)
, m_pSpeechContext(nullptr)
, m_pSpeechGrammar(nullptr)
#endif
{
}

DataStreamAudio::~DataStreamAudio()
{
    RemoveDevice();
}

void DataStreamAudio::Reset()
{
#ifdef KCB_ENABLE_SPEECH
    ResetSpeech();
#endif

	m_pKinectAudioStream.Release();

	m_pOutputBuffer.Release();
    m_pNuiAudioSource.Release();
}

#ifdef KCB_ENABLE_SPEECH
void DataStreamAudio::ResetSpeech()
{
    if (nullptr != m_pKinectAudioStream)
    {
        m_pKinectAudioStream->StopCapture();
    }

	if( nullptr != m_pSpeechRecognizer )
	{
		m_pSpeechRecognizer->SetRecoState(SPRST_INACTIVE);
	}

    m_pSpeechStream.Release();
    m_pSpeechRecognizer.Release();
    m_pSpeechContext.Release();
    m_pSpeechGrammar.Release();

    m_started = false;
}
#endif

void DataStreamAudio::RemoveDevice()
{
    AutoLock lock(m_nuiLock);

    Reset();

    DataStream::RemoveDevice();
}

void DataStreamAudio::Initialize(_In_ INuiSensor* pNuiSensor)
{
    DataStream::Initialize(pNuiSensor);
}

void DataStreamAudio::Initialize(_In_opt_ AEC_SYSTEM_MODE* eAECSystemMode, _In_opt_ bool* bGainBounder, _In_ INuiSensor* pNuiSensor)
{
    AutoLock lock(m_nuiLock);

    bool bChanged = false;

    if (nullptr != eAECSystemMode)
    {
        if (m_eAECSystemMode != *eAECSystemMode)
        {
            m_eAECSystemMode = *eAECSystemMode;
			m_bOverrideFeatureMode = true;
            bChanged = true;
        }
    }

	if( nullptr != bGainBounder )
	{
		if( m_bGainBounder != *bGainBounder )
		{
			m_bGainBounder = *bGainBounder;
			m_bOverrideFeatureMode = true;
			bChanged = true;
		}
	}

    if (bChanged)
    {
        m_started = false;
    }

    // send the sensor to the base class
    Initialize(pNuiSensor);
}

#ifdef KCB_ENABLE_SPEECH
void DataStreamAudio::Initialize(_In_ const WCHAR* wcGrammarFileName, _In_opt_ KCB_SPEECH_LANGUAGE* sLanguage, _In_opt_ ULONGLONG* ullEventInterest, _In_opt_ bool* bAdaptation)
{
    AutoLock lock(m_nuiLock);

    if (CLSID_ExpectedRecognizer != CLSID_SpInprocRecognizer)
    {
        MessageBoxW(NULL, L"This sample was compiled against an incompatible version of sapi.h.\nPlease ensure that Microsoft Speech SDK and other sample requirements are installed and then rebuild application.", L"Missing requirements", MB_OK | MB_ICONERROR);
    }

    bool bChanged = false;

    if (nullptr != wcGrammarFileName && 0 != m_wcGrammarFileName.compare(wcGrammarFileName))
    {
        m_wcGrammarFileName = wcGrammarFileName;
        bChanged = true;
    }

    if (nullptr != sLanguage && m_sLanguage != *sLanguage)
    {
        m_sLanguage = *sLanguage;
        bChanged = true;
    }

    if (nullptr != ullEventInterest && m_ullEventInterest != *ullEventInterest)
    {
        m_ullEventInterest = *ullEventInterest;
        bChanged = true;
    }
    
    if (nullptr != bAdaptation && m_bAdaptation != *bAdaptation)
    {
        m_bAdaptation = *bAdaptation;
        bChanged = true;
    }

    if (bChanged)
    {
        m_started = false;
    }
}
#endif

HRESULT DataStreamAudio::StartStream()
{
    AutoLock lock(m_nuiLock);

#ifdef KCB_ENABLE_SPEECH
    if (IsSpeechEnabled())
    {
        return StartSpeech();
    }
#endif

    HRESULT hr = OpenStream();

    if (SUCCEEDED(hr))
    {
        m_started = true;
    }
    else
    {
        RemoveDevice();
        m_started = false;
    }

    return hr;
}

#ifdef KCB_ENABLE_SPEECH
HRESULT DataStreamAudio::StartSpeech()
{
    AutoLock lock(m_nuiLock);

    // if there is no device, disable the stream 
    if (nullptr == m_pNuiSensor)
    {
        RemoveDevice();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    if (m_started)
    {
        return hr;
    }

	// Set up an audio input stream
	hr = OpenStream();

	ComSmartPtr<IStream> pStream;
	if (SUCCEEDED(hr))
	{
		hr = m_pKinectAudioStream->QueryInterface(IID_IStream, (void**) &pStream);

		if (SUCCEEDED(hr))
		{
			if( nullptr != m_pSpeechStream )
			{
				m_pSpeechStream.Release();
			}

            // same as calling "CoCreateInstance(CLSID_SpStream, NULL, CLSCTX_INPROC_SERVER, IID_ISpStream, reinterpret_cast<void**>(&m_pSpeechStream));"
			hr = m_pSpeechStream.CoCreateInstance(CLSID_SpStream);

			if (SUCCEEDED(hr))
			{
				hr = m_pSpeechStream->SetBaseStream(pStream, SPDFID_WaveFormatEx, &KINECT_WAVEFORMATEX);
			}
		}
	}

    if (SUCCEEDED(hr))
    {
		hr = CreateSpeechRecognizer();
	}

	if( SUCCEEDED(hr) )
	{
	    hr = m_pSpeechRecognizer->SetInput(m_pSpeechStream, FALSE);
    }

	// load grammar file
	if( SUCCEEDED(hr) )
	{
	    hr = SetSpeechGrammar(m_wcGrammarFileName.c_str());
	}

    // start capture thread
	if( SUCCEEDED(hr) )
	{
	    hr = m_pKinectAudioStream->StartCapture();

		if (SUCCEEDED(hr))
		{
			// Specify that all top level rules in grammar are now active
			hr = m_pSpeechGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
			if (FAILED(hr))
			{
				return hr;
			}

			// Specify that engine should always be reading audio
			hr = m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
			if (FAILED(hr))
			{
				return hr;
			}

			// Ensure that engine is recognizing speech and not in paused state
			m_pSpeechContext->Resume(0);
		}
    }

    if (SUCCEEDED(hr))
    {
        m_started = true;
    }
    else
    {
        RemoveDevice();
        m_started = false;
    }

    return hr;
}
#endif

void DataStreamAudio::StopStream()
{
    AutoLock lock(m_nuiLock);

    RemoveDevice();
}

void DataStreamAudio::PauseStream(bool bPause)
{
#ifdef KCB_ENABLE_SPEECH
	if( bPause && nullptr != m_pSpeechRecognizer)
	{
		m_pSpeechRecognizer->SetRecoState(SPRST_INACTIVE_WITH_PURGE);
	}
	else if( nullptr != m_pSpeechRecognizer )
	{
		m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);

		if( nullptr != m_pSpeechContext )
		{
			m_pSpeechContext->Resume(0);
		}
	}
#endif

	m_paused = bPause;
}

HANDLE DataStreamAudio::GetFrameReadyEvent()
{
    return m_hStreamHandle;
}

void DataStreamAudio::CopyData(_In_ void *pImageFrame)
{
    // do nothing
}

HRESULT DataStreamAudio::OpenStream()
{
    // if there is no device, disable the stream 
    if (nullptr == m_pNuiSensor)
    {
        RemoveDevice();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    if (m_started)
    {
        return hr;
    }

    { // setup DMO

        // Get the audio source
        INuiAudioBeam* pNuiAudioSource = nullptr;
        hr = m_pNuiSensor->NuiGetAudioSource(&pNuiAudioSource);
        if (FAILED(hr))
        {
            goto done;
        }

        // assign the interface to our instance
        m_pNuiAudioSource.Attach(pNuiAudioSource);

        ComSmartPtr<IMediaObject> pDMO;
        hr = m_pNuiAudioSource->QueryInterface(IID_IMediaObject, (void**) &pDMO);
        if (FAILED(hr))
        {
            goto done;
        }

        // Get the property store for the DMO
        ComSmartPtr<IPropertyStore> pPropertyStore;
        hr = m_pNuiAudioSource->QueryInterface(IID_IPropertyStore, (void**) &pPropertyStore);
        if (FAILED(hr))
        {
            goto done;
        }
		
		// set system mode for AEC processing
		// http://msdn.microsoft.com/en-us/library/windows/desktop/ff819427(v=vs.85).aspx
        PROPVARIANT pv;
        PropVariantInit(&pv);
        pv.vt = VT_I4;
        pv.lVal = m_eAECSystemMode;
        hr = pPropertyStore->SetValue(MFPKEY_WMAAECMA_SYSTEM_MODE, pv);
        PropVariantClear(&pv);

		// override voice dsp settings
		if( m_bOverrideFeatureMode )
		{
			PropVariantInit(&pv);
			pv.vt = VT_BOOL;
			pv.lVal = VARIANT_TRUE;
			hr = pPropertyStore->SetValue(MFPKEY_WMAAECMA_FEATURE_MODE, pv);
			PropVariantClear(&pv);

			if( 0 != m_lAES && m_lAES <= 2 )
			{
				// how many times to perform acoustic echo suppression (AES)
				// 0 default, 1 or 2
				// http://msdn.microsoft.com/en-us/library/windows/desktop/ff819411(v=vs.85).aspx
				PropVariantInit(&pv);
				pv.vt = VT_I4;
				pv.lVal = m_lAES;
				hr = pPropertyStore->SetValue(MFPKEY_WMAAECMA_FEATR_AES, pv);
				PropVariantClear(&pv);
			}

			if( !m_bGainBounder )
			{
				// gain control - MFPKEY_WMAAECMA_FEATR_AGC is disabled by default
				// MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER is on by default, may want to disable in the case
				// of manually setting gain level
				// http://msdn.microsoft.com/en-us/library/windows/desktop/ff819424(v=vs.85).aspx
				PropVariantInit(&pv);
				pv.vt = VT_BOOL;
				pv.lVal = VARIANT_FALSE;
				hr = pPropertyStore->SetValue(MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER, pv);
				PropVariantClear(&pv);
			}
		}

        // Set DMO output format
        DMO_MEDIA_TYPE mt = { 0 };
        hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
        if (FAILED(hr))
        {
            goto done;
        }

        // Set the type GUIDs.
        mt.majortype = MEDIATYPE_Audio;
        mt.subtype = MEDIASUBTYPE_PCM;
        mt.formattype = FORMAT_WaveFormatEx;

        mt.lSampleSize = 0;
        mt.bFixedSizeSamples = TRUE;
        mt.bTemporalCompression = FALSE;

        // Initialize the format block.
        memcpy_s(mt.pbFormat, sizeof(WAVEFORMATEX), &KINECT_WAVEFORMATEX, sizeof(WAVEFORMATEX));

        // configure the DMO output with the wfx format
        hr = pDMO->SetOutputType(0, &mt, 0);

        // free the media type
        MoFreeMediaType(&mt);
        if (FAILED(hr))
        {
            goto done;
        }

        // this calls ComSmartPtr::operator=(KinectAudioStream*)
        m_pKinectAudioStream = new KinectAudioStream(pDMO);
	}

done:

    return hr;
}

HRESULT DataStreamAudio::GetSample(
    _Out_ DWORD* cbProduced, _Out_ BYTE** ppbOutputBuffer,
    _Out_ DWORD* dwStatus, _Out_opt_ LONGLONG *llTimeStamp, _Out_opt_ LONGLONG *llTimeLength,
    _Out_opt_ double *beamAngle, _Out_opt_ double *sourceAngle, _Out_opt_ double *sourceConfidence)
{
    if (nullptr == cbProduced || nullptr == ppbOutputBuffer || nullptr == dwStatus)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_nuiLock);

    if (nullptr == m_pNuiSensor)
    {
        Reset();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    // if we haven't started the stream do it now
    if (!m_started)
    {
        hr = StartStream();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // reusable buffer
    if (nullptr == m_pOutputBuffer)
    {
        MediaBuffer::Create(KINECT_WAVEFORMATEX, &m_pOutputBuffer);
    }
    else
    {
        MediaBuffer::Reset(m_pOutputBuffer);
    }

    DMO_OUTPUT_DATA_BUFFER OutputBufferStruct = { 0 };
    OutputBufferStruct.pBuffer = m_pOutputBuffer;
    DWORD ignored = 0;

    IMediaObject* pDMO = m_pKinectAudioStream->GetAudioDMO();

	OutputBufferStruct.dwStatus = 0;
    if (!m_paused)
    {
        hr = pDMO->ProcessOutput(0, 1, &OutputBufferStruct, &ignored);
        *dwStatus = OutputBufferStruct.dwStatus;
        if (FAILED(hr))
        {
            return hr;
        }
    }
    else
    {
        // create silence
        MediaBuffer::Reset(m_pOutputBuffer, true);
    }

    if (hr != S_FALSE)
    {
        m_pOutputBuffer->GetBufferAndLength(ppbOutputBuffer, cbProduced);
    }
    else
    {
        return hr;
    }

    if (!m_paused)
    {
        // only set the timestampe when not paused
        m_llLastTimeStamp = OutputBufferStruct.rtTimestamp;
    }

    if (nullptr != llTimeStamp)
    {
        *llTimeStamp = m_llLastTimeStamp;
    }

    if (nullptr != llTimeLength)
    {
        if (!m_paused)
        {
            *llTimeLength = OutputBufferStruct.rtTimelength;
        }
        else
        {
            *llTimeLength = 0;
        }
    }

    if (nullptr != beamAngle && !m_paused)
    {
        hr = m_pNuiAudioSource->GetBeam(beamAngle);
    }

    if (nullptr != sourceAngle && nullptr != sourceConfidence && !m_paused)
    {
        m_pNuiAudioSource->GetPosition(sourceAngle, sourceConfidence);
    }

    return hr;
}

HRESULT DataStreamAudio::SetBeam(double angle)
{
    if (nullptr == m_pNuiSensor)
    {
        Reset();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    AutoLock lock(m_nuiLock);

    HRESULT hr = S_OK;

    // if we haven't started the stream do it now
    if (!m_started)
    {
        hr = StartStream();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    return m_pNuiAudioSource->SetBeam(angle);
}

HRESULT DataStreamAudio::SetInputVolumeLevel(float fLevelDB)
{
    HRESULT hr = S_OK;

    // create the enumerator for WASAPI
    ComSmartPtr<IMMDeviceEnumerator> pDeviceEnum;
    // same as calling "CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<void**>(&pDeviceEnum));"
    hr = pDeviceEnum.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    if (FAILED(hr))
    {
        return hr;
    }

    // get the colloection of capture endpoints
    ComSmartPtr<IMMDeviceCollection> pCollection;
    hr = pDeviceEnum->EnumAudioEndpoints(eCapture, DEVICE_STATEMASK_ALL, &pCollection);
    if (FAILED(hr))
    {
        return hr;
    }

    pDeviceEnum.Release(); // done with the enumerator

    // find out how many devices are available
    UINT count;
    hr = pCollection->GetCount(&count);
    if (FAILED(hr))
    {
        return hr;
    }

    // not expected
    assert(count != 0);

    ComSmartPtr<IMMDevice> pEndpoint;
    ComSmartPtr<IPropertyStore> pProps;
    std::wstring name;
    bool bFound = false;

    // Each loop until we find the Kinect USB Audio device
    for (ULONG i = 0; i < count; ++i)
    {
        hr = pCollection->Item(i, &pEndpoint);
        if (FAILED(hr))
        {
            return hr;
        }

        // get the property store from the device
        hr = pEndpoint->OpenPropertyStore(STGM_READ, &pProps);
        if (FAILED(hr))
        {
            return hr;
        }

        // Initialize container for property value.
        PROPVARIANT varName;
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        if (FAILED(hr))
        {
            return hr;
        }

        // get the name from the property store
        name = varName.pwszVal;
        if (std::string::npos != name.find(L"Kinect"))
        {
			HRESULT hr = S_OK;
			ComSmartPtr<IAudioEndpointVolume> pEndptVol;
			hr = pEndpoint->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, reinterpret_cast<void**>(&pEndptVol) );
			if( SUCCEEDED(hr) )
			{
				hr = pEndptVol->SetMasterVolumeLevelScalar(fLevelDB, NULL);
			}
        }

        PropVariantClear(&varName);
        pProps.Release();
        pEndpoint.Release();
    }

    return hr;
}

#ifdef KCB_ENABLE_SPEECH
HRESULT DataStreamAudio::CreateSpeechRecognizer()
{
    ComSmartPtr<ISpObjectToken> pEngineToken;

    HRESULT hr = SpFindBestToken(SPCAT_RECOGNIZERS, GetLanguage(m_sLanguage).c_str(), NULL, &pEngineToken);
    if (SUCCEEDED(hr))
    {
		if( nullptr != m_pSpeechRecognizer )
		{
			m_pSpeechRecognizer.Release();
		}
        // same as calling "CoCreateInstance(CLSID_SpInprocRecognizer, NULL, CLSCTX_INPROC_SERVER, IID_ISpRecognizer, reinterpret_cast<void**>(&m_pSpeechRecognizer));"
		hr =  m_pSpeechRecognizer.CoCreateInstance(CLSID_SpInprocRecognizer);
	}

    if (SUCCEEDED(hr))
    {
        hr = m_pSpeechRecognizer->SetRecognizer(pEngineToken);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pSpeechRecognizer->SetRecoState(SPRST_INACTIVE);
	}

	// create the recognizer context
    if (SUCCEEDED(hr))
    {
		if( nullptr != m_pSpeechContext )
		{
			m_pSpeechContext.Release();
		}
        hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
	}

	// subscribe to speech events defined during init
	if (SUCCEEDED(hr))
	{
		hr = m_pSpeechContext->SetInterest(m_ullEventInterest, m_ullEventInterest);

		// establish the Win32 event
		if (SUCCEEDED(hr))
		{
			hr = m_pSpeechContext->SetNotifyWin32Event();
		}

		if (SUCCEEDED(hr))
		{
			m_hStreamHandle = m_pSpeechContext->GetNotifyEventHandle();

			if (INVALID_HANDLE_VALUE == m_hStreamHandle)
			{
				// Notification handle unsupported
				hr = SPERR_UNINITIALIZED;
			}
		}
    }

    if (SUCCEEDED(hr) && !m_bAdaptation)
    {
        SetAcousticAdaption(m_bAdaptation);
    }

    return hr;
}

std::wstring DataStreamAudio::GetLanguage(short langId)
{
    std::wstring wcReqAttrib = L"Language=";

    std::wstringstream ss;
    ss << std::hex << langId;

    wcReqAttrib.append(ss.str());

    wcReqAttrib.append(L";Kinect=True");

    return wcReqAttrib;
}

// For long recognition sessions (a few hours or more), it may be beneficial to turn off adaptation of the acoustic model. 
// This will prevent recognition accuracy from degrading over time.

// In addition, as the speech engine is run continuously, 
// its RAM requirement grows. We also recommend, for long recognition sessions, 
// that the SpeechRecognitionEngine be recycled (destroyed and recreated) periodically, 
// say every 2 minutes based on your resource constraints.

void DataStreamAudio::SetAcousticAdaption(bool bAdaptation)
{
    m_bAdaptation = bAdaptation;

    if (nullptr != m_pSpeechRecognizer)
    {
        m_pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", m_bAdaptation ? 1 : 0);
    }
}

// Applications should generally not set this property. 
// Indicates whether Windows Speech Recognition is 
// in listening mode or in passive nonlistening mode, 
// and persists the setting in the registry.
void DataStreamAudio::SetUXListening(bool bListening)
{
    if (nullptr != m_pSpeechRecognizer)
    {
        m_pSpeechRecognizer->SetPropertyNum(L"UXIsListening", bListening ? 1 : 0);
    }

    HRESULT hr = S_OK;
}

HRESULT DataStreamAudio::SetSpeechGrammar(LPCWSTR pszFileName)
{
    if (0 != m_wcGrammarFileName.compare(pszFileName))
    {
        m_wcGrammarFileName = pszFileName;
    }

    if (nullptr != m_pSpeechGrammar)
    {
        m_pSpeechGrammar.Release();
    }

    HRESULT hr = m_pSpeechContext->CreateGrammar(1, &m_pSpeechGrammar);
    if (SUCCEEDED(hr))
    {
        // Populate recognition grammar from file
        hr = m_pSpeechGrammar->LoadCmdFromFile(m_wcGrammarFileName.c_str(), SPLO_STATIC);
    }

    return hr;
}

HRESULT DataStreamAudio::GetSpeechEvent(_In_ SPEVENT* pSPEvent, _In_ ULONG* pulFetched)
{
    if (nullptr == pSPEvent || nullptr == pulFetched)
    {
        return E_INVALIDARG;
    }

    AutoLock lock(m_nuiLock);

    if (nullptr == m_pNuiSensor)
    {
        Reset();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    HRESULT hr = S_OK;

    // if we haven't started the stream do it now
    if (!m_started)
    {
        hr = StartStream();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    if (m_paused)
    {
        *pulFetched = 0;
        return hr;
    }

    if (nullptr != m_pSpeechContext)
    {
        hr = m_pSpeechContext->GetEvents(1, pSPEvent, pulFetched);
    }

    return hr;
}
#endif
