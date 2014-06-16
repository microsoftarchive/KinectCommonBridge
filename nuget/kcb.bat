ren KinectCommonBridge KinectCommonBridge%1
git clone https://github.com/MSOpenTech/KinectCommonBridge
cd KinectCommonBridge 
git checkout nuget
call buildkcb2012
call buildkcb2013
call powershell -command "Write-NuGetPackage kcb.autopkg"
call powershell -command "Write-NuGetPackage kcbx64.autopkg"
call updateversion
..\nuget push *.nupkg





