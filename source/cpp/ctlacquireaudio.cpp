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
#include "errorcheck.h"
#include "sourceacquireopts.h"
#include "sourceselectopts.h"
#include "acquisitionarray.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_AcquireAudioNative(DTWAIN_SOURCE Source, LONG nMaxAudioClips,
                                                    DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nMaxAudioClips, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);

    DTWAIN_ARRAY Acquisitions = DTWAIN_CreateAcquisitionArray();
    AcquisitionArrayRAII raii(Acquisitions, false);
    auto bRet = DTWAIN_AcquireAudioNativeEx(Source, nMaxAudioClips, bShowUI, bCloseSource, Acquisitions, pStatus);
    if ( bRet )
        raii.bDestroy = false;

    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(Acquisitions)
    CATCH_BLOCK_LOG_PARAMS(nullptr)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioNativeEx(DTWAIN_SOURCE Source, LONG nMaxAudioClips, DTWAIN_BOOL bShowUI,
                                                     DTWAIN_BOOL bCloseSource, DTWAIN_ARRAY Acquisitions, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nMaxAudioClips, bShowUI, bCloseSource, Acquisitions, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);

    // Check if audio transfers are supported
    auto val = pSource->IsAudioTransferSupported();
    DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] {return !val; }, DTWAIN_ERR_AUDIO_TRANSFER_NOTSUPPORTED, nullptr, FUNC_MACRO);

    auto ret = dynarithmic::AcquireHelper(pHandle, pSource, 
                                                    ACQUIREAUDIONATIVEEX, false, 
                                                  0, false, Acquisitions, 
                                                      0, nMaxAudioClips, bShowUI, nullptr, pStatus);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(ret.second)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AcquireAudioFile(DTWAIN_SOURCE Source, LPCTSTR lpszFile, LONG lFileFlags,
                                                  LONG nMaxAudioClips, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpszFile, lFileFlags, nMaxAudioClips, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);

    // Check if audio transfers are supported
    auto val = pSource->IsAudioTransferSupported();
    DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] {return !val; }, DTWAIN_ERR_AUDIO_TRANSFER_NOTSUPPORTED, nullptr, FUNC_MACRO);

    lFileFlags &= ~DTWAIN_USELIST;

    FileAcquireOptions fileOps = {};
    fileOps.fileFlags = lFileFlags &= ~DTWAIN_USELIST;
    fileOps.fileList = nullptr;
    fileOps.fileName = lpszFile;
    
    const bool bRetval = dynarithmic::AcquireHelper(pHandle, pSource, 
        ACQUIREAUDIOFILE,false,0,false,
        nullptr,0,nMaxAudioClips,bShowUI, &fileOps, pStatus).second;

    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
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

