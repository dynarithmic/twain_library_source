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
#include "cppfunc.h"
#include "ctltwmgr.h"
#include "enumeratorfuncs.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetMaxAcquisitions(DTWAIN_SOURCE Source, LONG MaxAcquires)
{
    LOG_FUNC_ENTRY_PARAMS((Source, MaxAcquires))
    CTL_ITwainSource *p = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    bool bRetval = false;
    if (p)
    {
        // Check if array is of the correct type
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return (MaxAcquires < 0L && MaxAcquires != DTWAIN_MAXACQUIRE); },
            DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
        bRetval = true;
        p->SetMaxAcquisitions(MaxAcquires);
        p->SetUIMaxAcquisitions(MaxAcquires);
    }
    LOG_FUNC_EXIT_PARAMS(bRetval)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetMaxAcquisitions(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    CTL_ITwainSource *p = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);
    if (p)
    {
        const LONG Ret = p->GetMaxAcquisitions();
        LOG_FUNC_EXIT_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_PARAMS(DTWAIN_FAILURE2)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_GetMaxPagesToAcquire(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, DTWAIN_FAILURE2, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(DTWAIN_FAILURE2)
    const LONG Ret = p->GetMaxAcquireCount();
    LOG_FUNC_EXIT_PARAMS(Ret)
    CATCH_BLOCK(DTWAIN_FAILURE2)
}
