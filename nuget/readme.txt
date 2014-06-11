 
This is run from the github directory currently.

1) Renames the current KinectCommonBridge branch to backup.

2) Clones the latest KinectCommonBridge (or any repository).

3) Builds for 2012 and 2013 (this copies the files we need into the output directory for Nuget).

4) Calls powershell and pipes in the commands to build both Nuget packages (one for x64, which we might merge now that there is no longer a size restriction).

5) Calls nuget to publish the packages.

 

The only catches are:

 

1) The user has to first register the user with the nuget setapikey

(Not sure if there is a command line way to fetch that?)
 

2) The user has to have nuget in their path. We could probably set the path ourselves to automate this.

3) The user must (for now) manually update the version number in the autopkg file.  I could write a program to automate that in C, but there is probably an easier way, maybe in nuget?

Below is the batch - any suggestions welcome.

