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
#include "arrayfactory.h"
using namespace dynarithmic;

/* Duplex Scanner support */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetDuplexType(DTWAIN_SOURCE Source, LPLONG lpDupType)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpDupType))
    bool bRet = true;
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPDUPLEX))
    {
        if ( lpDupType )
            lpDupType = nullptr;
        bRet = false;
    }
    else
    {
        if ( lpDupType )
        {
            DTWAIN_ARRAY Array = nullptr;
            const DTWAIN_BOOL bRet2 = DTWAIN_GetCapValues(Source, DTWAIN_CV_CAPDUPLEX, DTWAIN_CAPGET, &Array) ? true : false;
            const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            DTWAINArrayLowLevel_RAII arr(pHandle, Array);
            if ( bRet2 && Array)
            {
                auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);
                if ( !vValues.empty() )
                    *lpDupType = vValues.front();
            }
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsDuplexSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    LONG DupType;
    bool bRet = false;
    if ( DTWAIN_GetDuplexType(Source, &DupType) )
    {
        if ( DupType == TWDX_1PASSDUPLEX ||
             DupType == TWDX_2PASSDUPLEX )
            bRet = true;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}
