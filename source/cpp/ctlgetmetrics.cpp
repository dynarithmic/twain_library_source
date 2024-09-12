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
#include "ctltr041.h"
#include "ctliface.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif
using namespace dynarithmic;

static bool GetMetrics(DTWAIN_SOURCE Source, LPLONG ImageCount, LPLONG SheetCount);

///////////////////////////////////////////////////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireMetrics(DTWAIN_SOURCE Source, LPLONG ImageCount, LPLONG SheetCount)
{
    LOG_FUNC_ENTRY_PARAMS((Source, ImageCount, SheetCount))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const DTWAIN_BOOL bRet = GetMetrics(Source, ImageCount, SheetCount)?TRUE:FALSE;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

static bool GetMetrics(DTWAIN_SOURCE Source, LPLONG ImageCount, LPLONG SheetCount)
{
    auto p = static_cast<CTL_ITwainSource*>(Source);
    CTL_DSMMetricsTriplet triplet(p->GetTwainSession(), p);
    const TW_UINT16 rc = triplet.Execute();
    switch (rc)
    {
        case TWRC_SUCCESS:
        {
            const TW_METRICS& metrics = triplet.getMetrics();
            if (ImageCount)
                *ImageCount = static_cast<LONG>(metrics.ImageCount);
            if (SheetCount)
                *SheetCount = static_cast<LONG>(metrics.SheetCount);
            return true;
        }
        default:
        {}
    }
    CTL_TwainAppMgr::ProcessReturnCodeOneValue(p, rc);
    return false;
}
