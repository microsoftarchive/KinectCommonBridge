ren KinectCommonBridge KinectCommonBridge%1
git clone https://github.com/MSOpenTech/KinectCommonBridge
cd KinectCommonBridge 
git checkout nuget
call nuget\buildkcb2012
call nuget\buildkcb2013
call nuget\cdnuget
call updateversion kcb.autopkg kcbx64.autopkg
call powershell -command "Write-NuGetPackage kcb.autopkg"
call powershell -command "Write-NuGetPackage kcbx64.autopkg"
call nugetpublish





