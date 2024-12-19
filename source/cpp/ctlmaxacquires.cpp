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
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetMaxAcquisitions(DTWAIN_SOURCE Source, LONG MaxAcquires)
{
    LOG_FUNC_ENTRY_PARAMS((Source, MaxAcquires))
    auto [pHandle, pSource] = VerifyHandles(Source);
    // Check if array is of the correct type
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return MaxAcquires < 0L && MaxAcquires != DTWAIN_MAXACQUIRE; },
        DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    pSource->SetMaxAcquisitions(MaxAcquires);
    pSource->SetUIMaxAcquisitions(MaxAcquires);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

LONG DLLENTRY_DEF DTWAIN_GetMaxAcquisitions(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const LONG Ret = pSource->GetMaxAcquisitions();
    LOG_FUNC_EXIT_NONAME_PARAMS(Ret)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_GetMaxPagesToAcquire(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const LONG Ret = pSource->GetMaxAcquireCount();
    LOG_FUNC_EXIT_NONAME_PARAMS(Ret)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_FAILURE2)
}
