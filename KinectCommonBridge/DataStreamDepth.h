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

class DataStreamDepth
    : public DataStream
{
public:
    DataStreamDepth();
    virtual ~DataStreamDepth();

    virtual void Initialize( _In_ INuiSensor* pNuiSensor );
    virtual void Initialize( bool bNearMode, NUI_IMAGE_RESOLUTION resolution, _In_ INuiSensor* pNuiSensor );

    virtual HRESULT StartStream();
    virtual void StopStream();

    // Depth
    void SetAlignToColor( bool bAlignToColor );
    void SetImageType( NUI_IMAGE_TYPE type );
    void SetNearMode( bool bNearMode );
    void SetImageResolution( NUI_IMAGE_RESOLUTION resolution );

    void GetFrameFormat( _Inout_ KINECT_IMAGE_FRAME_FORMAT* pFrame );
    HRESULT GetFrameData( ULONG cbBufferSize, _Out_cap_(cbBufferSize) BYTE* pDepthBuffer, _Out_opt_ LONGLONG* liTimeStamp );

private:
    HRESULT OpenStream();

private:
    NUI_IMAGE_TYPE m_imageType;
    NUI_IMAGE_RESOLUTION m_imageResolution;
    bool m_bNearMode;
    bool m_bAlignToColor;

    NUI_IMAGE_FRAME m_ImageFrame;
};

