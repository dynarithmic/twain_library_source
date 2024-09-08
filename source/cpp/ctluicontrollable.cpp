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
#include "ctltwmgr.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsUIControllable(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    CTL_ITwainSource* pSource = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);
    const auto pHandle = pSource->GetDTWAINHandle();

    // Open the source (if source is closed)
    bool bSourceOpen = false;
    if (CTL_TwainAppMgr::IsSourceOpen(pSource))
        bSourceOpen = true;
    else
    if (!CTL_TwainAppMgr::OpenSource(pHandle->m_pTwainSession, pSource))
       LOG_FUNC_EXIT_NONAME_PARAMS(false)
    bool bOk = false;

    // Check if capability UICONTROLLABLE is supported
    if (DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPUICONTROLLABLE))
    {
        // Get the capability value
        DTWAIN_ARRAY CapArray = nullptr;
        BOOL bGetUI = DTWAIN_GetCapValuesEx(Source, DTWAIN_CV_CAPUICONTROLLABLE, DTWAIN_CAPGET, DTWAIN_CONTONEVALUE, &CapArray);
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
    // Close source if opened in this function
    if (!bSourceOpen)
        DTWAIN_CloseSource(Source);
    LOG_FUNC_EXIT_NONAME_PARAMS(bOk ? TRUE : FALSE)
    CATCH_BLOCK_LOG_PARAMS(false)
}
