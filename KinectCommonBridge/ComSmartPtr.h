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

// require <assert>
#include "stdafx.h"

#ifndef _COMSMARTPTR_H_
#define _COMSMARTPTR_H_

#include <bemapiset.h>
#include <unknwn.h>

template<class INTERFACE, const IID* piid = nullptr>
class ComSmartPtr
{
public:
	ComSmartPtr(){m_Ptr = nullptr;}
	
	//TODO: try 'explicit'
	ComSmartPtr(INTERFACE* lPtr)
	{
		m_Ptr = nullptr;

		if (lPtr != nullptr)
		{
			m_Ptr = lPtr;
			m_Ptr->AddRef();
		}
	}

	ComSmartPtr(const ComSmartPtr<INTERFACE, piid>& refComPtr)
	{
		m_Ptr = nullptr;
		// using static cast here since conversion operator is explicitly defined
		m_Ptr = static_cast<INTERFACE*>(refComPtr);

		if (m_Ptr)
		{
			m_Ptr->AddRef();
		}
	}

	ComSmartPtr(IUnknown* pIUnknown, IID iid)
	{
		m_Ptr = nullptr;

		if (pIUnknown != NULL)
		{
			//TODO: reinterpret_cast
			pIUnknown->QueryInterface(iid, (void**)&m_Ptr);
		}
	}

	~ComSmartPtr()
	{
		if (m_Ptr)
		{
			m_Ptr->Release();
			m_Ptr = NULL;
		}
	}

public:
	operator INTERFACE*() const
    {
		assert(m_Ptr != nullptr);
        return m_Ptr;
    }

	INTERFACE& operator*() const
    {
        assert(m_Ptr != nullptr);
		return *m_Ptr;
    }

	INTERFACE** operator&()
    {
        assert(m_Ptr != nullptr);
        return &m_Ptr;
    }

	INTERFACE* operator->() const
	{
		assert(m_Ptr != nullptr);
		return m_Ptr;
	}

	bool operator==(const INTERFACE& lPtr) const
	{
		assert(m_Ptr != nullptr);
		return m_Ptr == lPtr;
	}

	bool operator!=(const INTERFACE& lPtr) const
	{
		assert(m_Ptr != nullptr);
		return !(m_Ptr == lPtr);
	}

	INTERFACE* operator=(INTERFACE* lPtr)
	{
		assert(lPtr != nullptr);
		if (IsEqualObject(lPtr))
		{
			return m_Ptr;
		}
		m_Ptr->Release();
		lPtr->AddRef();
		m_Ptr = lPtr;
		return m_Ptr;
	}

	INTERFACE* operator=(IUnknown* pIUnknown)
    {
		assert(pIUnknown != nullptr);
		assert(piid != nullptr);
		//TODO: reinterpret_cast
        pIUnknown->QueryInterface(*piid, (void**)&m_Ptr);
		assert(m_Ptr != nullptr);
		return m_Ptr;
    }

	INTERFACE* operator=(const ComSmartPtr<INTERFACE, piid>& RefComPtr)
	{
		assert(&RefComPtr != nullptr);
		// using static cast here since conversion operator is explicitly defined
		m_Ptr = static_cast<INTERFACE*>(RefComPtr);

		if (m_Ptr)
		{
			m_Ptr->AddRef();
		}
		return m_Ptr;
	}

	void Attach(INTERFACE* lPtr)
    {
        if (lPtr)
		{
			m_Ptr->Release();
			m_Ptr = lPtr;
		}
    }

    INTERFACE* Detach()
    {
        INTERFACE* lPtr = m_Ptr;
        m_Ptr = nullptr;
        return lPtr;
    }

	void Release()
	{
		if (m_Ptr)
		{
			m_Ptr->Release();
			m_Ptr = nullptr;
		}
	}

	bool IsEqualObject(IUnknown* pOther)
	{
		assert(pOther != nullptr);
		IUnknown* pUnknown = nullptr;
		//TODO: reinterpret_cast
		m_Ptr->QueryInterface(IID_IUnknown, (void**)&pUnknown);
        bool result = (pOther == pUnknown) ? true : false;
		pUnknown->Release();
		return result;
	}

private:
	INTERFACE* m_Ptr;
};

#endif // _COMSMARTPTR_H_
