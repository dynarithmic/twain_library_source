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
#include "ctltr008.h"
#include "ctltr027.h"
#include "ctltr034.h"
#include "ctltwses.h"
#include <ctltwsrc.h>

#include "ctliface.h"
#include "ctltwmgr.h"

using namespace dynarithmic;

/* Transfer started */
/* Scanner already has physically scanned a page.
 This is sent only once (when TWAIN actually does the transformation of the
 scanned image to the DIB) */

/* Sent when DTWAIN_Acquire...() functions are about to return */

CTL_ProcessEventTriplet::CTL_ProcessEventTriplet(CTL_ITwainSession* pSession,
                                                 CTL_ITwainSource* pSource,
                                                 MSG *pMsg,
                                                 bool isDSM2) : m_Event{}, m_bDSM2Used(isDSM2)
{
    SetSourcePtr(const_cast<CTL_ITwainSource*>(pSource));
    SetSessionPtr(pSession);
    m_pMsg = pMsg;
    m_Event.pEvent      = static_cast<TW_MEMREF>(pMsg);
    m_Event.TWMessage   = MSG_NULL;
    if (m_bDSM2Used)
        m_Event.TWMessage = static_cast<TW_UINT16>(pMsg->message);

    // Get the app manager's AppID
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

    if (pMgr && pMgr->IsValidTwainSession(pSession))
    {
        if (pSource)
        {
            Init(pSession->GetAppIDPtr(), pSource->GetSourceIDPtr(), DG_CONTROL, DAT_EVENT,
                MSG_PROCESSEVENT, static_cast<TW_MEMREF>(&m_Event));
            SetAlive(true);
        }
    }
}


TW_UINT16 CTL_ProcessEventTriplet::ExecuteEventHandler()
{
    TW_UINT16 rc = TWRC_SUCCESS;

    // if we are doing a legacy TWAIN call, then we need to
    // call the DSM
    if (!m_bDSM2Used)
        rc = CTL_TwainTriplet::Execute();

    CTL_ITwainSession* pSession = GetSessionPtr();
    CTL_ITwainSource* pSource = GetSourcePtr();

    const HWND hWnd =*pSession->GetWindowHandlePtr();
    const unsigned int nMsg = CTL_TwainAppMgr::GetRegisteredMsg();

   // Check message from source
    // There are some issues with Sources *not* sending MSG_XFERREADY in
    // a consistent manner to start the scan, and instead issue a MSG_CLOSEDSREQ.
    auto& user_map = CTL_TwainAppMgr::GetSourceToXferReadyMap();
    std::string prodName = pSource->GetProductNameA();
    auto iter = user_map.find(prodName);

    if (iter != user_map.end())
    {
        // Check if we received a close DS request before we have gotten
        // a "start transfer" request
        if (iter->second.m_bSeenUIClose && !iter->second.m_bSeenXferReady)
        {
            // Increment the message counter and test if we have run out
            // of messages to check
            iter->second.m_CurrentCount++;
            if (iter->second.m_CurrentCount >= iter->second.m_MaxThreshold)
            {
                // ran out of messages, so close the UI 
                CloseUI(pSource);
            }
        }
    }

    switch (m_Event.TWMessage)
    {
        case MSG_XFERREADY:
        {
            static int nCount = 0;
            // For those sources we are tracking for the close / transfer behavior
            // mark that we have received a transfer message
            if ( iter != user_map.end())
                iter->second.m_bSeenXferReady = true;

            pSource->SetXferReadySent(true);
            bool bNextAttemptIsRetry = false;
            pSource->SetState(SOURCE_STATE_XFERREADY);
            // Set the retry count
            pSource->SetCurrentRetryCount(0);

            // Set the transfer mechanism (??)
            // Set the pixel type and bit depth based on what the current values
            // found in the Source.
            CTL_TwainAppMgr::SetPixelAndBitDepth(pSource);

            // Send this message to the Twain window(s)
            #ifdef _WIN32
            ::SendMessage(hWnd, nMsg, MSG_XFERREADY, 0 );
            #endif
            // Remove Dibs if already scanned
            pSource->Reset();

            // Loop for all documents installed
            int  bPending = 1;
            nCount = 0;
            pSource->SetPendingJobNum(0);
            while (bPending != 0)
            {
                // Get the image information
                if ( !bNextAttemptIsRetry )
                {
                    if (!CTL_TwainAppMgr::GetImageInfo(pSource))
                    {
                        if ( !pSource->SkipImageInfoErrors() )
                        {
                            CTL_TwainAppMgr::WriteLogInfoA("Invalid Image Information on acquiring image");
                            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                                  nullptr,
                                                                  DTWAIN_TN_IMAGEINFOERROR,
                                                                  reinterpret_cast<LPARAM>(pSource));
                            break;
                        }
                    }
                    else
                    {
                    }
                }
                // Let TWAIN initiate the transfer
                // Send message that acquire has started if nCount is 0
                if ( nCount == 0 && !bNextAttemptIsRetry )
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr,
                                                          DTWAIN_TWAINAcquireStarted,
                                                          reinterpret_cast<LPARAM>(pSource));

                // Send message if nCount is 0 and manual duplex is on, and side 1 is
                // being acquired
                if ( nCount == 0 && pSource->IsManualDuplexModeOn() &&
                     pSource->GetCurrentSideAcquired() == 0 )
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr,
                                                          DTWAIN_TN_MANDUPSIDE1START,
                                                          reinterpret_cast<LPARAM>(pSource));

                // Also send that the acquisition is ready (this is sent for every page)
                const bool bContinue = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                                             nullptr,
                                                                             DTWAIN_TN_TRANSFERREADY,
                                                                             reinterpret_cast<LPARAM>(pSource))?true:false;
                if ( !bContinue )
                {
                    // Transfer aborted by callback
                    ResetTransfer();

                    // Send a message to close things down if
                    // there was no user interface chosen
                    if ( !pSource->IsUIOpenOnAcquire() )
                        CTL_TwainAppMgr::EndTwainUI(pSession, pSource);

                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr,
                                                          DTWAIN_TN_TRANSFERCANCELLED,
                                                          reinterpret_cast<LPARAM>(pSource));
                    bPending = 0;
                }
                else
                {
                    bNextAttemptIsRetry = false;
                    bPending = CTL_TwainAppMgr::TransferImage(pSource, nCount);
                    if ( bPending != -1 )  // Only if aborting or images have been retrieved
                        nCount++;
                    else
                        bNextAttemptIsRetry = true;
                }
            }
            pSource->SetState(SOURCE_STATE_UIENABLED);
            if ( !pSource->GetTransferDone() && nCount <= 1 )
            {
                CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                      nullptr,
                                                      DTWAIN_TN_ACQUIREDONE_EX,
                                                      reinterpret_cast<LPARAM>(pSource));

                break;  // No transfer occurred.  Cancellation or Failure occurred
            }
            if ( nCount > 0)
                CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                      nullptr,
                                                      DTWAIN_TN_ACQUIREDONE,
                                                      reinterpret_cast<LPARAM>(pSource));
        }
        break;

        case MSG_CLOSEDSREQ:
        case MSG_CLOSEDSOK:
        {
            // For those sources where we need to hold off on closing the UI,
            // record that we have received a close request
            if (iter != user_map.end())
                iter->second.m_bSeenUIClose = true;
            else
                CloseUI(pSource);
        }
        break;

        // Possible device event
        case MSG_DEVICEEVENT:
            DeviceEvent(pSource);
        break;

        case MSG_NULL:
        break;
    }

    // For the sources that require close / transfer message processing...
    if (iter != user_map.end())
    {
        // If we have seen both the transfer ready and close, then close the UI
        if (iter->second.m_bSeenXferReady && iter->second.m_bSeenUIClose)
            CloseUI(pSource);
    }
    return rc;
}


bool CTL_ProcessEventTriplet::ResetTransfer(TW_UINT16 Msg/*=MSG_RESET*/)
{
    CTL_ITwainSession* pSession = GetSessionPtr();
    CTL_ImagePendingTriplet Pending(pSession, GetSourcePtr(), Msg);
    const TW_UINT16 rc = Pending.Execute();

    switch( rc )
    {
        case TWRC_SUCCESS:
            return true;

        case TWRC_FAILURE:
        {
            const TW_UINT16 ccode = CTL_TwainAppMgr::GetConditionCode(pSession, nullptr);
            CTL_TwainAppMgr::ProcessConditionCodeError(ccode);
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, TWRC_FAILURE, ccode);
            return false;
        }
    }
    return false;
}

void CTL_ProcessEventTriplet::CloseUI(CTL_ITwainSource* pSource)
{
    // The source UI must be closed
    CTL_TwainAppMgr::EndTwainUI(pSource->GetTwainSession(), pSource);
}

void CTL_ProcessEventTriplet::DeviceEvent(CTL_ITwainSource* pSource)
{
    // Some dude has changed something on the device!!
    // Get the change
    auto pSession = pSource->GetTwainSession();
    CTL_DeviceEventTriplet DevTrip(pSession, pSource);
    DevTrip.Execute();
    if ( DevTrip.IsSuccessful() )
    {
        pSource->SetDeviceEvent( DevTrip.GetDeviceEvent() );
        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_DEVICEEVENT,reinterpret_cast<LPARAM>(pSource));

        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        // if there is a callback, call it now with the error notifications
        if ( pHandle->m_pCallbackFn )
        {
            const UINT uMsg = CTL_StaticData::s_nRegisteredDTWAINMsg;
            LogDTWAINMessage(nullptr, uMsg, DTWAIN_TN_DEVICEEVENT, 0, true);
            #ifdef WIN64
                (*pHandle->m_pCallbackFn)(DTWAIN_TN_DEVICEEVENT, 0, reinterpret_cast<LONG_PTR>(pSource));
            #else
                (*pHandle->m_pCallbackFn)(DTWAIN_TN_DEVICEEVENT, 0, reinterpret_cast<LONG>(pSource));
            #endif
        }

        // if there is a 64-bit callback, call it now with the error notifications
        if ( pHandle->m_pCallbackFn64 )
        {
            const UINT uMsg = CTL_StaticData::s_nRegisteredDTWAINMsg;
            LogDTWAINMessage(nullptr, uMsg, DTWAIN_TN_DEVICEEVENT, 0, true);
            (*pHandle->m_pCallbackFn64)(DTWAIN_TN_DEVICEEVENT, 0, reinterpret_cast<LONG_PTR>(pSource));
        }
    }
}