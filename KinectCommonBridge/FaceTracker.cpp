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

#include "stdafx.h"
#include "FaceTracker.h"
#include "AutoLock.h"
#include "KinectSensor.h"

#ifdef KCB_ENABLE_FT

FaceTracker::FaceTracker(KinectSensor* pSensor, bool bNearMode)
    : m_fZoomFactor(1.0f)
    , m_DrawMask(true)
    , m_pSensor(pSensor)
    , m_pFaceTracker(nullptr)
    , m_bNearMode(bNearMode)
{
    Reset();
}

FaceTracker::~FaceTracker()
{
    Reset();
}

void FaceTracker::Reset()
{
    m_ViewOffset.x = 0;
    m_ViewOffset.y = 0;
    m_fXCenter = 0;
    m_fYCenter = 0;

    m_pFaceTracker.Release();
    m_pFTResult.Release();
    m_pColorImage.Release();
    m_pDepthImage.Release();
}


HRESULT FaceTracker::Initialize(_In_ DataStreamColor* pColorStream, _In_ DataStreamDepth* pDepthStream)
{
    Reset();

    HRESULT hr = m_pSensor->StartStreams();

    if(SUCCEEDED(hr))
    {
        // Note: FTCreateFaceTracker already addRefs the returned interface
        auto ft = FTCreateFaceTracker(NULL);
        m_pFaceTracker.Attach(ft);
        if (nullptr == m_pFaceTracker)
        {
            hr = E_POINTER;
        }
    }

    if(SUCCEEDED(hr))
    {
        FT_CAMERA_CONFIG c = pColorStream->GetCameraConfig();
        FT_CAMERA_CONFIG d = pDepthStream->GetCameraConfig();
        hr = m_pFaceTracker->Initialize(&c, &d, NULL, NULL);
    }

    if(SUCCEEDED(hr))
    {
        hr = m_pFaceTracker->CreateFTResult(&m_pFTResult);
    }

    if(SUCCEEDED(hr))
    {
        // Initialize the RGB image.
        m_pColorImage = FTCreateImage();	
        if (!m_pColorImage)
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
            m_cameraConfig = pColorStream->GetCameraConfig();
            hr = m_pColorImage->Allocate(m_cameraConfig.Width,m_cameraConfig.Height, FTIMAGEFORMAT_UINT8_B8G8R8X8);
        }
    }

    if(SUCCEEDED(hr))
    {
        // Initialize the Depth image.
        m_pDepthImage = FTCreateImage();        
        if (!m_pDepthImage)
        {
            hr = E_OUTOFMEMORY;
        }

        if(SUCCEEDED(hr))
        {
            hr = m_pDepthImage->Allocate(pDepthStream->GetCameraConfig().Width,pDepthStream->GetCameraConfig().Height, FTIMAGEFORMAT_UINT16_D13P3);
        }
    }

    if(SUCCEEDED(hr))
    {
        pDepthStream->SetNearMode(m_bNearMode);
        SetCenterOfImage(NULL);

        m_LastTrackSucceeded = false;

        m_hint3D[0] = m_hint3D[1] = FT_VECTOR3D(0, 0, 0);
        for (int i = 0; i < NUI_SKELETON_COUNT; ++i)
        {
            m_HeadPoint[i] = m_NeckPoint[i] = FT_VECTOR3D(0, 0, 0);
            m_SkeletonTracked[i] = false;
        }
    }

    return hr;
}

void FaceTracker::SetCenterOfImage(IFTResult* pResult)
{
    float centerX = ((float)m_pColorImage->GetWidth())/2.0f;
    float centerY = ((float)m_pColorImage->GetHeight())/2.0f;
    if (pResult)
    {
        if (SUCCEEDED(pResult->GetStatus()))
        {
            RECT faceRect;
            pResult->GetFaceRect(&faceRect);
            centerX = (faceRect.left+faceRect.right)/2.0f;
            centerY = (faceRect.top+faceRect.bottom)/2.0f;
        }
        m_fXCenter += 0.02f*(centerX-m_fXCenter);
        m_fYCenter += 0.02f*(centerY-m_fYCenter);
    }
    else
    {
        m_fXCenter = centerX;
        m_fYCenter = centerY;
    }
}

HRESULT FaceTracker::GetFaceTrackingResult(IFTResult **pResult)
{

    if(pResult == nullptr)
        return E_POINTER;

    HRESULT hr = E_FAIL;

    if (m_pSensor->ColorFrameReady())
    {
        hr = m_pSensor->GetColorFrame(m_pColorImage->GetBufferSize(), m_pColorImage->GetBuffer(), nullptr);
        if (SUCCEEDED(hr) && m_pSensor->DepthFrameReady())
        {    
            hr = m_pSensor->GetDepthFrame(m_pDepthImage->GetBufferSize(), m_pDepthImage->GetBuffer(), nullptr);

            if(SUCCEEDED(hr))
            {
                FT_SENSOR_DATA sensorData;
                sensorData.pVideoFrame = m_pColorImage;
                sensorData.pDepthFrame = m_pDepthImage;
                sensorData.ZoomFactor = m_fZoomFactor;       
                sensorData.ViewOffset = m_ViewOffset;

                FT_VECTOR3D* hint = NULL;
                if (SUCCEEDED(GetClosestHint(m_hint3D)))
                {
                    hint = m_hint3D;
                }
                if (m_LastTrackSucceeded)
                {
                    hr = m_pFaceTracker->ContinueTracking(&sensorData, hint, m_pFTResult);
                }
                else
                {
                    hr = m_pFaceTracker->StartTracking(&sensorData, NULL, hint, m_pFTResult);
                }


            }
        }
    }

    m_LastTrackSucceeded = SUCCEEDED(hr) && SUCCEEDED(m_pFTResult->GetStatus());

    if(m_LastTrackSucceeded)		
    {	
        SubmitFraceTrackingResult(m_pFTResult);		
    }
    else
    {
        m_pFTResult->Reset();		
    }

    SetCenterOfImage(m_pFTResult);

    if(pResult == nullptr)
        return E_POINTER;
    else
        hr = m_pFTResult.CopyTo(pResult);	

    return hr;
}



bool FaceTracker::SubmitFraceTrackingResult(IFTResult* pResult)
{
    if (pResult != NULL && SUCCEEDED(pResult->GetStatus()))
    {
        if (m_DrawMask)
        {
            FLOAT* pSU = NULL;
            UINT numSU;
            BOOL suConverged;

            m_pFaceTracker->GetShapeUnits(NULL, &pSU, &numSU, &suConverged);
            POINT viewOffset = {0, 0}; 
            IFTModel* ftModel;
            HRESULT hr = m_pFaceTracker->GetFaceModel(&ftModel);
            if (SUCCEEDED(hr))
            {
                hr = VisualizeFaceModel(m_pColorImage, ftModel,&m_cameraConfig, pSU, 1.0, viewOffset,  0x00FFFF00);
                ftModel->Release();
            }
        }
    }
    return TRUE;
}

HRESULT FaceTracker::VisualizeFaceModel(IFTImage* pColorImg, IFTModel* pModel, FT_CAMERA_CONFIG const* pCameraConfig, FLOAT const* pSUCoef, 
                                        FLOAT zoomFactor, POINT viewOffset,  UINT32 color)
{
    if (!pColorImg || !pModel || !pCameraConfig || !pSUCoef || !m_pFTResult)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    UINT vertexCount = pModel->GetVertexCount();
    FT_VECTOR2D* pPts2D = reinterpret_cast<FT_VECTOR2D*>(_malloca(sizeof(FT_VECTOR2D) * vertexCount));
    if (pPts2D)
    {
        FLOAT *pAUs;
        UINT auCount;
        hr = m_pFTResult->GetAUCoefficients(&pAUs, &auCount);
        if (SUCCEEDED(hr))
        {
            FLOAT scale, rotationXYZ[3], translationXYZ[3];
            hr = m_pFTResult->Get3DPose(&scale, rotationXYZ, translationXYZ);
            if (SUCCEEDED(hr))
            {
                hr = pModel->GetProjectedShape(pCameraConfig, zoomFactor, viewOffset, pSUCoef, pModel->GetSUCount(), pAUs, auCount, 
                    scale, rotationXYZ, translationXYZ, pPts2D, vertexCount);
                if (SUCCEEDED(hr))
                {
                    POINT* p3DMdl   = reinterpret_cast<POINT*>(_malloca(sizeof(POINT) * vertexCount));
                    if (p3DMdl)
                    {
                        for (UINT i = 0; i < vertexCount; ++i)
                        {
                            p3DMdl[i].x = LONG(pPts2D[i].x + 0.5f);
                            p3DMdl[i].y = LONG(pPts2D[i].y + 0.5f);
                        }

                        FT_TRIANGLE* pTriangles;
                        UINT triangleCount;
                        hr = pModel->GetTriangles(&pTriangles, &triangleCount);
                        if (SUCCEEDED(hr))
                        {
                            struct EdgeHashTable
                            {
                                UINT32* pEdges;
                                UINT edgesAlloc;

                                void Insert(int a, int b) 
                                {
                                    UINT32 v = (min(a, b) << 16) | max(a, b);
                                    UINT32 index = (v + (v << 8)) * 49157, i;
                                    for (i = 0; i < edgesAlloc - 1 && pEdges[(index + i) & (edgesAlloc - 1)] && v != pEdges[(index + i) & (edgesAlloc - 1)]; ++i)
                                    {
                                    }
                                    pEdges[(index + i) & (edgesAlloc - 1)] = v;
                                }
                            } eht;

                            eht.edgesAlloc = 1 << UINT(log(2.f * (1 + vertexCount + triangleCount)) / log(2.f));
                            eht.pEdges = reinterpret_cast<UINT32*>(_malloca(sizeof(UINT32) * eht.edgesAlloc));
                            if (eht.pEdges)
                            {
                                ZeroMemory(eht.pEdges, sizeof(UINT32) * eht.edgesAlloc);
                                for (UINT i = 0; i < triangleCount; ++i)
                                { 
                                    eht.Insert(pTriangles[i].i, pTriangles[i].j);
                                    eht.Insert(pTriangles[i].j, pTriangles[i].k);
                                    eht.Insert(pTriangles[i].k, pTriangles[i].i);
                                }
                                for (UINT i = 0; i < eht.edgesAlloc; ++i)
                                {
                                    if(eht.pEdges[i] != 0)
                                    {
                                        pColorImg->DrawLine(p3DMdl[eht.pEdges[i] >> 16], p3DMdl[eht.pEdges[i] & 0xFFFF], color, 1);
                                    }
                                }
                                _freea(eht.pEdges);
                            }

                            // Render the face rect in magenta
                            RECT rectFace;
                            hr = m_pFTResult->GetFaceRect(&rectFace);
                            if (SUCCEEDED(hr))
                            {
                                POINT leftTop = {rectFace.left, rectFace.top};
                                POINT rightTop = {rectFace.right - 1, rectFace.top};
                                POINT leftBottom = {rectFace.left, rectFace.bottom - 1};
                                POINT rightBottom = {rectFace.right - 1, rectFace.bottom - 1};
                                UINT32 nColor = 0xff00ff;
                                SUCCEEDED(hr = pColorImg->DrawLine(leftTop, rightTop, nColor, 1)) &&
                                    SUCCEEDED(hr = pColorImg->DrawLine(rightTop, rightBottom, nColor, 1)) &&
                                    SUCCEEDED(hr = pColorImg->DrawLine(rightBottom, leftBottom, nColor, 1)) &&
                                    SUCCEEDED(hr = pColorImg->DrawLine(leftBottom, leftTop, nColor, 1));
                            }
                        }

                        _freea(p3DMdl); 
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
            }
        }
        _freea(pPts2D);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return hr;
}

HRESULT FaceTracker::GetClosestHint(FT_VECTOR3D* pHint3D)
{
    int selectedSkeleton = -1;
    float smallestDistance = 0;

    if (!pHint3D)
    {
        return(E_POINTER);
    }

    if (pHint3D[1].x == 0 && pHint3D[1].y == 0 && pHint3D[1].z == 0)
    {
        // Get the skeleton closest to the camera
        for (int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {
            if (m_SkeletonTracked[i] && (smallestDistance == 0 || m_HeadPoint[i].z < smallestDistance))
            {
                smallestDistance = m_HeadPoint[i].z;
                selectedSkeleton = i;
            }
        }
    }
    else
    {   // Get the skeleton closest to the previous position
        for (int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
        {
            if (m_SkeletonTracked[i])
            {
                float d = abs(m_HeadPoint[i].x - pHint3D[1].x) +
                    abs(m_HeadPoint[i].y - pHint3D[1].y) +
                    abs(m_HeadPoint[i].z - pHint3D[1].z);
                if (smallestDistance == 0 || d < smallestDistance)
                {
                    smallestDistance = d;
                    selectedSkeleton = i;
                }
            }
        }
    }
    if (selectedSkeleton == -1)
    {
        return E_FAIL;
    }

    pHint3D[0] = m_NeckPoint[selectedSkeleton];
    pHint3D[1] = m_HeadPoint[selectedSkeleton];

    return S_OK;
}

HRESULT FaceTracker::GetSkeletonFrame()
{

    NUI_SKELETON_FRAME pSkeleton = {0};
    HRESULT hr = m_pSensor->GetSkeletonFrame(pSkeleton);
    
    if(FAILED(hr))
    {
        return hr;
    }


    //// update the track states
    for( int i = 0 ; i < NUI_SKELETON_COUNT ; i++ )
    {
        if( pSkeleton.SkeletonData[i].eTrackingState == NUI_SKELETON_TRACKED &&
            NUI_SKELETON_POSITION_TRACKED == pSkeleton.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_HEAD] &&
            NUI_SKELETON_POSITION_TRACKED == pSkeleton.SkeletonData[i].eSkeletonPositionTrackingState[NUI_SKELETON_POSITION_SHOULDER_CENTER])
        {
            m_SkeletonTracked[i] = true;
            m_HeadPoint[i].x = pSkeleton.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].x;
            m_HeadPoint[i].y = pSkeleton.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].y;
            m_HeadPoint[i].z = pSkeleton.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_HEAD].z;

            m_NeckPoint[i].x = pSkeleton.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].x;
            m_NeckPoint[i].y = pSkeleton.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].y;
            m_NeckPoint[i].z = pSkeleton.SkeletonData[i].SkeletonPositions[NUI_SKELETON_POSITION_SHOULDER_CENTER].z;
        }
        else
        {
            m_HeadPoint[i] = m_NeckPoint[i] = FT_VECTOR3D(0, 0, 0);
            m_SkeletonTracked[i] = false;
        }
    }	

    return S_OK;
}


HRESULT FaceTracker::GetFaceTracker( IFTFaceTracker** pFaceTracker)
{
    if (nullptr == m_pFaceTracker)
        return E_FAIL;

    if(pFaceTracker == nullptr)
        return E_POINTER;
    else
        m_pFaceTracker.CopyTo(pFaceTracker);

    return S_OK;
}

#endif
