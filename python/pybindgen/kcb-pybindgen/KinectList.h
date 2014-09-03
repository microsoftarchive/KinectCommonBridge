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

#include <map>
#include "KCBSensor.h"

class KinectList
{
public:
    KinectList();
    ~KinectList();

    KCBHANDLE Add(_In_ IKCBSensor *pKCB);
    HRESULT Remove(KCBHANDLE handle, _COM_Outptr_ IKCBSensor** pKCB);
    HRESULT Find(KCBHANDLE handle, _COM_Outptr_ IKCBSensor** pKCB);

    // Accessor.
    UINT32 GetCount() const { return static_cast<UINT32>(m_pKCBMap.size()); }

protected:
    void Shutdown();
    HRESULT Add(KCBHANDLE handle, _In_ IKCBSensor* pKCB);

private:
    static unsigned int m_iNextHandleID;
    std::map<KCBHANDLE, IKCBSensor*> m_pKCBMap;

};

