/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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
#include <cstdio>
//#include <sstream>
#include <functional>
#include <algorithm>
#include "cppfunc.h"
#include "errorcheck.h"
#include "ctltr036.h"
using namespace dynarithmic;

/////////////////////////////////////////////////////////////
// Custom cap data functions

HANDLE DLLENTRY_DEF DTWAIN_GetCustomDSData( DTWAIN_SOURCE Source, LPBYTE Data, DWORD dSize, LPDWORD pActualSize, LONG nFlags )
{
    LOG_FUNC_ENTRY_PARAMS((Source, Data, dSize, pActualSize, nFlags))
    const bool bSupported = DTWAIN_IsCapSupported(Source, CAP_CUSTOMDSDATA)?true:false;

    if( !bSupported )
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)

    auto *p = static_cast<CTL_ITwainSource*>(Source);
    // Call TWAIN to get the custom data
    const auto pSession = p->GetTwainSession();
    CTL_GetCustomDSTriplet DST(pSession, p);

    // Get the custom data
    const int ret = DST.Execute();

    // return if TWAIN cannot complete this request
    if( ret != TWRC_SUCCESS )
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)

    // Copy actual size data to parameter
    if( pActualSize )
        *pActualSize = DST.GetDataSize();
    auto localActualSize = DST.GetDataSize();

    // Get the returned handle from TWAIN
    const HANDLE h = DST.GetData();

    // Return the handle if that is all user wants to do
    if( nFlags & DTWAINGCD_RETURNHANDLE )
        LOG_FUNC_EXIT_NONAME_PARAMS(h)

    // Copy data to user's data area.
    if( Data && (nFlags & DTWAINGCD_COPYDATA))
    {
        const char *pData = static_cast<char *>(ImageMemoryHandler::GlobalLock(h));
        auto nMinCopy = (std::max)((std::min<DWORD>)(dSize, localActualSize), 0UL);
        memcpy(Data, pData, nMinCopy);
        ImageMemoryHandler::GlobalUnlock(h);
        ImageMemoryHandler::GlobalFree(h);
        LOG_FUNC_EXIT_NONAME_PARAMS(HANDLE(1))
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((Data, pActualSize))
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK(HANDLE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCustomDSData( DTWAIN_SOURCE Source, HANDLE hData, LPCBYTE Data, DWORD dSize, LONG nFlags )
{
    LOG_FUNC_ENTRY_PARAMS((Source, hData, Data, dSize, nFlags))
    const bool bSupported = DTWAIN_IsCapSupported(Source, CAP_CUSTOMDSDATA)?true:false;

    if (!bSupported)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    auto p = static_cast<CTL_ITwainSource*>(Source);

    // Set up triplet for CUSTOMDSDATA call
    const auto pSession = p->GetTwainSession();
    auto pHandle = p->GetDTWAINHandle();

    CTL_SetCustomDSTriplet DST(pSession, p);

    // Check what options the user wants to do
    char *pData = nullptr;

    // Set data to the handle passed in
    if( nFlags & DTWAINSCD_USEHANDLE )
        DST.SetData(hData, dSize);
    else
    if( dSize == -1 )
    {
        if( !DTWAIN_GetCustomDSData(Source, nullptr, 0, &dSize, DTWAINGCD_COPYDATA) )
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }

    // Set data to the data passed in
    DTWAINGlobalHandleUnlockFree_RAII memHandler;

    if( Data && (nFlags & DTWAINSCD_USEDATA ))
    {
        // Allocate local copy of handle
        pData = static_cast<char*>(ImageMemoryHandler::GlobalAllocPr(GMEM_DDESHARE, dSize));
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pData == NULL; }, DTWAIN_ERR_OUT_OF_MEMORY, false, FUNC_MACRO);

        // Make sure memory is cleaned up at the end
        memHandler.reset(ImageMemoryHandler::GlobalHandle(pData));
        memcpy(pData, Data, dSize);
        DST.SetData(ImageMemoryHandler::GlobalHandle(pData), dSize);
    }

    // Call TWAIN
    const int ret = DST.Execute();

    // return TRUE or FALSE depending on return code of TWAIN
    if( ret != TWRC_SUCCESS )
    {
        pHandle->m_lLastError = -(IDS_TWRC_ERRORSTART + ret);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}
