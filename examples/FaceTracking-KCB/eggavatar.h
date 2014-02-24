//------------------------------------------------------------------------------
// <copyright file="eggavatar.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

static const float EyeInside = 1.0f/18.0f;
static const float EyeOutside = 1.0f/6.0f;
static const float EyeTop = 1.0f/40.0f;
static const float EyeBottom = -1.0f/40.0f;
static const float MouthRight = (EyeInside+EyeOutside)/2.0f;
static const float MouthLeft = -MouthRight;
static const float MouthVertical = -1.7f*(EyeOutside-EyeInside);
static const float EyebrowBottom = EyeTop + 1.0f/16.0f;
static const float EyebrowTop = EyebrowBottom + EyeTop/2.0f;
static const float EyebrowInside = EyeInside;
static const float EyebrowOutside = EyeOutside + 0.1f/16.0f;
static const float HairBottom = (0.5f+EyebrowTop)/2.0f;
static const float NoseTop = 0;
static const float NoseBottom = NoseTop - (EyeOutside-EyeInside);
static const float EyeCenterDepthCorrection = (7.0f/7.5f);
static const float PupilRadius = (3.14f/40.0f);
static const float HeadPoseSmoothingFactorMin = 0.001f;
static const float HeadPoseTranslationSmoothing = 0.05f;
static const float HeadPoseTranslationTrigger = 0.2f; // 20 centimeters?



static const int PointsPerEyeLid = 8;
static const int NumberEyePoints = 2*PointsPerEyeLid;
static const int NumberEyebrowPoints = 9;
static const int PointsPerLip = 8;
static const int NumberMouthPoints = 2*PointsPerLip;
static const int NumberNosePoints = 5;
static const int NumberCirclePoints = 64;
static const int PointPerSingleHair = 8;
static const int NumberOfHairs = 16;
static const int NumberFacePoints = 2*NumberEyePoints+2*NumberEyebrowPoints+NumberMouthPoints+NumberNosePoints+PointPerSingleHair*NumberOfHairs;
static const int PointsPerPupil = 32;
static const int NumberTotalPoints = NumberFacePoints + NumberCirclePoints + 2*PointsPerPupil;
static const int RightEyeFirstPoint = 0;
static const int LeftEyeFirstPoint = RightEyeFirstPoint+NumberEyePoints;
static const int RightEyebrowFirstPoint = LeftEyeFirstPoint+NumberEyePoints;
static const int LeftEyebrowFirstPoint = RightEyebrowFirstPoint+NumberEyebrowPoints;
static const int MouthFirstPoint = LeftEyebrowFirstPoint+NumberEyebrowPoints;
static const int NoseFirstPoint = MouthFirstPoint+NumberMouthPoints;
static const int HairFirstPoint = NoseFirstPoint+NumberNosePoints;
static const int RightPupilFirstPoint = NumberFacePoints;
static const int LeftPupilFirstPoint = RightPupilFirstPoint+PointsPerPupil;
static const int CircleFirstPoint = LeftPupilFirstPoint+PointsPerPupil;;

struct IFTImage;

class EggAvatar
{
public:
    EggAvatar(void);

    BOOL SetCandideAU(const float * AU, const int numberAU);
    BOOL SetRandomAU();

    BOOL SetRotations(const float pitchDegrees, const float yawDegrees, const float rollDegrees);
    BOOL SetRandomRotations();

    BOOL SetTranslations(const float tX, const float tY, const float tZ);

    BOOL SetScaleAndTranslationToWindow(int height, int width);
    void SetScale(float scale) { m_Scale = scale;}
    void SetTranslationX(float X){ m_TranslationX = X;}
    void SetTranslationY(float Y){ m_TranslationY = Y;}

    BOOL DrawImage(IFTImage* pImage);
    BOOL DrawBgLine(IFTImage* pImage, float x1, float y1, float x2, float y2, UINT32 color);

private:
    float m_FacePointLatLon[NumberFacePoints][2];
    float m_FacePointXYZ[NumberTotalPoints][3];

    // Weight of the different animated part in the egg avatar model
    float m_JawDrop;
    float m_UpperLipLift;
    float m_MouthStretch;
    float m_MouthCornerLift;
    float m_BrowLower;
    float m_OuterBrowRaiser;
    float m_UpperEyeLid;
    float m_LowerEyeLid;

    float m_Pitch;
    float m_Yaw;
    float m_Roll;
    float m_Scale;
    float m_TranslationX;
    float m_TranslationY;
    bool m_FacingUser;
    // Variables used for smoothing the head pose.
    bool m_HeadPoseFiltering;
    float m_ReportedPitchAverage;
    float m_ReportedYawAverage;
    float m_ReportedRollAverage;
    float m_TxAverage;
    float m_TyAverage;
    float m_TzAverage;
    unsigned int m_SamePositionCount;


    void LatLonEye(const bool left);
    void LatLonEyeBrow(const bool left);
    void LatLonMouth();
    void LatLonNose();
    void LatLonHair();
    void LatLonYaw();
    void LatLonToXYZ();
    void RollXYZ();
    void PitchXYZ();
    void CircleXYZ();
    void PupilCenter(float center[3], bool left);
    bool CanTrackPupil();
    void PupilXYZ(const bool left);
    void ScaleXYZ();
    void TranslateXYZ();
    void DrawHair(IFTImage* pImage, UINT32 color);
    void DrawPupil(IFTImage* pImage, bool left, UINT32 color);
    void DrawCurve(IFTImage* pImage, int firstPoint, int numberPoints, bool shouldClose, UINT32 color);
    void DrawSegment(IFTImage* pImage, int firstPoint, int secondPoint, UINT32 color);

    bool PointInsideCurve(float x, float y, int firstCurvePoint, int numberCurvePoints);

};
