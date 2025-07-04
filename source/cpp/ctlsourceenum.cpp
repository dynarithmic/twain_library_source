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
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSourcesEx()
{
    LOG_FUNC_ENTRY_PARAMS(())
    DTWAIN_ARRAY pArray = nullptr;
    DTWAIN_EnumSources(&pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(pArray)
    CATCH_BLOCK(DTWAIN_ARRAY{})
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSources(LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Array))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !Array; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    bool bEnumeratorExists = pHandle->m_ArrayFactory->is_valid(*Array);
    if (!bEnumeratorExists)
        *Array = nullptr;

    // Create a DTWAIN_ARRAY consisting of pointers to the source objects.
    DTWAIN_ARRAY aSource = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYSOURCE, 0);
    if (!aSource)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    DTWAIN_ARRAY pDTWAINArray = aSource;

    const auto& factory = pHandle->m_ArrayFactory;
    auto& vEnum = factory->underlying_container_t<CTL_ITwainSource*>(pDTWAINArray);

    // Start a session if not already started
    if (!pHandle->m_bSessionAllocated)
    {
        if (!DTWAIN_StartTwainSession(nullptr, nullptr))
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }

    CTL_TwainAppMgr::EnumSources(pHandle->m_pTwainSession);
    const auto& twainSources = pHandle->m_pTwainSession->GetTwainSources();

    // Copy results to user array
    std::copy(twainSources.begin(), twainSources.end(), std::back_inserter(vEnum));

    // Destroy old contents of user array if contents already existed and assign new info
    if (bEnumeratorExists)
        pHandle->m_ArrayFactory->destroy(*Array);
    *Array = aSource;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}
