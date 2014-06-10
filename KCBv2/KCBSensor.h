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

#include "AutoLock.h"
#include <vector>

// {45B3CDFF-CC72-4CBF-A626-D22B0401E8D7}
interface DECLSPEC_UUID("45B3CDFF-CC72-4CBF-A626-D22B0401E8D7") DECLSPEC_NOVTABLE IKCBSensor 
    : public IUnknown 
{
public:
    virtual HRESULT STDMETHODCALLTYPE GetKinectInterface(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject) = 0;
    //virtual HRESULT STDMETHODCALLTYPE GetStatus(_Out_ KinectStatus* pStatus) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE GetSource(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE OpenReader(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject) = 0;
    virtual HRESULT STDMETHODCALLTYPE OpenMultiSourceReader(DWORD eFrameSourceTypes, _COM_Outptr_opt_result_maybenull_ IMultiSourceFrameReader** ppvObject) = 0;
    
    virtual HRESULT STDMETHODCALLTYPE CreateColorFrameDescription(ColorImageFormat eFormat, _COM_Outptr_result_maybenull_ IFrameDescription** ppvObject) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetFrame(REFIID riid, _COM_Outptr_result_maybenull_ void** ppvObject) = 0;

    virtual HRESULT STDMETHODCALLTYPE GetAudioBuffer(ULONG cb, _Out_writes_bytes_to_(cb, *pcbRead) byte *cbBuffer, _Out_opt_ ULONG *pcbRead, _Out_opt_ FLOAT* beamAngle, _Out_opt_ FLOAT* sourceConfidence);

    virtual bool STDMETHODCALLTYPE IsFrameReady(KCBSourceType eSource) = 0;
    virtual bool STDMETHODCALLTYPE AnyFrameReady() = 0;
    virtual bool STDMETHODCALLTYPE AllFramesReady() = 0;
    virtual bool STDMETHODCALLTYPE MultiFrameReady() = 0;
};


class KCBSensor
    : public IKCBSensor
{
public:
    static HRESULT CreateInstance(_In_ IKinectSensor* pSensor, _COM_Outptr_result_maybenull_ IKCBSensor **ppKCBSensor);

    // IUnknown
    IFACEMETHOD(QueryInterface) (REFIID riid, _COM_Outptr_ void** ppvObject);
    IFACEMETHOD_(ULONG, AddRef) ();
    IFACEMETHOD_(ULONG, Release) ();

    // IKCBSensor
    IFACEMETHOD(GetKinectInterface)(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject);
    //IFACEMETHOD(GetStatus)(_Out_ KinectStatus* pStatus);
    
    IFACEMETHOD(GetSource)(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject);
    
    IFACEMETHOD(OpenReader)(REFIID riid, _COM_Outptr_opt_result_maybenull_ void** ppvObject);
    IFACEMETHOD(OpenMultiSourceReader)(DWORD eFrameSourceTypes, _COM_Outptr_opt_result_maybenull_ IMultiSourceFrameReader** ppvObject);
    
    IFACEMETHOD(CreateColorFrameDescription)(ColorImageFormat eFormat, _COM_Outptr_result_maybenull_ IFrameDescription** ppvObject);
    IFACEMETHOD(GetFrame)(REFIID riid, _COM_Outptr_result_maybenull_ void** ppvObject);

    IFACEMETHOD(GetAudioBuffer)(ULONG cb, _Out_writes_bytes_to_(cb, *pcbRead) byte *cbBuffer, _Inout_ ULONG *pcbRead, _Out_opt_ FLOAT* beamAngle, _Out_opt_ FLOAT* sourceConfidence);


    virtual bool STDMETHODCALLTYPE IsFrameReady(KCBSourceType eSource);
    virtual bool STDMETHODCALLTYPE AnyFrameReady();
    virtual bool STDMETHODCALLTYPE AllFramesReady();
    virtual bool STDMETHODCALLTYPE MultiFrameReady();

private:
    KCBSensor();
    ~KCBSensor();

    HRESULT Initialize(_In_ IKinectSensor* pSensor);
    void Shutdown();

    void KCBSensor::GetWaitEvents(_Inout_ std::vector<HANDLE>& events);

private:
    volatile long m_refCount;
    CriticalSection m_lock;

    // list of Interfaces
    IKinectSensor*              m_pKinectSensor;

    // Multi-Source frame reader
    DWORD                       m_eFrameSourceTypes;
    IMultiSourceFrameReader*    m_pMultiSourceFrameReader;

    // coordinate KCBMapper
    ICoordinateMapper*          m_pCoordinateMapper;

    // audio
    IAudioSource*               m_pAudioSource;
    IAudioBeam*                 m_pAudioBeam;
    IStream*                    m_pAudioStream;

    // body
    IBodyFrameSource*           m_pBodyFrameSource;
    IBodyFrameReader*           m_pBodyFrameReader;

    // body index
    IBodyIndexFrameSource*      m_pBodyIndexFrameSource;
    IBodyIndexFrameReader*      m_pBodyIndexFrameReader;

    // color
    IColorFrameSource*          m_pColorFrameSource;
    IFrameDescription*          m_pColorFrameDescription;
    IColorFrameReader*          m_pColorFrameReader;

    // depth
    IDepthFrameSource*          m_pDepthFrameSource;
    IDepthFrameReader*          m_pDepthFrameReader;

    // infrared
    IInfraredFrameSource*       m_pInfraredFrameSource;
    IInfraredFrameReader*       m_pInfraredFrameReader;

    // long ir
    ILongExposureInfraredFrameSource*   m_pLongExposureInfraredFrameSource;
    ILongExposureInfraredFrameReader*   m_pLongExposureInfraredFrameReader;
    
    // handles for events
    WAITABLE_HANDLE m_hFrameEvents[KCBSourceType_Count];

    WAITABLE_HANDLE m_hMultiSourceFrameEvent;
};
