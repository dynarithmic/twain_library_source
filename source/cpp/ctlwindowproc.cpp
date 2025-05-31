/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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
#include "dtwain.h"
#include "ctltwainmanager.h"
#include "ctliface.h"
#include "arrayfactory.h"
#include "errorcheck.h"

using namespace dynarithmic;

static void DisableAppWindows(bool bDisable);
static void DefaultDTWAINProcessing(CTL_ITwainSource *pSource, WPARAM wParam);
static LRESULT ExecuteDTWAINCallbacks(CTL_TwainDLLHandle *pHandle, HWND hWnd, UINT uMsg,
                                      WPARAM wParam, LPARAM lParam, bool bPassMsg,
                                      bool bCallDefProcs);

#define CALLBACK32_EXISTS(pHandle) ((pHandle)->m_pCallbackFn != 0)
#define CALLBACK64_EXISTS(pHandle) ((pHandle)->m_pCallbackFn64 != 0)
#define CALLBACK_EXISTS(pHandle) (CALLBACK32_EXISTS(pHandle) || CALLBACK64_EXISTS(pHandle))

/////////////////////////////////////////////////////////////////////////

static void SetNotification(CTL_TwainDLLHandle* pHandle, bool& notification, DTWAIN_BOOL bSet)
{
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    notification = (bSet?true:false);
}

LONG DLLENTRY_DEF DTWAIN_GetRegisteredMsg()
{
    LOG_FUNC_ENTRY_PARAMS(())
    LOG_FUNC_EXIT_NONAME_PARAMS(CTL_StaticData::GetRegisteredMessage())
    CATCH_BLOCK(0L)
}


DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_EnableMsgNotify(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    SetNotification(pHandle, pHandle->m_bNotificationsUsed, bSet);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_EnableTripletsNotify(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    SetNotification(pHandle, pHandle->m_bNotifyTripletsUsed, bSet);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_IsMsgNotifyEnabled()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_bNotificationsUsed)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_IsNotifyTripletsEnabled()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_bNotifyTripletsUsed)
    CATCH_BLOCK(false)
}

DTWAIN_CALLBACK_PROC DLLENTRY_DEF DTWAIN_SetCallback(DTWAIN_CALLBACK_PROC Fn, LONG UserData)
{
    LOG_FUNC_ENTRY_PARAMS((Fn, UserData))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const DTWAIN_CALLBACK_PROC oldProc = pHandle->m_pCallbackFn;
    pHandle->m_pCallbackFn = Fn;
    pHandle->m_lCallbackData = UserData;
    if (Fn)
        Fn(DTWAIN_TN_SETCALLBACKINIT, 0, UserData);
    LOG_FUNC_EXIT_NONAME_PARAMS(oldProc)
    CATCH_BLOCK(DTWAIN_CALLBACK_PROC())
}

DTWAIN_CALLBACK_PROC64 DLLENTRY_DEF DTWAIN_SetCallback64(DTWAIN_CALLBACK_PROC64 Fn, DTWAIN_LONG64 UserData)
{
    LOG_FUNC_ENTRY_PARAMS((Fn, UserData))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const DTWAIN_CALLBACK_PROC64 oldProc = pHandle->m_pCallbackFn64;
    pHandle->m_pCallbackFn64 = Fn;
    pHandle->m_lCallbackData64 = UserData;
    if (Fn)
        Fn(DTWAIN_TN_SETCALLBACK64INIT, 0, UserData);
    LOG_FUNC_EXIT_NONAME_PARAMS(oldProc)
    CATCH_BLOCK(DTWAIN_CALLBACK_PROC64())
}

DTWAIN_CALLBACK_PROC DLLENTRY_DEF DTWAIN_GetCallback()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_pCallbackFn)
    CATCH_BLOCK(DTWAIN_CALLBACK_PROC())
}

DTWAIN_CALLBACK_PROC64 DLLENTRY_DEF DTWAIN_GetCallback64()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_pCallbackFn64)
    CATCH_BLOCK(DTWAIN_CALLBACK_PROC64())
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_AddCallback(DTWAIN_CALLBACK_PROC Fn, LONG UserData)
{
    LOG_FUNC_ENTRY_PARAMS((Fn, UserData))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

template <typename CallbackType, typename UserType>
struct CallbackFinder
{
    CallbackFinder(CallbackType Fn) : theFn(Fn) {}
    bool operator () (const CallbackInfo<CallbackType, UserType>& Info)
    {
        return Info.Fn == theFn;
    }

    private:
        CallbackType theFn;
};

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RemoveCallback(DTWAIN_CALLBACK_PROC Fn)
{
    LOG_FUNC_ENTRY_PARAMS((Fn))
    VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const CallbackFinder<DTWAIN_CALLBACK_PROC,LONG> Finder(Fn);
    auto& callbacks = CTL_StaticData::GetCallbacks();
    const auto found = std::find_if(callbacks.begin(), callbacks.end(), Finder);

    if ( found != callbacks.end())
    {
        callbacks.erase(found);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }

    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)

}

#ifdef _WIN32
LRESULT DLLENTRY_DEF dynarithmic::DTWAIN_WindowProc(HWND hWnd,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
    bool bPassMsg = false;
    auto pHandle = FindHandle( hWnd, FALSE );
    if ( !pHandle )
    {
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    if ( uMsg == static_cast<UINT>(CTL_StaticData::GetRegisteredMessage()) )
    {
        LogDTWAINMessage(hWnd, uMsg, wParam, lParam);
        auto& callbacks = CTL_StaticData::GetCallbacks();
        switch( wParam )
        {
            case DTWAIN_TWAINAcquireStarted:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                pSource->SetAcquireStarted(true);
                pSource->ResetFileAutoIncrementData();
                if (pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                {
                    // Send this message on
                    bPassMsg = true;
                    DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,pHandle, pSource, wParam, reinterpret_cast<LPARAM>(pSource) );
                }
            }
            break;

            case DTWAIN_TN_UIOPENED:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                pSource->SetAcquireStarted(false);
                DisableAppWindows(true);
            }

            case DTWAIN_TN_ACQUIRESTARTED:
            case DTWAIN_TN_MANDUPSIDE1START:
            case DTWAIN_TN_MANDUPSIDE2START:
            case DTWAIN_TN_TRANSFERSTRIPREADY:
            case DTWAIN_TN_TRANSFERSTRIPDONE:
            case DTWAIN_TN_TRANSFERSTRIPFAILED:
            case DTWAIN_TN_TRANSFERTILEREADY:
            case DTWAIN_TN_TRANSFERTILEDONE:
            case DTWAIN_TN_INVALIDIMAGEFORMAT:
            case DTWAIN_TN_ACQUIRETERMINATED:
            case DTWAIN_TN_ACQUIRECANCELLED:
            case DTWAIN_TN_IMAGEINFOERROR:
            case DTWAIN_AcquireTerminated:
            case DTWAIN_TN_TRANSFERCANCELLED:
            case DTWAIN_TN_FILESAVECANCELLED:
            case DTWAIN_TN_FILESAVEOK:
            case DTWAIN_TN_FILESAVEERROR:
            case DTWAIN_TN_FILEPAGESAVEERROR:
            case DTWAIN_TN_FILEPAGESAVEOK:
            case DTWAIN_TN_TWAINPAGECANCELLED:
            case DTWAIN_TN_TWAINPAGEFAILED:
            case DTWAIN_TN_FILEPAGESAVING:
            case DTWAIN_TN_FEEDERLOADED:
            case DTWAIN_TN_PAGEDISCARDED:
            case DTWAIN_TN_APPUPDATEDDIB:
            case DTWAIN_TN_EOJBEGINFILESAVE:
            case DTWAIN_TN_EOJENDFILESAVE:
            case DTWAIN_TN_CROPFAILED:
            case DTWAIN_TN_BLANKPAGEDISCARDED1:
            case DTWAIN_TN_BLANKPAGEDISCARDED2:
            case DTWAIN_TN_PROCESSDIBFINALACCEPTED:
            case DTWAIN_TN_PROCESSDIBACCEPTED:
            case DTWAIN_TN_UIOPENFAILURE:
            case DTWAIN_TN_UIOPENING:
            case DTWAIN_TN_FILENAMECHANGING:
            case DTWAIN_TN_FILENAMECHANGED:
            case DTWAIN_TN_INVALID_TWAINDSM2_BITMAP:
            case DTWAIN_TN_IMAGE_RESAMPLE_FAILURE:
            case DTWAIN_TN_FILECOMPRESSTYPEMISMATCH:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                auto& acquireFileStatus = pSource->GetAcquireFileStatusRef();
                if (wParam == DTWAIN_TN_ACQUIRESTARTED)
                    pSource->SetImagesStored(false);

                if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;
                if (wParam == DTWAIN_TN_FILEPAGESAVEOK)
                    acquireFileStatus.SetFileSavePageCount(acquireFileStatus.GetFileSavePageCount() + 1);
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,pHandle,pSource, wParam, reinterpret_cast<LPARAM>(pSource) );
            }
            break;

            // Sent after all images have been transferred
            case DTWAIN_TN_ACQUIREDONE:
            case DTWAIN_TN_ACQUIREDONE_EX:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty() )
                    bPassMsg = true;
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                    wParam, lParam );

                // Increment the acquire count
                pSource->SetAcquireCount( pSource->GetAcquireCount() + 1 );

                // Add the array of DIBs in the source
                if ( wParam == DTWAIN_TN_ACQUIREDONE && !pSource->ImagesStored() )
                {
                    LOG_FUNC_STRING(DTWAIN_ACQUIREDONE -- Copying DIBS to Source...)
                    DTWAIN_ARRAY aDibs = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYHANDLE, 0);
                    DTWAIN_GetAllSourceDibs( static_cast<DTWAIN_SOURCE>(pSource), aDibs );
                    pSource->AddDibsToAcquisition(aDibs);
                    LOG_FUNC_STRING(DTWAIN_ACQUIREDONE -- Finished Copying DIBS to Source...)
                }
                else
                // Program canceled after XFERREADY was retrieved
                {
                    // Nothing to do
                }
                // Check if acquire count has reached the max
                if ( pSource->GetAcquireCount() == pSource->GetMaxAcquisitions() || !pSource->IsUIOpenOnAcquire())
                {
                    // End the TWAIN UI session
                    CTL_TwainAppMgr::EndTwainUI( pHandle->m_pTwainSession, pSource );

                    // Check if Source can be closed after acquisition
                    if ( !pSource->IsOpenAfterAcquire() )
                    {
                        // Post a message back to the app and main window that the source will be closed
                        ::SendMessage( hWnd, uMsg, DTWAIN_AcquireSourceClosed, reinterpret_cast<LPARAM>(pSource));
                    }
                }
                if ( !pSource->IsModal())
                    pSource->SetAcquireAttempt(false); // Reset.  UI may still
                                                // be open for more acquisitions!!
            }
            break;

            case DTWAIN_TN_PAGEFAILED:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                pSource->SetAcquireAttempt(false);
                pHandle->m_ArrayFactory->destroy(pSource->m_pUserPtr);

                // Couldn't acquire the first page, so acquire failed totally!
                if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;
                if ( pSource->GetPendingImageNum() == 0 )
                {
                    if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                        bPassMsg = true;
                }

                if ( !pHandle->m_bNotificationsUsed )
                    return DTWAIN_RETRY_EX;

                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                    wParam, static_cast<LPARAM>(pSource->GetAcquireNum()) );
            }
            break;

            // One image was transferred
            case DTWAIN_TN_TRANSFERDONE:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                if (  pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                      bPassMsg = true;
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                     wParam, lParam );
            }
            break;

          case DTWAIN_TN_TRANSFERREADY:
          case DTWAIN_TN_PAGECONTINUE:
          case DTWAIN_TN_MANDUPSIDE1DONE:
          case DTWAIN_TN_MANDUPSIDE2DONE:
          case DTWAIN_TN_MANDUPPAGECOUNTERROR:
          case DTWAIN_TN_MANDUPMERGEERROR:
          case DTWAIN_TN_QUERYPAGEDISCARD:
          case DTWAIN_TN_BLANKPAGEDETECTED1:
          case DTWAIN_TN_BLANKPAGEDETECTED2:
          case DTWAIN_TN_BLANKPAGEDETECTED3:
          case DTWAIN_TN_QUERYOCRTEXT:
          case DTWAIN_TN_PROCESSEDDIB:
          case DTWAIN_TN_PROCESSEDDIBFINAL:
          case DTWAIN_TN_TWAINTRIPLETBEGIN:
          case DTWAIN_TN_TWAINTRIPLETEND:
          {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                if (  pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;
                else
                {
                    DefaultDTWAINProcessing(pSource, wParam);
                    return TRUE;
                }
                if ( !pHandle->m_bNotificationsUsed )
                    return TRUE;

                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                     wParam, lParam );
            }
            break;

            case DTWAIN_TN_PAGECANCELLED:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                pHandle->m_ArrayFactory->destroy(pSource->m_pUserPtr);
                if (  pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;
                // Couldn't acquire the first page, so acquire failed totally!
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                     wParam, lParam );

                // Execute the callback for DTWAIN_TN_PAGECANCELLED:
                ExecuteDTWAINCallbacks(pHandle, hWnd, uMsg, wParam, lParam, bPassMsg, false);

                // Execute DTWAIN_TN_ACQUIRECANCELLED_EX if there are no more
                // pages left
                if ( pSource->GetPendingImageNum() == 0 )
                {
                    wParam = DTWAIN_TN_ACQUIRECANCELLED_EX;
                    if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                        bPassMsg = true;
                }
            }
            break;

            case DTWAIN_TN_ACQUIRECANCELLED_EX:
            {
                // Source dialog has been cancelled
                CTL_ITwainSource *pSource = nullptr;
                if ( lParam != -1)
                {
                    pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                    pHandle->m_ArrayFactory->destroy(pSource->m_pUserPtr);
                }
                if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                    wParam, lParam);
                // Post a message back to the app and main window that the source will be closed
                ::SendMessage( hWnd, uMsg, DTWAIN_AcquireSourceClosed, reinterpret_cast<LPARAM>(pSource));
            }
            break;

            case DTWAIN_SelectSourceFailed:
            {
            }
            break;

            case DTWAIN_TN_UICLOSING:
            case DTWAIN_TN_ENDOFJOBDETECTED:
            case DTWAIN_TN_EOJDETECTED_XFERDONE:
                if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;
            break;

            case DTWAIN_TN_UICLOSED:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);

                if ( pSource->IsUIOnly() )
                {
                    if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                        bPassMsg = true;
                    pSource->SetUIOnly(FALSE);
                    pSource->SetUIOpen(FALSE);
                    break;
                }

                if (!pSource->IsAcquireStarted())
                {
                    if ( pHandle->m_hNotifyWnd )
                        ::SendMessage( pHandle->m_hNotifyWnd, uMsg, DTWAIN_TN_ACQUIRECANCELLED_EX, lParam );
                    DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                        static_cast<DTWAIN_HANDLE>(pHandle),
                                        static_cast<DTWAIN_SOURCE>(pSource),
                                        static_cast<WPARAM>(DTWAIN_TN_ACQUIRECANCELLED), 0 );
                }

                if ( pSource->IsAcquireAttempt() ) // Didn't really acquire the image
                    pSource->SetAcquireAttempt(false);

                if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                    bPassMsg = true;

                DisableAppWindows(false);
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                    static_cast<DTWAIN_HANDLE>(pHandle),
                                    static_cast<DTWAIN_SOURCE>(pSource),
                                     wParam, 0 );
                // Post a message back to the app and main window that the source will be closed
                ::SendMessage( hWnd, uMsg, DTWAIN_TN_ACQUIRECANCELLED, reinterpret_cast<LPARAM>(pSource) );
                ::SendMessage( hWnd, uMsg, DTWAIN_AcquireSourceClosed, reinterpret_cast<LPARAM>(pSource));
                ::SendMessage( hWnd, uMsg, DTWAIN_AcquireTerminated, reinterpret_cast<LPARAM>(pSource));
                ::PostMessage( hWnd, uMsg, DTWAIN_TN_ACQUIRETERMINATED, lParam );
            }
            break;

            case DTWAIN_AcquireSourceClosed:
            {
                auto pSession = pHandle->m_pTwainSession;
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                if ( pSource && pSource->IsOpened() )
                {
                    pSource->SetAcquireAttempt(false);
                    // Close Source only if modal and Source wasn't started by client app
                    if (!pSource->IsOpenAfterAcquire() )
                    {
                        if (!pSource->ImagesStored())
                        {
                            // Check if UI-less mode.  DIBs need to be saved here
                            if (!pSource->IsUIOpenOnAcquire())
                            {
                                // Save the image handles
                                char buf[25] = {};
                                LOG_FUNC_STRING(No UI Mode Done -- Copying DIBS to Source...)
                                DTWAIN_ARRAY aDibs = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYHANDLE, 0);
                                DTWAIN_GetAllSourceDibs(pSource, aDibs);
                                int nDibs = static_cast<int>(pHandle->m_ArrayFactory->size(aDibs));
                                StringStreamA strm;
                                strm << buf;
                                LOG_FUNC_VALUES(strm.str().c_str())
                                LOG_FUNC_STRING(No UI Mode -- Finished Copying DIBS to Source...)
                                if (nDibs > 0)
                                    pSource->AddDibsToAcquisition(aDibs);
                                else
                                    pHandle->m_ArrayFactory->destroy(aDibs);
                                pSource->SetImagesStored(true);
                            }

                            // Check if there is a pending prompt for a filename save
                            if (pSource->IsPromptPending())
                            {

                            }
                            // Close the source
                            CTL_TwainAppMgr::CloseSource(pSession, pSource);

                            if (pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                                bPassMsg = true;
                            DTWAIN_InvokeCallback(DTWAIN_CallbackMESSAGE,
                                static_cast<DTWAIN_HANDLE>(pHandle),
                                static_cast<DTWAIN_SOURCE>(pSource),
                                wParam, 0);

                            // Check if source should be reopened after acquisition
                            if (pSource->IsReopenAfterAcquire())
                                CTL_TwainAppMgr::OpenSource(pSession, pSource);
                        }
                    }
                    pHandle->EraseAcquireNum( pSource->GetAcquireNum() );
                }
            }
            break;

            case DTWAIN_TN_CLIPTRANSFERDONE:
            {
                auto pSource = reinterpret_cast<CTL_ITwainSource*>(lParam);
                if ( pSource )
                {
                    if ( pHandle->m_hNotifyWnd || CALLBACK_EXISTS(pHandle) || !callbacks.empty())
                        bPassMsg = true;
                    DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE,
                                        static_cast<DTWAIN_HANDLE>(pHandle),
                                        static_cast<DTWAIN_SOURCE>(pSource),
                                         wParam, 0L );
                }
            }
            break;
        }
        // Send message to other notification windows
        pHandle->NotifyWindows( uMsg, wParam, lParam );

        // Do not let window process this message again
        if ( !bPassMsg )
        {
            return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
        }
    }

    else

    switch ( uMsg )
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            /* If this window was subclassed, the program must not close this window!!! */
            if ( pHandle->m_hWndTwain && pHandle->m_bUseProxy )
                DTWAIN_InvokeCallback( DTWAIN_CallbackMESSAGE, pHandle, nullptr, DTWAIN_AcquireSourceClosed, 0 );

        }
        break;
    }

    return ExecuteDTWAINCallbacks(pHandle, hWnd, uMsg, wParam, lParam, bPassMsg, true);
}
#endif // _WIN32

template <typename CallbackType, typename UserType>
LRESULT CallOneCallback(CallbackType Fn, WPARAM wParam, LPARAM lParam, UserType UserData)
{
    return (*Fn)(wParam, lParam, UserData);
}

template <typename CallbackType, typename UserType>
struct CallBatchProcessor
{
    CallBatchProcessor(WPARAM wParam, LPARAM lParam) :thewparm(wParam), thelparm(lParam) {}

    void operator() (CallbackInfo<CallbackType, UserType>& Info)
    {
        Info.retvalue = CallOneCallback(Info.Fn, thewparm, thelparm, Info.UserData );
    }

    private:
        WPARAM thewparm;
        LPARAM thelparm;
};

template <typename CallbackType, typename UserType>
LRESULT ExecuteCallback(CallbackType Fn, HWND hWnd, UINT uMsg,
                        WPARAM wParam, LPARAM lParam, UserType uType)
{
    LRESULT lResult = -1;
    try
    {
        LogDTWAINMessage(hWnd, uMsg, wParam, lParam, true);
        lResult = CallOneCallback(Fn, wParam, lParam, uType);
    }
    catch (...)
    {
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
        {
            const std::string sError = "In ExecuteCallback: Exception encountered when logging using callback...";
            LogWriterUtils::WriteLogInfoIndentedA(sError);
        }
    }
    return lResult;
}

LRESULT ExecuteDTWAINCallbacks(CTL_TwainDLLHandle *pHandle, HWND hWnd, UINT uMsg,
                               WPARAM wParam, LPARAM lParam, bool bPassMsg, bool bCallDefProcs)
{
    auto& callbacks = CTL_StaticData::GetCallbacks();
    LRESULT lResult = 0;
    if ( pHandle )
    {
        if ( !pHandle->m_bNotificationsUsed )
            bPassMsg = FALSE;

        // Check if there is a session window defined, if so call it's window proc
        // This is also a notification message, so we can safely skip the CallWindowProc
        // return if there is also a callback defined
        if ( bPassMsg && !pHandle->m_bUseProxy
            #ifdef _WIN32
                && pHandle->m_hOrigProc
            #endif
          )
        {
            #ifdef _WIN32
            lResult = CallWindowProc(static_cast<WNDPROC>(pHandle->m_hOrigProc), hWnd, uMsg, wParam, lParam);
            #endif
            // Called if only callback exists
            if (CALLBACK32_EXISTS(pHandle))
                lResult = ExecuteCallback<DTWAIN_CALLBACK_PROC, LONG>(pHandle->m_pCallbackFn, hWnd, uMsg,
                                                                      wParam, lParam, pHandle->m_lCallbackData);
            if (CALLBACK64_EXISTS(pHandle))
                lResult = ExecuteCallback<DTWAIN_CALLBACK_PROC64, LONGLONG>(pHandle->m_pCallbackFn64, hWnd, uMsg,
                                                                            wParam, lParam, pHandle->m_lCallbackData64);
            // call the callbacks that are in the queue
            if ( !callbacks.empty())
            {
                LogDTWAINMessage(hWnd, uMsg, wParam, lParam, true);
                const CallBatchProcessor<DTWAIN_CALLBACK_PROC, LONG> BP(wParam, lParam);
                std::for_each(callbacks.begin(), callbacks.end(), BP);
                lResult = callbacks.back().retvalue;
            }

            return lResult;
        }

        else
        // Check if there is no session window, but a callback has been defined
        if ( pHandle->m_hWndTwain && pHandle->m_bUseProxy)
        {
            lResult = 0;
            #ifdef _WIN32
            if ( bCallDefProcs )
                lResult = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
            #endif
            if ( bPassMsg && (CALLBACK_EXISTS(pHandle) || !callbacks.empty()))
            {
                // Called if only callback exists
                if (CALLBACK32_EXISTS(pHandle))
                    lResult = ExecuteCallback<DTWAIN_CALLBACK_PROC, LONG>(pHandle->m_pCallbackFn, hWnd, uMsg,
                    wParam, lParam, pHandle->m_lCallbackData);

                if (CALLBACK64_EXISTS(pHandle))
                    lResult = ExecuteCallback<DTWAIN_CALLBACK_PROC64, LONGLONG>(pHandle->m_pCallbackFn64, hWnd, uMsg,
                    wParam, lParam, pHandle->m_lCallbackData64);

                // call the callbacks that are in the queue
                if ( !callbacks.empty())
                {
                    LogDTWAINMessage(hWnd, uMsg, wParam, lParam, true);
                    const CallBatchProcessor<DTWAIN_CALLBACK_PROC, LONG> BP(wParam, lParam);
                    std::for_each(callbacks.begin(), callbacks.end(),BP);
                    lResult = callbacks.back().retvalue;
                }
            }
            return lResult;
        }

        else
        // Check if there is a session window, but the bPassMsg is FALSE
        if (!pHandle->m_bUseProxy
            #ifdef _WIN32
            && pHandle->m_hOrigProc
            #endif
        )
        {
            #ifdef _WIN32
            lResult = CallWindowProc(pHandle->m_hOrigProc, hWnd, uMsg, wParam, lParam);
            return lResult;
            #else
            return 0;
            #endif
        }
        else
        // Now test for just a callback
        if ( bPassMsg && (CALLBACK_EXISTS(pHandle) || !callbacks.empty()))
        {
            LogDTWAINMessage(hWnd, uMsg, wParam, lParam, true);
            // Now check if there is a Callback defined
            // Called if only callback exists
            if (CALLBACK32_EXISTS(pHandle))
                lResult = ExecuteCallback<DTWAIN_CALLBACK_PROC, LONG>(pHandle->m_pCallbackFn, hWnd, uMsg,
                wParam, lParam, pHandle->m_lCallbackData);

            if (CALLBACK64_EXISTS(pHandle))
                lResult = ExecuteCallback<DTWAIN_CALLBACK_PROC64, LONGLONG>(pHandle->m_pCallbackFn64, hWnd, uMsg,
                wParam, lParam, pHandle->m_lCallbackData64);

            // call the callbacks that are in the queue
            if ( !callbacks.empty())
            {
                LogDTWAINMessage(hWnd, uMsg, wParam, lParam, true);
                const CallBatchProcessor<DTWAIN_CALLBACK_PROC, LONG> BP(wParam, lParam);
                std::for_each(callbacks.begin(), callbacks.end(),BP);
                lResult = callbacks.back().retvalue;
            }

            return lResult;
        }
    }
    #ifdef _WIN32
    return lResult = ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    #else
    return 0;
    #endif // _WIN32
}

void dynarithmic::LogDTWAINMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bToCallback)
{
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_NOTIFICATIONS)
    {
        CTL_ErrorStruct e;
        std::string s;
        if ( bToCallback )
            s = "To callback: ";
        s += e.GetDTWAINMessageAndDataInfo(hWnd, uMsg, wParam, lParam);
        LogWriterUtils::WriteMultiLineInfoIndentedA(s, "\n");
    }
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_DisableAppWindow(HWND hWnd, DTWAIN_BOOL bDisable)
{
   LOG_FUNC_ENTRY_PARAMS((hWnd, bDisable))
   #ifdef _WIN32
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !IsWindow( hWnd );}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

   auto& appWindows = CTL_StaticData::GetAppWindowsToDisable();
   if ( bDisable )
       appWindows.insert( hWnd );
   else
       appWindows.erase( hWnd );
   #endif
   LOG_FUNC_EXIT_NONAME_PARAMS(true)
   CATCH_BLOCK(false)
}

void DisableAppWindows(bool bDisable)
{
#ifdef _WIN32
    auto& appWindows = CTL_StaticData::GetAppWindowsToDisable();
    std::for_each(appWindows.begin(), appWindows.end(), [&](HWND TheWnd) 
                    {
                    if (IsWindow(TheWnd))
                        EnableWindow(TheWnd, !bDisable);
                    });
#endif
}

void DefaultDTWAINProcessing(CTL_ITwainSource * /*pSource*/, WPARAM /*wParam*/)
{
    return;
}
