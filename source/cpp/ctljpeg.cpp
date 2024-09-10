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
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwmgr.h"

using namespace dynarithmic;

/////////////////////////////  JPEG Settings ///////////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetJpegValues(DTWAIN_SOURCE Source, LONG Quality, LONG Progressive)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Quality, Progressive))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetJpegValues(Quality, Progressive?true:false);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetJpegValues(DTWAIN_SOURCE Source, LPLONG pQuality, LPLONG Progressive)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pQuality, Progressive))
    auto [pHandle, pSource] = VerifyHandles(Source);
    bool PrTemp;
    pSource->GetJpegValues(*pQuality, PrTemp);
    *Progressive = static_cast<LONG>(PrTemp);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

