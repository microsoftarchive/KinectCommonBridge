//------------------------------------------------------------------------------
// <copyright file="SpeechBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "TurtleController.h"
#include "resource.h"

#include "KinectCommonBridgeLib.h"

// For configuring DMO properties
#include <wmcodecdsp.h>

/// <summary>
/// Main application class for SpeechBasics sample.
/// </summary>
class CSpeechBasics
{
public:
    /// <summary>
    /// Constructor
    /// </summary>
    CSpeechBasics();

    /// <summary>
    /// Destructor
    /// </summary>
    ~CSpeechBasics();

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
    static LPCWSTR          GrammarFileName;
    
    // Main application dialog window
    HWND                    m_hWnd;

    // Factory used to create Direct2D objects
    ID2D1Factory*           m_pD2DFactory;

    // Object that controls moving turtle around and displaying it
    TurtleController*       m_pTurtleController;

    // handle to KCB
    KCBHANDLE               m_kcbHandle;

    /// <summary>
    /// Process recently triggered speech recognition events.
    /// </summary>
    void                    ProcessSpeech();

    /// <summary>
    /// Maps a specified speech semantic tag to the corresponding action to be performed on turtle.
    /// </summary>
    /// <returns>
    /// Action that matches <paramref name="pszSpeechTag"/>, or TurtleActionNone if no matches were found.
    /// </returns>
    TurtleAction            MapSpeechTagToAction(LPCWSTR pszSpeechTag);

    /// <summary>
    /// Set the status bar message.
    /// </summary>
    /// <param name="szMessage">message to display.</param>
    void                    SetStatusMessage(const WCHAR* szMessage);
};
