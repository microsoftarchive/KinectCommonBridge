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

#include "stdafx.h"

#include "SensorManager.h"
#include "AutoLock.h"

// initialized the static variables
std::shared_ptr<SensorManager> SensorManager::m_pInstance(nullptr);
CriticalSection SensorManager::m_managerLock;

// called to acquire the singleton
std::shared_ptr<SensorManager> SensorManager::GetInstance()
{
    m_managerLock.Lock();

    if( nullptr == m_pInstance )
    {
        m_pInstance.reset( new (std::nothrow) SensorManager() );
        m_pInstance->Initialize();
    }

    m_managerLock.UnLock();

    return m_pInstance;
}

// Ctor
SensorManager::SensorManager()
    : m_wcTempPortID( L"USB\\VID_0000&PID_0000\\0" )
    , m_iHandleCount( 0x0000f000 )
{
    m_handleMap.clear();
    m_kinectSensors.clear();
}

// Dtor
SensorManager::~SensorManager()
{
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    // explicit clean up of all the sensor objects
    for (auto iter = m_kinectSensors.begin(); iter != m_kinectSensors.end(); ++iter)
    {
        // release the created pointer
        iter->second.reset();
    }

    m_handleMap.clear();
    m_kinectSensors.clear();
}

// when the instance is first created, it will create an initial
// set of sensor objects
void SensorManager::Initialize()
{
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    CreateListOfAvailableSensors();

    // Set the sensor status callback
    NuiSetDeviceStatusCallback( NuiStatusCallback, this );
}

// called once on initialization
void SensorManager::CreateListOfAvailableSensors()
{
    // gets the count of sensors from Kinect for Windows API
    int iCount = 0;
    HRESULT hr = NuiGetSensorCount( &iCount );
    if( FAILED(hr) )
    {
        return;
    }

    // if there are no sensors, create a temporary one
    if( 0 == iCount )
    {
        // when the first sensor is connected, it will be added
        // with the correct id
        if( m_kinectSensors.size() == 0 )
        {
            AddSensorToList( m_wcTempPortID );
        }
    }

    // continue with the known sensors to the list
    for (int i = 0; i < iCount; ++i)
    {
        ComSmartPtr<INuiSensor> pNui;
        if( SUCCEEDED( NuiCreateSensorByIndex(i, &pNui) ) )
        {
            const WCHAR* wcPortID = pNui->NuiDeviceConnectionId();  // contains the connections string for the device
                                                                    // plugged into a specific USB port. 
                                                                    // removing the sensor and using a different port will have
                                                                    // a different ID.
            AddSensorToList( wcPortID );
        }
    }

}

// creates the instance of the wrapper class based on the PortID
bool SensorManager::AddSensorToList( _In_z_ const WCHAR* wcPortID )
{    
    // basic checks on the PortID to see that it is valid
    if( nullptr == wcPortID || *wcPortID == '\0' )
    {
        return false;
    }

    // create the Kinect Sensor Wrapper
    std::shared_ptr<KinectSensor> pSensor = std::shared_ptr<KinectSensor>( new (std::nothrow) KinectSensor(wcPortID) );
    if( nullptr == pSensor ) // E_OUTOFMEMORY
    {
        return false;
    }

    m_kinectSensors.insert( std::make_pair(wcPortID, pSensor) );

    return true;
}

// for tracking new sensors that connect to the system
void SensorManager::NuiStatusCallback( HRESULT hrStatus, const OLECHAR* connectionId, const OLECHAR* uniqueDeviceName, void* pUserData )
{
    SensorManager* pThis = nullptr;
    pThis = reinterpret_cast<SensorManager*>(pUserData);
    if( nullptr != pThis )
    {
        pThis->UpdateSensorOnList(connectionId, hrStatus);
    }
}
void SensorManager::UpdateSensorOnList( _In_z_ const WCHAR* wcPortID, _In_ HRESULT hrStatus )
{
    // set the lock on the list
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    // find the sensor id on the list
    auto iter = m_kinectSensors.find( wcPortID );
    if( iter == m_kinectSensors.end() )
    {
        // we didn't find this port id of a sensor
        // look for a temporary sensor that we can assign this to

        // select the first one on the list 
        iter = m_kinectSensors.begin();
        if( m_kinectSensors.size() == 1 && !iter->first.compare( this->m_wcTempPortID ) )
        {
            // hold on to the instance of the sensor
            // have to re-add it to the list with the correct PortID
            auto pSensor = iter->second;
            assert( nullptr != pSensor );

            // has a handle been issued for the temporary sensor
            std::wstring wsPortID( pSensor->GetPortID() );
            for( auto handleIter = m_handleMap.begin(); handleIter != m_handleMap.end(); ++handleIter )
            {
                if( 0 == wsPortID.compare( handleIter->second ) )
                {
                    // update the portID for the handle
                    handleIter->second = wcPortID;
                    break;
                }
            }

            // now remove it from the sensor map
            // set the lock/wait to modify the map of sensors
            m_kinectSensors.erase( iter );

            // add it with the correct id
            m_kinectSensors.insert( std::make_pair(wcPortID, pSensor) );
        }
        else
        {
            // wasn't a sensor available based on the wcPortID
            // so we can create one since the id seems valid
            AddSensorToList( wcPortID );
        }

        // should be there now since we can account to all id's
        // on the system
        iter = m_kinectSensors.find( wcPortID );
    }

    // notify the wrapper of the change
    iter->second->NuiStatusNotification( iter->first.c_str(), hrStatus );
}

// gets the first available sensor that is not in use.
// in the instance that there are multiple sensors we try each one before
// defaulting to the first one
std::shared_ptr<KinectSensor> SensorManager::GetDefaultSensor()
{    
    auto iter = m_kinectSensors.begin();    // there will always be one in the list
    for(; iter != m_kinectSensors.end(); ++iter)
    {
        // check to see if we can use it; 
        if (m_kinectSensors.size() == 1 || !iter->second->IsStarted())
        {
            // it is available so return this one;
            break;
        }
    }

    // if we have gone through all of them and reached the end
    // then select the first one
    if( iter == m_kinectSensors.end() )
    {
        iter = m_kinectSensors.begin();
    }

    // return the pointer to the sensor
    return iter->second;
}

// gets an instance of the sensor for a particular port string that is 
// found by enumerating or one that is already known
std::shared_ptr<KinectSensor> SensorManager::GetSensor( _In_z_ const WCHAR* wcPortID )
{
    if( nullptr == wcPortID )
    {
        return nullptr;
    }

    auto iter = m_kinectSensors.find( wcPortID );
    if( iter == m_kinectSensors.end() )
    {
        // if the id has not been found, add it to our list to track
        if( AddSensorToList(wcPortID) )
        {
            iter = m_kinectSensors.find( wcPortID );
        }
        else
        {
            return nullptr;
        }
    }

    return iter->second;
}

bool SensorManager::GetKinectSensor( KCBHANDLE kcbHandle, std::shared_ptr<KinectSensor>& pSensor  )
{
    assert( nullptr == pSensor );

    if( KCB_INVALID_HANDLE == kcbHandle )
    {
        return false;
    }

    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    // find the port id for the handle
    auto iter = m_handleMap.find( kcbHandle );
    if( iter == m_handleMap.end() )
    {
        return nullptr;
    }

    // from the Port ID, gets the instance of the sensor object
    auto sensorIter = m_kinectSensors.find( iter->second );

    assert( nullptr != sensorIter->second );

    pSensor = sensorIter->second;

    return true;
}

// provides the wrapper a handle to the C api
KCBHANDLE SensorManager::OpenDefaultSensor()
{
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    auto pSensor = GetDefaultSensor();

    assert( nullptr != pSensor );

    // get a handle for this port id
    KCBHANDLE kcbHandle = GetHandle( pSensor->GetPortID() );

    // set the select flag on the sensor
    pSensor->Open();

    return kcbHandle;
}

// create the handle with a known PortID
KCBHANDLE SensorManager::OpenSensorByPortID( _In_z_ const WCHAR* wcPortID )
{
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    auto pSensor = GetSensor( wcPortID );
    if( nullptr == pSensor )
    {
        return KCB_INVALID_HANDLE;
    }
    
    // get a handle for this port id
    KCBHANDLE kcbHandle = GetHandle( pSensor->GetPortID() );

    // open the default streams
    pSensor->Open();

    return kcbHandle;
}

// gets a handle for the port, should not have duplicate handles
KCBHANDLE SensorManager::GetHandle( _In_z_ const WCHAR* wcPortID )
{
    // do we already have a handle to this instance
    std::wstring wsPortID( wcPortID );
    for( auto iter = m_handleMap.begin(); iter != m_handleMap.end(); ++iter )
    {
        if( wsPortID.compare( iter->second ) == 0 )
        {
            return iter->first;
        }
    }
    
    // not already a handle for this id
    // add it to our list, increase the count and return
    m_handleMap.insert( std::make_pair(m_iHandleCount, wsPortID) );

    return m_iHandleCount++;
}

void SensorManager::CloseSensorHandle( KCBHANDLE& kcbHandle )
{
    assert( KCB_INVALID_HANDLE != kcbHandle );

    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    auto iter = m_handleMap.find( kcbHandle );
    if( iter == m_handleMap.end() )
    {
        return;
    }

    // found the handle, find the sensor for this handle
    KinectSensor* pSensor = nullptr;
    auto sensorIter = m_kinectSensors.find( iter->second );
    if( sensorIter != m_kinectSensors.end() )
    {
        sensorIter->second->Close();
    }

    // remove the handle from the list
    m_handleMap.erase(iter);

    kcbHandle = KCB_INVALID_HANDLE;
}

// gets the current amount of senosrs on the list
UINT SensorManager::GetSensorCount() 
{ 
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    UINT count = (UINT)m_kinectSensors.size(); 

    return count;
}

// gets the PortID based on the number of sensors it found.
bool SensorManager::GetPortIDByIndex( UINT index, ULONG cchPortID, _Out_cap_(cchPortID) WCHAR* pwcPortID )
{
    // set the lock
    AutoLock sensorLock( m_csSensorLock );
    AutoLock handleLock( m_csHandleLock );

    if( index >= m_kinectSensors.size() )
    {
        return false;
    }

    UINT count = 0;
    for(auto iter = m_kinectSensors.begin(); iter !=  m_kinectSensors.end(); ++iter )
    {
        if( count++ == index )
        {
            wcsncpy_s( pwcPortID, cchPortID, iter->first.c_str(), iter->first.size() );
            break;
        }
    }

    return true;
}
