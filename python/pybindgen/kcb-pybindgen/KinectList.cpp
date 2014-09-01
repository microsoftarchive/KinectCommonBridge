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
#include "KinectList.h"

unsigned int KinectList::m_iNextHandleID = KCB_DEFAULT_HANDLE;

KinectList::KinectList(void)
{
    m_pKCBMap.clear();
}

KinectList::~KinectList(void)
{
    Shutdown();
}

void KinectList::Shutdown()
{
    for(auto iter = m_pKCBMap.begin(); iter != m_pKCBMap.end(); ++iter)
    {
        SAFE_RELEASE(iter->second);
    }
    m_pKCBMap.clear();
}

// will ref count the interface
KCBHANDLE KinectList::Add(_In_ IKCBSensor *pSensor)
{
    KCBHANDLE kcbHandle = m_iNextHandleID++;

    // ensure this was not already added
    auto iter = m_pKCBMap.find(kcbHandle);
    if (iter != m_pKCBMap.end())
    {
        m_iNextHandleID--;
        return KCB_INVALID_HANDLE;
    }

    m_pKCBMap.insert(std::make_pair(kcbHandle, pSensor));
    pSensor->AddRef();

    return kcbHandle;
}

HRESULT KinectList::Add(KCBHANDLE handle, _In_ IKCBSensor *pSensor)
{
    // ensure this was not already added
    auto iter = m_pKCBMap.find(handle);
    if (iter != m_pKCBMap.end())
    {
        return E_INVALIDARG;
    }

    m_pKCBMap.insert(std::make_pair(handle, pSensor));
    pSensor->AddRef();

    return S_OK;
}

// returns the sensor the client had passed
// caller to release the object
HRESULT KinectList::Remove(KCBHANDLE handle, _COM_Outptr_ IKCBSensor** ppSensor)
{
    auto iter = m_pKCBMap.find( handle );
    if( iter != m_pKCBMap.end() )
    {
        (*ppSensor) = iter->second;
        
        m_pKCBMap.erase(iter);
        
        return S_OK;
    }

    return E_NOINTERFACE;
}

// returns an addref'd sensor
// caller to release the object
HRESULT KinectList::Find(KCBHANDLE handle, _COM_Outptr_ IKCBSensor** ppSensor)
{
    auto iter = m_pKCBMap.find( handle );
    if( iter != m_pKCBMap.end() )
    {
        (*ppSensor) = iter->second;
        (*ppSensor)->AddRef();
        
        return S_OK;
    }

    return E_NOINTERFACE;
}
