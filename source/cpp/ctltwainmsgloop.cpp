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
#include "ctltwmgr.h"
#include "sourceacquireopts.h"
#include "ctltr040.h"
#include "ctltwainmsgloop.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

std::queue<MSG> TwainMessageLoopV2::s_MessageQueue;

std::pair<bool, DTWAIN_ACQUIRE> dynarithmic::StartModalMessageLoop(DTWAIN_SOURCE Source, SourceAcquireOptions& opts)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if (pHandle->m_lAcquireMode == DTWAIN_MODELESS)
        return { true, 0 };

    CTL_ITwainSource* pSource = VerifySourceHandle(pHandle, Source);

    if (!pSource)
        return { false, -1 };

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
    pImpl->PerformMessageLoop(pSource, opts.getIsUIOnly());
    return { true, pImpl->GetAcquireNum() };
}

bool TwainMessageLoopImpl::IsSourceOpen(CTL_ITwainSource* pSource, bool bUIOnly)
{
    if (bUIOnly)
        return pSource->IsUIOpen() ? true : false;
    return !m_pDLLHandle->m_bTransferDone == true && !m_pDLLHandle->m_bSourceClosed == true;
}

bool TwainMessageLoopImpl::IsAcquireTerminated(CTL_ITwainSource* pSource, bool bUIOnly)
{
    if (bUIOnly)
        return !pSource->IsUIOpen();
    return !IsSourceOpen(pSource, bUIOnly);
}

void TwainMessageLoopWindowsImpl::PerformMessageLoop(CTL_ITwainSource *pSource, bool isUIOnly)
{
    MSG msg;
    struct UIScopedRAII
    {
        CTL_ITwainSource* m_pSource;
        bool m_bOld;
        UIScopedRAII(CTL_ITwainSource* pSource) : m_pSource(pSource), m_bOld(m_pSource->IsUIOnly()) {}
        ~UIScopedRAII() { m_pSource->SetUIOnly(m_bOld); }
    };

    UIScopedRAII raii(pSource);
    pSource->SetUIOnly(isUIOnly);
#ifdef _WIN32
    bool bInitializeAcquisitionProcess = false;

    // Make sure message loop is not empty.  Post a WM_NULL message to the
    // message queue to ensure we're not stuck forever on waiting for a 
    // message in the GetMessage() call.
    HWND theWnd = *pSource->GetTwainSession()->GetWindowHandlePtr();
    ::PostMessage(theWnd, WM_NULL, static_cast<WPARAM>(0), static_cast<LPARAM>(0));

    // Start the message loop
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        // If in UIOnly mode, set it up here
        if (isUIOnly && !bInitializeAcquisitionProcess)
        {
            LLSetupUIOnly(pSource);
            bInitializeAcquisitionProcess = true;
        }

        // If acquire has been terminated, break out of this loop
        if (IsAcquireTerminated(pSource, isUIOnly))
            break;

        // If we haven't set up the TWAIN device for the acquisition,
        // do it now.  The LLAcquireImage() will also eventually show
        // the user-interface of the device, or acquire immediately if
        // no user-interface is being used.
        if (!bInitializeAcquisitionProcess)
        {
            m_AcquireNum = LLAcquireImage(sOpts);
            bInitializeAcquisitionProcess = true;

            // Didn't get an acquisition number, so something failed
            if (m_AcquireNum == -1L)
                break;
        }

        // This will test for TWAIN messages, Data Source messages or application messages.
        if (CanEnterDispatch(&msg))
        {
            TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
    }
#else
    while (IsSourceOpen(pSource, isUIOnly))
    {
        CanEnterDispatch(&msg);
    }
#endif
}

TW_UINT16 TW_CALLINGSTYLE TwainMessageLoopV2::TwainVersion2MsgProc(pTW_IDENTITY , pTW_IDENTITY, TW_UINT32, TW_UINT16, TW_UINT16 MSG_, TW_MEMREF)
{
    MSG msg = MSG();
    msg.message = MSG_;
    s_MessageQueue.push(msg);
    return TWRC_SUCCESS;
}

bool TwainMessageLoopV2::IsSourceOpen(CTL_ITwainSource* pSource, bool bUIOnly)
{
    return !s_MessageQueue.empty() || TwainMessageLoopImpl::IsSourceOpen(pSource, bUIOnly);
}

void dynarithmic::DTWAIN_AcquireProc(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE, WPARAM Data1, LPARAM)
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
