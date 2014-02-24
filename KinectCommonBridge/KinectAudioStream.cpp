//------------------------------------------------------------------------------
// <copyright file="KinectAudioStream.cpp" company="Microsoft">
//      
//     Copyright 2013 Microsoft Corporation 
//      
//    Licensed under the Apache License, Version 2.0 (the "License"); 
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//      
//         http://www.apache.org/licenses/LICENSE-2.0 
//      
//    Unless required by applicable law or agreed to in writing, software 
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
//    See the License for the specific language governing permissions and 
//    limitations under the License. 
//      
// </copyright>
// <summary>
//   Implementation for KinectAudioStream methods.
//   KinectAudioStream wraps Kinect audio DMO and captures audio in a dedicated thread
//   while clients are allowed to read data from any other thread.
// </summary>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "KinectAudioStream.h"

#include "KinectCommonBridgeLib.h"
#include <stdio.h>

#pragma comment (lib, "Avrt.lib")

/// <summary>
/// KinectAudioStream constructor.
/// </summary>
KinectAudioStream::KinectAudioStream(IMediaObject *pKinectDmo) 
    : m_cRef(1)
    , m_pKinectDmo(pKinectDmo) // assigment for CComPtr AddRefs
    , m_CurrentWriteBuffer(NULL)
    , m_CurrentReadBuffer(NULL)
    , m_CurrentReadBufferIndex(0)
    , m_BytesRead(0)
    , m_hStopEvent(NULL)
    , m_hDataReady(NULL)
    , m_hCaptureThread(NULL)
{
    InitializeCriticalSection(&m_Lock);
}

/// <summary>
/// KinectAudioStream destructor.
/// </summary>
KinectAudioStream::~KinectAudioStream()
{
    m_pKinectDmo->Release();
    DeleteCriticalSection(&m_Lock);
}

/// <summary>
/// Starts capturing audio data from Kinect sensor.
/// </summary>
/// <returns>S_OK on success, otherwise failure code.</returns>
HRESULT KinectAudioStream::StartCapture()
{
    HRESULT hr = S_OK;

    m_hStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hDataReady = CreateEvent( NULL, FALSE, FALSE, NULL );
    m_BytesRead = 0;

    // create pool of result buffers
    for (UINT i = 0; i < NumBuffers; i++)
    {
        IMediaBuffer* pBuf = nullptr;
        hr = MediaBuffer::Create( KINECT_WAVEFORMATEX, &pBuf );
        if(FAILED(hr))
        {
            return hr;
        }
        m_BufferPool.push(pBuf);
    }

    m_CurrentWriteBuffer = NULL;

    m_hCaptureThread = CreateThread( NULL, 0, CaptureThread, this, 0, NULL );

    return hr;
}

/// <summary>
/// Starts capturing audio data from Kinect sensor.
/// </summary>
/// <returns>S_OK on success, otherwise failure code.</returns>
HRESULT KinectAudioStream::StopCapture()
{
    HRESULT hr = S_OK;
    if ( NULL != m_hStopEvent )
    {
        // Signal the thread
        SetEvent(m_hStopEvent);

        // Wait for thread to stop
        if ( NULL != m_hCaptureThread )
        {
            WaitForSingleObject( m_hCaptureThread, INFINITE );
            CloseHandle( m_hCaptureThread );
            m_hCaptureThread = NULL;
        }
        CloseHandle( m_hStopEvent );
        m_hStopEvent = NULL;
    }

    if (NULL != m_hDataReady)
    {
        SetEvent(m_hDataReady);
        CloseHandle(m_hDataReady);
        m_hDataReady = NULL;
    }

    // Release all buffers to buffer pool and then free all buffers in pool
    ReleaseAllBuffers();
    while (!m_BufferPool.empty())
    {
        IMediaBuffer* mediaBuffer = m_BufferPool.top();
        mediaBuffer->Release();
        m_BufferPool.pop();
    }

    return hr;
}

/////////////////////////////////////////////
// IStream methods
STDMETHODIMP KinectAudioStream::Read(void *pBuffer, ULONG cbBuffer, ULONG *pcbRead)
{
    HRESULT hr = S_OK;

   if (pcbRead == NULL)
   {
       return E_INVALIDARG;
   }

    ULONG bytesPendingToRead = cbBuffer;
    while (bytesPendingToRead > 0 && IsCapturing())
    {
        ReadOneBuffer((BYTE**)&pBuffer, &bytesPendingToRead);

        if (NULL == m_CurrentReadBuffer) //no data, wait ...
        {
            WaitForSingleObject(m_hDataReady, INFINITE);
        }
    }
    ULONG bytesRead = cbBuffer - bytesPendingToRead;
    m_BytesRead += bytesRead;

    *pcbRead = bytesRead;

    return hr;
}

STDMETHODIMP KinectAudioStream::Write(const void *,ULONG,ULONG *)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::Seek(LARGE_INTEGER dlibMove,DWORD dwOrigin, ULARGE_INTEGER *plibNewPosition )   
{
    if (plibNewPosition != NULL)
    {
        plibNewPosition->QuadPart = m_BytesRead + dlibMove.QuadPart;
    }
    return S_OK;
}

STDMETHODIMP KinectAudioStream::SetSize(ULARGE_INTEGER)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::CopyTo(IStream *,ULARGE_INTEGER,ULARGE_INTEGER *,ULARGE_INTEGER *)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::Commit(DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::Revert()
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::LockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::UnlockRegion(ULARGE_INTEGER,ULARGE_INTEGER,DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::Stat(STATSTG *,DWORD)
{
    return E_NOTIMPL;
}

STDMETHODIMP KinectAudioStream::Clone(IStream **)
{
    return E_NOTIMPL;
}

/////////////////////////////////////////////
// Private KinectAudioStream methods

/// <summary>
/// Get the next buffer available for writing audio data.
/// First attempts to find a free buffer in buffer pool and, if none is available, grabs the
/// oldest (most stale) buffer from circular queue of buffers ready for reading and re-uses it.
/// </summary>
/// <returns>Media buffer ready for audio capture, or NULL if none was found.</returns>
IMediaBuffer *KinectAudioStream::GetWriteBuffer()
{
    IMediaBuffer *pBuf = NULL;

    EnterCriticalSection(&m_Lock);

    //Get a free buffer if available. Otherwise, get the oldest buffer
    //from the read queue. This is a way of overwriting the oldest data
    if (m_BufferPool.size() > 0)
    {   
        pBuf = m_BufferPool.top();
        m_BufferPool.pop();
        pBuf->SetLength(0);
    }                           
    else if (m_ReadBufferQueue.size() > 0)
    {
        pBuf = m_ReadBufferQueue.front();
        m_ReadBufferQueue.pop();
        pBuf->SetLength(0);
    }

    LeaveCriticalSection(&m_Lock);

    return pBuf;
}


/// <summary>
/// Release an audio buffer back into buffer pool.
/// </summary>
/// <param name="pBuffer">Buffer to be released.</param>
void KinectAudioStream::ReleaseBuffer(IMediaBuffer* pBuffer)
{
    if (pBuffer != NULL)
    {
        EnterCriticalSection(&m_Lock);
        pBuffer->SetLength(0);
        m_BufferPool.push(pBuffer);
        LeaveCriticalSection(&m_Lock);
    }
}

/// <summary>
/// Release all audio buffers back into buffer pool.
/// </summary>
void KinectAudioStream::ReleaseAllBuffers()
{
    EnterCriticalSection(&m_Lock);
    while (m_ReadBufferQueue.size() > 0)
    {
        IMediaBuffer *pBuf = m_ReadBufferQueue.front();
        m_ReadBufferQueue.pop();
        ReleaseBuffer(pBuf);
    }
    if (m_CurrentReadBuffer != NULL)
    {
        ReleaseBuffer(m_CurrentReadBuffer);
    }

    m_CurrentReadBufferIndex = 0;
    m_CurrentReadBuffer = NULL;
    LeaveCriticalSection(&m_Lock);
}

/// <summary>
/// Add captured audio data to the circular queue of buffers ready for client reading.
/// </summary>
/// <param name="pData">Pointer to audio data to be added to queue.</param>
/// <param name="cbData">Number of bytes to be added to queue.</param>
void KinectAudioStream::QueueCapturedData(BYTE *pData, UINT cbData)
{
    BYTE *pWriteData = NULL;
    DWORD cbWriteData = 0;
    DWORD cbMaxLength = 0;

    if (cbData <= 0)
    {
        return;
    }

    if (NULL == m_CurrentWriteBuffer)
    {
        m_CurrentWriteBuffer = GetWriteBuffer();
    }

    m_CurrentWriteBuffer->GetBufferAndLength(&pWriteData, &cbWriteData);
    m_CurrentWriteBuffer->GetMaxLength(&cbMaxLength);

    if (cbWriteData + cbData < cbMaxLength)
    {
        memcpy(pWriteData + cbWriteData, pData, cbData);
        m_CurrentWriteBuffer->SetLength(cbWriteData + cbData);
    }
    else
    {
        QueueCapturedBuffer(m_CurrentWriteBuffer);

        m_CurrentWriteBuffer = GetWriteBuffer();
        m_CurrentWriteBuffer->GetBufferAndLength(&pWriteData, &cbWriteData);

        memcpy(pWriteData, pData, cbData);
        m_CurrentWriteBuffer->SetLength(cbData);
    }
}

/// <summary>
/// Add buffer full of captured audio data to the circular queue of buffers ready for client reading.
/// </summary>
/// <param name="pBuffer">Buffer holding captured audio data.</param>
void KinectAudioStream::QueueCapturedBuffer(IMediaBuffer *pBuffer)
{
    EnterCriticalSection(&m_Lock);
    m_ReadBufferQueue.push(pBuffer);
    SetEvent(m_hDataReady);

    LeaveCriticalSection(&m_Lock);
}

/// <summary>
/// Perform one read operation from current read buffer until either we completely fill the specified
/// client buffer or we read until the end of (exhaust) the current read buffer.
/// If we exhaust the current read buffer, we get the oldest available buffer from circular queue and
/// make that into our current read buffer.
/// </summary>
/// <param name="ppbData">
/// In/Out pointer to client's audio data buffer to be filled.
/// </param>
/// <param name="pcbData">
/// In/Out pointer to number of bytes remaining to be filled in client's audio data buffer.
/// </param>
void KinectAudioStream::ReadOneBuffer(BYTE **ppbData, ULONG* pcbData)
{
    EnterCriticalSection(&m_Lock);

    //Do we already have a buffer we are reading from? Otherwise grab one from the queue
    if (m_CurrentReadBuffer == NULL) 
    {
        if(m_ReadBufferQueue.size() != 0)
        {
            m_CurrentReadBuffer = m_ReadBufferQueue.front();
            m_ReadBufferQueue.pop();
        }
    }

    if (m_CurrentReadBuffer != NULL) 
    {
        //Copy as much data as we can or need
        BYTE *pData = NULL;
        DWORD dwDataLength = 0;
        m_CurrentReadBuffer->GetBufferAndLength(&pData, &dwDataLength);

        ULONG cbToCopy = min(dwDataLength - m_CurrentReadBufferIndex, *pcbData);
        memcpy(*ppbData, pData + m_CurrentReadBufferIndex, cbToCopy);
        *ppbData = (*ppbData)+cbToCopy;
        *pcbData = (*pcbData)-cbToCopy;
        m_CurrentReadBufferIndex += cbToCopy;

        //If we are done with this buffer put it back in the queue
        if (m_CurrentReadBufferIndex >= dwDataLength)
        {
            ReleaseBuffer(m_CurrentReadBuffer);
            m_CurrentReadBuffer = NULL;
            m_CurrentReadBufferIndex = 0;

            if(m_ReadBufferQueue.size() != 0)
            {
                m_CurrentReadBuffer = m_ReadBufferQueue.front();
                m_ReadBufferQueue.pop();
            }
        }
    }

    LeaveCriticalSection(&m_Lock);
}

/// <summary>
/// Starting address for audio capture thread.
/// </summary>
/// <param name="pParam">
/// Thread data passed to the function using the lpParameter parameter of the CreateThread function.
/// </param>
/// <returns>Non-zero if thread ended successfully, zero in case of failure</returns>
DWORD WINAPI KinectAudioStream::CaptureThread(LPVOID pParam)
{
    KinectAudioStream *pthis = (KinectAudioStream *) pParam;
    return pthis->CaptureThread();
}

/// <summary>
/// Audio capture thread. Captures audio data in a loop until it is signaled to stop.
/// </summary>
/// <returns>Non-zero if thread ended successfully, zero in case of failure</returns>
DWORD WINAPI KinectAudioStream::CaptureThread()
{
    HANDLE mmHandle = NULL;
    DWORD mmTaskIndex = 0;
    HRESULT hr = S_OK;
    bool bContinue = true;
    BYTE *pbOutputBuffer = NULL;
    
    IMediaBuffer* outputBuffer;
    MediaBuffer::Create(KINECT_WAVEFORMATEX, &outputBuffer);

    DMO_OUTPUT_DATA_BUFFER OutputBufferStruct = {0};
    OutputBufferStruct.pBuffer = outputBuffer;
    DWORD dwStatus = 0;
    ULONG cbProduced = 0;

    // Set high priority to avoid getting preempted while capturing sound
    mmHandle = AvSetMmThreadCharacteristics(L"Audio", &mmTaskIndex);

    while (bContinue)
    {
        if (WaitForSingleObject(m_hStopEvent, 0) == WAIT_OBJECT_0)
        {
            bContinue = false;
            continue;
        }

        do
        {
            MediaBuffer::Reset(outputBuffer);
            OutputBufferStruct.dwStatus = 0;
            hr = m_pKinectDmo->ProcessOutput(0, 1, &OutputBufferStruct, &dwStatus);
            if (FAILED(hr))
            {
                bContinue = false;
                break;
            }

            if (hr == S_FALSE)
            {
                cbProduced = 0;
            }
            else
            {
                outputBuffer->GetBufferAndLength(&pbOutputBuffer, &cbProduced);
            }

            // Queue audio data to be read by IStream client
            if (cbProduced > 0)
            {
                QueueCapturedData(pbOutputBuffer, cbProduced);
            }
        } while (OutputBufferStruct.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);

        Sleep(10); //sleep 10ms
    }

    outputBuffer->Release();

    SetEvent(m_hDataReady);
    AvRevertMmThreadCharacteristics(mmHandle);

    if (FAILED(hr))
    {
        return 0;
    }

    return 1;
}

