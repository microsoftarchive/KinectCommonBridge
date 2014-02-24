//------------------------------------------------------------------------------
// <copyright file="AudioBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "AudioPanel.h"
#include "resource.h"
#include "KinectCommonBridgeLib.h"

// For IMediaObject and related interfaces
#include <dmo.h>

//// For configuring DMO properties
//#include <wmcodecdsp.h>
//
//// For WAVEFORMATEX
//#include <mmreg.h>
//
//// For FORMAT_WaveFormatEx and such
//#include <uuids.h>
//
//// For Kinect SDK APIs
//#include <NuiApi.h>
//
//
//// Format of Kinect audio stream
//static const WORD       AudioFormat = WAVE_FORMAT_PCM;
//
//// Number of channels in Kinect audio stream
//static const WORD       AudioChannels = 1;
//
// Samples per second in Kinect audio stream
static const DWORD      AudioSamplesPerSecond = 16000;

//// Average bytes per second in Kinect audio stream
//static const DWORD      AudioAverageBytesPerSecond = 32000;
//
//// Block alignment in Kinect audio stream
//static const WORD       AudioBlockAlign = 2;
//
//// Bits per audio sample in Kinect audio stream
//static const WORD       AudioBitsPerSample = 16;
//
///// <summary>
///// IMediaBuffer implementation for a statically allocated buffer.
///// </summary>
//class CStaticMediaBuffer : public IMediaBuffer
//{
//public:
//    // Constructor
//    CStaticMediaBuffer() : m_dataLength(0) {}
//
//    // IUnknown methods
//    STDMETHODIMP_(ULONG) AddRef() { return 2; }
//    STDMETHODIMP_(ULONG) Release() { return 1; }
//    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
//    {
//        if (riid == IID_IUnknown)
//        {
//            AddRef();
//            *ppv = (IUnknown*)this;
//            return NOERROR;
//        }
//        else if (riid == IID_IMediaBuffer)
//        {
//            AddRef();
//            *ppv = (IMediaBuffer*)this;
//            return NOERROR;
//        }
//        else
//        {
//            return E_NOINTERFACE;
//        }
//    }
//
//    // IMediaBuffer methods
//    STDMETHODIMP SetLength(DWORD length) {m_dataLength = length; return NOERROR;}
//    STDMETHODIMP GetMaxLength(DWORD *pMaxLength) {*pMaxLength = sizeof(m_pData); return NOERROR;}
//    STDMETHODIMP GetBufferAndLength(BYTE **ppBuffer, DWORD *pLength)
//    {
//        if (ppBuffer)
//        {
//            *ppBuffer = m_pData;
//        }
//        if (pLength)
//        {
//            *pLength = m_dataLength;
//        }
//        return NOERROR;
//    }
//    void Init(ULONG ulData)
//    {
//        m_dataLength = ulData;
//    }
//
//protected:
//    // Statically allocated buffer used to hold audio data returned by IMediaObject
//    BYTE m_pData[AudioSamplesPerSecond * AudioBlockAlign];
//
//    // Amount of data currently being held in m_pData
//    ULONG m_dataLength;
//};

/// <summary>
/// Main application class for AudioBasics sample.
/// </summary>
class CAudioBasics
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    CAudioBasics();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CAudioBasics();

    /// <summary>
    /// Handles window messages, passes most to the class instance to handle
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    static LRESULT CALLBACK MessageRouter(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Handle windows messages for a class instance
    /// </summary>
    /// <param name="hWnd">window message is for</param>
    /// <param name="uMsg">message</param>
    /// <param name="wParam">message data</param>
    /// <param name="lParam">additional message data</param>
    /// <returns>result of message processing</returns>
    LRESULT CALLBACK        DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance">handle to the application instance</param>
    /// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
    int                     Run(HINSTANCE hInstance, int nCmdShow);

private:
    // ID of timer that drives audio capture.
    static const int        iAudioReadTimerId = 1;

    // Time interval, in milliseconds, for timer that drives audio capture.
    static const int        iAudioReadTimerInterval = 50;

    // ID of timer that drives energy stream display.
    static const int        iEnergyRefreshTimerId = 2;

    // Time interval, in milliseconds, for timer that drives energy stream display.
    static const int        iEnergyRefreshTimerInterval = 10;

    // Number of audio samples captured from Kinect audio stream accumulated into a single
    // energy measurement that will get displayed.
    static const int        iAudioSamplesPerEnergySample = 40;

    // Number of energy samples that will be visible in display at any given time.
    static const int        iEnergySamplesToDisplay = 780;

    // Number of energy samples that will be stored in the circular buffer.
    // Always keep it higher than the energy display length to avoid overflow.
    static const int        iEnergyBufferLength = 1000;

    // Main application dialog window.
    HWND                    m_hWnd;

    // Factory used to create Direct2D objects.
    ID2D1Factory*           m_pD2DFactory;

    // Object that controls displaying Kinect audio data.
    AudioPanel*             m_pAudioPanel;    

    //// Current Kinect sensor.
    //INuiSensor*             m_pNuiSensor;

    //// Audio source used to query Kinect audio beam and sound source angles.
    //INuiAudioBeam*          m_pNuiAudioSource;

    //// Media object from which Kinect audio stream is captured.
    //IMediaObject*           m_pDMO;

    //// Property store used to configure Kinect audio properties.
    //IPropertyStore*         m_pPropertyStore;

    //// Buffer to hold captured audio data.
    //CStaticMediaBuffer      m_csmCaptureBuffer;

    // Buffer used to store audio stream energy data as we read audio.
    float                   m_rgfltEnergyBuffer[iEnergyBufferLength];

    // Buffer used to store audio stream energy data ready to be displayed.
    float                   m_rgfltEnergyDisplayBuffer[iEnergySamplesToDisplay];

    // Sum of squares of audio samples being accumulated to compute the next energy value.
    float                   m_fltAccumulatedSquareSum;

    // Error between time slice we wanted to display and time slice that we ended up
    // displaying, given that we have to display in integer pixels.
    float					m_fltEnergyError;

    // Number of audio samples accumulated so far to compute the next energy value.
    int                     m_iAccumulatedSampleCount;

    // Index of next element available in audio energy buffer.
    int                     m_iEnergyIndex;

    // Number of newly calculated audio stream energy values that have not yet been displayed.
    int                     m_iNewEnergyAvailable;

    // Index of first energy element that has never (yet) been displayed to screen.
    int                     m_iEnergyRefreshIndex;

    // Last time energy visualization was rendered to screen.
    DWORD                   m_dwLastEnergyRefreshTime;        

    // handle to KCB
    KCBHANDLE               m_kcbHandle;

    ///// <summary>
    ///// Create the first connected Kinect found.
    ///// </summary>
    ///// <returns>S_OK on success, otherwise failure code.</returns>
    //HRESULT                 CreateFirstConnected();

    /// <summary>
    /// Initialize Kinect audio capture/control objects.
    /// </summary>
    /// <returns>
    /// <para>S_OK on success, otherwise failure code.</para>
    /// </returns>
    HRESULT                 InitializeAudioSource();

    /// <summary>
    /// Capture new audio data.
    /// </summary>
    void                    ProcessAudio();

    /// <summary>
    /// Display latest audio data.
    /// </summary>
    void                    Update();

    /// <summary>
    /// Set the status bar message.
    /// </summary>
    /// <param name="szMessage">message to display.</param>
    void                    SetStatusMessage(WCHAR* szMessage);
};
