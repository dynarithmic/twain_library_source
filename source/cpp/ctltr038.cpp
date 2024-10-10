/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    FOR ANY PART OF THE COVERED WORK IN WHICH THE COPYRIGHT IS OWNED BY
    DYNARITHMIC SOFTWARE. DYNARITHMIC SOFTWARE DISCLAIMS THE WARRANTY OF NON INFRINGEMENT
    OF THIRD PARTY RIGHTS.
 */
#include "ctltr038.h"

#include "ctliface.h"
#include "ctltr010.h"
#include "ctltwainmanager.h"

using namespace dynarithmic;
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

CTL_ExtImageInfoTriplet::CTL_ExtImageInfoTriplet(CTL_ITwainSession *pSession,
                                                 CTL_ITwainSource* pSource,
                                                 int nInfo)
                                               :  CTL_TwainTriplet(), m_pExtImageInfo{},m_memHandle{}, m_nNumInfo{}, m_bRetrievedInfo{}
{
    InitInfo(pSession, pSource, nInfo);
}

void CTL_ExtImageInfoTriplet::swap(CTL_ExtImageInfoTriplet& left, const CTL_ExtImageInfoTriplet& right) noexcept
{
    left.m_pExtImageInfo = right.m_pExtImageInfo;
    left.m_memHandle = right.m_memHandle;
    left.m_nNumInfo = right.m_nNumInfo;
    left.m_ExtInfoMap = right.m_ExtInfoMap;
    left.m_bRetrievedInfo = right.m_bRetrievedInfo;
}

CTL_ExtImageInfoTriplet::CTL_ExtImageInfoTriplet(CTL_ExtImageInfoTriplet&& rhs) noexcept
{
    swap(*this, rhs);
    rhs.m_pExtImageInfo = nullptr;
    rhs.m_memHandle = {};
    rhs.m_nNumInfo = {};
    rhs.m_ExtInfoMap = {};
    rhs.m_bRetrievedInfo = {};
}

void CTL_ExtImageInfoTriplet::InitInfo(CTL_ITwainSession *pSession,
                                       CTL_ITwainSource* pSource,
                                       int nInfo)
{
    SetSessionPtr(pSession);
    SetSourcePtr( pSource );

    // Make sure we have one item
    auto& extImageInfoMap = CTL_StaticData::GetExtendedImageInfoMap();
    m_ExtInfoMap.clear();
    for (auto& pr : extImageInfoMap)
    {
       TW_INFO Info{};
       Info.InfoID = static_cast<TW_UINT16>(pr.first);
       Info.ReturnCode = TWRC_DATANOTAVAILABLE;
       m_ExtInfoMap.insert({ static_cast<TW_UINT16>(pr.first), Info });
    }
    m_nNumInfo = extImageInfoMap.size();
}

void CTL_ExtImageInfoTriplet::DestroyInfo()
{
    if (!m_pExtImageInfo)
        return;
    auto sessionHandle = GetSessionPtr()->GetTwainDLLHandle();
    if (!sessionHandle)
        return;

    // Use a std::set to gather the handles, since this ensures we will 
    // not be unlocking and calling free two (or more times) on the same handle.
    std::set<TW_HANDLE> allocatedHandleSet;
    for (TW_UINT32 i = 0; i < m_pExtImageInfo->NumInfos; i++)
    {
        TW_INFO* pInfo = &m_pExtImageInfo->Info[i];
        // Go to next item if not really supported
        if (pInfo->ReturnCode == TWRC_INFONOTSUPPORTED ||
            pInfo->ReturnCode == TWRC_DATANOTAVAILABLE)
            continue;

        // "Remove" the items by adding to the set
        if (dynarithmic::GetTwainItemSize(pInfo->ItemType) * pInfo->NumItems > sizeof(TW_HANDLE))
        {
            TW_HANDLE SubHandle = reinterpret_cast<TW_HANDLE>(pInfo->Item);
            if (pInfo->ItemType == TWTY_HANDLE)
            {
                // Only "top-level" TWTY_HANDLE's are deallocated by DTWAIN.  If the TWTY_HANDLE points
                // to a sub-level set of TWTY_HANDLE's that may have been allocated, DTWAIN does not know 
                // for certain that the handles were actually allocated.  Therefore the application  
                // using DTWAIN has to process these sub-handles when destroying the TW_EXTIMAGEINFO data.
                const TW_HANDLE* pHandle = (TW_HANDLE*)sessionHandle->m_TwainMemoryFunc->LockMemory(SubHandle);
                if (pHandle)
                {
                    // Empty, since right now, it is the app that has to figure out what to do with these
                    // handles.
                    for (int curHandle = 1; curHandle < pInfo->NumItems; curHandle++)
                    {
                    }
                }
            }
            // This item can be safely removed by DTWAIN, so add to the allocated handle set
            allocatedHandleSet.insert(SubHandle);
        }
    }

    // Add the original memory handle for the TW_EXTIMAGEINFO struct to our set of handles to delete
    allocatedHandleSet.insert(m_memHandle);

    // Delete all the allocated handles
    std::for_each(allocatedHandleSet.begin(), allocatedHandleSet.end(), [&](TW_HANDLE h)
        {
            sessionHandle->m_TwainMemoryFunc->UnlockMemory(h);
            sessionHandle->m_TwainMemoryFunc->FreeMemory(h);
        });
}

TW_UINT16 CTL_ExtImageInfoTriplet::Execute()
{
    if (!m_ExtInfoMap.empty())
    {
        CreateExtImageInfo();
        ResolveTypes();
        const TW_UINT16 rc = CTL_TwainTriplet::Execute();
        m_bRetrievedInfo = true;
        CopyInfoToVector();
        return rc;
    }
    return static_cast<TW_UINT16>(-1);
}

bool CTL_ExtImageInfoTriplet::CreateExtImageInfo()
{
    CTL_ITwainSession *pSession = GetSessionPtr();
    CTL_ITwainSource *pSource = GetSourcePtr();
    auto sessionHandle = pSession->GetTwainDLLHandle();
    const size_t nInfos = m_ExtInfoMap.size();

    // Allocate memory for TW_INFO structure
    m_memHandle = sessionHandle->m_TwainMemoryFunc->AllocateMemory(static_cast<TW_UINT32>(sizeof(TW_INFO) * nInfos + sizeof(TW_EXTIMAGEINFO)));
    m_pExtImageInfo = static_cast<TW_EXTIMAGEINFO*>(sessionHandle->m_TwainMemoryFunc->LockMemory(m_memHandle));

    // Set up the base triplet information here
    if ( m_pExtImageInfo )
    {
        m_pExtImageInfo->NumInfos = static_cast<TW_UINT32>(nInfos);
        // Get the app manager's AppID
        const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
        if ( pMgr && pMgr->IsValidTwainSession( pSession ))
        {
            if ( pSource )
            {
                Init( pSession->GetAppIDPtr(),
                      pSource->GetSourceIDPtr(),
                      DG_IMAGE,
                      DAT_EXTIMAGEINFO,
                      MSG_GET,
                      static_cast<TW_MEMREF>(static_cast<pTW_EXTIMAGEINFO>(m_pExtImageInfo)));
                SetAlive (true);
            }
        }
    }
    return true;
}

void CTL_ExtImageInfoTriplet::ResolveTypes()
{
    int i = 0;
    for (auto& pr : m_ExtInfoMap)
    {
        // Set the info within the allocated memory here
        TW_INFO& Info = pr.second;
        memcpy(&m_pExtImageInfo->Info[i], &Info, sizeof(TW_INFO));
        // Go to next INFO entry
        ++i;
    }
}

void CTL_ExtImageInfoTriplet::CopyInfoToVector()
{
    auto it = m_ExtInfoMap.begin();
    int i = 0;
    while ( it != m_ExtInfoMap.end())
    {
        memcpy(&it->second, &m_pExtImageInfo->Info[i], sizeof(TW_INFO));

        // Go to next INFO entry
        ++it;
        ++i;
    }
}

CTL_ExtImageInfoTriplet::~CTL_ExtImageInfoTriplet()
{
    try
    {
        DestroyInfo();
    }
    catch(...)
    {

    }
}

std::pair<bool, int32_t> CTL_ExtImageInfoTriplet::GetItemData(int nWhichItem, int nSearch, int nWhichValue, LPVOID Data, LPVOID* pHandleData, size_t* pItemSize/*=NULL*/) const
{
    auto sessionHandle = GetSourcePtr()->GetDTWAINHandle();
    if (!sessionHandle)
        return { false, DTWAIN_ERR_BAD_HANDLE };

    TW_INFO Info = {};

    // Check if info has been found
    Info = GetInfo(nWhichItem, nSearch);

    // Check the number of items
    if ( nWhichValue >= Info.NumItems )
        return { false, DTWAIN_ERR_INDEX_BOUNDS };

    // Go to next item if not really supported
    if (Info.ReturnCode == TWRC_INFONOTSUPPORTED ||
        Info.ReturnCode == TWRC_DATANOTAVAILABLE)
        return { false, Info.ReturnCode == TWRC_INFONOTSUPPORTED ? DTWAIN_ERR_UNSUPPORTED_EXTINFO : DTWAIN_ERR_UNAVAILABLE_EXTINFO };

    const TW_UINT16 nSize = dynarithmic::GetTwainItemSize(Info.ItemType);
    if (Data && (nSize * Info.NumItems > sizeof(TW_HANDLE)))
    {
        TW_HANDLE SubHandle = reinterpret_cast<TW_HANDLE>(Info.Item);
        const TW_HANDLE* pHandle = (TW_HANDLE*)sessionHandle->m_TwainMemoryFunc->LockMemory(SubHandle);

        // Check if this is a handle
        if (pHandle && Info.ItemType == TWTY_HANDLE)
        {
            // Just get the handle and let application figure out to lock this handle
            // or not.  This is the safest thing to do, given the vagueness of the 
            // TWAIN specification.
            HANDLE actualHandle = static_cast<HANDLE*>(SubHandle)[nWhichValue];
            *pHandleData = actualHandle;
        }
        else
        if ( Info.ItemType == TWTY_STR255)
        {
            // This needs to be locked to get the data, according to the TWAIN spec.
            HANDLE actualHandle = static_cast<HANDLE*>(SubHandle)[nWhichValue];
            char* pActualData = (char*)sessionHandle->m_TwainMemoryFunc->LockMemory(actualHandle);
            if (pActualData)
                memcpy(Data, pActualData, nSize);
            sessionHandle->m_TwainMemoryFunc->UnlockMemory(actualHandle);
        }
        else
        {
            // Just "regular" data that doesn't need to be locked.
            HANDLE actualHandle = static_cast<HANDLE*>(SubHandle)[nWhichValue];
            memcpy(Data, &actualHandle, nSize);
        }
        sessionHandle->m_TwainMemoryFunc->UnlockMemory(SubHandle);
    }
    else
    {
        if (Info.ItemType == TWTY_HANDLE)
            // The application has to deal with this
            *pHandleData = reinterpret_cast<HANDLE>(Info.Item);
        else
        // Data that is < sizeof(HANDLE) byte threshold
        if (Data)
            // just return the item data
            memcpy(Data, &Info.Item, nSize);
    }
    if (pItemSize)
        *pItemSize = nSize;
    return { true, DTWAIN_NO_ERROR };
}

TW_INFO CTL_ExtImageInfoTriplet::GetInfo(size_t nWhich, int nSearch) const
{
    TW_INFO Info = {};
    if ( nSearch == DTWAIN_BYPOSITION )
    {
        if ( nWhich >= m_nNumInfo || !m_pExtImageInfo )
        {
            return Info;
        }
        return m_pExtImageInfo->Info[nWhich];
    }
    else
    if ( nSearch == DTWAIN_BYID )
    {
        auto iter = m_ExtInfoMap.find(static_cast<TW_UINT16>(nWhich));
        if (iter != m_ExtInfoMap.end())
            return iter->second;
    }
    return Info;
}

bool CTL_ExtImageInfoTriplet::SetInfo(TW_INFO Info, size_t nWhich)
{
    if ( nWhich >= m_nNumInfo || !m_pExtImageInfo )
        return false;
    memcpy(&m_pExtImageInfo->Info[nWhich], &Info, sizeof(TW_INFO));
    return true;
}


bool CTL_ExtImageInfoTriplet::IsItemHandle(size_t nWhich) const
{
    return !(nWhich >= m_nNumInfo || !m_pExtImageInfo );
}

// Alternate
bool CTL_ExtImageInfoTriplet::AddInfo(TW_INFO Info)
{
    m_ExtInfoMap.insert({ Info.InfoID, Info });
    return true;
}

// Function assumes that DAT_EXTIMAGEINFO exists for the Source
bool CTL_ExtImageInfoTriplet::EnumSupported(CTL_ITwainSource *pSource,
                                            CTL_ITwainSession *pSession,
                                            CTL_IntArray &rArray)
{
    size_t NumAttr = CTL_StaticData::GetExtendedImageInfoMap().size();
    rArray.clear();

    // Set up a TWAIN triplet
    CTL_ExtImageInfoTriplet Trip(pSession, pSource, static_cast<int>(NumAttr));
    CTL_ExtImageInfoTriplet* pTripletToUse = &Trip;

    TW_UINT16 rc = TWRC_SUCCESS;
    auto triplet = pSource->GetExtImageInfoTriplet();

    // No member variable triplet, so use local instance
    if (!triplet)
        rc = Trip.Execute();
    else
    {
        // member variable triplet exists, so use it
        pTripletToUse = triplet.get();
        if (!triplet->HasRetrievedInfo())
            // not retrieved the info yet, so do this.
            rc = triplet->Execute();
    }

    switch (rc)
    {
        case TWRC_SUCCESS:
        {
            for ( size_t i = 0; i < NumAttr; i++)
            {
                TW_INFO Info = pTripletToUse->GetInfo(i, DTWAIN_BYPOSITION);
                if ( Info.ReturnCode != TWRC_INFONOTSUPPORTED && Info.ReturnCode != TWRC_DATANOTAVAILABLE)
                    rArray.push_back(Info.InfoID);
            }
            return true;
        }
        break;

        case TWRC_FAILURE:
        {
            const TW_UINT16 cc = CTL_TwainAppMgr::GetConditionCode(pSession, pSource);
            CTL_TwainAppMgr::ProcessConditionCodeError(cc);
            return false;
        }

        default:
            return false;
    }
    return false;
}
