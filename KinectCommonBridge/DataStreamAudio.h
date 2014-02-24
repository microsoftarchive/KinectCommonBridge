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

#include "DataStream.h"
#include "MediaBuffer.h"
#include "KinectAudioStream.h"

class DataStreamAudio :
    public DataStream
{
public:
    DataStreamAudio();
    virtual ~DataStreamAudio();

    // can enable the stream with just a valid sensor
    virtual void Initialize(_In_ INuiSensor* pNuiSensor);
    virtual void Initialize(_In_opt_ AEC_SYSTEM_MODE* eAECSystemMode, _In_opt_ bool* bGainBounder, _In_ INuiSensor* pNuiSensor);

    // start stop the INuiSensor 
    virtual HRESULT StartStream();
    virtual void StopStream();
	virtual void PauseStream(bool bPaused);

    HRESULT GetSample( 
        _Out_ DWORD* cbProduced, BYTE** ppbOutputBuffer,
        _Out_ DWORD* dwStatus, _Out_opt_ LONGLONG *llTimeStamp, _Out_opt_ LONGLONG *llTimeLength,
        _Out_opt_ double *beamAngle, _Out_opt_ double *sourceAngle, _Out_opt_ double *sourceConfidence );

    virtual HANDLE GetFrameReadyEvent();

    HRESULT SetInputVolumeLevel(float fLevelDB);

#ifdef KCB_ENABLE_SPEECH
	virtual void Initialize(_In_ const WCHAR* wcGrammarFileName, _In_opt_ KCB_SPEECH_LANGUAGE* sLanguage, _In_opt_ ULONGLONG* ullEventInterest, _In_opt_ bool* bAdaptation);
    HRESULT StartSpeech();
    HRESULT GetSpeechEvent( _In_ SPEVENT* pSPEvent, _In_ ULONG* pulFetched );
    bool IsSpeechEnabled() { return (0 != m_wcGrammarFileName.size());  }

    void SetUXListening(bool bListening);
    HRESULT SetSpeechGrammar(LPCWSTR pszFileName);
#endif

private:
    virtual void RemoveDevice();
    virtual void CopyData(_In_ void* pImageFrame);
    void Reset();

    virtual HRESULT OpenStream();

    HRESULT SetBeam( double angle );

#ifdef KCB_ENABLE_SPEECH
    void ResetSpeech();
    HRESULT CreateSpeechRecognizer();
    static std::wstring GetLanguage(short langID);
    void SetAcousticAdaption(bool bAdaptionOn);
#endif

private:
    // Audio variables and interfaces
    AEC_SYSTEM_MODE		m_eAECSystemMode;
	bool				m_bOverrideFeatureMode;
	LONG				m_lAES;
	bool				m_bGainBounder;
    CComPtr<INuiAudioBeam>      m_pNuiAudioSource;

    CComPtr<IMediaBuffer>       m_pOutputBuffer;
    LONGLONG                    m_llLastTimeStamp;

    CComPtr<KinectAudioStream>  m_pKinectAudioStream;

    // Speech variables and interfaces
#ifdef KCB_ENABLE_SPEECH
    bool                        m_bAdaptation;
    std::wstring                m_wcGrammarFileName;
    short                       m_sLanguage;
    ULONGLONG                   m_ullEventInterest;

    CComPtr<ISpStream>          m_pSpeechStream;
    CComPtr<ISpRecognizer>      m_pSpeechRecognizer;
    CComPtr<ISpRecoContext>     m_pSpeechContext;
    CComPtr<ISpRecoGrammar>     m_pSpeechGrammar;
#endif
};
