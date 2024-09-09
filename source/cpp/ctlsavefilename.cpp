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
#include <algorithm>
#include "cppfunc.h"
#include "ctltwmgr.h"
#include "ctliface.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

//////////////////// Source information functions /////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetSaveFileName(DTWAIN_SOURCE Source, LPCTSTR fileName)
{
    LOG_FUNC_ENTRY_PARAMS((Source, fileName))
    auto [pHandle, pSource] = VerifySourceHandle(Source);
    pSource->SetActualFileName(fileName);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetSaveFileName(DTWAIN_SOURCE Source, LPTSTR fileName, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Source, fileName, nMaxLen))
    auto [pHandle, pSource] = VerifySourceHandle(Source);
    const LONG nTotalBytes = StringWrapper::CopyInfoToCString(pSource->GetActualFileName(), fileName, nMaxLen);
    LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
    CATCH_BLOCK_LOG_PARAMS(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetCurrentFileName(DTWAIN_SOURCE Source, LPTSTR szName, LONG MaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szName, MaxLen))
    auto [pHandle, pSource] = VerifySourceHandle(Source);
    const CTL_StringType s = pSource->GetLastAcquiredFileName();
    const size_t sLen = s.length()  + 1;
    if (!szName)
        LOG_FUNC_EXIT_NONAME_PARAMS((LONG)sLen)

    const size_t nLenToUse = (std::min)(sLen, static_cast<size_t>(MaxLen));
    const CTL_StringType::value_type* sCopy = s.c_str();
    std::copy(sCopy, sCopy + nLenToUse - 1, szName);
    szName[nLenToUse-1] = _T('\0');
    LOG_FUNC_EXIT_NONAME_PARAMS((LONG)sLen)
    CATCH_BLOCK_LOG_PARAMS(-1L)
}

