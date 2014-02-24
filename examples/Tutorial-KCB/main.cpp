// Tutorial-KCB.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "KinectCommonBridgeLib.h"

void MinimalCode();

int _tmain(int argc, _TCHAR* argv[])
{
    MinimalCode();

    return 0;
}

void MinimalCode()
{
    // Setup:
    // get a handle to the sensor you wish to use
    printf( "Opening default Kinect sensor...\r\n");
    KCBHANDLE kcbHandle = KinectOpenDefaultSensor();
    if( KCB_INVALID_HANDLE == kcbHandle )
    {
        // this rarely happens and may be a memory issue typically
        return;
    }

    // Create an empty IMAGE_FRAME_FORMAT structure to get the details of the stream you wish to use
    KINECT_IMAGE_FRAME_FORMAT format = { sizeof(KINECT_IMAGE_FRAME_FORMAT), 0 }; // populate dwStructSize with the size in bytes    
    KinectGetColorFrameFormat( kcbHandle, &format );

    // create the buffer, this will be the data structure you pass into the framework and data will be copied to it
    BYTE* pColorBuffer = new BYTE[format.cbBufferSize];
   
    // Update() and Draw():
    {
        // popular graphics frameworks may have Update() and Draw() functions
        // this is to simulate that functionality
        bool quit = false;
        int loopCount = 200000; 
        LONGLONG timeStamp = 0;

        while( !quit )
        {
            // Update() function
            {
                if( KinectIsColorFrameReady(kcbHandle) && SUCCEEDED( KinectGetColorFrame( kcbHandle, format.cbBufferSize, pColorBuffer, &timeStamp ) ) )
                {
                    // ProcessColorFrameData(&format, pColorBuffer); // may need to convert from BGR to RGB formats
                    // we will just output the timestamp of the frame for demostration
                    printf( "Color frame acquired:  %I64u\r\n", timeStamp );
                }
            }

            // Draw()
            {
                // simulates calling some function to a draw routine
                // * DO NOT DO THIS IN YOUR REAL CODE *
		        Sleep(15); 

            }
            // check for exit criteria
		    if( loopCount-- < 0 )
            {
			    quit = true;
            }

        }
    }

    // Shutdown:
    {
        // here we clean-up any objects we were using.
        delete [] pColorBuffer;		// clean up your buffer
        KinectCloseSensor(kcbHandle); // release the handle
    }
}


