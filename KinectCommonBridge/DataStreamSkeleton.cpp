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

#include "StdAfx.h"

#include "DataStreamSkeleton.h"
#include "AutoLock.h"

DataStreamSkeleton::DataStreamSkeleton()
    : DataStream()
    , m_bSmoothParams(false)
    , m_seated(false)
    , m_chooserMode(SkeletonSelectionModeDefault)
    , m_activityLevel(0.0f)
    , m_dwActiveID1(0)
    , m_dwActiveID2(0)
{
    m_stickyIDs[FirstTrackID] = 0;
    m_stickyIDs[SecondTrackID] = 0;

    ZeroMemory( &m_smoothParams, sizeof(NUI_TRANSFORM_SMOOTH_PARAMETERS) );
}
DataStreamSkeleton::~DataStreamSkeleton()
{
    StopStream();
}

void DataStreamSkeleton::Initialize(_In_ INuiSensor* pNuiSensor)
{
    DataStream::Initialize(pNuiSensor);
}

bool DataStreamSkeleton::IsEqual( NUI_TRANSFORM_SMOOTH_PARAMETERS& param1, NUI_TRANSFORM_SMOOTH_PARAMETERS& param2 )
{
   if( ( param1.fCorrection == param2.fCorrection ) &&
       ( param1.fJitterRadius == param2.fJitterRadius ) && 
       ( param1.fMaxDeviationRadius == param2.fMaxDeviationRadius ) && 
       ( param1.fPrediction == param2.fPrediction ) && 
       ( param1.fSmoothing == param2.fSmoothing ) )
   {
       return true;
   }
   
   return false;
}

void DataStreamSkeleton::Initialize( bool bSeated, KINECT_SKELETON_SELECTION_MODE mode, _In_ INuiSensor* pNuiSensor, _Inout_opt_ NUI_TRANSFORM_SMOOTH_PARAMETERS* pSmoothParams )
{
    AutoLock lock(m_nuiLock);

    bool bChanged = false;

    if( m_seated != bSeated )
    {
        m_seated = bSeated;
        bChanged = true;
    }

    if( m_chooserMode != mode )
    {
        m_chooserMode = mode;
        bChanged = true;
    }

    if( nullptr != pSmoothParams )
    {
        NUI_TRANSFORM_SMOOTH_PARAMETERS smoothParams = *pSmoothParams ;
        if ( !m_bSmoothParams || !IsEqual( m_smoothParams, smoothParams ) )
        {
            m_bSmoothParams = true;
            m_smoothParams = *pSmoothParams;
            bChanged = true;
        }
    }
    else
    {
        if ( m_bSmoothParams )
        {
            m_bSmoothParams = false;
            bChanged = false;
        }
    }
    
    if( bChanged )
    {
        m_started = false;
    }

    // send the sensor to the base class
    Initialize( pNuiSensor );
}

void DataStreamSkeleton::SetSeatedMode( bool seated )
{
    AutoLock lock(m_nuiLock);

    if (m_seated != seated)
    {
        m_seated = seated;
        StartStream();  // Restart stream with new parameter value
    }
}
void DataStreamSkeleton::SetChooserMode( KINECT_SKELETON_SELECTION_MODE mode )
{
    AutoLock lock(m_nuiLock);

    if (m_chooserMode != mode)
    {
        m_chooserMode = mode;
        StartStream();  // Restart stream with new parameter value
    }
}

HRESULT DataStreamSkeleton::StartStream(bool bPaused)
{
    AutoLock lock(m_nuiLock);

    // if there is no device, disable the stream 
    if( nullptr == m_pNuiSensor )
    {
        RemoveDevice();
        return E_NUI_STREAM_NOT_ENABLED;
    }

    if( m_started && !m_paused && !bPaused)
    {
        return S_OK;
    }

    if( HasSkeletalEngine(m_pNuiSensor) )
    {
        if( bPaused ) 
        {
            m_paused = bPaused;

            // Disable tracking skeleton
            return m_pNuiSensor->NuiSkeletonTrackingDisable();
        }
        else
        {
            // Enable tracking skeleton
            DWORD flags = 
                (m_seated ? NUI_SKELETON_TRACKING_FLAG_ENABLE_SEATED_SUPPORT : 0) 
                | NUI_SKELETON_TRACKING_FLAG_ENABLE_IN_NEAR_RANGE
                | (SkeletonSelectionModeDefault != m_chooserMode ? NUI_SKELETON_TRACKING_FLAG_TITLE_SETS_TRACKED_SKELETONS : 0);

            HRESULT hr = m_pNuiSensor->NuiSkeletonTrackingEnable( GetFrameReadyEvent() , flags );

            if( SUCCEEDED(hr) )
            {
                m_paused = false;
                m_started = true;
            }

            return hr;
        }
    }

    m_started = false;

    return E_NUI_STREAM_NOT_ENABLED;
}

HRESULT DataStreamSkeleton::StartStream()
{
    AutoLock lock(m_nuiLock);

    return StartStream(m_paused);
}

void DataStreamSkeleton::StopStream()
{
    AutoLock lock(m_nuiLock);

    RemoveDevice();
}

void DataStreamSkeleton::PauseStream(bool bPaused)
{
    AutoLock lock(m_nuiLock);

    StartStream(bPaused);
}

HRESULT DataStreamSkeleton::GetFrameData( _Inout_ NUI_SKELETON_FRAME& pSkeletonFrame )
{
    AutoLock lock(m_nuiLock);

    HRESULT hr = S_OK;

    // if we haven't started the stream do it now
    if( !m_started )
    {
        hr = StartStream();
        if( FAILED(hr) )
        {
            return hr;
        }
    }

    if( m_paused ) 
    {
        return S_OK;
    }

    // Populate the frame
    hr = ProcessSkeletonFrame( pSkeletonFrame );
    if( FAILED(hr) )
    {
        return hr;
    }

    if( m_bSmoothParams )
    {
        hr = m_pNuiSensor->NuiTransformSmooth( &pSkeletonFrame, &m_smoothParams );
        if( FAILED(hr) )
        {
            return hr;
        }
    }

    UpdateTrackedSkeletons( pSkeletonFrame );
    
    return hr;
}

void DataStreamSkeleton::CopyData(_In_ void* pImageFrame)
{
    NUI_SKELETON_FRAME* pFrame = reinterpret_cast<NUI_SKELETON_FRAME*>( pImageFrame );
}

void DataStreamSkeleton::UpdateTrackedSkeletons( _In_ NUI_SKELETON_FRAME& pSkeletonFrame )
{
    DWORD trackIDs[TrackIDIndexCount] = {0};

    if( SkeletonSelectionModeClosest1 == m_chooserMode || SkeletonSelectionModeClosest2 == m_chooserMode )
    {
        ChooseClosestSkeletons(trackIDs, pSkeletonFrame);
    }
    else if( SkeletonSelectionModeSticky1 == m_chooserMode || SkeletonSelectionModeSticky2 == m_chooserMode )
    {
        ChooseStickySkeletons(trackIDs, pSkeletonFrame);
    }
    else if( SkeletonSelectionModeActive1 == m_chooserMode || SkeletonSelectionModeActive2 == m_chooserMode )
    {
        ChooseMostActiveSkeletons(trackIDs);
    }

    if (SkeletonSelectionModeClosest1 == m_chooserMode || SkeletonSelectionModeSticky1 == m_chooserMode || SkeletonSelectionModeActive1 == m_chooserMode)
    {
        // Track only one player ID. The second ID is not used
        trackIDs[SecondTrackID] = 0;
    }

    m_pNuiSensor->NuiSkeletonSetTrackedSkeletons( trackIDs );
}
/// <summary>
/// Find closest skeleton to set tracked
/// </summary>
/// <param name="trackIDs">Array of skeleton tracking IDs</param>
void DataStreamSkeleton::ChooseClosestSkeletons( DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& pSkeletonFrame )
{
    ZeroMemory(trackIDs, TrackIDIndexCount * sizeof(DWORD));

    // Initial depth array with max posible value
    USHORT nearestDepth[TrackIDIndexCount] = {NUI_IMAGE_DEPTH_MAXIMUM, NUI_IMAGE_DEPTH_MAXIMUM};

    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
    {
        if (NUI_SKELETON_NOT_TRACKED != pSkeletonFrame.SkeletonData[i].eTrackingState)
        {
            LONG   x, y;
            USHORT depth;

            // Transform skeleton coordinates to depth image
            NuiTransformSkeletonToDepthImage(pSkeletonFrame.SkeletonData[i].Position, &x, &y, &depth);

            // Compare depth to peviously found item
            if (depth < nearestDepth[FirstTrackID])
            {
                // Move depth and track ID in first place to second place and assign with the new closer one
                nearestDepth[SecondTrackID] = nearestDepth[FirstTrackID];
                nearestDepth[FirstTrackID]  = depth;

                trackIDs[SecondTrackID] = trackIDs[FirstTrackID];
                trackIDs[FirstTrackID]  = pSkeletonFrame.SkeletonData[i].dwTrackingID;
            }
            else if (depth < nearestDepth[SecondTrackID])
            {
                // Replace old depth and track ID in second place with the newly found closer one
                nearestDepth[SecondTrackID] = depth;
                trackIDs[SecondTrackID]     = pSkeletonFrame.SkeletonData[i].dwTrackingID;
            }
        }
    }
}

/// <summary>
/// Find sticky skeletons to set tracked
/// </summary>
/// <param name="trackIDs">Array of skeleton tracking IDs</param>
void DataStreamSkeleton::ChooseStickySkeletons(DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& pSkeletonFrame)
{
    ZeroMemory(trackIDs, TrackIDIndexCount * sizeof(DWORD));

    FindStickyIDs(trackIDs, pSkeletonFrame);
    AssignNewStickyIDs(trackIDs, pSkeletonFrame);

    // Update stored sticky IDs
    m_stickyIDs[FirstTrackID]  = trackIDs[FirstTrackID];
    m_stickyIDs[SecondTrackID] = trackIDs[SecondTrackID];
}

/// <summary>
/// Verify if stored tracked IDs are found in new skeleton frame
/// </summary>
/// <param name="trackIDs">Array of skeleton tracking IDs</param>
void DataStreamSkeleton::FindStickyIDs(DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& pSkeletonFrame)
{
    for(int i = 0; i < TrackIDIndexCount; i++)
    {
        for(int j = 0; j < NUI_SKELETON_COUNT; j++)
        {
            if(NUI_SKELETON_NOT_TRACKED != pSkeletonFrame.SkeletonData[j].eTrackingState)
            {
                DWORD trackID = pSkeletonFrame.SkeletonData[j].dwTrackingID;
                if(trackID == m_stickyIDs[i])
                {
                    trackIDs[i] = trackID;
                    break;
                }
            }
        }
    }
}

/// <summary>
/// Assign a new ID if old sticky is not found in new skeleton frame
/// </summary>
/// <param name="trackIDs">Array of skeleton tracking IDs</param>
void DataStreamSkeleton::AssignNewStickyIDs(DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& pSkeletonFrame)
{
    for (int i = 0; i < NUI_SKELETON_COUNT; i++)
    {
        if (trackIDs[FirstTrackID] && trackIDs[SecondTrackID])
        {
            break;
        }

        if (NUI_SKELETON_NOT_TRACKED != pSkeletonFrame.SkeletonData[i].eTrackingState)
        {
            DWORD trackID = pSkeletonFrame.SkeletonData[i].dwTrackingID;

            if (!trackIDs[FirstTrackID] && trackID != trackIDs[SecondTrackID])
            {
                trackIDs[FirstTrackID] = trackID;
            }
            else if (!trackIDs[SecondTrackID] && trackID != trackIDs[FirstTrackID])
            {
                trackIDs[SecondTrackID] = trackID;
            }
        }
    }
}

/// <summary>
/// Find most active skeletons to set tracked
/// </summary>
/// <param name="trackIDs">Array of skeleton tracking IDs</param>
void DataStreamSkeleton::ChooseMostActiveSkeletons(DWORD trackIDs[TrackIDIndexCount])
{
    ZeroMemory(trackIDs, TrackIDIndexCount * sizeof(DWORD));

    // Find out highest activity level IDs
    FindMostActiveIDs(trackIDs);
}

/// <summary>
/// Find most active IDs
/// </summary>
/// <param name="trackIDs">Array of skeleton tracking IDs</param>
void DataStreamSkeleton::FindMostActiveIDs(DWORD trackIDs[TrackIDIndexCount])
{
    // Initialize activity levels with negtive valus so they can replaced by any found activity levels
    FLOAT activityLevels[TrackIDIndexCount] = {-1.0f, -1.0f};

    // Compare to previously found activity levels
    if (m_activityLevel > activityLevels[FirstTrackID])
    {
        // Move first track ID and activity level to second place. Assign newly found higher activity level and ID to first place
        activityLevels[SecondTrackID] = activityLevels[FirstTrackID];
        activityLevels[FirstTrackID]  = m_activityLevel;

        trackIDs[SecondTrackID]       = trackIDs[FirstTrackID];
        trackIDs[FirstTrackID]        = m_dwActiveID1;
    }
    else if (m_activityLevel > activityLevels[SecondTrackID])
    {
        // Replace the previous one
        activityLevels[SecondTrackID] = m_activityLevel;
        trackIDs[SecondTrackID]       = m_dwActiveID2;
    }
}
