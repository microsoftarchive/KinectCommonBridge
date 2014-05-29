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

#include "DataStreamColor.h"
#include "DataStreamDepth.h"
#include "DataStreamSkeleton.h"
#include "DataStreamAudio.h"
#include "CoordinateMapper.h"

class FaceTracker;

class KinectSensor
{
public:
    KinectSensor( _In_z_ const WCHAR* wcPortID );
    virtual ~KinectSensor();

public:
    // control for the streams
    HRESULT Open();
    void Close();

    // enabling streams with full parameters exposed
    void EnableColorStream( NUI_IMAGE_TYPE type, NUI_IMAGE_RESOLUTION resolution );
    void EnableDepthStream( bool nearModeOn, NUI_IMAGE_RESOLUTION resolution );
    void EnableSkeletonStream( bool bSeatedSkeletons, KINECT_SKELETON_SELECTION_MODE mode, _Inout_opt_ NUI_TRANSFORM_SMOOTH_PARAMETERS *pSmoothParams );

    // start stream
    HRESULT StartStreams();
    HRESULT StartColorStream();
    HRESULT StartDepthStream();
    HRESULT StartSkeletonStream();

    // pause stream
    void PauseStreams( bool bPause );
    void PauseColorStream( bool bPause );
    void PauseDepthStream( bool bPause );
    void PauseSkeletonStream( bool bPause );

    // stop stream
    void StopStreams();
    void StopColorStream();
    void StopDepthStream();
    void StopSkeletonStream();

    // get stream status
    KINECT_STREAM_STATUS GetColorStreamStatus();
    KINECT_STREAM_STATUS GetDepthStreamStatus();
    KINECT_STREAM_STATUS GetSkeletonStreamStatus();

    // are the frames ready
    bool AllFramesReady();
    bool AnyFrameReady();
    bool ColorFrameReady();
    bool DepthFrameReady();
    bool SkeletonFrameReady();

    // get frame data format
    void GetColorFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    void GetDepthFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );

    // get the frame data
    HRESULT GetColorFrame( ULONG cBufferSize, _Inout_cap_(cBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp );
    HRESULT GetDepthFrame( ULONG cBufferSize, _Inout_cap_(cBufferSize) BYTE* pDepthBuffer, _Out_opt_ LONGLONG* liTimeStamp );
    HRESULT GetSkeletonFrame( _Inout_ NUI_SKELETON_FRAME& skeletonFrame );
    HRESULT GetDepthPixels( ULONG cDepthPixels, _Inout_cap_(cDepthPixels) NUI_DEPTH_IMAGE_PIXEL* pDepthPixels, _Out_opt_ LONGLONG* liTimeStamp );
    HRESULT GetColorFrameFromDepthPoints(
        DWORD cDepthPoints, _In_count_(cDepthPoints) NUI_DEPTH_IMAGE_POINT *pDepthPoints,
        ULONG cBufferSize, _Inout_cap_(cBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp);

    // audio/speech stream
    void EnableAudioStream(_In_opt_ AEC_SYSTEM_MODE* eAECSystemMode, _In_opt_ bool* bGainBounder);
    HRESULT StartAudioStream();
    void PauseAudioStream(bool bPause);
    void StopAudioStream();

    KINECT_STREAM_STATUS GetAudioStreamStatus();
    HRESULT GetAudioSample(
        _Out_ DWORD* cbProduced, _Out_ BYTE** ppbOutputBuffer,
        _Out_ DWORD* dwStatus, _Out_opt_ LONGLONG *llTimeStamp, _Out_opt_ LONGLONG *llTimeLength,
        _Out_opt_ double *beamAngle, _Out_opt_ double *sourceAngle, _Out_opt_ double *sourceConfidence );
    
    HRESULT SetInputVolumeLevel(float fLevelDB);

#ifdef KCB_ENABLE_SPEECH
	void EnableSpeech(_In_ const WCHAR* wcGrammarFileName, _In_opt_ KCB_SPEECH_LANGUAGE* sLanguage, _In_opt_ ULONGLONG* ullEventInterest, _In_opt_ bool* bAdaptation);
    HRESULT StartSpeech();
    bool SpeechEventReady();
    HRESULT GetSpeechEvent( _In_ SPEVENT* pSPEvent, _In_ ULONG* pulFetched );
#endif

#ifdef KCB_ENABLE_FT
    HRESULT EnableFaceTracking(bool bNearMode = false);
    void DisableFaceTracking();
    bool GetColorStreamCameraConfig(_Out_ FT_CAMERA_CONFIG& config) const;
    bool GetDepthStreamCameraConfig(_Out_ FT_CAMERA_CONFIG& config) const;

    HRESULT GetFaceTrackingResult( _Out_ IFTResult** ppResult );
	float GetXCenterFace();
	float GetYCenterFace();
    HRESULT GetFaceTrackingImage(IFTImage** pImage);
	HRESULT GetFaceTracker( IFTFaceTracker** pFaceTracker);

#endif

public: // getters
    const WCHAR* GetPortID() const { return m_wsPortID.c_str(); }
    KINECT_SENSOR_STATUS GetKinectSensorStatus() { return m_eStatus; }
    CoordinateMapper& GetCoordinateMapper() { return *m_pCoordinateMapper.get(); }

protected:
    // for sensor interaction with the object
    friend class SensorManager;

    // handle sensor notification updates that come from the manager
    void NuiStatusNotification( _In_z_ const WCHAR* wcPortID, HRESULT hrStatus );
    bool IsStarted();

private:
    // internal for setting up the device
    // and select the default enabled streams
    HRESULT SelectSensor( bool bSelected );
    HRESULT UpdateSensor();

    // helper function for creating the sensor and attaching to instance
    HRESULT CreateNuiDevice();
    void ResetDevice();

    HRESULT GetNUISensorStatus( bool bCheckInUse = false );

    // enable/create for the default stream
    void EnableColorStream();
    void EnableDepthStream();
    void EnableSkeletonStream();
    void EnableAudioStream();
    void EnableSpeech();

    // populate the list of events
    void GetWaitEvents( _Inout_ std::vector<HANDLE>& events );

    static bool IsSensorConflict( _In_ INuiSensor* pNuiSensor );
    static void NuiSensorStatus( _In_ INuiSensor* pNuiSensor, _Out_ HRESULT& hr, _Out_ KINECT_SENSOR_STATUS& curStatus, bool bCheckConflict = false );

private:
    CriticalSection     m_nuiLock;
    std::wstring        m_wsPortID;
    ComSmartPtr<INuiSensor> m_pNuiSensor;

    bool    m_bSelected;
    bool    m_bInitialized;

    HRESULT                 m_hrLast;
    KINECT_SENSOR_STATUS    m_eStatus;

    std::unique_ptr<DataStreamColor>    m_pColorStream;
    std::unique_ptr<DataStreamDepth>    m_pDepthStream;
    std::unique_ptr<DataStreamSkeleton> m_pSkeletonStream;
    std::unique_ptr<DataStreamAudio>    m_pAudioStream;

    std::unique_ptr<CoordinateMapper>   m_pCoordinateMapper;
#ifdef KCB_ENABLE_FT
    std::unique_ptr<FaceTracker>        m_pFaceTracker;
#endif
};
