/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetMaxRetryAttempts(DTWAIN_SOURCE Source, LONG nAttempts)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nAttempts))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(false)
        p->SetMaxRetryAttempts(nAttempts);
    LOG_FUNC_EXIT_PARAMS(true)
        CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetMaxRetryAttempts(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -2L, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(-2L)
    const LONG retval = p->GetMaxRetryAttempts();
    LOG_FUNC_EXIT_PARAMS(retval)
        CATCH_BLOCK(DTWAIN_FAILURE2)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCurrentRetryCount(DTWAIN_SOURCE Source, LONG nCount)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nCount))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(false)
        p->SetCurrentRetryCount(nCount);
    LOG_FUNC_EXIT_PARAMS(true)
        CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetCurrentRetryCount(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(-1L)
    const LONG retval = p->GetCurrentRetryCount();
    LOG_FUNC_EXIT_PARAMS(retval)
        CATCH_BLOCK(DTWAIN_FAILURE1)
}
