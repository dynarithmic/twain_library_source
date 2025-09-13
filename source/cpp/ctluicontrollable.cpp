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
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "ctlsetgetcaps.h"
#include "ctlclosesource.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsUIControllable(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto getSupport = pSource->IsUIControllable();

    // If status of UI support already determined, return result.
    if (getSupport.value != boost::tribool::indeterminate_value)
        LOG_FUNC_EXIT_NONAME_PARAMS(getSupport ? true : false)

    // Open the source (if source is closed)
    bool bSourceOpen = false;
    if (CTL_TwainAppMgr::IsSourceOpen(pSource))
        bSourceOpen = true;
    else
    if (!CTL_TwainAppMgr::OpenSource(pHandle->m_pTwainSession, pSource))
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    // Make sure we close this source if the source had to be opened
    // to test for UI controlability by utilizing RAII class
    SourceCloserRAII so(pSource, !bSourceOpen);

    bool bOk = false;

    // Check if capability UICONTROLLABLE is supported
    if (pSource->IsCapInSupportedList(CAP_UICONTROLLABLE))
    {
        // Get the capability value
        DTWAIN_ARRAY CapArray = nullptr;
        BOOL bGetUI = GetCapValuesEx2_Internal(pSource, CAP_UICONTROLLABLE, DTWAIN_CAPGET, 
                                                DTWAIN_CONTONEVALUE, DTWAIN_DEFAULT, &CapArray);
        if (bGetUI && CapArray && !pHandle->m_ArrayFactory->empty(CapArray))
        {
            DTWAINArrayLowLevel_RAII arr(pHandle, CapArray);
            bOk = pHandle->m_ArrayFactory->underlying_container_t<LONG>(CapArray)[0] ? true : false;
        }
    }
    else
    {
        // Source UI must be tested
        bOk = CTL_TwainAppMgr::ShowUserInterface(pSource, true);
    }
    pSource->SetUIControllable(bOk);
    LOG_FUNC_EXIT_NONAME_PARAMS(bOk ? TRUE : FALSE)
    CATCH_BLOCK_LOG_PARAMS(false)
}
