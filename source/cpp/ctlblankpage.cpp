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
#include "errorcheck.h"
using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionExString(DTWAIN_SOURCE Source, LPCTSTR threshold,
                                                              LONG autodetect, LONG detectOpts, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, threshold, autodetect, detectOpts, bSet))
    const DTWAIN_FLOAT val1 = StringWrapper::ToDouble(threshold);
    const DTWAIN_BOOL bRet = DTWAIN_SetBlankPageDetectionEx(Source, val1, autodetect, detectOpts, bSet);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionString(DTWAIN_SOURCE Source, LPCTSTR threshold,
                                                            LONG autodetect, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, threshold, autodetect, bSet))
    const DTWAIN_FLOAT val1 = StringWrapper::ToDouble(threshold);
    const DTWAIN_BOOL bRet = DTWAIN_SetBlankPageDetection(Source, val1, autodetect, bSet);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetection(DTWAIN_SOURCE Source, DTWAIN_FLOAT threshold,
                                                      LONG autodetect, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, threshold, autodetect, bSet))
    const DTWAIN_BOOL bRet = DTWAIN_SetBlankPageDetectionEx(Source, threshold, autodetect,
                                                            DTWAIN_BP_DETECTADJUSTED | DTWAIN_BP_DETECTORIGINAL, bSet);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBlankPageDetectionEx(DTWAIN_SOURCE Source, DTWAIN_FLOAT threshold,
                                                        LONG autodetect, LONG detectOpts, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, threshold, autodetect, detectOpts, bSet))

    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);

    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(false)
    if (detectOpts & DTWAIN_BP_DETECTORIGINAL )
        pSource->SetBlankPageDetectionNoSampleOn(bSet ? true : false);
    if (detectOpts & DTWAIN_BP_DETECTADJUSTED)
        pSource->SetBlankPageDetectionSampleOn(bSet ? true : false);
    pSource->SetBlankPageDetectionOn(pSource->IsBlankPageDetectionNoSampleOn() || pSource->IsBlankPageDetectionSampleOn());
    pSource->SetBlankPageThreshold(threshold / 100.0);
    if (autodetect > DTWAIN_BP_AUTODISCARD_ANY || autodetect < DTWAIN_BP_AUTODISCARD_NONE)
        autodetect = DTWAIN_BP_AUTODISCARD_NONE;
    pSource->SetBlankPageAutoDetect(autodetect);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetBlankPageAutoDetection(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    CTL_ITwainSource *pSource = VerifySourceHandle( pHandle, Source );

    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(-1)
    const LONG retval = pSource->GetBlankPageAutoDetect();
    LOG_FUNC_EXIT_PARAMS(retval)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsBlankPageDetectionOn(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    CTL_ITwainSource *pSource = VerifySourceHandle( pHandle, Source );

    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(-1)
    const DTWAIN_BOOL retval = pSource->IsBlankPageDetectionOn()?1:0;
    LOG_FUNC_EXIT_PARAMS(retval)
    CATCH_BLOCK(false)
}


LONG DLLENTRY_DEF DTWAIN_IsDIBBlankString(HANDLE hDib, LPCTSTR threshold)
{
    LOG_FUNC_ENTRY_PARAMS((hDib, threshold))
    const DTWAIN_FLOAT val = StringWrapper::ToDouble( threshold );
    const LONG retval = DTWAIN_IsDIBBlank(hDib, val);
    LOG_FUNC_EXIT_PARAMS(retval)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_IsDIBBlank(HANDLE hDib, DTWAIN_FLOAT threshold)
{
    LOG_FUNC_ENTRY_PARAMS((hDib, threshold))
    const LONG retval = CDibInterface::IsBlankDIB(hDib, threshold/100.0)?1:0;
    LOG_FUNC_EXIT_PARAMS(retval)
    CATCH_BLOCK(0)
}
