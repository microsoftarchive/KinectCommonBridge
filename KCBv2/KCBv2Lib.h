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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN     // Exclude rarely-used stuff from Windows headers
#endif

// Windows Header Files:
#include <windows.h>
#include <objbase.h>

#include <Kinect.h>                         // will need $(KINECTSDK20_DIR)inc; in the includes directory for project

#ifdef WIN32
    #ifdef KCBV2_STATIC
        #define KINECT_CB
    #else
        #ifdef KCBV2_EXPORTS
            #define KINECT_CB __declspec(dllexport)
        #else
            #define KINECT_CB __declspec(dllimport)
            #pragma comment (lib, "KCBv2")      // add $(KINECTSDK20_DIR)lib\x64 or x86 to the library directory for project
        #endif // DLL_EXPORTS
    #endif // !KCBV2_STATIC
#endif //_WIN32

#ifndef __KCB_HANDLE__
#define __KCB_HANDLE__

typedef int KCBHANDLE;
static const int KCB_INVALID_HANDLE = 0xffffffff;

#endif

typedef struct KCBFrameDescription
{
    int width;
    int height;
    float horizontalFieldOfView;
    float verticalFieldOfView;
    float diagonalFieldOfView;
    unsigned int lengthInPixels;
    unsigned int bytesPerPixel;
} KCBFrameDescription;

typedef struct KCBAudioFrame
{
    ULONG cAudioBufferSize;
    byte* pAudioBuffer; // ieee float 4bytes
    ULONG ulBytesRead;
    FLOAT fBeamAngle;
    FLOAT fBeamAngleConfidence;
} KCBAudioFrame;

typedef struct KCBBodyFrame
{
    UINT Count;
    IBody** Bodies;
    LONGLONG TimeStamp;
} KCBBodyFrame;

typedef struct KCBBodyIndexFrame
{
    ULONG Size;
    BYTE* Buffer;
    LONGLONG TimeStamp;
} KCBBodyIndexFrame;

typedef struct KCBColorFrame
{
    ColorImageFormat Format; 
    ULONG Size; 
    BYTE* Buffer;
    LONGLONG TimeStamp;
} KCBColorFrame;

typedef struct KCBDepthFrame
{
    ULONG Size;
    UINT16* Buffer;
    LONGLONG TimeStamp;
} KCBDepthFrame, KCBInfraredFrame, KCBLongExposureInfraredFrame;

extern "C"
{
    KINECT_CB KCBHANDLE APIENTRY KCBOpenDefaultSensor();
    KINECT_CB HRESULT APIENTRY KCBCloseSensor(_Inout_ KCBHANDLE* kcbHandle);

    // get the native IFrameDescription, caller must release the object
    KINECT_CB HRESULT APIENTRY KCBGetAudioFormat(_In_ KCBHANDLE kcbHandle, _Inout_ WAVEFORMATEX* pAudioFormat);
    KINECT_CB HRESULT APIENTRY KCBGetBodyIndexFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription);
    KINECT_CB HRESULT APIENTRY KCBGetColorFrameDescription(_In_ KCBHANDLE kcbHandle, ColorImageFormat eFormat, _Inout_ KCBFrameDescription* pFrameDescription);
    KINECT_CB HRESULT APIENTRY KCBGetDepthFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription);
    KINECT_CB HRESULT APIENTRY KCBGetInfraredFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription);
    KINECT_CB HRESULT APIENTRY KCBGetLongExposureInfraredFrameDescription(_In_ KCBHANDLE kcbHandle, _Inout_ KCBFrameDescription* pFrameDescription);

    // initializing frame buffers
    KINECT_CB HRESULT APIENTRY KCBCreateBodyFrame(_Out_ KCBBodyFrame** ppBodyFrame);
    KINECT_CB HRESULT APIENTRY KCBCreateBodyIndexFrame(KCBFrameDescription frameDescription, _Out_ KCBBodyIndexFrame** ppBodyIndexFrame);
    KINECT_CB HRESULT APIENTRY KCBCreateColorFrame(ColorImageFormat colorFormat, KCBFrameDescription frameDescription, _Out_ KCBColorFrame** ppColorFrame);
    KINECT_CB HRESULT APIENTRY KCBCreateDepthFrame(KCBFrameDescription frameDescription, _Out_ KCBDepthFrame** ppDepthFrame);
    KINECT_CB HRESULT APIENTRY KCBCreateInfraredFrame(KCBFrameDescription frameDescription, _Out_ KCBInfraredFrame** ppInfraredFrame);
    KINECT_CB HRESULT APIENTRY KCBCreateLongExposureInfraredFrame(KCBFrameDescription frameDescription, _Out_ KCBLongExposureInfraredFrame** ppLongExposureInfraredFrame);

    // clean-up frame buffer
    KINECT_CB HRESULT APIENTRY KCBReleaseBodyFrame(_Inout_ KCBBodyFrame** pBodyFrame);
    KINECT_CB HRESULT APIENTRY KCBReleaseBodyIndexFrame(_Inout_ KCBBodyIndexFrame** pBodyIndexFrame);
    KINECT_CB HRESULT APIENTRY KCBReleaseColorFrame(_Inout_ KCBColorFrame** pColorFrame);
    KINECT_CB HRESULT APIENTRY KCBReleaseDepthFrame(_Inout_ KCBDepthFrame** pDepthFrame);
    KINECT_CB HRESULT APIENTRY KCBReleaseInfraredFrame(_Inout_ KCBInfraredFrame** pInfraredFrame);
    KINECT_CB HRESULT APIENTRY KCBReleaseLongExposureInfraredFrame(_Inout_ KCBLongExposureInfraredFrame** pLongExposureInfraredFrame);

    // copy method by passing struct
    KINECT_CB HRESULT APIENTRY KCBGetAudioFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBAudioFrame* pstAudioFrame);
    KINECT_CB HRESULT APIENTRY KCBGetBodyFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBBodyFrame* pstBodyFrame);
    KINECT_CB HRESULT APIENTRY KCBGetBodyIndexFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBBodyIndexFrame* pstBodyIndexFrame);
    KINECT_CB HRESULT APIENTRY KCBGetColorFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBColorFrame* pstColorFrame);
    KINECT_CB HRESULT APIENTRY KCBGetDepthFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBDepthFrame* pstDepthFrame);
    KINECT_CB HRESULT APIENTRY KCBGetInfraredFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBInfraredFrame* pstInfraredFrame);
    KINECT_CB HRESULT APIENTRY KCBGetLongExposureInfraredFrame(_In_ KCBHANDLE kcbHandle, _Inout_ KCBLongExposureInfraredFrame* pstLongExposureInfraredFrame);

    // copy methods just to get the buffer data
    KINECT_CB HRESULT APIENTRY KCBGetAudioData(_In_ KCBHANDLE kcbHandle, ULONG cAudioBufferSize, _Inout_cap_(cAudioBufferSize) byte*, _Out_ ULONG* ulBytesRead, _Out_opt_ FLOAT* beamAngle, _Out_opt_ FLOAT* sourceConfidence);
    KINECT_CB HRESULT APIENTRY KCBGetBodyData(_In_ KCBHANDLE kcbHandle, UINT capacity, _Inout_updates_all_(capacity) IBody **bodies, _Out_opt_ LONGLONG* llTimeStamp);
    KINECT_CB HRESULT APIENTRY KCBGetBodyIndexData(_In_ KCBHANDLE kcbHandle, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pbBuffer, _Out_opt_ LONGLONG* llTimeStamp);
    KINECT_CB HRESULT APIENTRY KCBGetColorData(_In_ KCBHANDLE kcbHandle, ColorImageFormat eColorFormat, ULONG cbBufferSize, _Inout_cap_(cbBufferSize) BYTE* pbBuffer, _Out_opt_ LONGLONG* llTimeStamp);
    KINECT_CB HRESULT APIENTRY KCBGetDepthData(_In_ KCBHANDLE kcbHandle, ULONG cuiBufferSize, _Inout_cap_(cuiBufferSize) UINT16* puiBuffer, _Out_opt_ LONGLONG* llTimeStamp);
    KINECT_CB HRESULT APIENTRY KCBGetInfraredData(_In_ KCBHANDLE kcbHandle, ULONG cuiBufferSize, _Inout_cap_(cuiBufferSize) UINT16* puiBuffer, _Out_opt_ LONGLONG* llTimeStamp);
    KINECT_CB HRESULT APIENTRY KCBGetLongExposureInfraredData(_In_ KCBHANDLE kcbHandle, ULONG cuiBufferSize, _Inout_cap_(cuiBufferSize) UINT16* puiBuffer, _Out_opt_ LONGLONG* llTimeStamp);

    // copy all frame by using the MultiSourceFrameReader
    KINECT_CB HRESULT APIENTRY KCBGetAllFrameData(_In_ KCBHANDLE kcbHandle, 
        _Inout_opt_ KCBBodyFrame* pstBodyFrame,
        _Inout_opt_ KCBBodyIndexFrame* pstBodyIndexFrame,
        _Inout_opt_ KCBColorFrame* pstColorFrame,
        _Inout_opt_ KCBDepthFrame* pstDepthFrame,
        _Inout_opt_ KCBInfraredFrame* pstInfraredFrame,
        _Inout_opt_ KCBLongExposureInfraredFrame* pstLongExposureInfraredFrame);

    KINECT_CB bool APIENTRY KCBIsFrameReady(_In_ KCBHANDLE kcbHandle, FrameSourceTypes eSourceType);
    KINECT_CB bool APIENTRY KCBAnyFrameReady(_In_ KCBHANDLE kcbHandle);
    KINECT_CB bool APIENTRY KCBAllFramesReady(_In_ KCBHANDLE kcbHandle);
    KINECT_CB bool APIENTRY KCBMultiFrameReady(_In_ KCBHANDLE kcbHandle);

    // straight calls to the coordinate mapper
    KINECT_CB HRESULT APIENTRY KCBMapCameraPointToDepthSpace(_In_ KCBHANDLE kcbHandle, 
        CameraSpacePoint cameraPoint, 
        _Out_ DepthSpacePoint* depthPoint);
    KINECT_CB HRESULT APIENTRY KCBMapCameraPointToColorSpace(_In_ KCBHANDLE kcbHandle, 
        CameraSpacePoint cameraPoint, 
        _Out_ ColorSpacePoint *colorPoint);
    KINECT_CB HRESULT APIENTRY KCBMapDepthPointToCameraSpace(_In_ KCBHANDLE kcbHandle, 
        DepthSpacePoint depthPoint, UINT16 depth, 
        _Out_ CameraSpacePoint *cameraPoint);
    KINECT_CB HRESULT APIENTRY KCBMapDepthPointToColorSpace(_In_ KCBHANDLE kcbHandle, 
        DepthSpacePoint depthPoint, UINT16 depth, 
        _Out_ ColorSpacePoint *colorPoint);
    KINECT_CB HRESULT APIENTRY KCBMapCameraPointsToDepthSpace(_In_ KCBHANDLE kcbHandle, 
        UINT cameraPointCount, _In_reads_(cameraPointCount) const CameraSpacePoint *cameraPoints, 
        UINT depthPointCount, 
        _Out_writes_all_(depthPointCount) DepthSpacePoint *depthPoints);
    KINECT_CB HRESULT APIENTRY KCBMapCameraPointsToColorSpace(_In_ KCBHANDLE kcbHandle,
        UINT cameraPointCount, _In_reads_(cameraPointCount) const CameraSpacePoint *cameraPoints,
        UINT colorPointCount, 
        _Out_writes_all_(colorPointCount) ColorSpacePoint *colorPoints);
    KINECT_CB HRESULT APIENTRY KCBMapDepthPointsToCameraSpace(_In_ KCBHANDLE kcbHandle, 
        UINT depthPointCount, _In_reads_(depthPointCount) const DepthSpacePoint *depthPoints,
        UINT depthCount, _In_reads_(depthCount) const UINT16 *depths,
        UINT cameraPointCount, 
        _Out_writes_all_(cameraPointCount) CameraSpacePoint *cameraPoints);
    KINECT_CB HRESULT APIENTRY KCBMapDepthPointsToColorSpace(_In_ KCBHANDLE kcbHandle,
        UINT depthPointCount, _In_reads_(depthPointCount) const DepthSpacePoint *depthPoints,
        UINT depthCount, _In_reads_(depthCount) const UINT16 *depths,
        UINT colorPointCount, 
        _Out_writes_all_(colorPointCount) ColorSpacePoint *colorPoints);
    KINECT_CB HRESULT APIENTRY KCBMapDepthFrameToCameraSpace(_In_ KCBHANDLE kcbHandle, 
        UINT depthPointCount, _In_reads_(depthPointCount) const UINT16 *depthFrameData, 
        UINT cameraPointCount, 
        _Out_writes_all_(cameraPointCount) CameraSpacePoint *cameraSpacePoints);
    KINECT_CB HRESULT APIENTRY KCBMapDepthFrameToColorSpace(_In_ KCBHANDLE kcbHandle,
        UINT depthPointCount, _In_reads_(depthPointCount) const UINT16 *depthFrameData,
        UINT colorPointCount, 
        _Out_writes_all_(colorPointCount) ColorSpacePoint *colorSpacePoints);
    KINECT_CB HRESULT APIENTRY KCBMapColorFrameToDepthSpace(_In_ KCBHANDLE kcbHandle,
        UINT depthDataPointCount, _In_reads_(depthDataPointCount) const UINT16 *depthFrameData,
        UINT depthPointCount, 
        _Out_writes_all_(depthPointCount) DepthSpacePoint *depthSpacePoints);
    KINECT_CB HRESULT APIENTRY KCBMapColorFrameToCameraSpace(_In_ KCBHANDLE kcbHandle,
        UINT depthDataPointCount, _In_reads_(depthDataPointCount)  const UINT16 *depthFrameData,
        UINT cameraPointCount, 
        _Out_writes_all_(cameraPointCount) CameraSpacePoint *cameraSpacePoints);
    KINECT_CB HRESULT APIENTRY GetDepthFrameToCameraSpaceTable(_In_ KCBHANDLE kcbHandle,
        _Out_  UINT32 *tableEntryCount,
        _Outptr_result_bytebuffer_(*tableEntryCount) PointF **tableEntries);

    // get the native IxxxFrame, caller must release the object
    // if you just want the data, use the other frame functions.
    KINECT_CB HRESULT APIENTRY KCBGetIBodyFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IBodyFrame **ppBodyFrame);
    KINECT_CB HRESULT APIENTRY KCBGetIBodyIndexFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IBodyIndexFrame **ppBodyIndexFrame);
    KINECT_CB HRESULT APIENTRY KCBGetIColorFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IColorFrame **ppColorFrame);
    KINECT_CB HRESULT APIENTRY KCBGetIDepthFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IDepthFrame **ppDepthFrame);
    KINECT_CB HRESULT APIENTRY KCBGetIInfraredFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ IInfraredFrame **ppInfraredFrame);
    KINECT_CB HRESULT APIENTRY KCBGetILongExposureInfraredFrame(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ ILongExposureInfraredFrame **ppLongExposureInfraredFrame);

    // multi-source frame reader, caller must release the object
    KINECT_CB HRESULT APIENTRY KCBGetIMultiSourceFrame(_In_ KCBHANDLE kcbHandle, DWORD dwFrameSourceTypes, _COM_Outptr_result_maybenull_ IMultiSourceFrame **ppMultiSourceFrame);

    // Coordinate mapper, caller must release the object
    KINECT_CB HRESULT APIENTRY KCBGetICoordinateMapper(_In_ KCBHANDLE kcbHandle, _COM_Outptr_result_maybenull_ ICoordinateMapper** ppCoordinateMapper);
}
