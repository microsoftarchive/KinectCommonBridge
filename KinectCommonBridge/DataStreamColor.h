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

#include "DataStreamDepth.h"

class DataStreamColor
    : public DataStream
{
public:
    DataStreamColor();
    virtual ~DataStreamColor();

    // for more advanced configuration options
    virtual void Initialize(_In_ INuiSensor* pNuiSensor);
    virtual void Initialize(NUI_IMAGE_TYPE type, NUI_IMAGE_RESOLUTION resolution, _In_ INuiSensor* pNuiSensor);

    // since we interact with the Nui device
    // we have to check the HRESULTS
    virtual HRESULT StartStream();
    virtual void StopStream();

    // Color
    NUI_IMAGE_TYPE GetImageType() { return m_imageType; }
    void SetImageType( NUI_IMAGE_TYPE type );

    NUI_IMAGE_RESOLUTION GetImageResolution() { return m_imageResolution; }
    void SetImageResolution( NUI_IMAGE_RESOLUTION resolution );

    void GetFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    HRESULT GetFrameData( ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pColorBuffer, _Out_opt_ LONGLONG* liTimeStamp );

    HRESULT GetColorAlignedToDepth( 
        ULONG cDepthPoints, _Inout_cap_(cDepthPoints) const NUI_DEPTH_IMAGE_POINT* pDepthPoints, 
        ULONG cBufferSize, _Inout_cap_(cBufferSize) BYTE* pImageBuffer, _Out_opt_ LONGLONG* liTimeStamp );

protected:
    virtual void CopyData(_In_ void* pImageFrame);

#ifdef KCB_ENABLE_FT
    void SetCameraConfig();
#endif

private:
    HRESULT OpenStream();
    virtual void CopyColorToDepth(_In_ NUI_IMAGE_FRAME *pImageFrame);

private:
    NUI_IMAGE_TYPE m_imageType;
    NUI_IMAGE_RESOLUTION m_imageResolution;
    DWORD m_dwWidth, m_dwHeight;

    NUI_IMAGE_FRAME m_ImageFrame;

    ULONG m_cBufferSize;
    BYTE* m_pImageBuffer;

    DWORD m_cDepthPoints;
    const NUI_DEPTH_IMAGE_POINT* m_pDepthPoints;
};

