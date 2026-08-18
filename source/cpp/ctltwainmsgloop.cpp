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
#include "ctltwainmanager.h"
#include "sourceacquireopts.h"
#include "ctltr040.h"
#include "ctltwainmsgloop.h"
#include "windowsinit_impl.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif
#include <cppfunc.h>
#include <errorcheck.h>
#include "ctlshowuionly.h"
#include "ctldtwainhandle.h"
#include "ctlsourceacquire.h"

namespace
{
    bool DTWAIN_ShouldUseGetMessage()
    {
        if (!dynarithmic::CTL_StaticData::IsTestForGetMessage())
            return false;

        MSG msg;

        // 1) If no window belongs to this thread, likely script host
        DWORD thisThread = GetCurrentThreadId();
        bool hasWindow = false;

        EnumThreadWindows(thisThread,
            [](HWND, LPARAM lParam) -> BOOL
            {
                *reinterpret_cast<bool*>(lParam) = true;
                return FALSE;
            },
            reinterpret_cast<LPARAM>(&hasWindow));

        if (!hasWindow)
            return true; // safer to block


        // 2) Probe message responsiveness WITHOUT timing
        constexpr int kProbeCount = 3;  // small, deterministic

        for (int i = 0; i < kProbeCount; ++i)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_NOREMOVE))
                return false; // messages are flowing, so PeekMessage loop OK

            WaitMessage(); // cooperative yield (debugger-safe)
        }

        // No messages after several real waits, so prefer GetMessage
        return true;
    }
}

namespace dynarithmic
{
    std::pair<int, DTWAIN_ACQUIRE> StartModalMessageLoop(CTL_ITwainSource* pSource, SourceAcquireOptions& opts)
    {
        if (!pSource)
            return { false, -1 };

        const auto pHandle = pSource->GetDTWAINHandle();
        if (pHandle->m_lAcquireMode == DTWAIN_MODELESS)
            return { true, 0 };


        // Start a message loop here
        // TWAIN 1.x loop implementation
        TwainMessageLoopV1 v1Impl(pHandle);

        // TWAIN 2.x loop implementation
        TwainMessageLoopV2 v2Impl(pHandle);

        // default to version 1
        TwainMessageLoopImpl* pImpl = &v1Impl;

        // check for version 2 implementation
        if (CTL_TwainAppMgr::IsVersion2DSMUsed())
        {
            // assign the callback procedure
            CTL_DSMCallbackTripletRegister callbackSetter(CTL_TwainAppMgr::GetCurrentSession(), pSource, &TwainMessageLoopV2::TwainVersion2MsgProc);
            if (callbackSetter.Execute() == TWRC_SUCCESS)
                pImpl = &v2Impl;
        }

        // do any prep work before we loop
        pImpl->PrepareLoop();
        pImpl->SetAcquireOptions(opts);
        int retCode = pImpl->PerformMessageLoop(pSource, opts.getIsUIOnly());
        if (pSource->IsShutdownAcquire())
            return { retCode, -1 };
        return { retCode, pImpl->GetAcquireNum() };
    }
    
    void DTWAIN_AcquireProc(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE, WPARAM Data1, LPARAM)
    {
        const auto p = static_cast<CTL_TwainDLLHandle *>(DLLHandle);

        switch (Data1)
        {
            case DTWAIN_TN_ACQUIRESTARTED:
                p->m_bTransferDone = false;
                p->m_bSourceClosed = false;
                p->m_lLastAcqError = 0;
                break;

            case DTWAIN_TN_ACQUIREDONE:
                p->m_lLastAcqError = DTWAIN_TN_ACQUIREDONE;
                break;

            case DTWAIN_TN_ACQUIREFAILED:
                p->m_lLastAcqError = DTWAIN_TN_ACQUIREFAILED;
                break;

            case DTWAIN_TN_ACQUIRECANCELLED:
                p->m_lLastAcqError = DTWAIN_TN_ACQUIRECANCELLED;
                break;

            case DTWAIN_AcquireSourceClosed:
                break;

            case DTWAIN_TN_UICLOSED:
                if (p->m_lLastAcqError == 0)
                    p->m_lLastAcqError = DTWAIN_TN_ACQUIRECANCELLED;
                break;

            case DTWAIN_AcquireTerminated:
                p->m_bTransferDone = true;
                p->m_bSourceClosed = true;
                if (p->m_lLastAcqError == 0)
                    p->m_lLastAcqError = DTWAIN_TN_ACQUIRECANCELLED;
                break;
        }
    }

}

using namespace dynarithmic;

std::queue<MSG> TwainMessageLoopV2::s_MessageQueue;

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnablePeekMessageLoop(DTWAIN_SOURCE Source, BOOL bSet)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, bSet))
        if ( !Source )
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        auto [pHandle, pSource] = VerifyHandles(Source);
        auto pS = pSource;

        // Cannot change TWAIN message loop implementation while acquiring images
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] {return pS->IsTwainLoopStarted(); },
                                            DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

        pSource->SetUsePeekMessage(bSet);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnableGetMessageLoop(DTWAIN_SOURCE Source, BOOL bSet)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, bSet))
        auto bRet = DTWAIN_EnablePeekMessageLoop(Source, FALSE);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
        CATCH_BLOCK_LOG_PARAMS(false)
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPeekMessageLoopEnabled(DTWAIN_SOURCE Source)
    {
        LOG_FUNC_ENTRY_PARAMS((Source))
        if ( !Source )
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        auto [pHandle, pSource] = VerifyHandles(Source);
        auto bRet = pSource->IsUsePeekMessage();
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsGetMessageLoopEnabled(DTWAIN_SOURCE Source)
    {
        LOG_FUNC_ENTRY_PARAMS((Source))
        auto bRet = !DTWAIN_IsPeekMessageLoopEnabled(Source);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
        CATCH_BLOCK_LOG_PARAMS(false)
    }
}


bool TwainMessageLoopImpl::IsSourceOpen(CTL_ITwainSource* pSource)
{
    return !m_pDLLHandle->m_bTransferDone == true && !m_pDLLHandle->m_bSourceClosed == true;
}

bool TwainMessageLoopImpl::IsAcquireTerminated(CTL_ITwainSource* pSource, bool bUIOnly)
{
    if (pSource->IsShutdownAcquire())
        return true;
    if (bUIOnly)
        return !pSource->IsUIOpen();
    return !IsSourceOpen(pSource);
}

// Depending on the setting in pSource, we want to either loop
// on PeekMessage(), or rely on the return value of GetMessage().
// The settings are found in dtwain32.ini or dtwain64.ini under the
// "TwainLoopGetMsg" section.
namespace 
{
    struct ContinueLoopTraitsPeek
    {
        static constexpr bool isPeekMsg = true;
        static bool ContinueLoop(MSG* msg)
        {
            PeekMessage(msg, nullptr, 0, 0, PM_REMOVE);
            return true;
        }
    };

    struct ContinueLoopTraitsGet
    {
        static constexpr bool isPeekMsg = false;
        static bool ContinueLoop(MSG* msg)
        {
            auto bRet = GetMessage(msg, nullptr, 0, 0);
            return bRet != 0 && bRet != -1;
        }
    };

    template <typename LoopTraits = ContinueLoopTraitsGet>
    struct ContinueLoopTraits
    {
        static bool InvokeLoop(TwainMessageLoopImpl* pImpl, CTL_ITwainSource* pSource, bool isUIOnly)
        {
            MSG msg;
            auto& acquireRef = pImpl->GetAcquireNumRef();
            auto& sOpts = pImpl->GetAcquireOptions();
            bool bInitializeAcquisitionProcess = false;

            struct TwainWatchdog
            {
                DWORD lastProgressTick;
                DWORD timeoutMs;
                bool  triggered;
            };

            TwainWatchdog wd{ GetTickCount(), 3000, false };

            DWORD lastTwainProgressTick = GetTickCount();

            // Start the message loop.
            while (LoopTraits::ContinueLoop(&msg))
            {
                // If in UIOnly mode, set it up here
                if (isUIOnly && !bInitializeAcquisitionProcess)
                {
                    LLSetupUIOnly(pSource);
                    bInitializeAcquisitionProcess = true;
                    lastTwainProgressTick = GetTickCount();
                }

                // If acquire has been terminated, break out of this loop
                if (pImpl->IsAcquireTerminated(pSource, isUIOnly))
                {
                    if (isUIOnly)
                        pSource->SetUIOnly(false);
                    lastTwainProgressTick = GetTickCount();
                    break;
                }

                // If we haven't set up the TWAIN device for the acquisition,
                // do it now.  The LLAcquireImage() will also eventually show
                // the user-interface of the device, or acquire immediately if
                // no user-interface is being used.
                if (!bInitializeAcquisitionProcess)
                {
                    acquireRef = LLAcquireImage(sOpts);
                    bInitializeAcquisitionProcess = true;
                    lastTwainProgressTick = GetTickCount();

                    // Didn't get an acquisition number, so something failed
                    if (acquireRef == -1L)
                        break;
                }

                // This will test for TWAIN messages, Data Source messages or application messages.
                if (pImpl->CanEnterDispatch(&msg))
                {
                    TranslateMessage(&msg);
                    ::DispatchMessage(&msg);
                }

#if 0 // Note that this has not been implemented
                // PeekMessage watchdog
                if (LoopTraits::isPeekMsg)
                {
                    const DWORD timeoutMs = 3000;
                    if (GetTickCount() - lastTwainProgressTick > timeoutMs)
                    {
                        // no progress for timeout, exit loop
                        wd.triggered = true;
                        break;
                    }
                }
#endif
                // Optional throttle to avoid CPU spin 
                Sleep(1);
            }
            return wd.triggered;
        }
    };
}

int TwainMessageLoopWindowsImpl::PerformMessageLoop(CTL_ITwainSource* pSource, bool isUIOnly)
{
    int returnCode = DTWAIN_NO_ERROR;
    struct UIScopedRAII
    {
        CTL_ITwainSource* m_pSource;
        bool m_bOld;
        UIScopedRAII(CTL_ITwainSource* pSource) : m_pSource(pSource), m_bOld(m_pSource->IsUIOnly()) 
        { m_pSource->SetTwainLoopStarted(true); }
        ~UIScopedRAII() { m_pSource->SetTwainLoopStarted(false); m_pSource->SetUIOnly(m_bOld); }
    };

    UIScopedRAII raii(pSource);
    pSource->SetUIOnly(isUIOnly);

    // Determine if the message pump should be PeekMessage().  The only
    // way for GetMessage() to be universally used is if the host application is a
    // scripting engine or similar.  "Normal" Windows apps and TWAIN
    // Sources will always work using PeekMessage().

    // If we detect that GetMessage() should be used, we use it.  If the client
    // has explicitly stated to use GetMessage() by either calling 
    // DTWAIN_EnablePeekMessageLoop() to FALSE, or the DTWAIN32/64.INI
    // has the Source listed as one that must use GetMessage(), then we use GetMessage().
    auto isGetMessageRequired = !pSource->IsUsePeekMessage() || DTWAIN_ShouldUseGetMessage();
    if (isGetMessageRequired)
        pSource->SetUsePeekMessage(false);

    bool bLogMessages = (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS) ? true : false;

    if (bLogMessages)
    {
        std::string msg = "Using PeekMessage() for TWAIN acquisitions ...";
        if (!pSource->IsUsePeekMessage())
            msg = "Using GetMessage() for TWAIN acquisitions ...";
        LogWriterUtils::WriteLogInfoIndentedA(msg);
    }

#ifdef _WIN32
    // Make sure message loop is not empty.  Post a WM_NULL message to the
    // message queue to ensure we're not stuck forever on waiting for a 
    // message in the GetMessage() call.
    HWND theWnd = *pSource->GetTwainSession()->GetWindowHandlePtr();
    ::PostMessage(theWnd, WM_NULL, 0, 0);

    bool watchdog_triggered = false;
    if (pSource->IsUsePeekMessage())
    {
        // Use the PeekMessage() version of the message loop
        watchdog_triggered = ::ContinueLoopTraits<::ContinueLoopTraitsPeek>::InvokeLoop(this, pSource, isUIOnly);
    }
    else
    {
        // Use the GetMessage() version of the message loop
        watchdog_triggered = ::ContinueLoopTraits<::ContinueLoopTraitsGet>::InvokeLoop(this, pSource, isUIOnly);
    }

    if (watchdog_triggered)
    {
        returnCode = DTWAIN_ERR_TIMEOUT;
    }
#else
    while (IsSourceOpen(pSource))
    {
        CanEnterDispatch(&msg);
    }
    break;
#endif
    return returnCode;
}

TW_UINT16 TW_CALLINGSTYLE TwainMessageLoopV2::TwainVersion2MsgProc(pTW_IDENTITY , pTW_IDENTITY, TW_UINT32, TW_UINT16, TW_UINT16 MSG_, TW_MEMREF)
{
    MSG msg{};
    msg.message = MSG_;
    s_MessageQueue.push(msg);
    return TWRC_SUCCESS;
}

bool TwainMessageLoopV2::IsSourceOpen(CTL_ITwainSource* pSource)
{
    return !s_MessageQueue.empty() || TwainMessageLoopImpl::IsSourceOpen(pSource);
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnableGetMessageLoopDetection(DTWAIN_BOOL bEnable)
    {
        LOG_FUNC_ENTRY_PARAMS((bEnable))
        auto& bTest = CTL_StaticData::IsTestForGetMessage();
        bTest = (bEnable ? true : false);
        LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
        CATCH_BLOCK(0)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsGetMessageLoopDetectionOn(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        LOG_FUNC_EXIT_NONAME_PARAMS(CTL_StaticData::IsTestForGetMessage()?TRUE:FALSE)
        CATCH_BLOCK(0)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTwainMsg(MSG *pMsg)
    {
        LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG((pMsg))

        // Ensure we don't log low-level TWAIN calls when 
        // for DTWAIN_IsTwainMsg unless user specifies this.

        // This remembers the old flags when restoring at the
        // return of this function.
        struct FlagRAII
        {
            LONG oldFlags = 0;
            FlagRAII(LONG oFlags) : oldFlags(oFlags) {}
            ~FlagRAII()
            {
                CTL_StaticData::GetLogFilterFlags() = oldFlags;
            }
        };

        auto& logFlags = CTL_StaticData::GetLogFilterFlags();
        FlagRAII raii(logFlags);

        // Turn off low-level TWAIN logging if no logging for
        // IsTwainMsg
        if (!(logFlags & DTWAIN_LOG_ISTWAINMSG))
            logFlags = logFlags & ~DTWAIN_LOG_LOWLEVELTWAIN;

        if (!TwainMessageLoopV2::s_MessageQueue.empty())
        {
            MSG msg = TwainMessageLoopV2::s_MessageQueue.front();
            TwainMessageLoopV2::s_MessageQueue.pop();
            CTL_TwainAppMgr::IsTwainMsg(&msg, true);  // make sure we perform what we need to do for TWAIN 2.x.
        }
        // make sure we perform any default message handling here.
        const DTWAIN_BOOL bRet = CTL_TwainAppMgr::IsTwainMsg( pMsg, false );
        LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(bRet)
        CATCH_BLOCK(false)
    }
}

