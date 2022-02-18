/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#include "ctltwmgr.h"

using namespace dynarithmic;
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

TW_UINT16 CTL_ExtImageInfoTriplet::s_AllAttr[] = {
        DTWAIN_EI_BARCODEX               ,
        DTWAIN_EI_BARCODEY               ,
        DTWAIN_EI_BARCODETEXT            ,
        DTWAIN_EI_BARCODETYPE            ,
        DTWAIN_EI_DESHADETOP             ,
        DTWAIN_EI_DESHADELEFT            ,
        DTWAIN_EI_DESHADEHEIGHT          ,
        DTWAIN_EI_DESHADEWIDTH           ,
        DTWAIN_EI_DESHADESIZE            ,
        DTWAIN_EI_SPECKLESREMOVED        ,
        DTWAIN_EI_HORZLINEXCOORD         ,
        DTWAIN_EI_HORZLINEYCOORD         ,
        DTWAIN_EI_HORZLINELENGTH         ,
        DTWAIN_EI_HORZLINETHICKNESS      ,
        DTWAIN_EI_VERTLINEXCOORD         ,
        DTWAIN_EI_VERTLINEYCOORD         ,
        DTWAIN_EI_VERTLINELENGTH         ,
        DTWAIN_EI_VERTLINETHICKNESS      ,
        DTWAIN_EI_PATCHCODE              ,
        DTWAIN_EI_ENDORSEDTEXT           ,
        DTWAIN_EI_FORMCONFIDENCE         ,
        DTWAIN_EI_FORMTEMPLATEMATCH      ,
        DTWAIN_EI_FORMTEMPLATEPAGEMATCH  ,
        DTWAIN_EI_FORMHORZDOCOFFSET      ,
        DTWAIN_EI_FORMVERTDOCOFFSET      ,
        DTWAIN_EI_BARCODECOUNT           ,
        DTWAIN_EI_BARCODECONFIDENCE      ,
        DTWAIN_EI_BARCODEROTATION        ,
        DTWAIN_EI_BARCODETEXTLENGTH      ,
        DTWAIN_EI_DESHADECOUNT           ,
        DTWAIN_EI_DESHADEBLACKCOUNTOLD   ,
        DTWAIN_EI_DESHADEBLACKCOUNTNEW   ,
        DTWAIN_EI_DESHADEBLACKRLMIN      ,
        DTWAIN_EI_DESHADEBLACKRLMAX      ,
        DTWAIN_EI_DESHADEWHITECOUNTOLD   ,
        DTWAIN_EI_DESHADEWHITECOUNTNEW   ,
        DTWAIN_EI_DESHADEWHITERLMIN      ,
        DTWAIN_EI_DESHADEWHITERLAVE      ,
        DTWAIN_EI_DESHADEWHITERLMAX      ,
        DTWAIN_EI_BLACKSPECKLESREMOVED   ,
        DTWAIN_EI_WHITESPECKLESREMOVED   ,
        DTWAIN_EI_HORZLINECOUNT          ,
        DTWAIN_EI_VERTLINECOUNT          ,
        DTWAIN_EI_DESKEWSTATUS           ,
        DTWAIN_EI_SKEWORIGINALANGLE      ,
        DTWAIN_EI_SKEWFINALANGLE         ,
        DTWAIN_EI_SKEWCONFIDENCE         ,
        DTWAIN_EI_SKEWWINDOWX1           ,
        DTWAIN_EI_SKEWWINDOWY1           ,
        DTWAIN_EI_SKEWWINDOWX2           ,
        DTWAIN_EI_SKEWWINDOWY2           ,
        DTWAIN_EI_SKEWWINDOWX3           ,
        DTWAIN_EI_SKEWWINDOWY3           ,
        DTWAIN_EI_SKEWWINDOWX4           ,
        DTWAIN_EI_SKEWWINDOWY4           ,
        DTWAIN_EI_BOOKNAME               ,
        DTWAIN_EI_CHAPTERNUMBER          ,
        DTWAIN_EI_DOCUMENTNUMBER         ,
        DTWAIN_EI_PAGENUMBER             ,
        DTWAIN_EI_CAMERA                 ,
        DTWAIN_EI_FRAMENUMBER            ,
        DTWAIN_EI_FRAME                  ,
        DTWAIN_EI_PIXELFLAVOR            ,
        DTWAIN_EI_ICCPROFILE             ,
        DTWAIN_EI_LASTSEGMENT            ,
        DTWAIN_EI_SEGMENTNUMBER          ,
        DTWAIN_EI_MAGDATA                ,
        DTWAIN_EI_MAGTYPE                ,
        DTWAIN_EI_PAGESIDE               ,
        DTWAIN_EI_FILESYSTEMSOURCE       ,
        DTWAIN_EI_IMAGEMERGED            ,
        DTWAIN_EI_MAGDATALENGTH          ,
        DTWAIN_EI_PAPERCOUNT             ,
        DTWAIN_EI_PRINTERTEXT            ,
        DTWAIN_EI_TWAINDIRECTMETADATA    ,
        0};


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
    left.m_vInfo = right.m_vInfo;
}

CTL_ExtImageInfoTriplet::CTL_ExtImageInfoTriplet(CTL_ExtImageInfoTriplet&& rhs) noexcept
{
    swap(*this, rhs);
    rhs.m_pExtImageInfo = nullptr;
    rhs.m_memHandle = {};
    rhs.m_nNumInfo = {};
    rhs.m_vInfo = {};
}

void CTL_ExtImageInfoTriplet::InitInfo(CTL_ITwainSession *pSession,
                                       CTL_ITwainSource* pSource,
                                       int nInfo)
{
    SetSessionPtr(pSession);
    SetSourcePtr( pSource );

    // Make sure we have one item
    if ( nInfo <= 0 )
        nInfo = 1;
    m_nNumInfo = nInfo;
    TW_INFO Info;
    m_vInfo.resize(0);
    constexpr size_t NumAttr = std::size(s_AllAttr);
    size_t i;
    m_vInfo.reserve(NumAttr);
    for ( i = 0; i < NumAttr; ++i )
    {
        if ( s_AllAttr[i] == 0 )
            break;
       memset(&Info, 0, sizeof(TW_INFO));
       Info.InfoID = s_AllAttr[i];
       Info.ReturnCode = TWRC_DATANOTAVAILABLE;
       m_vInfo.push_back(Info);
    }
    m_nNumInfo = i;
}

void CTL_ExtImageInfoTriplet::DestroyInfo()
{
    if ( !m_pExtImageInfo )
        return;
    for (TW_UINT32 i = 0; i < m_pExtImageInfo->NumInfos; i++)
    {
        TW_INFO* pInfo = &m_pExtImageInfo->Info[i];
        // Go to next item if not really supported
        if (pInfo->ReturnCode == TWRC_INFONOTSUPPORTED ||
            pInfo->ReturnCode == TWRC_DATANOTAVAILABLE)
            continue;

        // Remove the items
        if ((CTL_CapabilityTriplet::GetItemSize(pInfo->ItemType) * pInfo->NumItems) > sizeof(TW_HANDLE))
        {
            // Get the data
            auto h = reinterpret_cast<TW_HANDLE>(pInfo->Item);
            TW_UINT16 Count = pInfo->NumItems;
            for (size_t j = 0; j < Count; ++j)
            {
                TW_HANDLE TempHandle = nullptr;
                switch (pInfo->ItemType)
                {
                case TWTY_UINT32:
                    TempHandle = reinterpret_cast<TW_HANDLE>(static_cast<TW_UINT32*>(h)[j]);
                    break;

                case TWTY_UINT16:
                    TempHandle = reinterpret_cast<TW_HANDLE>(static_cast<TW_UINT16*>(h)[j]);
                    break;

                case TWTY_STR255:
                    TempHandle = static_cast<TW_HANDLE>(static_cast<TW_STR255*>(h)[j]);
                    break;

                case TWTY_FRAME:
                {
                    LPCSTR pH = static_cast<LPCSTR>(h)+sizeof(TW_FRAME) * j;
                    TempHandle = (TW_HANDLE)pH;
                }
                break;

                default:
                {
                    // Check for text items
                    if (pInfo->InfoID == DTWAIN_EI_BARCODETEXT)
                    {
                        // Get the count of characters
                        LONG CountChars;
                        bool CountItems = GetItemData(DTWAIN_EI_BARCODECOUNT, DTWAIN_BYID, static_cast<int>(j), &CountChars);

                        // Check if we have the count.  Return if failure
                        if (!CountItems)
                            continue;

                        // If this is the first item, then this is easy
                        if (j == 0)
                        {
                            // Copy the characters
                            TempHandle = reinterpret_cast<TW_HANDLE>(static_cast<TW_UINT8*>(h)[0]);
                        }
                        else
                        {
                            // If this is the second or later item, need the count for the previous item
                            size_t HandlePos;
                            CountItems = GetItemData(DTWAIN_EI_BARCODECOUNT, DTWAIN_BYID, static_cast<int>(j - 1), &HandlePos);
                            if (!CountItems)
                                continue;
                            TempHandle = reinterpret_cast<TW_HANDLE>(static_cast<TW_UINT8*>(h)[HandlePos]);
                        }
                    }
                }
                break;
                }
                CTL_TwainDLLHandle::s_TwainMemoryFunc->UnlockMemory(TempHandle);
                CTL_TwainDLLHandle::s_TwainMemoryFunc->FreeMemory(TempHandle);
            }
        }
    }
    CTL_TwainDLLHandle::s_TwainMemoryFunc->UnlockMemory(m_memHandle);
    CTL_TwainDLLHandle::s_TwainMemoryFunc->FreeMemory(m_memHandle);
}


TW_UINT16 CTL_ExtImageInfoTriplet::Execute()
{
    if ( !m_vInfo.empty())
    {
        CreateExtImageInfo();
        ResolveTypes();
        TW_UINT16 rc = CTL_TwainTriplet::Execute();
        CopyInfoToVector();
        return rc;
    }
    return static_cast<TW_UINT16>(-1);
}

bool CTL_ExtImageInfoTriplet::CreateExtImageInfo()
{
    CTL_ITwainSession *pSession = GetSessionPtr();
    CTL_ITwainSource *pSource = GetSourcePtr();

    const size_t nInfos = m_vInfo.size();

    // Allocate memory for TW_INFO structure
    m_memHandle = CTL_TwainDLLHandle::s_TwainMemoryFunc->AllocateMemory(static_cast<TW_UINT32>(sizeof(TW_INFO) * nInfos + sizeof(TW_EXTIMAGEINFO)));
    m_pExtImageInfo = static_cast<TW_EXTIMAGEINFO*>(CTL_TwainDLLHandle::s_TwainMemoryFunc->LockMemory(m_memHandle));

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
    auto it = m_vInfo.begin();
    int i = 0;
    while ( it != m_vInfo.end())
    {
        if ( (*it).InfoID < CAP_CUSTOMBASE )
        {
            switch ((*it).InfoID)
            {
                case DTWAIN_EI_BARCODETEXT:
                    (*it).ItemType = 0;
                break;

                case DTWAIN_EI_ENDORSEDTEXT:
                case DTWAIN_EI_FORMTEMPLATEMATCH:
                case DTWAIN_EI_BOOKNAME:
                case DTWAIN_EI_CAMERA:
                    (*it).ItemType = TWTY_STR255;
                break;

                case DTWAIN_EI_FRAME:
                    (*it).ItemType = TWTY_FRAME;
                break;

                case DTWAIN_EI_PIXELFLAVOR:
                    (*it).ItemType = TWTY_UINT16;
                break;

                default:
                    (*it).ItemType = TWTY_UINT32;
                break;
            }
        }

        // Set the info within the allocated memory here
        TW_INFO Info = (*it);

        memcpy(&m_pExtImageInfo->Info[i], &Info, sizeof(TW_INFO));

        // Go to next INFO entry
        ++it;
        ++i;
    }
}

void CTL_ExtImageInfoTriplet::CopyInfoToVector()
{
    auto it = m_vInfo.begin();
    int i = 0;
    while ( it != m_vInfo.end())
    {
        memcpy(&m_vInfo[i], &m_pExtImageInfo->Info[i], sizeof(TW_INFO));

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

bool CTL_ExtImageInfoTriplet::GetItemData(int nWhichItem, int nSearch, int nWhichValue, LPVOID Data, size_t* pItemSize/*=NULL*/) const
{
    TW_INFO Info;

    // Check if info has been found
    Info = GetInfo(nWhichItem, nSearch);

    // Check the number of items
    if ( nWhichValue >= Info.NumItems )
        return false;

    // Go to next item if not really supported
    if (Info.ReturnCode == TWRC_INFONOTSUPPORTED ||
        Info.ReturnCode == TWRC_DATANOTAVAILABLE)
        return false;

    // Check if this is a handle
    const TW_UINT16 nSize = CTL_CapabilityTriplet::GetItemSize(Info.ItemType);
    if ( nSize * Info.NumItems > sizeof(TW_HANDLE))
    {
        // Get the data
        const TW_HANDLE h = reinterpret_cast<TW_HANDLE>(Info.Item);
        const char *p = static_cast<char*>(h) + nSize * nWhichValue;
        switch ( Info.ItemType)
        {
            case TWTY_UINT32:
                if ( Data )
                    memcpy(Data, p, nSize);
                if ( pItemSize )
                    *pItemSize = nSize;
            break;

            case TWTY_UINT16:
            {
                const LONG nValue = static_cast<LONG>(static_cast<TW_UINT16*>(h)[nWhichValue]);
                if ( Data )
                    memcpy(Data, &nValue, sizeof(LONG));
                if ( pItemSize )
                    *pItemSize = sizeof(LONG);
            }
            break;

            case TWTY_STR255:
                if ( Data )
                    memcpy(Data, &static_cast<TW_STR255*>(h)[nWhichValue], nSize);
                if ( pItemSize )
                    *pItemSize = nSize;
            break;

            case TWTY_FRAME:
                if ( Data )
                    memcpy(Data, &static_cast<TW_FRAME*>(h)[nWhichValue], nSize);
                if ( pItemSize )
                    *pItemSize = nSize;
            break;

            default:
            {
                // Check for text items
                if ( Info.InfoID == DTWAIN_EI_BARCODETEXT )
                {
                    // Get the count of characters
                    LONG Count;
                    bool CountItems = GetItemData(DTWAIN_EI_BARCODECOUNT, DTWAIN_BYID, nWhichValue, &Count);

                    // Check if we have the count.  Return if failure
                    if ( !CountItems )
                        return false;

                    // If this is the first item, then this is easy
                    if ( nWhichValue == 0 )
                    {
                        if ( Data )
                            // Copy the characters
                            memcpy(Data, reinterpret_cast<void*>(static_cast<TW_UINT8*>(h)[0]), Count);
                        if ( pItemSize )
                            *pItemSize = Count;
                        return true;
                    }

                    // If this is the second or later item, need the count for the previous item
                    LONG HandlePos;
                    CountItems = GetItemData(DTWAIN_EI_BARCODECOUNT, DTWAIN_BYID, nWhichValue - 1, &HandlePos);
                    if ( !CountItems )
                        return false;
                    if ( Data )
                        memcpy(Data, reinterpret_cast<void*>(static_cast<TW_UINT8*>(h)[HandlePos]), Count);
                    if ( pItemSize )
                        *pItemSize = Count;
                    return true;
                }
            }
        }
        return true;
    }
    else
    {
        if ( Data )
            // just return the item data
            memcpy(Data, &Info.Item, nSize);
        if ( pItemSize )
            *pItemSize = nSize;
        return true;
    }
    return false;
}

TW_INFO CTL_ExtImageInfoTriplet::GetInfo(size_t nWhich, int nSearch) const
{
    TW_INFO Info = {};
    Info.NumItems = static_cast<TW_UINT16>(-1);
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
        for ( size_t i = 0; i < m_nNumInfo; i++ )
        {
            if ( m_pExtImageInfo->Info[i].InfoID == nWhich )
                return m_pExtImageInfo->Info[i];
        }
    }
    return Info;
}

bool CTL_ExtImageInfoTriplet::SetInfo(const TW_INFO &Info, size_t nWhich)
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
bool CTL_ExtImageInfoTriplet::AddInfo(const TW_INFO& Info)
{
    m_vInfo.push_back(Info);
    return true;
}

bool CTL_ExtImageInfoTriplet::RetrieveInfo(TWINFOVector &v) const
{
    v.clear();
    v = m_vInfo;
    return true;
}


// Function assumes that DAT_EXTIMAGEINFO exists for the Source
bool CTL_ExtImageInfoTriplet::EnumSupported(CTL_ITwainSource *pSource,
                                            CTL_ITwainSession *pSession,
                                            CTL_IntArray &rArray)
{
    rArray.clear();
    const int NumAttr = std::size(s_AllAttr);
    CTL_ExtImageInfoTriplet Trip(pSession, pSource, NumAttr);
    const TW_UINT16 rc = Trip.Execute();
    switch (rc)
    {
        case TWRC_SUCCESS:
        {
            for ( int i = 0; i < NumAttr && s_AllAttr[i] != 0; i++)
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
