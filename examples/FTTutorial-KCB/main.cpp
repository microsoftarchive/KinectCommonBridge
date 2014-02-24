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

    // enable face tracking
    // set near mode to true
    HRESULT hr = KinectEnableFaceTracking(kcbHandle, true);
    if(FAILED(hr))
    {
         printf("KinectEnableFaceTracking: unable to enable face tracking. Error: %ld\r\n", hr);
    }

    printf("Move your head around until you get some results...\r\n", hr);

     // Update() and Draw():
    {
        // popular graphics frameworks may have Update() and Draw() functions
        // this is to simulate that functionality
        bool quit = false;
        int loopCount = 200000; 
        LONGLONG timeStamp = 0;

        while( !quit )
        {
            if(KinectIsFaceTrackingResultReady(kcbHandle))
            {
                IFTResult* pResult;
                if (SUCCEEDED(KinectGetFaceTrackingResult(kcbHandle, &pResult)) && SUCCEEDED(pResult->GetStatus()))
                {
                    RECT r;
                    pResult->GetFaceRect(&r);
                    printf( "Face Tracking result acquired:  Face rect: %d %d %d %d\r\n", r.left, r.top, r.right, r.bottom );
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
        KinectCloseSensor(kcbHandle); // release the handle
    }
}


