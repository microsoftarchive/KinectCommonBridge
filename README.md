Kinect Common Bridge 2.0
========================

**Table of Contents**

- [Introduction](#introduction)
- [Requirements](#requirements)
- [Troubleshooting](#troubleshooting)
- [Samples](#samples)
- [Additional Resources](#additional-resources)

## Introduction

[The new Kinect for Windows Sensor](http://msopentech.com/blog/2014/06/11/get-your-hands-on-the-kinect-common-bridge-v2-beta/) is now available to everyone, with a brand new SDK for developers, and MS Open Tech has updated the Kinect Common Bridge (KCB) open source project so that C++ developers may more easily build desktop apps integrating advanced Kinect functionality.  

**Kinect Common Bridge 2.0** is a complement to the Kinect for Windows SDK 2.0 that makes it easy to integrate Kinect scenarios in creative development libraries and toolkits.

## Requirements

The Hardware and Software below are required to build the library:

- A **Kinect for Windows V2 sensor**:
	A Kinect for Windows V2 Sensor is required to capture sound and images that will be used by the Kinect Common Bridge 2.0 on top of the Kinect for Windows SDK. You can find info about the Kinect For Windows sensor on [http://www.microsoft.com/en-us/kinectforwindows/Purchase/developer-sku.aspx](http://www.microsoft.com/en-us/kinectforwindows/Purchase/developer-sku.aspx)

- **Visual Studio**:
	The library builds with Visual Studio versions 2012 Express and above. 
	
- **Kinect for Windows SDK**:
	In order to build this library, you first need to download the Kinect for Windows SDK V2 (download link coming soon). 

## Troubleshooting

Installing the SDK automatically sets up environment variable ```KINECTSDK20_DIR``` used within solution settings. If for some reason it doesn’t work for you please see additional steps below:

 - Update ```Settings->VC++ Directories``` to use direct path to SDK instead of  the environment variable mentioned above. 
 Default is ```C:\Program Files\Microsoft SDKs\Kinect\v2.0-DevPreview1406\```

 - Add ```Kinect.lib``` to ```Settings->Linker->Input->Additional Dependencies``` to fix a linker error.

## Samples

In addition to the work on KCB, MS Open Tech and the Kinect for Windows product group have also set up an open source repository on GitHub
 to gather [community-contributed samples](https://github.com/MSOpenTech/Kinect-for-Windows-Samples). 
 
The process to submit a new sample is actually fairly simple: fork, sign CLA, pull request, wait for approval…. Initial examples will be added shortly, and all of these will be ready to be forked! 

 1. Kinect Evolution (includes ground plane/joint orientation)
 2. "Construct" (3D World)
 3. HD Face Tracking/Avateering
 4. Add Kinect Interactions to a Windows Store App
 5. Background Removal
 6. Real-world Object Interaction (hotspot detection)
 7. Exercise Recognition
 8. Multi-user Engagement
 9. Adaptive UI
 10. Interactions Gallery
 11. Kinect Studio
 12. Augmented Reality
 13. Camera View based on Face Orientation
 14. Cross-platform Shooter Sample
 15. Fusion (body scanning)
 16. Fusion Explorer
 17. Twitch-based Game Experience

You can see that will be a great start. Yet we are eager to see what comes out of this community in terms of contribution, comments and creativity :)

Some ideas for samples that might be interesting are:

 - Exercise recognition – ask the user to move in a particular way and determine if the user actually did that. This can be useful in a home exercise app.
 - Adaptive UI – show how a UI can change based on distance from the sensor.  For example, at 10 ft away, the UI shows a couple of large buttons to interact with.  As the user moves closer, the UI changes to show content and support interactions that are more granular.  The Adaptive UI sample in K4W v1.8 is a good starting point.
 - Multi-user Engagement – build an app that shows how to handle multiple bodies being detected, such that each can interact with the app without the app confusing them
 - Real-world Object Interaction (hotspot detection) – show how to use Kinect for Windows to detect when one object gets closer to another object in the real world.
 - ….

###An Added Incentive###
And if inventing new ways to take advantage of the new Kinect for Windows is not enough, we can help you prove to your friends and family that at least some of your “playtime” time on the computer is productive. We’ll contact you after the contribution is accepted and you'll be able to earn a coveted “Kinect certified contributor ” t-shirt .

## Additional Resources

* Kinect for Windows - Getting Started

	[http://msdn.microsoft.com/en-us/library/hh855354.aspx](http://msdn.microsoft.com/en-us/library/hh855354.aspx)

* KCB for Cinder developers

	[https://github.com/wieden-kennedy/Cinder-KCB](https://github.com/wieden-kennedy/Cinder-KCB)

* KCB for Open Frameworks developers

	[https://github.com/joshuajnoble/ofxKinectCommonBridge](https://github.com/joshuajnoble/ofxKinectCommonBridge)
