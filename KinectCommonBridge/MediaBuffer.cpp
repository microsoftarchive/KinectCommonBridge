#include "stdafx.h"
#include "MediaBuffer.h"

#pragma comment(lib, "dmoguids.lib")

MediaBuffer::MediaBuffer(DWORD cbMaxLength, HRESULT& hr)
    : m_nRefCount(1)
    , m_cbMaxLength(cbMaxLength)
    , m_cbLength(0)
    , m_pbData(nullptr)
{
    m_pbData.reset(new (std::nothrow) BYTE[cbMaxLength]);
    if(nullptr == m_pbData)
    {
        hr = E_OUTOFMEMORY;
    }
}
MediaBuffer::~MediaBuffer()
{
    m_pbData.reset();
}

// IUnknown methods
STDMETHODIMP_(ULONG) MediaBuffer::AddRef() 
{ 
    return InterlockedIncrement(&m_nRefCount);
}
STDMETHODIMP_(ULONG) MediaBuffer::Release() 
{ 
    LONG lRef = InterlockedDecrement(&m_nRefCount);
    if (lRef == 0)
    {
        delete this;
        // m_cRef is no longer valid! Return lRef.
    }
    return lRef;
}
STDMETHODIMP MediaBuffer::QueryInterface(REFIID riid, void **ppv)
{
    if (ppv == NULL)
    {
        return E_POINTER;
    }
    else if (riid == IID_IMediaBuffer || riid == IID_IUnknown)
    {
        *ppv = static_cast<IMediaBuffer *>(this);
        AddRef();
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

// IMediaBuffer methods
STDMETHODIMP MediaBuffer::SetLength(DWORD cbLength)
{ 
    if (cbLength > m_cbMaxLength)
    {
        return E_INVALIDARG;
    }
    m_cbLength = cbLength;
    return S_OK;
}
STDMETHODIMP MediaBuffer::GetMaxLength(DWORD *pcbMaxLength)
{ 
    if (pcbMaxLength == NULL)
    {
        return E_POINTER;
    }
    *pcbMaxLength = m_cbMaxLength;
    return S_OK;
}
STDMETHODIMP MediaBuffer::GetBufferAndLength(BYTE **ppbBuffer, DWORD *pcbLength)
{
    // Either parameter can be NULL, but not both.
    if (nullptr == ppbBuffer && nullptr == pcbLength)
    {
        return E_POINTER;
    }
    if (ppbBuffer)
    {
        *ppbBuffer = m_pbData.get();
    }
    if (pcbLength)
    {
        *pcbLength = m_cbLength;
    }
    return S_OK;
}


HRESULT MediaBuffer::Create(WAVEFORMATEX wfx, IMediaBuffer **ppBuffer)
{
    if (nullptr == ppBuffer)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    DWORD cbMaxLen = wfx.nSamplesPerSec * wfx.nBlockAlign;
    IMediaBuffer* pBuffer = new (std::nothrow) MediaBuffer(cbMaxLen, hr);
    if (nullptr == pBuffer)
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppBuffer = pBuffer;
        (*ppBuffer)->AddRef();
    }

    if (nullptr != pBuffer)
    {
        pBuffer->Release();
    }

    return hr;
}

HRESULT MediaBuffer::Reset( IMediaBuffer* pBuffer, bool zeroBuffer )
{
    if( nullptr == pBuffer )
    {
        return E_POINTER;
    }

    MediaBuffer* pMediaBuffer = static_cast<MediaBuffer*>(pBuffer);
    if( nullptr == pMediaBuffer )
    {
        return E_INVALIDARG;
    }

    pMediaBuffer->Reset(zeroBuffer);

    return S_OK;
}

void MediaBuffer::Reset( bool zeroBuffer )
{
    if( zeroBuffer )
    {
        memset( &m_pbData, 0, m_cbMaxLength );
    }
    m_cbLength = 0;
}
