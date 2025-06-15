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
#include "cppfunc.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "ctlsetgetcaps.h"
using namespace dynarithmic;

static std::pair<bool, int> GetDuplexType(DTWAIN_SOURCE Source)
{
    if (!DTWAIN_IsCapSupported(Source, CAP_DUPLEX))
        return { false, TWDX_NONE };

    DTWAIN_ARRAY Array = nullptr;
    auto pSource = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = pSource->GetDTWAINHandle();
    const DTWAIN_BOOL bRet2 = GetCapValuesEx2_Internal(pSource, CAP_DUPLEX, DTWAIN_CAPGET,
                                                       DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &Array) ? true : false;
    DTWAINArrayLowLevel_RAII arr(pHandle, Array);
    if (bRet2 && Array)
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);
        if (!vValues.empty())
            return { true, vValues.front() };
    }
    return { false, TWDX_NONE };
}

// Duplex Scanner support 
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDuplexType(DTWAIN_SOURCE Source, LPLONG lpDupType)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpDupType))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    bool bRet = true;

    auto getSupport = pSource->GetDuplexSupport();

    // If status of duplex support already determined, return result.
    if (getSupport.first.value != boost::tribool::indeterminate_value)
    {
        if (lpDupType)
            *lpDupType = getSupport.second;
        bRet = getSupport.first.value;
    }
    else
    {
        auto dupType = GetDuplexType(Source);
        pSource->SetDuplexSupport(dupType.first, dupType.second);
        if (lpDupType)
            *lpDupType = dupType.second;
        bRet = dupType.first;
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpDupType))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsDuplexSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);

    auto getSupport = pSource->GetDuplexSupport();

    // If status of duplex support already determined, return result.
    if (getSupport.first.value != boost::tribool::indeterminate_value)
        LOG_FUNC_EXIT_NONAME_PARAMS(getSupport.first?true:false)

    bool bRet = false;

    auto retValue = GetDuplexType(Source);
    if (retValue.second == TWDX_1PASSDUPLEX ||
        retValue.second == TWDX_2PASSDUPLEX)
        bRet = true;

    pSource->SetDuplexSupport(retValue.first, retValue.second);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}
