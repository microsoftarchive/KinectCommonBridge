/***********************************************************************************************************
Copyright © Microsoft Open Technologies, Inc.
All Rights Reserved        
Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
except in compliance with the License. You may obtain a copy of the License at 
http://www.apache.org/licenses/LICENSE-2.0 

THIS CODE IS PROVIDED *AS IS* BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER 
EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION ANY IMPLIED WARRANTIES OR 
CONDITIONS OF TITLE, FITNESS FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT. 

See the Apache 2 License for the specific language governing permissions and limitations under the License.
***********************************************************************************************************/

#pragma once

#include "KinectSensor.h"

class SensorManager
{
public:
    ~SensorManager();
    // get the singleton instance
    static std::shared_ptr<SensorManager> GetInstance();    
    
    // gets a handle to a sensor
    KCBHANDLE OpenDefaultSensor();
    KCBHANDLE OpenSensorByPortID( _In_z_ const WCHAR* wcPortID );
    
    // closes the sensor and removes the handle
    void CloseSensorHandle( KCBHANDLE& kcbHandle );

    // converts the handle to a instance of the class
    bool GetKinectSensor( KCBHANDLE kcbHandle, std::shared_ptr<KinectSensor>& pSensor );

    // find the senosr count
    UINT GetSensorCount();
    bool GetPortIDByIndex( UINT index, ULONG cchPortID, _Out_cap_(cchPortID) WCHAR* pwcPortID );

private:
    SensorManager();
    void Initialize();

    // get the next handle id 
    KCBHANDLE GetHandle( _In_z_ const WCHAR* wcPortID );

    std::shared_ptr<KinectSensor> GetDefaultSensor();
    std::shared_ptr<KinectSensor> GetSensor( _In_z_ const WCHAR* wcPortID );

    // create list of available sensors on startup
    void CreateListOfAvailableSensors();

    // managing the list of instances of KinectSensor objects
    bool AddSensorToList( _In_z_ const WCHAR* wcPortID );

    // message handler from Nui
    static void CALLBACK NuiStatusCallback( HRESULT hrStatus, const OLECHAR* connectionId, const OLECHAR* uniqueDeviceName, void* pUserData );
    
    // decides what to do with the notification
    void UpdateSensorOnList( _In_z_ const WCHAR* wcPortID, HRESULT hrStatus );

private:
    static CriticalSection m_managerLock; 
    static std::shared_ptr<SensorManager> m_pInstance; // singleton

    const WCHAR*    m_wcTempPortID;

    CriticalSection    m_csSensorLock;
    CriticalSection    m_csHandleLock;

    int m_iHandleCount;
    std::map<int, std::wstring> m_handleMap;

    std::map<std::wstring, std::shared_ptr<KinectSensor> > m_kinectSensors;
};
