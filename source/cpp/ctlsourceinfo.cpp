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
#include "ctliface.h"
#include "ctltwmgr.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

typedef CTL_StringType (CTL_ITwainSource::*SOURCEINFOFUNC)() const;
static LONG GetSourceInfo(CTL_ITwainSource *p, SOURCEINFOFUNC pFunc, LPTSTR szInfo, LONG nMaxLen);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceValid(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if (!Source)
        LOG_FUNC_EXIT_PARAMS(false)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    bool bRet = false;
    if (pHandle)
    {
        CTL_ITwainSource* p = static_cast<CTL_ITwainSource*>(Source);
        bRet = CTL_TwainAppMgr::IsValidTwainSource(pHandle->m_pTwainSession, p);
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

LONG   DLLENTRY_DEF DTWAIN_GetSourceManufacturer( DTWAIN_SOURCE Source, LPTSTR szMan, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szMan, nMaxLen))
    CTL_ITwainSource *p = VerifySourceHandle( GetDTWAINHandle_Internal(), Source );
    if (p)
    {
        const LONG Ret = GetSourceInfo(p, static_cast<SOURCEINFOFUNC>(&CTL_ITwainSource::GetManufacturer), szMan, nMaxLen);
        LOG_FUNC_EXIT_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG   DLLENTRY_DEF DTWAIN_GetSourceProductFamily( DTWAIN_SOURCE Source, LPTSTR szProduct, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szProduct, nMaxLen))
    CTL_ITwainSource *p = VerifySourceHandle( GetDTWAINHandle_Internal(), Source );
    if (p)
    {
        const LONG Ret = GetSourceInfo(p, static_cast<SOURCEINFOFUNC>(&CTL_ITwainSource::GetProductFamily),
                                       szProduct, nMaxLen);
        LOG_FUNC_EXIT_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG   DLLENTRY_DEF DTWAIN_GetSourceProductName(DTWAIN_SOURCE Source,LPTSTR szProduct,LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szProduct, nMaxLen))
    CTL_ITwainSource *p = VerifySourceHandle( GetDTWAINHandle_Internal(), Source );
    if (p)
    {
        const LONG Ret = GetSourceInfo(p, static_cast<SOURCEINFOFUNC>(&CTL_ITwainSource::GetProductName),
                                       szProduct, nMaxLen);
        LOG_FUNC_EXIT_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_GetSourceVersionInfo(DTWAIN_SOURCE Source, LPTSTR szVInfo, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szVInfo, nMaxLen))
    CTL_ITwainSource *p = VerifySourceHandle( GetDTWAINHandle_Internal(), Source );
    if (p)
    {
        const TW_VERSION *pV = p->GetVersion();
        CTL_StringType pName = StringConversion::Convert_AnsiPtr_To_Native(pV->Info);
        const size_t nLen = pName.length();
        if ( szVInfo == nullptr)
            LOG_FUNC_EXIT_PARAMS((LONG)nLen)

        std::copy_n(pName.begin(), nLen, szVInfo);
        szVInfo[nLen] = _T('\0');
        LOG_FUNC_EXIT_PARAMS((LONG)nLen)
    }
    LOG_FUNC_EXIT_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

static LONG GetSourceInfo(CTL_ITwainSource *p,SOURCEINFOFUNC pFunc,LPTSTR szInfo, LONG nMaxLen)
{
    return StringWrapper::CopyInfoToCString((p->*pFunc)(), szInfo, nMaxLen);
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetSourceVersionNumber( DTWAIN_SOURCE Source, LPLONG pMajor, LPLONG pMinor)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pMajor, pMinor))
    CTL_ITwainSource *p = VerifySourceHandle( GetDTWAINHandle_Internal(), Source );
    if (p)
    {
        const TW_VERSION *pV = p->GetVersion();
        if ( pMajor )
            *pMajor = pV->MajorNum;
        if ( pMinor)
            *pMinor = pV->MinorNum;
        LOG_FUNC_EXIT_PARAMS(true)
    }
    if ( pMajor )
        *pMajor = -1L;
    if ( pMinor )
        *pMinor = -1L;
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

