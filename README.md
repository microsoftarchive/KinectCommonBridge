Kinect Common Bridge 
====================

## Introduction

Kinect Common Bridge is a complement to the Kinect for Windows SDK that makes it easy to integrate Kinect scenarios in creative development libraries and toolkits.

### Why KCB?

When working with the openFrameworks and Cinder community members, it was evident that they needed something similar to the managed APIs but for C++. The graphics libraries they use are written entirely in native C++ for “down to the bare metal” performance to accomplish their craft. As for the functionality, they wanted something lightweight to keep the extensions to their libraries as lightweight as possible. 
If you are not familiar with these libraries or any type of game development model, they do not have a typical application design pattern. They need to run as fast as possible to run simulations, update positions of objects, and then render those on screen either as fast as possible or locked in sync with the refresh of the display. This can run at typical 60 frames per second (fps) and as high as the CPU/GPU can handle.

Many familiar with Kinect know the maximum frame rate is 30fps. Using an event based model doesn’t work well for this type of development since it needs to grab the frame of data when it wants, regardless of what Kinect is doing and if it isn’t there, it will catch it next time around. It cannot block the thread that does this update/query cycle.

Taking a look at the common use case scenarios, the common tasks when working with the Kinect for Windows SDK and the device are: 

1.	Select a sensor
2.	Get the color/IR, depth, and skeleton data from it.

That was the goal of KCB: allow any framework that is capable of loading the DLL direct access to the data.


## Requirements

The Hardware and Software below are required to build the library:

- A **Kinect for Windows sensor**:
	A Kinect for Windows Sensor is required to capture sound and image that will be used by the Kinect Common Bridge on top of the Kinect for Windows SDK. You can find info about the Kinect For Windows sensor on [http://www.microsoft.com/en-us/kinectforwindows/Purchase/Overview.aspx](http://www.microsoft.com/en-us/kinectforwindows/Purchase/Overview.aspx)

- **Visual Studio**:
	The library builds with Visual Studio versions 2010 and 2012 Express and above. You find download links on this page: [http://www.microsoft.com/en-us/kinectforwindowsdev/Downloads.aspx](http://www.microsoft.com/en-us/kinectforwindowsdev/Downloads.aspx)
	
- **Kinect for Windows SDK**:
	In order to build this library, you first need to download the Kinect for Windows SDK from [http://www.microsoft.com/en-us/kinectforwindowsdev/Downloads.aspx](http://www.microsoft.com/en-us/kinectforwindowsdev/Downloads.aspx). You can also find more information on the system requirements to install and use the Kinect SDK on this page: [http://msdn.microsoft.com/en-us/library/hh855359.aspx](http://msdn.microsoft.com/en-us/library/hh855359.aspx)
	
Additionally, to take advantage of the face tracking and speech recognition capabilities you need to install:

- **Speech Server SDK**:
	It is available at [http://www.microsoft.com/en-us/download/details.aspx?id=27226](http://www.microsoft.com/en-us/download/details.aspx?id=27226). Note that depending on the OS version and target platform that you are building for, you may need to have either x86, or x64, or both on your machine.

- **Kinect for Windows Developer Toolkit**:
    It is available at [http://go.microsoft.com/fwlink/?LinkID=323589](http://go.microsoft.com/fwlink/?LinkID=323589) and is necessary for face tracking functionality. After the installation, make sure that the `KINECT_TOOLKIT_DIR` environment variable is set. Usually its value will be something like `C:\Program Files\Microsoft SDKs\Kinect\Developer Toolkit v1.8.0`. 
	**Tip:** reboot your machine after installation even if Windows does not prompt you. Environment variables may not be updated until you do so, causing build errors.

## Getting Started

Once you have built `KinectCommonBridge.dll`, you can load it in your C++ app and start grabbing a frame of data with literally 5 lines of code:

    KCBHANDLE kcbHandle = KinectOpenDefaultSensor();
    
    KINECT_IMAGE_FRAME_FORMAT format = { sizeof(KINECT_IMAGE_FRAME_FORMAT), 0 };
    KinectGetColorFrameFormat( kcbHandle, &format );
    
    BYTE* pColorBuffer = new BYTE[format.cbBufferSize];
    
    KinectGetColorFrame( kcbHandle, format.cbBufferSize, pColorBuffer, null );
    

To make this more realistic, let's just add an infinite loop that gets the time stamps:


    LONGLONG timeStamp = 0;
    while( true )
    {
    	if( KinectIsColorFrameReady(kcbHandle) && SUCCEEDED( 
				KinectGetColorFrame( kcbHandle, format.cbBufferSize, pColorBuffer, &timeStamp ) ) )
		{
    		printf( "Color frame acquired:  %I64u\r\n", timeStamp );
   		}
    }
    

## More advanced functionality: face tracking and voice recognition

KCB has additional support for more advanced features of the sensor such as face tracking and voice recognition. Check out the samples folder for working code that illustrates how to get up and running quickly. 

## Additional Resources

* Kinect for Windows - Getting Started

	[http://msdn.microsoft.com/en-us/library/hh855354.aspx](http://msdn.microsoft.com/en-us/library/hh855354.aspx)

* KCB for Cinder developers

	[https://github.com/wieden-kennedy/Cinder-KCB](https://github.com/wieden-kennedy/Cinder-KCB)

* KCB for Open Frameworks developers

	[https://github.com/joshuajnoble/ofxKinectCommonBridge](https://github.com/joshuajnoble/ofxKinectCommonBridge)