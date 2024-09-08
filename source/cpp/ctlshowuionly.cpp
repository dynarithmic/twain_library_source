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
#include "ctltwmgr.h"
#include "errorcheck.h"
#include "ctltr040.h"
#include "ctltwainmsgloop.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

// This function allows the user to only show the UI
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ShowUIOnly(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    CTL_ITwainSource *pSource = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);
    const auto pHandle = pSource->GetDTWAINHandle();

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return DTWAIN_IsSourceAcquiring(Source); },
    DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return pSource->IsUIOpen(); },
    DTWAIN_ERR_UI_ALREADY_OPENED, false, FUNC_MACRO);

    // Open the source (if source is closed)
    bool bCloseSource = false;
    const bool bIsSourceOpen = DTWAIN_IsSourceOpen(Source) ? true : false;

    if (!bIsSourceOpen && DTWAIN_GetTwainMode() == DTWAIN_MODAL)
    {
        bCloseSource = true;
        if (!DTWAIN_OpenSource(Source))
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    else
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !bIsSourceOpen; }, DTWAIN_ERR_SOURCE_NOT_OPEN, false, FUNC_MACRO);

    // Check if capability is supported
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPENABLEDSUIONLY); },
        DTWAIN_ERR_UIONLY_NOT_SUPPORTED, false, FUNC_MACRO);

    // Start a thread depending on Twain Mode.
    if (DTWAIN_GetTwainMode() == DTWAIN_MODELESS) // No thread
    {
        const DTWAIN_BOOL bRet2 = CTL_TwainAppMgr::ShowUserInterface(pSource, false, true);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet2)
    }

    else
    {
        SourceAcquireOptions opts;
        opts.setIsUIIOnly(true);
        dynarithmic::StartModalMessageLoop(pSource, opts);
    }

    // Close the source if opened artificially
    if (bCloseSource)
        DTWAIN_CloseSource(Source);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

void dynarithmic::LLSetupUIOnly(CTL_ITwainSource* pSource)
{
    // show the interface -- this is where we may get a message right away in the loop
    CTL_TwainAppMgr::ShowUserInterface(pSource, false, true);
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ForceScanOnNoUI(DTWAIN_SOURCE Source, BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bSet))
    CTL_ITwainSource* pSource = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);

    // return the file name that would be acquired
    pSource->SetForceScanOnNoUI(bSet ? true : false);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

