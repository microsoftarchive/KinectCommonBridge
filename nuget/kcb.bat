ren KinectCommonBridge KinectCommonBridge%1
git clone https://github.com/MSOpenTech/KinectCommonBridge
cd KinectCommonBridge 
git checkout nuget
call nuget\buildkcb2012
call nuget\buildkcb2013
call powershell -command "Write-NuGetPackage nuget\kcb.autopkg"
call powershell -command "Write-NuGetPackage nuget\kcbx64.autopkg"
call nuget\updateversion
..\..\nuget push *.nupkg





