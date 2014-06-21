 
This is run from the github directory currently.

0) Usage: kcb x
1) Renames the current KinectCommonBridge branch to backup (appending x).
2) Clones the latest KinectCommonBridge (or any repository).
3) Builds for 2012 and 2013 (this copies the files we need into the output directory for Nuget).
4) Builds updatenugetpackage.exe.
5) Runs updatenugetpackage.exe on two specified autopackage files (kcb and kcbx64).
6) Calls powershell and pipes in the commands to build both Nuget packages (one for x64, which we might merge now that there is no longer a size restriction).
7) Calls nuget to publish the packages.
8) Publishes the updates autopkg file on github (this requires the user enter their username and password (about 5 minutes) after running the script).

The only catches are:

1) The user has to first register the user with: nuget setapikey (it is a GUID found in their online account settings).

2) The user has to have nuget in their path. We could probably set the path ourselves to automate this.
The author put nuget.exe in the github directory but an environment variable would be better.



