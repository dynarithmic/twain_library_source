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
#include "errorcheck.h"
#include "sourceacquireopts.h"
#include "ctltwainmsgloop.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#pragma warning (disable:4714)
#endif

using namespace dynarithmic;

DTWAIN_BOOL   DLLENTRY_DEF DTWAIN_AcquireBufferedEx(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, DTWAIN_ARRAY Acquisitions,
                                                    LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PixelType, nMaxPages, bShowUI, bCloseSource, Acquisitions, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pHandle).setSource(Source).setPixelType(PixelType).setMaxPages(nMaxPages).
        setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).setUserArray(Acquisitions).
        setAcquireType(ACQUIREBUFFEREX);

    const bool bRet = AcquireExHelper(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_AcquireBuffered(DTWAIN_SOURCE Source, LONG PixelType, LONG nMaxPages, DTWAIN_BOOL bShowUI, DTWAIN_BOOL bCloseSource, LPLONG   pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, PixelType, nMaxPages, bShowUI, bCloseSource, pStatus))
    auto [pHandle, pSource] = VerifyHandles(Source);
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pHandle).setSource(Source).setPixelType(PixelType).setMaxPages(nMaxPages).
                                                        setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).
                                                        setAcquireType(ACQUIREBUFFER);
    const DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_NONAME_PARAMS(aDibs)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_ARRAY(0))
}

static int CheckTiledBufferedSupport(CTL_ITwainSource* pSource)
{
    if (!CTL_TwainAppMgr::IsSourceOpen(pSource))
        return DTWAIN_ERR_SOURCE_NOT_OPEN;
    if (!pSource->IsCapInSupportedList(ICAP_TILES))
        return DTWAIN_ERR_TILES_NOT_SUPPORTED;
    return DTWAIN_NO_ERROR;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetBufferedTileMode(DTWAIN_SOURCE Source, DTWAIN_BOOL bTileMode)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bTileMode))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto bRet = CheckTiledBufferedSupport(pSource);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return bRet != DTWAIN_NO_ERROR; }, bRet, false, FUNC_MACRO);
    pSource->SetTileMode(bTileMode);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsBufferedTileModeOn(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    LOG_FUNC_EXIT_NONAME_PARAMS(pSource->IsTileModeOn())
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsBufferedTileModeSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto bRet = CheckTiledBufferedSupport(pSource);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return bRet != DTWAIN_NO_ERROR; }, bRet, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(true);
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireBuffered(SourceAcquireOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    const DTWAIN_SOURCE Source = opts.getSource();
    auto pSource = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = pSource->GetDTWAINHandle();

    if (pSource->IsTileModeOn())
    {
        // Set the ICAP_TILES capability on here
        DTWAIN_ARRAY arr = dynarithmic::CreateArrayFromCap(pHandle, pSource, ICAP_TILES, 1);
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(arr);
        vValues[0] = 1;
        bool bTilesSet = DTWAIN_SetCapValues(Source, ICAP_TILES, DTWAIN_CAPSET, arr);
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !bTilesSet; }, DTWAIN_ERR_TILEMODE_NOTSET, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
    }

    LONG compressionType;

    if (!DTWAIN_GetCompressionType(Source, &compressionType, TRUE))
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return false; }, DTWAIN_ERR_COMPRESSION, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);

    pSource->SetCompressionType(compressionType);
    opts.setActualAcquireType(TWAINAcquireType_Buffer);
    if (pHandle->m_lAcquireMode == DTWAIN_MODELESS)
        return LLAcquireImage(opts);
    auto pr = dynarithmic::StartModalMessageLoop(opts.getSource(), opts);
    LOG_FUNC_EXIT_NONAME_PARAMS(pr.second)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}
