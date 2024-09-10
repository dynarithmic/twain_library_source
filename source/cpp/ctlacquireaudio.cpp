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
#include "sourceacquireopts.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_AcquireAudioNative(DTWAIN_SOURCE Source, LONG nMaxAudioClips,
                                                    DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nMaxAudioClips, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pSource->GetDTWAINHandle()).setSource(Source).
                                                           setMaxPages(nMaxAudioClips).
                                                           setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).
                                                           setAcquireType(ACQUIREAUDIONATIVE);
    const DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_ARRAY(0))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioNativeEx(DTWAIN_SOURCE Source, LONG nMaxAudioClips, DTWAIN_BOOL bShowUI,
                                                     DTWAIN_BOOL bCloseSource, DTWAIN_ARRAY Acquisitions, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nMaxAudioClips, bShowUI, bCloseSource, Acquisitions, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setSource(Source).setMaxPages(nMaxAudioClips).
    setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).setUserArray(Acquisitions).
    setAcquireType(ACQUIREAUDIONATIVEEX).setHandle(pHandle);

    const bool bRet = AcquireExHelper(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioFile(DTWAIN_SOURCE Source, LPCTSTR lpszFile, LONG lFileFlags,
                                                  LONG nMaxAudioClips, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpszFile, lFileFlags, nMaxAudioClips, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);
    lFileFlags &= ~DTWAIN_USELIST;
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pHandle).setSource(Source).
                                setFileName(lpszFile).setFileFlags(lFileFlags).
                                setMaxPages(nMaxAudioClips).setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).setAcquireType(ACQUIREAUDIOFILE);
    const bool bRetval = AcquireFileHelper(opts, ACQUIREAUDIOFILE);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireAudioNative(SourceAcquireOptions& opts)
{
    opts.setActualAcquireType(TWAINAcquireType_AudioNative);
    return LLAcquireImage(opts);
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireAudioFile(SourceAcquireOptions& opts)
{
    opts.setActualAcquireType(TWAINAcquireType_AudioFile);
    return DTWAIN_LLAcquireFile(opts);
}

