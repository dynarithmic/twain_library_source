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
                                               :  CTL_TwainTriplet(), m_pExtImageInfo{},m_memHandle{}, m_nNumInfo{}
{
    InitInfo(pSession, pSource, nInfo);
}

void CTL_ExtImageInfoTriplet::swap(CTL_ExtImageInfoTriplet& left, const CTL_ExtImageInfoTriplet& right) noexcept
{
    left.m_pExtImageInfo = right.m_pExtImageInfo;
    left.m_memHandle = right.m_memHandle;
    left.m_nNumInfo = right.m_nNumInfo;
    left.m_ExtInfoMap = right.m_ExtInfoMap;
}

CTL_ExtImageInfoTriplet::CTL_ExtImageInfoTriplet(CTL_ExtImageInfoTriplet&& rhs) noexcept
{
    swap(*this, rhs);
    rhs.m_pExtImageInfo = nullptr;
    rhs.m_memHandle = {};
    rhs.m_nNumInfo = {};
    rhs.m_ExtInfoMap = {};
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
    if ( !m_pExtImageInfo )
        return;
    auto sessionHandle = GetSessionPtr()->GetTwainDLLHandle();
    if (!sessionHandle)
        return;
    for (TW_UINT32 i = 0; i < m_pExtImageInfo->NumInfos; i++)
    {
        TW_INFO* pInfo = &m_pExtImageInfo->Info[i];
        // Go to next item if not really supported
        if (pInfo->ReturnCode == TWRC_INFONOTSUPPORTED ||
            pInfo->ReturnCode == TWRC_DATANOTAVAILABLE)
            continue;

        // Remove the items
        if (ConstexprUtils::GetTwainItemSize(pInfo->ItemType) * pInfo->NumItems > sizeof(TW_HANDLE))
        {
            TW_HANDLE SubHandle = reinterpret_cast<TW_HANDLE>(pInfo->Item);
            if (pInfo->ItemType == TWTY_HANDLE)
            {
                // Currently, the application using DTWAIN has to process the handles
                const TW_HANDLE* pHandle = (TW_HANDLE*)sessionHandle->m_TwainMemoryFunc->LockMemory(SubHandle);
                if (pHandle)
                {
                    for (int curHandle = 0; curHandle < pInfo->NumItems; curHandle++)
                    {
                    }
                }
            }
            // This items can be safely removed by DTWAIN
            sessionHandle->m_TwainMemoryFunc->UnlockMemory(SubHandle);
            sessionHandle->m_TwainMemoryFunc->FreeMemory(SubHandle);
        }
    }
}


TW_UINT16 CTL_ExtImageInfoTriplet::Execute()
{
    if ( !m_ExtInfoMap.empty())
    {
        CreateExtImageInfo();
        ResolveTypes();
        const TW_UINT16 rc = CTL_TwainTriplet::Execute();
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
        TW_INFO& theInfo = pr.second;
        if (theInfo.InfoID < CAP_CUSTOMBASE )
        {
            switch (theInfo.InfoID)
            {
                case DTWAIN_EI_BARCODETEXT:
                case DTWAIN_EI_BARCODETEXT2:
                    theInfo.ItemType = 0;
                break;

                case DTWAIN_EI_ENDORSEDTEXT:
                case DTWAIN_EI_FORMTEMPLATEMATCH:
                case DTWAIN_EI_BOOKNAME:
                case DTWAIN_EI_CAMERA:
                    theInfo.ItemType = TWTY_STR255;
                break;

                case TWEI_IAFIELDA_VALUE:
                case TWEI_IAFIELDB_VALUE:
                case TWEI_IAFIELDC_VALUE:
                case TWEI_IAFIELDD_VALUE:
                case TWEI_IAFIELDE_VALUE:
                    theInfo.ItemType = TWTY_STR32;
                break;

                case DTWAIN_EI_FRAME:
                    theInfo.ItemType = TWTY_FRAME;
                break;

                case DTWAIN_EI_PIXELFLAVOR:
                case DTWAIN_EI_IALEVEL:
                case DTWAIN_EI_PRINTER:
                    theInfo.ItemType = TWTY_UINT16;
                break;

                default:
                    theInfo.ItemType = TWTY_UINT32;
                break;
            }
        }

        // Set the info within the allocated memory here
        TW_INFO Info = pr.second;
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

bool CTL_ExtImageInfoTriplet::GetItemData(int nWhichItem, int nSearch, int nWhichValue, LPVOID Data, LPVOID* pHandleData, size_t* pItemSize/*=NULL*/) const
{
    auto sessionHandle = GetSourcePtr()->GetDTWAINHandle();
    if (!sessionHandle)
        return false;

    TW_INFO Info = {};

    // Check if info has been found
    Info = GetInfo(nWhichItem, nSearch);

    // Check the number of items
    if ( nWhichValue >= Info.NumItems )
        return false;

    // Go to next item if not really supported
    if (Info.ReturnCode == TWRC_INFONOTSUPPORTED ||
        Info.ReturnCode == TWRC_DATANOTAVAILABLE)
        return false;

    const TW_UINT16 nSize = ConstexprUtils::GetTwainItemSize(Info.ItemType);
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
    return true;
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
    rArray.clear();
    size_t NumAttr = CTL_StaticData::GetExtendedImageInfoMap().size();
    CTL_ExtImageInfoTriplet Trip(pSession, pSource, NumAttr);
    const TW_UINT16 rc = Trip.Execute();
    switch (rc)
    {
        case TWRC_SUCCESS:
        {
            for ( size_t i = 0; i < NumAttr; i++)
            {
                TW_INFO Info = Trip.GetInfo(i, DTWAIN_BYPOSITION);
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
