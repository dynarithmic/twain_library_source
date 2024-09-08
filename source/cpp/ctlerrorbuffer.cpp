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
#include <deque>
#include <algorithm>

#include "cppfunc.h"
#include "dtwain.h"
#include "ctliface.h"
#include "arrayfactory.h"
#include "ctltwmgr.h"
using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetErrorBuffer(LPDTWAIN_ARRAY ArrayBuffer)
{
    LOG_FUNC_ENTRY_PARAMS((ArrayBuffer))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    const size_t nEntries = (std::min)(static_cast<size_t>(pHandle->m_nErrorBufferThreshold), pHandle->m_vErrorBuffer.size());
    const DTWAIN_ARRAY A = CreateArrayFromFactory(DTWAIN_ARRAYLONG, static_cast<LONG>(nEntries));
    if ( A )
    {
        auto& vIn = pHandle->m_ArrayFactory->underlying_container_t<LONG>(A);
        std::copy_n(pHandle->m_vErrorBuffer.begin(), nEntries, vIn.begin());
        *ArrayBuffer = A;
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ClearErrorBuffer(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    std::deque<int> tempdeque;
    tempdeque.swap(pHandle->m_vErrorBuffer);

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetErrorBufferThreshold(LONG nErrors)
{
    LOG_FUNC_ENTRY_PARAMS((nErrors))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    // Minimum of 50 errors
    const LONG nEntries = (std::max)(nErrors, static_cast<LONG>(50));

    // clear buffer
    pHandle->m_nErrorBufferThreshold = nEntries;


    std::deque<int> tempdeque;
    tempdeque.swap(pHandle->m_vErrorBuffer);

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetErrorBufferThreshold(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, -1 ) )
        LOG_FUNC_EXIT_NONAME_PARAMS(-1)

    const LONG nValues = pHandle->m_nErrorBufferThreshold;
    LOG_FUNC_EXIT_NONAME_PARAMS(nValues)
    CATCH_BLOCK(-1)
}

