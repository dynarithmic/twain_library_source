/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

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
#include <algorithm>
#include "cppfunc.h"
#include "dtwainx.h"
#include "ctldtwainhandle.h"
#include "errorcheck.h"

using namespace dynarithmic;

/////////////////////////////  JPEG Settings ///////////////////////////////
extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetJpegValues(DTWAIN_SOURCE Source, LONG Quality, LONG Progressive)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, Quality, Progressive))
        auto [pHandle, pSource] = VerifyHandles(Source);
        Quality = std::clamp(Quality, 0L, 100L);
        pSource->SetJpegValues(Quality, Progressive?true:false);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetJpegXRValues(DTWAIN_SOURCE Source, LONG Quality, LONG Progressive)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, Quality, Progressive))
        auto [pHandle, pSource] = VerifyHandles(Source);
        Quality = std::clamp(Quality, 0L, 100L);
        pSource->SetJpegXRValues(Quality, Progressive?true:false);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetJpegXLQuality(DTWAIN_SOURCE Source, LONG Quality)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, Quality))
        auto [pHandle, pSource] = VerifyHandles(Source);
        Quality = std::clamp(Quality, 0L, 100L);
        pSource->SetJpegXLValues(Quality);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetJpegXLThreadCount(LONG ThreadCount)
    {
        LOG_FUNC_ENTRY_PARAMS((ThreadCount))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if (ThreadCount < DTWAIN_DEFAULT)
            ThreadCount = DTWAIN_DEFAULT;
        CTL_StaticData::GetJPEGXLNumThreads() = ThreadCount;
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    LONG DLLENTRY_DEF DTWAIN_GetJpegXLThreadCount()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto threadCount = CTL_StaticData::GetJPEGXLNumThreads();
        LOG_FUNC_EXIT_NONAME_PARAMS(threadCount)
        CATCH_BLOCK_LOG_PARAMS(DTWAIN_FAILURE2)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetJpegValues(DTWAIN_SOURCE Source, LPLONG pQuality, LPLONG Progressive)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, pQuality, Progressive))
        auto [pHandle, pSource] = VerifyHandles(Source);
        bool PrTemp;
        LONG tempQuality;
        pSource->GetJpegValues(tempQuality, PrTemp);
        if (pQuality)
            *pQuality = tempQuality;
        if ( Progressive )
            *Progressive = static_cast<LONG>(PrTemp);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pQuality, Progressive))
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetJpegXRValues(DTWAIN_SOURCE Source, LPLONG pQuality, LPLONG Progressive)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, pQuality, Progressive))
        auto [pHandle, pSource] = VerifyHandles(Source);
        bool PrTemp;
        LONG tempQuality;
        pSource->GetJpegXRValues(tempQuality, PrTemp);
        if ( pQuality )
            *pQuality = tempQuality;
        if ( Progressive)
            *Progressive = static_cast<LONG>(PrTemp);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pQuality, Progressive))
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetJpegXLQuality(DTWAIN_SOURCE Source, LPLONG pQuality)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, pQuality))
        auto [pHandle, pSource] = VerifyHandles(Source);
        LONG tempQuality;
        pSource->GetJpegXLValues(tempQuality);
        if ( pQuality )
            *pQuality = tempQuality;
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pQuality))
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
        
    }
}