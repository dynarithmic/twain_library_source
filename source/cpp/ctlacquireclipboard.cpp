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
#include "arrayfactory.h"
#include "sourceacquireopts.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_ARRAY  DLLENTRY_DEF DTWAIN_AcquireToClipboard(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, LONG nTransferMode, DTWAIN_BOOL bDiscardDibs, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource,
    LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PixelType, nMaxPages, nTransferMode, bDiscardDibs, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pHandle).setSource(Source).setPixelType(PixelType).setMaxPages(nMaxPages).
        setTransferMode(nTransferMode).setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).
        setAcquireType(ACQUIRECLIPBOARD).setDiscardDibs(bDiscardDibs ? true : false);

    DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    if (aDibs && bDiscardDibs)
    {
        auto& factory = pHandle->m_ArrayFactory;
        DTWAINArrayLowLevel_RAII arrAcq(pHandle, aDibs);
        auto& vValues = factory->underlying_container_t<HANDLE>(aDibs);
        const LONG nCount = static_cast<LONG>(vValues.size());
        for (LONG i = 0; i < nCount; i++)
        {
            DTWAIN_ARRAY aDibHandle = vValues[i];
            DTWAINArrayLowLevel_RAII arr(pHandle, aDibHandle);
            const auto& vDibs = factory->underlying_container_t<HANDLE>(aDibHandle);

            if (vDibs.empty())
                continue;

            const LONG nCount2 = static_cast<LONG>(vDibs.size());
            // Leave last DIB.  This is the one in the clipboard!!
            LONG nLastDib = nCount2;
            if (i == nCount - 1)
                nLastDib = nCount2 - 1;

            for (int j = 0; j < nLastDib; j++)
            {
                const HANDLE hDib = DTWAIN_GetAcquiredImage(aDibs, i, j);
                if (pHandle->m_TwainMemoryFunc->LockMemory(hDib))
                {
                    pHandle->m_TwainMemoryFunc->UnlockMemory(hDib);
                    pHandle->m_TwainMemoryFunc->FreeMemory(hDib);
                }
            }
        }
    }
    else
    if (!aDibs)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)

    if (!bDiscardDibs)
        LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)

    aDibs = reinterpret_cast<HANDLE>(1);
    LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_ARRAY(0))
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireToClipboard(SourceAcquireOptions& opts)
{
    LONG lTransferMode = opts.getTransferMode();
    if (lTransferMode != DTWAIN_USENATIVE && lTransferMode != DTWAIN_USEBUFFERED)
        lTransferMode = DTWAIN_USENATIVE;
    opts.setActualAcquireType(TWAINAcquireType_Clipboard).setTransferMode(lTransferMode);
    return LLAcquireImage(opts);
}
