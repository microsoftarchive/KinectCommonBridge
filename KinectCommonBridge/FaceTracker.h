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
#include "KinectSensor.h"



#include <memory>

#ifdef KCB_ENABLE_FT

class FaceTracker
{
public:
    FaceTracker(KinectSensor *pSensor,  bool bNearMode = false);
    ~FaceTracker();

    HRESULT Initialize(_In_ DataStreamColor* pColorStream, _In_ DataStreamDepth* pDepthStream);
    HRESULT GetFaceTrackingResult(IFTResult **pResult);
    HRESULT GetColorImage(IFTImage** destImage)   { return m_pColorImage.CopyTo(destImage);}
    HRESULT GetFaceTracker( IFTFaceTracker** pFaceTracker);
    float GetXCenterFace()      { return(m_fXCenter);}
    float GetYCenterFace()      { return(m_fYCenter);}
    IFTFaceTracker* GetTracker() { return(m_pFaceTracker);}

private:
    void Reset();
    void SetCenterOfImage(IFTResult* pResult);	

    HRESULT GetClosestHint(FT_VECTOR3D* pHint3D);
    HRESULT GetSkeletonFrame();
    bool SubmitFraceTrackingResult(IFTResult* pResult);
    HRESULT VisualizeFaceModel(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
        FLOAT zoomFactor, POINT viewOffset,  UINT32 color);

private:
    FT_VECTOR3D m_NeckPoint[NUI_SKELETON_COUNT];
    FT_VECTOR3D m_HeadPoint[NUI_SKELETON_COUNT];
    bool m_SkeletonTracked[NUI_SKELETON_COUNT];

    ComSmartPtr<IFTFaceTracker>     m_pFaceTracker;
    ComSmartPtr<IFTResult>          m_pFTResult;
    ComSmartPtr<IFTImage>           m_pColorImage;
    ComSmartPtr<IFTImage>           m_pDepthImage;

    FT_CAMERA_CONFIG            m_cameraConfig;
    KinectSensor                *m_pSensor;
    FT_VECTOR3D		            m_hint3D[2];
    bool	                    m_LastTrackSucceeded;
    bool                        m_bNearMode;

    float                       m_fXCenter;
    float                       m_fYCenter;
    FLOAT                       m_fZoomFactor;	// 1.0f is there is no zoom
    POINT                       m_ViewOffset;		// offset from the top left corner

    bool                        m_DrawMask;
};

#endif
