#pragma once

#include "DataStreamDepth.h"

// Tracked player ID index
enum TrackIDIndex
{
    FirstTrackID = 0,
    SecondTrackID,
    TrackIDIndexCount
};


class DataStreamSkeleton
    : public DataStream
{
public:
    DataStreamSkeleton();
    virtual ~DataStreamSkeleton();

    virtual void Initialize(_In_ INuiSensor* pNuiSensor);
    virtual void Initialize(bool bSeated, KINECT_SKELETON_SELECTION_MODE mode, _In_ INuiSensor* pNuiSensor, _Inout_opt_ NUI_TRANSFORM_SMOOTH_PARAMETERS* pSmoothParams);

    virtual HRESULT StartStream();
    virtual void StopStream();
    virtual void PauseStream(bool bPaused);

    void SetSeatedMode( bool seated );
    void SetChooserMode( KINECT_SKELETON_SELECTION_MODE mode );

    HRESULT GetFrameData( _Inout_ NUI_SKELETON_FRAME& skeletonFrame );
	DWORD* GetTrackedIDs() { return m_stickyIDs; }

protected:
    virtual void CopyData(_In_ void* pImageFrame);

private:
    // compare the skeleton smooth params
    static bool IsEqual( NUI_TRANSFORM_SMOOTH_PARAMETERS& param1, NUI_TRANSFORM_SMOOTH_PARAMETERS& param2 );

    /// <summary>
    /// Update tracked skeletons according to current chooser mode
    /// </summary>
    void UpdateTrackedSkeletons( _In_ NUI_SKELETON_FRAME& skeletonFrame );

    /// <summary>
    /// Find sticky skeletons to set tracked
    /// </summary>
    /// <param name="trackIDs">Array of skeleton tracking IDs</param>
    void ChooseStickySkeletons( _In_ DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& skeletonFrame );

    /// <summary>
    /// Find closest skeleton to set tracked
    /// </summary>
    /// <param name="trackIDs">Array of skeleton tracking IDs</param>
    void ChooseClosestSkeletons( _In_ DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& skeletonFrame );

    /// <summary>
    /// Find most active skeletons to set tracked
    /// </summary>
    /// <param name="trackIDs">Array of skeleton tracking IDs</param>
    void ChooseMostActiveSkeletons( _In_ DWORD trackIDs[TrackIDIndexCount] );

    /// <summary>
    /// Verify if stored tracked IDs are found in new skeleton frame
    /// </summary>
    /// <param name="trackIDs">Array of skeleton tracking IDs</param>
    void FindStickyIDs( _In_ DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& skeletonFrame );

    /// <summary>
    /// Assign a new ID if old sticky is not found in new skeleton frame
    /// </summary>
    /// <param name="trackIDs">Array of skeleton tracking IDs</param>
    void AssignNewStickyIDs( _In_ DWORD trackIDs[TrackIDIndexCount], _In_ NUI_SKELETON_FRAME& skeletonFrame );

    /// <summary>
    /// Find most active IDs
    /// </summary>
    /// <param name="trackIDs">Array of skeleton tracking IDs</param>
    void FindMostActiveIDs( _In_ DWORD trackIDs[TrackIDIndexCount] );

private:
    bool    m_bSmoothParams;
    bool    m_seated;
    DWORD    m_stickyIDs[TrackIDIndexCount];
    KINECT_SKELETON_SELECTION_MODE m_chooserMode;
    bool    m_bAlignToColor;

    NUI_TRANSFORM_SMOOTH_PARAMETERS m_smoothParams;

    FLOAT   m_activityLevel;
    DWORD    m_dwActiveID1;
    DWORD    m_dwActiveID2;
};
