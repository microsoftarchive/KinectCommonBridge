Kinect Common Bridge 2.0
========================

## Introduction

Kinect Common Bridge 2.0 is a complement to the Kinect for Windows SDK 2.0 that makes it easy to integrate Kinect scenarios in creative development libraries and toolkits.

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

## Getting Started

Coming soon!!


## Additional Resources

* Kinect for Windows - Getting Started

	[http://msdn.microsoft.com/en-us/library/hh855354.aspx](http://msdn.microsoft.com/en-us/library/hh855354.aspx)

* KCB for Cinder developers

	[https://github.com/wieden-kennedy/Cinder-KCB](https://github.com/wieden-kennedy/Cinder-KCB)

* KCB for Open Frameworks developers

	[https://github.com/joshuajnoble/ofxKinectCommonBridge](https://github.com/joshuajnoble/ofxKinectCommonBridge)
