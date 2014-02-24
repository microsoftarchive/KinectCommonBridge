#pragma once 

//  CMediaBuffer class.
class MediaBuffer 
    : public IMediaBuffer
{
public:
    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // IMediaBuffer methods.
    STDMETHODIMP SetLength(DWORD cbLength);
    STDMETHODIMP GetMaxLength(DWORD *pcbMaxLength);
    STDMETHODIMP GetBufferAndLength(BYTE **ppbBuffer, DWORD *pcbLength);

    //static methods
    static HRESULT Create(WAVEFORMATEX wfx, IMediaBuffer **ppBuffer);
    static HRESULT Reset(IMediaBuffer* ppBuffer, bool zeroBuffer = false);

private:
    MediaBuffer(DWORD cbMaxLength, HRESULT& hr);
    virtual ~MediaBuffer(); // will delete when all ref counts hit 0

    // MediaBuffer
    void Reset( bool zeroBuffer = false );

private:
    // Reference count
    LONG m_nRefCount;

    DWORD        m_cbLength;
    const DWORD  m_cbMaxLength;
    std::unique_ptr<BYTE[]>     m_pbData;
};
