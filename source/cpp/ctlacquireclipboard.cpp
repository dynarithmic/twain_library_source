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
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "sourceacquireopts.h"
#include "acquisitionarray.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

static bool CopyDibToClipboard(HANDLE hDib)
{
#ifdef _WIN32
    if (hDib)
    {
        // Open the clipboard
        if (OpenClipboard(nullptr/*hWnd*/))
        {
            // Empty the clipboard
            if (EmptyClipboard())
            {
                SetClipboardData(CF_DIB, hDib);
                CloseClipboard();
                return true;
            }
            CloseClipboard();
            return true;
        }
    }
    return false;
#endif
}

DTWAIN_ARRAY  DLLENTRY_DEF DTWAIN_AcquireToClipboard(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, LONG nTransferMode, DTWAIN_BOOL bDiscardDibs, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource,
                                                     LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PixelType, nMaxPages, nTransferMode, bDiscardDibs, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);

    DTWAIN_ARRAY aDibs = DTWAIN_CreateAcquisitionArray();
    if (!aDibs)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    AcquisitionArrayRAII raii(aDibs, true);

    int actualAcquireMode = ACQUIREBUFFEREDEX;
    if (nTransferMode == DTWAIN_USENATIVE)
        actualAcquireMode = ACQUIRENATIVEEX;

    bool bRet = dynarithmic::AcquireHelper(pHandle, pSource, actualAcquireMode, bDiscardDibs, 
                                       nTransferMode, true, 
                                       aDibs, PixelType, nMaxPages, bShowUI,nullptr, pStatus).second;
    if (bRet)
    {
        LONG numAcquisitions = DTWAIN_GetNumAcquisitions(aDibs);
        if (numAcquisitions > 0)
        {
            LONG numImages = DTWAIN_GetNumAcquiredImages(aDibs, numAcquisitions - 1);
            if (numImages > 0)
            {
                const HANDLE last_dib = DTWAIN_GetAcquiredImage(aDibs, numAcquisitions - 1, numImages - 1);
                if (last_dib)
                {
                    HANDLE copied_dib = CDibInterface::CopyDib(last_dib);
                    if (copied_dib)
                    {
                        bool bInClip = CopyDibToClipboard(copied_dib);
                        if (!bInClip)
                            LogWin32Error(GetLastError());
                        else
                            CTL_TwainAppMgr::SendTwainMsgToWindow(pSource->GetTwainSession(), nullptr, 
                                                                  DTWAIN_TN_CLIPTRANSFERDONE, 
                                                                  reinterpret_cast<LPARAM>(pSource));
                    }
                }
            }
        }
    }
    if (!bDiscardDibs)
        raii.bDestroy = false;
    else
        aDibs = reinterpret_cast<DTWAIN_ARRAY>(1);

    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)
    CATCH_BLOCK_LOG_PARAMS(nullptr)
}


#if 0
// This has been deprecated as of version 5.9.3.  
// This function will now default to DTWAIN_AcquireNative or DTWAIN_AcquireBuffered.
// If data is required to be on the clipboard, the application should call the DTWAIN_CopyDIBToClipboard function.
DTWAIN_ARRAY  DLLENTRY_DEF DTWAIN_AcquireToClipboard(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, LONG nTransferMode, DTWAIN_BOOL bDiscardDibs, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource,
    LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PixelType, nMaxPages, nTransferMode, bDiscardDibs, bShowUI, bCloseSource, pStatus))
        auto [pHandle, pSource] = VerifyHandles(Source);

    DTWAIN_ARRAY aDibs = {};

    int actualAcquireMode = ACQUIREBUFFERED;
    if (nTransferMode == DTWAIN_USENATIVE)
        actualAcquireMode = ACQUIRENATIVE;

    aDibs = dynarithmic::AcquireHelper(pHandle, pSource, actualAcquireMode, bDiscardDibs,
        nTransferMode, true,
        nullptr, PixelType, nMaxPages, bShowUI, pStatus).first;

    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
        LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)
        CATCH_BLOCK_LOG_PARAMS(nullptr)
}
#endif
