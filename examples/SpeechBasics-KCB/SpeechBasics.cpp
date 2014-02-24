//------------------------------------------------------------------------------
// <copyright file="SpeechBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "SpeechBasics.h"
#include "resource.h"
#include <sapi.h>

#define INITGUID
#include <guiddef.h>

// Static initializers
LPCWSTR CSpeechBasics::GrammarFileName = L"SpeechBasics-D2D.grxml";

// This is the class ID we expect for the Microsoft Speech recognizer.
// Other values indicate that we're using a version of sapi.h that is
// incompatible with this sample.
DEFINE_GUID(CLSID_ExpectedRecognizer, 0x495648e7, 0xf7ab, 0x4267, 0x8e, 0x0f, 0xca, 0xfb, 0x7a, 0x33, 0xc1, 0x60);

/// <summary>
/// Entry point for the application
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="hPrevInstance">always 0</param>
/// <param name="lpCmdLine">command line arguments</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
/// <returns>status</returns>
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (SUCCEEDED(hr))
    {
        {
            CSpeechBasics application;
            application.Run(hInstance, nCmdShow);
        }

        CoUninitialize();
    }

    return EXIT_SUCCESS;
}

/// <summary>
/// Constructor
/// </summary>
CSpeechBasics::CSpeechBasics() :
    m_pD2DFactory(NULL),
    m_pTurtleController(NULL),
    m_kcbHandle(KCB_INVALID_HANDLE)
{
}

/// <summary>
/// Destructor
/// </summary>
CSpeechBasics::~CSpeechBasics()
{
    // clean up Direct2D renderer
    delete m_pTurtleController;
    m_pTurtleController = NULL;

    // clean up Direct2D
    SafeRelease(m_pD2DFactory);

    // close KCB
    KinectCloseSensor(m_kcbHandle);
}

/// <summary>
/// Creates the main window and begins processing
/// </summary>
/// <param name="hInstance">handle to the application instance</param>
/// <param name="nCmdShow">whether to display minimized, maximized, or normally</param>
int CSpeechBasics::Run(HINSTANCE hInstance, int nCmdShow)
{
    MSG       msg = {0};
    WNDCLASS  wc;

    // Dialog custom window class
    ZeroMemory(&wc, sizeof(wc));
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.cbWndExtra    = DLGWINDOWEXTRA;
    wc.hInstance     = hInstance;
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hIcon         = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_APP));
    wc.lpfnWndProc   = DefDlgProcW;
    wc.lpszClassName = L"SpeechBasicsAppDlgWndClass";

    if (!RegisterClassW(&wc))
    {
        return 0;
    }

    // Create main application window
    HWND hWndApp = CreateDialogParamW(
        hInstance,
        MAKEINTRESOURCE(IDD_APP),
        NULL,
        (DLGPROC)CSpeechBasics::MessageRouter, 
        reinterpret_cast<LPARAM>(this));

    // Show window
    ShowWindow(hWndApp, nCmdShow);

    // Main message loop
    while (WM_QUIT != msg.message)
    {
     
        // Explicitly check for new speech recognition event
        if (KinectIsSpeechEventReady(m_kcbHandle))
        {
            ProcessSpeech();
        }

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // If a dialog message will be taken care of by the dialog proc
            if ((hWndApp != NULL) && IsDialogMessageW(hWndApp, &msg))
            {
                continue;
            }

            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    return static_cast<int>(msg.wParam);
}

/// <summary>
/// Handles window messages, passes most to the class instance to handle
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CSpeechBasics::MessageRouter(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CSpeechBasics* pThis = NULL;
    
    if (WM_INITDIALOG == uMsg)
    {
        pThis = reinterpret_cast<CSpeechBasics*>(lParam);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    }
    else
    {
        pThis = reinterpret_cast<CSpeechBasics*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
    }

    if (NULL != pThis)
    {
        return pThis->DlgProc(hWnd, uMsg, wParam, lParam);
    }

    return 0;
}

/// <summary>
/// Handle windows messages for the class instance
/// </summary>
/// <param name="hWnd">window message is for</param>
/// <param name="uMsg">message</param>
/// <param name="wParam">message data</param>
/// <param name="lParam">additional message data</param>
/// <returns>result of message processing</returns>
LRESULT CALLBACK CSpeechBasics::DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = FALSE;

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // Bind application window handle
            m_hWnd = hWnd;

            // Init Direct2D
            D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

            // Create and initialize a new Direct2D image renderer (take a look at ImageRenderer.h)
            // We'll use this to draw the data we receive from the Kinect to the screen
            m_pTurtleController = new TurtleController();
            HRESULT hr = m_pTurtleController->Initialize(GetDlgItem(m_hWnd, IDC_AUDIOVIEW), m_pD2DFactory);
            if (FAILED(hr))
            {
                SetStatusMessage(L"Failed to initialize the Direct2D draw device.");
                break;
            }

            // Look for a connected Kinect, and create it if found
            m_kcbHandle = KinectOpenDefaultSensor();
            if (KCB_INVALID_HANDLE == m_kcbHandle)
            {
                break;
            }

            KCB_SPEECH_LANGUAGE lang = KCBSpeechENUS;

            ULONGLONG ullInterest =
                SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_INTERFERENCE);

            KinectEnableSpeech(m_kcbHandle, GrammarFileName, &lang, &ullInterest, nullptr);
            if (KinectStreamStatusError == KinectGetSpeechStatus(m_kcbHandle))
            {
                break;
            }

            SetStatusMessage(L"Say: \"Forward\", \"Back\", \"Turn Left\" or \"Turn Right\"");

            result = FALSE;
            break;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hWnd, &ps);

            m_pTurtleController->Draw();

            EndPaint(hWnd, &ps);
            result = TRUE;
            break;
        }

        // If the titlebar X is clicked, destroy app
        case WM_CLOSE:
            KinectStopSpeech(m_kcbHandle);

            DestroyWindow(hWnd);
            result = TRUE;
            break;

        case WM_DESTROY:
            // Quit the main message pump
            PostQuitMessage(0);
            result = TRUE;
            break;
    }

    return result;
}

/// <summary>
/// Process recently triggered speech recognition events.
/// </summary>
void CSpeechBasics::ProcessSpeech()
{
    const float ConfidenceThreshold = 0.3f;

    SPEVENT curEvent;
    ULONG fetched = 0;
    HRESULT hr = S_OK;

    KinectGetSpeechEvent(m_kcbHandle, &curEvent, &fetched);

    while (fetched > 0)
    {
        switch (curEvent.eEventId)
        {
            case SPEI_RECOGNITION:
                if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType)
                {
                    // this is an ISpRecoResult
                    ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
                    SPPHRASE* pPhrase = NULL;
                    
                    hr = result->GetPhrase(&pPhrase);
                    if (SUCCEEDED(hr))
                    {
                        if ((pPhrase->pProperties != NULL) && (pPhrase->pProperties->pFirstChild != NULL))
                        {
                            const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
                            if (pSemanticTag->SREngineConfidence > ConfidenceThreshold)
                            {
                                TurtleAction action = MapSpeechTagToAction(pSemanticTag->pszValue);
                                m_pTurtleController->DoAction(action);
                            }
                        }
                        ::CoTaskMemFree(pPhrase);
                    }
                }
                break;
            case SPEI_INTERFERENCE:
                float fVolumeDB = .25f;
                hr = KinectSetInputVolumeLevel(m_kcbHandle, fVolumeDB);
                break;
        }
        KinectGetSpeechEvent(m_kcbHandle, &curEvent, &fetched);
    }

    return;
}

/// <summary>
/// Maps a specified speech semantic tag to the corresponding action to be performed on turtle.
/// </summary>
/// <returns>
/// Action that matches <paramref name="pszSpeechTag"/>, or TurtleActionNone if no matches were found.
/// </returns>
TurtleAction CSpeechBasics::MapSpeechTagToAction(LPCWSTR pszSpeechTag)
{
    struct SpeechTagToAction
    {
        LPCWSTR pszSpeechTag;
        TurtleAction action;
    };
    const SpeechTagToAction Map[] =
    {
        {L"FORWARD", TurtleActionForward},
        {L"BACKWARD", TurtleActionBackward},
        {L"LEFT", TurtleActionTurnLeft},
        {L"RIGHT", TurtleActionTurnRight}
    };

    TurtleAction action = TurtleActionNone;

    for (int i = 0; i < _countof(Map); ++i)
    {
        if (0 == wcscmp(Map[i].pszSpeechTag, pszSpeechTag))
        {
            action = Map[i].action;
            break;
        }
    }

    return action;
}

/// <summary>
/// Set the status bar message
/// </summary>
/// <param name="szMessage">message to display</param>
void CSpeechBasics::SetStatusMessage(const WCHAR* szMessage)
{
    SendDlgItemMessageW(m_hWnd, IDC_STATUS, WM_SETTEXT, 0, (LPARAM)szMessage);
}