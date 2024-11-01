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
#include "ctltwainmanager.h"
#include "ctltwainmsgloop.h"
#include "sourceacquireopts.h"
#include "dtwain_paramlogger.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif
using namespace dynarithmic;

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_AcquireNative(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG pStatus)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS(Source, PixelType, nMaxPages, bShowUI, bCloseSource, pStatus) 
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pSource->GetDTWAINHandle()).setSource(Source).setPixelType(PixelType).setMaxPages(nMaxPages).
                                                           setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).setAcquireType(ACQUIRENATIVE);
    const DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    if (pSource->GetLastAcquireError() != 0)
        CTL_TwainAppMgr::SetError(pSource->GetLastAcquireError(),"",false);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_ARRAY(0))
}

DTWAIN_BOOL   DLLENTRY_DEF  DTWAIN_AcquireNativeEx(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource,DTWAIN_ARRAY Acquisitions,
                                                   LPLONG pStatus)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS(Source, PixelType, nMaxPages, bShowUI, bCloseSource, Acquisitions, pStatus)
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setSource(Source).setPixelType(PixelType).setMaxPages(nMaxPages).
            setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).setUserArray(Acquisitions).
            setAcquireType(ACQUIRENATIVEEX).setHandle(pHandle);

    const bool bRet = AcquireExHelper(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    if (pSource->GetLastAcquireError() != 0)
        CTL_TwainAppMgr::SetError(pSource->GetLastAcquireError(), "", false);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireNative(SourceAcquireOptions& opts)
{
    opts.setActualAcquireType(TWAINAcquireType_Native);
    const auto pHandle = static_cast<CTL_ITwainSource*>(opts.getSource())->GetDTWAINHandle();
    if ( pHandle->m_lAcquireMode == DTWAIN_MODELESS )
         return LLAcquireImage(opts);
    auto pr = dynarithmic::StartModalMessageLoop(opts.getSource(), opts);
    return pr.second;
}
