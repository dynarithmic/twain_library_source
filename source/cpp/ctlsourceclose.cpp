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
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

static DTWAIN_BOOL DTWAIN_CloseSourceUnconditional(CTL_TwainDLLHandle *pHandle, CTL_ITwainSource *pSource);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_CloseSource(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    bool bRetval = false;
    const auto sProductName = pSource->GetProductName();
    bRetval = DTWAIN_CloseSourceUnconditional(pHandle, pSource)?true:false;
    if (bRetval)
    {
        pHandle->m_mapStringToSource.erase(sProductName);
        pHandle->m_aFeederSources.erase(Source);
    }
    std::string sProductNameA = StringConversion::Convert_Native_To_Ansi(sProductName);
    auto& sourceMap = CTL_StaticData::GetSourceStatusMap();
    auto iter = sourceMap.find(sProductNameA);
    if (iter != sourceMap.end())
    {
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_OPEN, false);
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_SELECECTED, false);
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, false);
        iter->second.SetSourceHandle({});
        iter->second.SetThreadID({});
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_CloseSourceUI(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    CTL_TwainAppMgr::EndTwainUI(pHandle->m_pTwainSession, pSource);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DTWAIN_CloseSourceUnconditional(CTL_TwainDLLHandle *pHandle, CTL_ITwainSource *p)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    bool bRetval = false;

    if (pHandle->m_nSourceCloseMode == DTWAIN_SourceCloseModeFORCE &&
        p->IsAcquireAttempt())
    {
        CTL_TwainAppMgr::DisableUserInterface(p);
        p->SetAcquireAttempt(false);
    }
    else
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return p->IsAcquireAttempt(); },
        DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

    bRetval = CTL_TwainAppMgr::CloseSource(pHandle->m_pTwainSession, p)?true:false;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval)
    CATCH_BLOCK(false)
}
