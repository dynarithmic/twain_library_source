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
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwmgr.h"
#include "enumeratorfuncs.h"

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetJobControl(DTWAIN_SOURCE Source, LONG JobControl, DTWAIN_BOOL bSetCurrent)
{
    LOG_FUNC_ENTRY_PARAMS((Source, JobControl, bSetCurrent))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *p = VerifySourceHandle( pHandle, Source );
    if (!p)
        LOG_FUNC_EXIT_PARAMS(false)
    LONG SetType = DTWAIN_CAPSET;
    if ( !bSetCurrent )
    {
        SetType = DTWAIN_CAPRESET;
        JobControl = TWJC_NONE;
    }
    const DTWAIN_ARRAY Array = DTWAIN_ArrayCreateFromCap(nullptr, DTWAIN_CV_CAPJOBCONTROL, 1);
    if ( !Array )
        LOG_FUNC_EXIT_PARAMS(false)
    DTWAINArrayLL_RAII a(Array);

    auto& vValues = EnumeratorVector<LONG>(Array);
    vValues[0] = JobControl;

    const DTWAIN_BOOL bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPJOBCONTROL, SetType, Array );
    if ( bRet )
    {
        // Set the source value in the cache
        CTL_ITwainSource *pSource = VerifySourceHandle( pHandle, Source );
        pSource->SetCurrentJobControl(static_cast<TW_UINT16>(JobControl));
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsJobControlSupported(DTWAIN_SOURCE Source, LONG JobControl)
{
    LOG_FUNC_ENTRY_PARAMS((Source, JobControl))
    DTWAIN_ARRAY Array = nullptr;
    DTWAINArrayLL_RAII raii(Array);
    DTWAIN_BOOL bRet = FALSE;
    if ( DTWAIN_EnumJobControls(Source, &Array) )
    {
        auto& vValues = EnumeratorVector<int>(Array);
        const LONG lCount = static_cast<LONG>(vValues.size());
        if ( lCount < 1 )
            LOG_FUNC_EXIT_PARAMS(false)
        const LONG CurType = vValues[0];
        if ( lCount == 1 && CurType == TWJC_NONE)
            LOG_FUNC_EXIT_PARAMS(false)
        if ( JobControl == DTWAIN_ANYSUPPORT )
            LOG_FUNC_EXIT_PARAMS(true)

        const auto it = find(vValues.begin(), vValues.end(), JobControl);
        if ( it != vValues.end())
            bRet = TRUE;
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnableJobFileHandling(DTWAIN_SOURCE Source, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bSet))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *p = VerifySourceHandle( pHandle, Source );
    if (!p)
        LOG_FUNC_EXIT_PARAMS(false)
    p->SetJobFileHandling(bSet?true:false);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}
