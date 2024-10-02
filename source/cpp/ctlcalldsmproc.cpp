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
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

TWAIN_IDENTITY DLLENTRY_DEF DTWAIN_GetTwainAppID()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if (!pHandle->m_bSessionAllocated)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    TW_IDENTITY *pIdentity = pHandle->m_pTwainSession->GetAppIDPtr();
    LOG_FUNC_EXIT_NONAME_PARAMS(((TWAIN_IDENTITY)pIdentity))
    CATCH_BLOCK(TWAIN_IDENTITY(0))
}

TWAIN_IDENTITY DLLENTRY_DEF DTWAIN_GetTwainAppIDEx(TW_IDENTITY* pIdentity)
{
    LOG_FUNC_ENTRY_PARAMS((pIdentity))
    const TWAIN_IDENTITY thisID = DTWAIN_GetTwainAppID();
    if (thisID)
        memcpy(pIdentity, thisID, sizeof(TW_IDENTITY));
    LOG_FUNC_EXIT_NONAME_PARAMS(((TWAIN_IDENTITY)pIdentity))
    CATCH_BLOCK(TWAIN_IDENTITY(0))
}

TWAIN_IDENTITY DLLENTRY_DEF DTWAIN_GetSourceID(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    TWAIN_IDENTITY Id = static_cast<TWAIN_IDENTITY>(pSource->GetSourceIDPtr());
    LOG_FUNC_EXIT_NONAME_PARAMS(Id)
    CATCH_BLOCK_LOG_PARAMS(TWAIN_IDENTITY())
}

TWAIN_IDENTITY  DLLENTRY_DEF DTWAIN_GetSourceIDEx(DTWAIN_SOURCE Source, TW_IDENTITY* pIdentity)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pIdentity))
    const TWAIN_IDENTITY thisID = DTWAIN_GetSourceID(Source);
    if (thisID)
        memcpy(pIdentity, thisID, sizeof(TW_IDENTITY));
    LOG_FUNC_EXIT_NONAME_PARAMS(((TWAIN_IDENTITY)pIdentity))
    CATCH_BLOCK(TWAIN_IDENTITY(0))
}

LONG DLLENTRY_DEF DTWAIN_CallDSMProc(TWAIN_IDENTITY AppID, TWAIN_IDENTITY SourceId, LONG lDG, LONG lDAT, LONG lMSG, LPVOID pData)
{
    LOG_FUNC_ENTRY_PARAMS((AppID, SourceId, lDG, lDAT, lMSG, pData))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto pH = pHandle;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return !pH->m_bSessionAllocated; }, DTWAIN_ERR_NO_SESSION, -1L, FUNC_MACRO);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return !AppID && !SourceId; }, DTWAIN_ERR_INVALID_PARAM, -1L, FUNC_MACRO);
    const LONG Ret = CTL_TwainAppMgr::CallDSMEntryProc(static_cast<TW_IDENTITY*>(AppID),
                                                       static_cast<TW_IDENTITY*>(SourceId),
                                                       static_cast<TW_UINT32>(lDG),
                                                       static_cast<TW_UINT16>(lDAT),
                                                       static_cast<TW_UINT16>(lMSG),
                                                       pData);

    LOG_FUNC_EXIT_NONAME_PARAMS(Ret)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCurrentTwainTriplet(TW_IDENTITY* pAppID, TW_IDENTITY* pSourceID,
                                                       LPLONG lpDG, LPLONG lpDAT, LPLONG lpMsg, LPLONG64 lpMemRef)
{
    LOG_FUNC_ENTRY_PARAMS((pAppID, pSourceID, lpDAT, lpDG, lpMsg, lpMemRef))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto pH = pHandle;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !pH->m_bSessionAllocated; }, DTWAIN_ERR_NO_SESSION, -1L, FUNC_MACRO);
    const CTL_TwainTriplet* currentTriplet = CTL_TwainAppMgr::GetInstance()->GetCurrentTriplet();
    if (currentTriplet)
    {
        if ( pAppID )
        {
            const TW_IDENTITY* trip = currentTriplet->GetOriginID();
            if ( trip )
                memcpy(pAppID, trip, sizeof(TW_IDENTITY));
        }

        if ( pSourceID )
        {
            const TW_IDENTITY* trip = currentTriplet->GetDestinationID();
            if (trip)
                memcpy(pSourceID, trip, sizeof(TW_IDENTITY));
        }

        if ( lpDG )
            *lpDG = static_cast<LONG>(currentTriplet->GetDG());
        if (lpDAT)
            *lpDAT = currentTriplet->GetDAT();
        if (lpMsg)
            *lpMsg = currentTriplet->GetMSG();

        if ( lpMemRef )
        {
             const TW_MEMREF memref = currentTriplet->GetMemRef();
             if (memref)
                *lpMemRef = reinterpret_cast<LONG64>(memref);
            else
                *lpMemRef = {};
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(FALSE)
}
