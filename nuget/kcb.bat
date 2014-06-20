ren KinectCommonBridge KinectCommonBridge%1
git clone https://github.com/MSOpenTech/KinectCommonBridge
cd KinectCommonBridge 
git checkout nuget
call nuget\buildkcb2012
call nuget\buildkcb2013
call nuget\cdnuget
call powershell -command "Write-NuGetPackage kcb.autopkg"
call updateversion
..\..\nuget push *.nupkg





