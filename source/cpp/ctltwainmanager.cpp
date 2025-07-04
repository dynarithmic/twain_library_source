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
#ifdef _MSC_VER
#pragma warning( disable : 4786)
#pragma warning( disable : 4996)
#pragma warning( disable : 4702)
#endif

#include <cstdio>
#include <algorithm>
#include <set>
#include <memory>
#include <cstring>
#include <sstream>
#include <array>
#include <boost/format.hpp>
#include <boost/dll/shared_library.hpp>
#include "dtwain_resource_constants.h"
#include "dtwain_filesystem.h"
#include "dtwain.h"
#include "ctltrall.h"
#include "ctldib.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "twainfix32.h"
#include "dtwinverex.h"
#include "cppfunc.h"
#include "logwriterutils.h"
#include "ctltripletbase.h"
#include "ctlconstexprutils.h"
using namespace dynarithmic;

static constexpr std::array<std::pair<int, int>, 32> mapCondCode = { {
    {TWCC_SUCCESS         ,IDS_ErrCCFalseAlarm},
    {TWCC_BUMMER          ,IDS_ErrCCBummer},
    {TWCC_LOWMEMORY       ,IDS_ErrCCLowMemory},
    {TWCC_NODS            ,IDS_ErrCCNoDataSource},
    {TWCC_MAXCONNECTIONS  ,IDS_ErrCCMaxConnections},
    {TWCC_OPERATIONERROR  ,IDS_ErrCCOperationError},
    {TWCC_BADCAP          ,IDS_ErrCCBadCapability},
    {TWCC_BADPROTOCOL     ,IDS_ErrCCBadProtocol},
    {TWCC_BADVALUE        ,IDS_ErrCCBadValue},
    {TWCC_SEQERROR        ,IDS_ErrCCSequenceError},
    {TWCC_BADDEST         ,IDS_ErrCCBadDestination},
    {TWCC_CAPUNSUPPORTED  ,IDS_ErrCCCapNotSupported},
    {TWCC_CAPBADOPERATION ,IDS_ErrCCCapBadOperation},
    {TWCC_CAPSEQERROR     ,IDS_ErrCCCapSequenceError},
    {TWCC_DENIED          ,IDS_ErrCCFileProtected},
    {TWCC_FILEEXISTS      ,IDS_ErrCCFileExists},
    {TWCC_FILENOTFOUND    ,IDS_ErrCCFileNotFound},
    {TWCC_NOTEMPTY        ,IDS_ErrCCDirectoryNotEmpty},
    {TWCC_PAPERJAM        ,IDS_ErrCCFeederJammed},
    {TWCC_PAPERDOUBLEFEED ,IDS_ErrCCFeederMultPages},
    {TWCC_FILEWRITEERROR  ,IDS_ErrCCFileWriteError},
    {TWCC_CHECKDEVICEONLINE,IDS_ErrCCDeviceOffline},
    {TWCC_INTERLOCK,       IDS_ErrCCInterlock},
    {TWCC_DAMAGEDCORNER,   IDS_ErrCCDamagedCorner},
    {TWCC_FOCUSERROR,      IDS_ErrCCFocusError},
    {TWCC_DOCTOOLIGHT,     IDS_ErrCCDoctooLight},
    {TWCC_DOCTOODARK,      IDS_ErrCCDoctooDark},
    {TWCC_NOMEDIA   ,      IDS_ErrCCNoMedia},
    {TWAIN_ERR_NULL_CONTAINER, TWAIN_ERR_NULL_CONTAINER},
    {DTWAIN_ERR_EXCEPTION_ERROR, DTWAIN_ERR_EXCEPTION_ERROR},
    {-TWAIN_ERR_NULL_CONTAINER, TWAIN_ERR_NULL_CONTAINER},
    {-DTWAIN_ERR_EXCEPTION_ERROR, DTWAIN_ERR_EXCEPTION_ERROR}} };

constexpr int TWRC_Error = 1;
constexpr int TWCC_Error = 2;

void CTL_TwainAppMgr::SetDLLInstance(HINSTANCE hDLLInstance)
{
    s_ThisInstance = hDLLInstance;
}

CTL_TwainAppMgrPtr CTL_TwainAppMgr::Create(CTL_TwainDLLHandle* pHandle,
                                           HINSTANCE hInstance, HINSTANCE hThisInstance,
                                           LPCTSTR lpszDLLName)
{
    if ( s_pGlobalAppMgr )
        return s_pGlobalAppMgr;

    s_ThisInstance = hThisInstance;
    s_nLastError = 0;
    try { s_pGlobalAppMgr.reset(new CTL_TwainAppMgr( pHandle, lpszDLLName, hInstance, hThisInstance ));}
    catch(...)
    { return CTL_TwainAppMgrPtr(); }
    if ( !s_pGlobalAppMgr->LoadSourceManager(lpszDLLName) )
    { s_pGlobalAppMgr.reset(); }
    return s_pGlobalAppMgr;
}

void CTL_TwainAppMgr::Destroy()
{
    if ( s_pGlobalAppMgr )
    {
        s_pGlobalAppMgr->DestroyAllTwainSessions();
        s_pGlobalAppMgr->CloseLogFile();
        /* Use for this APP only */
        s_pGlobalAppMgr->UnloadSourceManager();
    }
    s_pGlobalAppMgr.reset();
}

CTL_ITwainSession* CTL_TwainAppMgr::CreateTwainSession(CTL_TwainDLLHandle *pHandle, LPCTSTR pAppName, HWND* hAppWnd)
{
    // Make sure we destroy the TWAIN session if something goes wrong in CreateTwainSession
    struct TwainSessionRAII
    {
        CTL_ITwainSession* m_pSession;
        bool m_bDestroy = true;
        TwainSessionRAII(CTL_ITwainSession* pSession) : m_pSession(pSession) {}
        void DestroySession(bool bDestroy) 
        {
            // Turn on/off the auto-destruction of the session
            m_bDestroy = bDestroy;
        }
        ~TwainSessionRAII()
        {
            if (m_bDestroy)
                DestroyTwainSession(m_pSession);
        }
    };

    // Try to load the source manager ( set in state 2 if not already
    //    in state 2 or 3)
    if ( !s_pGlobalAppMgr )
        DTWAIN_ERROR_CONDITION(IDS_ErrTwainMgrInvalid, nullptr, true)

    // Load if not already loaded
    if ( !s_pGlobalAppMgr->m_hLibModule )
    {
        if ( !s_pGlobalAppMgr->LoadSourceManager() )
            return nullptr;
    }

    const auto pSession = CTL_ITwainSession::Create(pHandle, pAppName, hAppWnd);

    // Store this session as an RAII object, so that destruction is possible if
    // something goes wrong...

    TwainSessionRAII raii(pSession);

    // We need to now set the ProtocolMajor, ProtocolMinor, and Supported Groups
    // to the proper levels here.  We support 1.9 for TWAIN_32.DLL (LEGACY) and 2.x for
    // TWAINDSM.DLL.
    // DTWAIN assumes 2.x, but must change for legacy TWAIN_32.DLL source manager
    if ( pHandle->m_SessionStruct.DSMName == TWAINDLLVERSION_1 )
    {
        TW_IDENTITY *pIdentity = pSession->GetAppIDPtr();
        pIdentity->ProtocolMajor = 1;
        pIdentity->ProtocolMinor = 9;
        pIdentity->SupportedGroups = DG_IMAGE | DG_CONTROL | DG_AUDIO;
    }

    // Add the session to the global TWAIN application manager
    s_pGlobalAppMgr->m_arrTwainSession.push_back({});
    s_pGlobalAppMgr->m_arrTwainSession.back().reset(pSession);

    if ( !OpenSourceManager( pSession ) )
    {
        // Something went wrong...
        return nullptr;
    }

    // Now set up the pointers to the memory functions if necessary
    if (pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_LATESTVERSION ||
        pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_VERSION2)
    {
        CTL_GetEntryPointTriplet entryPoints(pSession);
        TW_UINT16 rc = entryPoints.Execute();
        switch (rc)
        {
            case TWRC_SUCCESS:
                pHandle->m_Twain2Func.m_EntryPoint = entryPoints.getEntryPoint();
                pHandle->m_TwainMemoryFunc = &pHandle->m_Twain2Func;
                break;
            default:
                // Function entry points for the TWAIN 2.x DSM are missing, so this is no good...
                LogWriterUtils::WriteLogInfoIndentedA("The entry points for the TWAINDSM.DLL were not found");
                return nullptr;
        }
    }
    else
        pHandle->m_TwainMemoryFunc = &pHandle->m_TwainLegacyFunc;

    // Turn off the auto-destruction of the created session
    raii.DestroySession(false);
    s_pSelectedSession = s_pGlobalAppMgr->m_arrTwainSession.back().get();
    return s_pSelectedSession;
}

bool CTL_TwainAppMgr::OpenSourceManager( CTL_ITwainSession* pSession )
{
    CTL_TwainDLLHandle* pHandle = pSession->GetTwainDLLHandle();
    CTL_TwainOpenSMTriplet SM( pSession );
    if ( SM.Execute() != TWRC_SUCCESS )
        return false;
    pHandle->m_nDSMState = DSM_STATE_OPENED;
    pHandle->m_nDSMVersion = SM.GetDSMVersion();
    pHandle->m_TwainMemoryFunc = &pHandle->m_TwainLegacyFunc;

    if ( pHandle->m_nDSMVersion == DTWAIN_TWAINDSM_VERSION2)
    {
        // For 2.0 data sources.  Set the handle to the memory functions
        CTL_GetEntryPointTriplet EntryPoint( pSession );
        if ( EntryPoint.Execute() == TWRC_SUCCESS )
        {
            // assign the memory functions to whatever the DSM has returned for
            // the memory management functions
            pHandle->m_Twain2Func.m_EntryPoint = EntryPoint.getEntryPoint();
            pHandle->m_TwainMemoryFunc = &pHandle->m_Twain2Func;
        }
        else
            return false;
    }
    return true;
}

bool CTL_TwainAppMgr::IsVersion2DSMUsed()
{
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    return pHandle->m_nDSMVersion == DTWAIN_TWAINDSM_VERSION2;
}

int CTL_TwainAppMgr::CopyFile(CTL_StringType strIn, CTL_StringType strOut)
{
    copy_file(strIn, strOut, filesys::copy_options::overwrite_existing);
    return 1;
}

const CTL_ITwainSource* CTL_TwainAppMgr::SelectSourceDlg(  CTL_ITwainSession *pSession, LPCTSTR pProduct/*=nullptr*/)
{
    if ( !pSession )
        return nullptr;
    CTL_SelectSourceDlgTriplet SelectSource( pSession, pProduct );
    const TW_UINT16 rc = SelectSource.Execute();

    switch (rc)
    {
        case TWRC_CANCEL:
            SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_ACQUIRECANCELLED_EX, -1L);
            DTWAIN_ERROR_CONDITION(DTWAIN_ERR_SOURCESELECTION_CANCELED, NULL, false)
        break;

        case TWRC_FAILURE:
        {
            auto ccode = CTL_TwainAppMgr::GetLastConditionCodeError();
            SendTwainMsgToWindow(pSession, nullptr, DTWAIN_SelectSourceFailed, ccode);
            return nullptr;
        }
    }

    return pSession->GetSelectedSource();
}


unsigned int CTL_TwainAppMgr::GetRegisteredMsg()
{
    if ( s_pGlobalAppMgr )
        return s_pGlobalAppMgr->m_nTwainMsg;
    return 0;
}

void CTL_TwainAppMgr::EnumSources( CTL_ITwainSession* pSession )
{
    if ( !IsValidTwainSession( pSession ) )
        return;
    pSession->GetNumSources();
}

void CTL_TwainAppMgr::DestroyTwainSession(const CTL_ITwainSession* pSession)
{
    if ( s_pGlobalAppMgr )
        s_pGlobalAppMgr->DestroySession( pSession );
}

bool CTL_TwainAppMgr::IsValidTwainSession(const CTL_ITwainSession* pSession)
{
    if (!s_pGlobalAppMgr)
        return false;
    return FindSession(pSession) != s_pGlobalAppMgr->m_arrTwainSession.end();
}

bool CTL_TwainAppMgr::IsValidTwainSource( const CTL_ITwainSession* pSession, const CTL_ITwainSource *pSource)
{
    return IsValidTwainSession( pSession ) && pSession->IsValidSource( pSource );
}

// This unloads the TWAIN source manager.  This function detaches
// ALL apps from TWAIN.  This means that if you have a third-party app that uses
// TWAIN, that app will not function or will GPF when a scanning request is given.
void CTL_TwainAppMgr::UnloadSourceManager()
{
    CTL_TwainDLLHandle* pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if ( s_pGlobalAppMgr && s_pGlobalAppMgr->m_hLibModule.is_loaded() )
    {
        while (s_pGlobalAppMgr->m_hLibModule.is_loaded())
            s_pGlobalAppMgr->m_hLibModule.unload();
    }
    pHandle->m_nDSMState = DSM_STATE_NONE;
}

CTL_TwainSessionArray::iterator CTL_TwainAppMgr::FindSession(const CTL_ITwainSession* pSession)
{
    return std::find_if(s_pGlobalAppMgr->m_arrTwainSession.begin(),
                        s_pGlobalAppMgr->m_arrTwainSession.end(), [&](auto& session)
    {
        return session.get() == pSession;
    });
}

HWND* CTL_TwainAppMgr::GetWindowHandlePtr( const CTL_ITwainSession* pSession )
{
    if ( !s_pGlobalAppMgr )
        return nullptr;
    const auto iter = FindSession(pSession);
    if ( iter != s_pGlobalAppMgr->m_arrTwainSession.end() )
        return pSession->GetWindowHandlePtr();
    return nullptr;
}

CTL_ITwainSource* CTL_TwainAppMgr::GenericSourceSelector(CTL_ITwainSession* pSession, const CTL_ITwainSource* pSource, LPCTSTR lpszSource, int nWhich)
{
    if (!s_pGlobalAppMgr)
        return nullptr;
    const auto iter = FindSession(pSession);
    if (iter != s_pGlobalAppMgr->m_arrTwainSession.end())
    {
        bool bSelectedSource;
        switch (nWhich)
        {
            case 0: // Get source via TWAIN dialog
                bSelectedSource = pSession->SelectSource(pSource);
            break;
            case 1: // Get source via product name
                bSelectedSource = pSession->SelectSource(lpszSource);
            break;
            case 2: // Get the default source
                bSelectedSource = pSession->SelectSource(static_cast<CTL_ITwainSource*>(nullptr));
            break;
            default:
                return nullptr;
        }
        return bSelectedSource?pSession->GetSelectedSource():nullptr;
    }
    return nullptr;
}

CTL_ITwainSource* CTL_TwainAppMgr::SelectSource( CTL_ITwainSession* pSession, const CTL_ITwainSource* pSource)
{
    return GenericSourceSelector(pSession, pSource, nullptr, 0);
}

CTL_ITwainSource* CTL_TwainAppMgr::SelectSource(CTL_ITwainSession* pSession, LPCTSTR strSource)
{
    auto retVal = GenericSourceSelector(pSession, nullptr, strSource, 1);
    if ( retVal == nullptr )
        static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal())->m_lLastError = DTWAIN_ERR_SOURCENAME_NOTINSTALLED;
    return retVal;
}

CTL_ITwainSource*  CTL_TwainAppMgr::GetDefaultSource(CTL_ITwainSession* pSession)
{
    return GenericSourceSelector(pSession, nullptr, nullptr, 2);
}

bool CTL_TwainAppMgr::SetDefaultSource(CTL_ITwainSource* pSource)
{
    // This only works for TWAIN DSM 2.x and above
    if (pSource && pSource->GetDTWAINHandle()->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_VERSION2)
    {
        const auto pSession = pSource->GetTwainSession();
        return SetDefaultSource(pSession, pSource);
    }
    return false;
}

bool CTL_TwainAppMgr::OpenSource( CTL_ITwainSession* pSession, const CTL_ITwainSource* pSource/*=nullptr*/)
{
    if ( !s_pGlobalAppMgr )
        return false;
    const auto iter = FindSession(pSession);
    if ( iter != s_pGlobalAppMgr->m_arrTwainSession.end() )
    {
        const bool bSourceOpened = pSession->OpenSource(pSource);
        if ( bSourceOpened )
            s_pSelectedSession = pSession;
        else
            s_pSelectedSession = nullptr;
        return bSourceOpened;
    }

    return false;
}

LONG CTL_TwainAppMgr::DoCapContainerTest(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, TW_UINT16 nCap, LONG lGetType)
{
    const auto pSession = pSource->GetTwainSession();
    CTL_CapabilityGetTriplet CapTester(pSession, pSource, static_cast<TW_UINT16>(lGetType), static_cast<TW_UINT16>(nCap), 0);
    CapTester.SetTestMode(true);
    const TW_UINT16 rc = CapTester.Execute();
    if (rc == TWRC_SUCCESS)
        return CapTester.GetSupportedContainer();
    return 0;
}

template <typename LayoutTriplet>
static void GetLayoutComponents(LayoutTriplet* LayoutTrip, CTL_RealArray& rArray)
{
    rArray.push_back(LayoutTrip->GetLeft());
    rArray.push_back(LayoutTrip->GetTop());
    rArray.push_back(LayoutTrip->GetRight());
    rArray.push_back(LayoutTrip->GetBottom());
}
    
bool CTL_TwainAppMgr::GetImageLayoutSize(const CTL_ITwainSource* pSource, CTL_RealArray& rArray, TW_UINT16 GetType)
{
    const auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();

    rArray.clear();
    std::unique_ptr<CTL_TwainTriplet> layOutTriplet;
    if (GetType == MSG_GET)
        layOutTriplet = std::make_unique<CTL_GetImageLayoutTriplet>(pSession, pTempSource);
    else
        layOutTriplet = std::make_unique<CTL_GetDefaultImageLayoutTriplet>(pSession, pTempSource);

    const TW_UINT16 rc = layOutTriplet->Execute();
    if ( rc == TWRC_SUCCESS )
    {
        if ( GetType == MSG_GET )
            GetLayoutComponents(static_cast<CTL_GetImageLayoutTriplet*>(layOutTriplet.get()), rArray);
        else
            GetLayoutComponents(static_cast<CTL_GetDefaultImageLayoutTriplet*>(layOutTriplet.get()), rArray);
        return true;
    }
    return false;
}


bool CTL_TwainAppMgr::SetImageLayoutSize(const CTL_ITwainSource* pSource,
                                         const CTL_RealArray& rArray,
                                         CTL_RealArray& rActual,
                                         TW_UINT16 SetType)
{
    rActual.clear();

    const auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();

    std::unique_ptr<CTL_TwainTriplet> layOutTriplet;
    if (::IsMSGResetType(static_cast<TW_UINT16>(SetType)))
        layOutTriplet = std::make_unique<CTL_ResetImageLayoutTriplet>(pSession, pTempSource, nullptr);
    else
        layOutTriplet = std::make_unique<CTL_SetImageLayoutTriplet>(pSession, pTempSource, &rArray);

    TW_UINT16 rc = layOutTriplet->Execute();
    switch ( rc )
    {
        case TWRC_SUCCESS:
            GetImageLayoutSize( pSource, rActual, MSG_GET );
            return true;

        case TWRC_CHECKSTATUS:
        {
            // Get the results and set them in the array
            GetImageLayoutSize( pSource, rActual, MSG_GET );
            return true;
        }

        default:
        {
        }
        return false;
    }
}

bool CTL_TwainAppMgr::CloseSource(CTL_ITwainSession* pSession,
                                  const CTL_ITwainSource* pSource/*=nullptr*/,
                                  bool bForce)
{
    if ( !s_pGlobalAppMgr )
        return false;
    const auto iter = FindSession(pSession);
    if ( iter != s_pGlobalAppMgr->m_arrTwainSession.end() )
        return pSession->CloseSource( pSource, bForce )?true:false;
    return true;
}

bool CTL_TwainAppMgr::ShowUserInterface( CTL_ITwainSource *pSource, bool bTest, bool bShowUIOnly )
{
    struct origSourceState
    {
        CTL_ITwainSource *pSource;
        CTL_ITwainSession *pSession;
        SourceState oldState;
        bool isModal;
        bool isUIOnly;
        origSourceState(CTL_ITwainSource *p, CTL_ITwainSession *pS) : pSource(p), pSession(pS)
        {
            oldState = pSource->GetState();
            isModal = pSource->IsModal();
            isUIOnly = pSource->IsUIOnly();
            SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_UIOPENED, reinterpret_cast<LPARAM>(pSource));
        }

        void restoreState() const
        {
            pSource->SetState(oldState);
            pSource->SetModal(isModal);
            pSource->SetUIOnly(isUIOnly); 
            SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_UIOPENFAILURE, reinterpret_cast<LPARAM>(pSource));
        }
    };

    const auto pTempSource = static_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();

    if ( pTempSource->IsUIOpen() )
        return true;
    // Check if testing the UI
    if ( bTest )
    {
        return false;  // Just return that UI can not be tested.  Assume no UI cannot be done.
    }

    std::unique_ptr<CTL_TwainTriplet> pUITrip;
    if ( bShowUIOnly )
        pUITrip = std::make_unique<CTL_DisplayUserInterfaceOnlyTriplet>(pSession, pTempSource, pTempSource->GetTWUserInterface());
    else
        pUITrip = std::make_unique<CTL_EnableUserInterfaceTriplet>(pSession, pTempSource, pTempSource->GetTWUserInterface(),
                                                                   static_cast<TW_BOOL>(pTempSource->IsUIOpenOnAcquire()));
    origSourceState oState(pTempSource, pSession);

    // Show the user interface (UI) now.
    const TW_UINT16 rc = pUITrip->Execute();

    // Failed to show the UI
    if (rc != TWRC_SUCCESS )
    {
        oState.restoreState();

        switch ( rc )
        {
            // Check if User interface suppression was attempted
            case TWRC_CHECKSTATUS:
            case TWRC_FAILURE:
                if ( !pSource->IsUIOpenOnAcquire() )
                    DisableUserInterface( pSource );
            break;

            //Note:  This is a bug in the Galaxy Phone TWAIN driver.  TWRC_CANCEL is returned
            // if the user interface dialog is canceled.  This is an incorrect usage, but we
            // don't have the source code to the Samsung TWAIN driver to fix it, unfortunately.
            case TWRC_CANCEL:
            {
                // This is a workaround for Sources that do not
                // Send the MSG_xxx messages to close the user interface.
                // This is actually a bug in the TWAIN device, not a
                // bug in DTWAIN.
                SendTwainMsgToWindow(pSession,nullptr,DTWAIN_TN_UICLOSING,reinterpret_cast<LPARAM>(pSource));
                DisableUserInterface(pSource);

                // Force setting the transfer done now.
                const auto pHandle = pSource->GetDTWAINHandle();
                pHandle->m_bTransferDone = true;

                SendTwainMsgToWindow(pSession,nullptr,DTWAIN_TN_UICLOSED,reinterpret_cast<LPARAM>(pSource));
                return false;
            }
        }

        return false;
    }

    return true;
}

bool CTL_TwainAppMgr::DisableUserInterface(const CTL_ITwainSource *pSource)
{
    // Check if source is valid
    const auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();
    TW_USERINTERFACE *pTWUI = pTempSource->GetTWUserInterface();
    CTL_DisableUserInterfaceTriplet UI(pSession, pTempSource, pTWUI );
    bool bRet = true;

    // Turn off user interface
    if ( UI.Execute() != TWRC_SUCCESS )
    {
        auto ccode = GetLastConditionCodeError();
        if ( ccode != TWCC_SUCCESS )
        {
            bRet = false;
        }
    }

    // Check if we are acquiring to a multi-page file, and will
    // save the file when the Source UI is closed.
    if ( pTempSource->IsMultiPageModeUIMode() )
        pTempSource->ProcessMultipageFile();
    pTempSource->SetState(SOURCE_STATE_OPENED);

    return bRet;
}


void CTL_TwainAppMgr::EndTwainUI(const CTL_ITwainSession* pSession, CTL_ITwainSource* pSource)
{
    // The source UI must be closed for modeless Source
    if ( pSource->IsUIOpen())
    {
        SendTwainMsgToWindow(pSession,
                             nullptr,
                             DTWAIN_TN_UICLOSING,
                             reinterpret_cast<LPARAM>(pSource));
        DisableUserInterface(pSource);

        SendTwainMsgToWindow(pSession,
                             nullptr,
                             DTWAIN_TN_UICLOSED,
                             reinterpret_cast<LPARAM>(pSource));
        if ( pSource->IsMultiPageModeUIMode() )
            pSource->ProcessMultipageFile();
    }
}

bool CTL_TwainAppMgr::GetImageInfo(CTL_ITwainSource *pSource, CTL_ImageInfoTriplet *pTrip/*=NULL*/)
{
    const auto pTempSource = static_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();
    CTL_ImageInfoTriplet ImageInfo(pSession, pTempSource);
    if ( !pTrip )
        pTrip = &ImageInfo;
    if ( pTrip->Execute() != TWRC_SUCCESS )
    {
        auto ccode = GetLastConditionCodeError();
        if ( ccode != TWCC_SUCCESS )
            return false;
        return true;
    }
    pSource->SetImageInfo(pTrip->GetImageInfoBuffer());
    return true;
}

CTL_ITwainSession* CTL_TwainAppMgr::GetCurrentSession()
{
    return s_pSelectedSession;
}

CTL_ITwainSession* CTL_TwainAppMgr::GetNthSession(int nSession)
{
    if ( !s_pGlobalAppMgr )
        return nullptr;
    const size_t nSize = s_pGlobalAppMgr->m_arrTwainSession.size();
    if ( static_cast<int>(nSize) > nSession )
        return s_pGlobalAppMgr->m_arrTwainSession[nSession].get();
    return nullptr;
}


TW_UINT16 dynarithmic::CTL_TwainAppMgr::GetConditionCode( CTL_ITwainSession *pSession,
                                             CTL_ITwainSource *pSource/*=nullptr*/,
                                             TW_UINT16 rc/*=1*/)
{
    if ( rc == -DTWAIN_ERR_EXCEPTION_ERROR )
        return TWRC_FAILURE;
    CTL_ConditionCodeTriplet CC(pSession, pSource);
    if ( CC.Execute() == TWRC_SUCCESS )
        return CC.GetConditionCode();
    return TWRC_FAILURE;
}


bool CTL_TwainAppMgr::ProcessConditionCodeError(TW_UINT16 nError)
{
    auto resID = FindConditionCode(nError);
    if ( IsValidConditionCode(resID))
    {
        static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal())->m_lLastError = resID;
        if ( s_pGlobalAppMgr )
            s_pGlobalAppMgr->SetLastTwainError(nError, TWCC_Error);
        if ( nError != TWCC_SUCCESS)
            DTWAIN_ERROR_CONDITION(resID, false, false)
    }
    return true;
}


bool CTL_TwainAppMgr::IsTwainMsg(MSG *pMsg, bool bFromUserQueue/*=false*/)
{
    if ( !s_pGlobalAppMgr )
        return false;

    if ( !IsValidTwainSession( s_pSelectedSession ) )
        return false;

    CTL_ITwainSource* ptrSource = s_pSelectedSession->GetSelectedSource();
    s_pSelectedSession->SetTwainMessageFlag(true);
    if (!IsSourceOpen(ptrSource))
        return false;

    // Create a triplet to determine if message is TWAIN message
    CTL_ProcessEventTriplet processEvent( s_pSelectedSession, ptrSource, pMsg, bFromUserQueue && IsVersion2DSMUsed());

    // execute triplet
    bool retVal = false;
    const TW_UINT16 rc = processEvent.ExecuteEventHandler();
    if ( rc != TWRC_NOTDSEVENT )
        s_pGlobalAppMgr->WriteToLogFile( rc );
    switch (rc)
    {
        case TWRC_NOTDSEVENT:
            retVal = false;
        break;

        case TWRC_DSEVENT:
            retVal = true;
        break;

        case TWRC_FAILURE:
        {
            retVal = false;
        }
        break;

        case TWRC_XFERDONE:
            retVal = true;
        break;
    }
    return retVal;
}

void CTL_TwainAppMgr::NotifyFeederStatus()
{
#ifdef _WIN32
    // Make sure that user set a callback
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const auto pFn = DTWAIN_GetCallback();
    if (!pFn)
        return;
    // Check if any open source supports feeder
    auto it = pHandle->m_aFeederSources.begin();
    const auto it2 = pHandle->m_aFeederSources.end();

    while (it != it2)
    {
        const auto pSource = static_cast<CTL_ITwainSource*>(*it);
        const auto sourceState = pSource->GetState();
        if (sourceState != SOURCE_STATE_XFERREADY &&
            sourceState != SOURCE_STATE_TRANSFERRING)
        {
            if (DTWAIN_IsFeederLoaded(*it))
                (*pFn)(DTWAIN_TN_FEEDERLOADED, reinterpret_cast<LPARAM>(*it), pHandle->m_lCallbackData);
            else
                (*pFn)(DTWAIN_TN_FEEDERNOTLOADED, reinterpret_cast<LPARAM>(*it), pHandle->m_lCallbackData);
        }
        ++it;
    }
#endif
}

bool CTL_TwainAppMgr::SetFeederEnableMode( CTL_ITwainSource *pSource, bool bMode)
{
    if ( !s_pGlobalAppMgr )
        return false;

    if ( !s_pGlobalAppMgr->IsSourceOpen( pSource ))
        return false;

    pSource->SetFeederEnableMode(bMode);
    return true;
}


int CTL_TwainAppMgr::TransferImage(const CTL_ITwainSource *pSource, int nImageNum)
{
    const auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
    if ( !s_pGlobalAppMgr )
        return 0;

    if ( !IsValidTwainSession( s_pSelectedSession ) )
        return 0;
    // Test native transfer
    // if (transfer == NATIVE)  {
    pTempSource->SetPendingImageNum( nImageNum );
    const auto pSession = pTempSource->GetTwainSession();

    const CTL_TwainAcquireEnum AcquireType = pTempSource->GetAcquireType();
    pTempSource->SetNumCompressBytes(0);

    // Get the layout information for any transfer
    if ( !StoreImageLayout(pTempSource) )
        pTempSource->SetImageLayoutValid(false);
    else
        pTempSource->SetImageLayoutValid(true);

    switch ( AcquireType )
    {
        case TWAINAcquireType_Native:
            return NativeTransfer( pSession, pTempSource);

        case TWAINAcquireType_FileUsingNative:
        {
            long lFileFlags = pSource->GetAcquireFileStatus().GetAcquireFileFlags();
            if ( lFileFlags & DTWAIN_USENATIVE )
                return NativeTransfer(pSession, pTempSource);
            else
                return BufferTransfer(pSession, pTempSource);
        }
        break;

        case TWAINAcquireType_File:
        case TWAINAcquireType_MemFile:
        case TWAINAcquireType_AudioFile:
            return FileTransfer(pSession, pTempSource, AcquireType);

        case TWAINAcquireType_Buffer:
            return BufferTransfer( pSession, pTempSource );

        case TWAINAcquireType_AudioNative:
            return AudioNativeTransfer(pSession, pTempSource);

        case TWAINAcquireType_Clipboard:
            return ClipboardTransfer( pSession, pTempSource );
        default:
            break;
    }

    return 0;
}

/* Remember the image layout for transfer */
bool CTL_TwainAppMgr::StoreImageLayout(CTL_ITwainSource *pSource)
{
    FloatRect fRect;
    const auto pTempSource = static_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();


    // First, see if ICAP_UNDEFINED image size is used
    TW_UINT16 nValue;
    if ( GetOneTwainCapValue( pSource, &nValue, ICAP_UNDEFINEDIMAGESIZE, MSG_GET, TWTY_BOOL) )
    {
        if ( nValue == 1 )
        {
            // The layout can only be determined after the image has been scanned.
            fRect.left = -1.0;
            fRect.top = -1.0;
            fRect.right = -1.0;
            fRect.bottom = -1.0;
            pTempSource->SetImageLayout(&fRect);
            return true;
        }
    }

    CTL_GetImageLayoutTriplet LayoutTrip( pSession, pTempSource );

    TW_UINT16 rc = LayoutTrip.Execute();
    if ( rc == TWRC_SUCCESS )
    {
        TW_IMAGELAYOUT IL;
        memcpy(&IL, LayoutTrip.GetImageLayout(), sizeof(TW_IMAGELAYOUT));
        fRect.left = Fix32ToFloat( IL.Frame.Left );
        fRect.top = Fix32ToFloat( IL.Frame.Top );
        fRect.right = Fix32ToFloat( IL.Frame.Right );
        fRect.bottom = Fix32ToFloat( IL.Frame.Bottom );
        pTempSource->SetImageLayout(&fRect);
        return true;
    }
    return false;
}


// All sources provide this transfer capability 
int CTL_TwainAppMgr::NativeTransfer( CTL_ITwainSession *pSession,
                                      CTL_ITwainSource  *pSource)
{
    CTL_ImageXferTriplet IXfer(pSession, pSource, DAT_IMAGENATIVEXFER);
    return StartTransfer( pSession, pSource, &IXfer );
}

// All sources provide this transfer capability 
int CTL_TwainAppMgr::AudioNativeTransfer(CTL_ITwainSession *pSession, CTL_ITwainSource  *pSource)
{
    CTL_ImageXferTriplet AXfer(pSession, pSource, DAT_AUDIONATIVEXFER);
    return StartTransfer(pSession, pSource, &AXfer);
}

int CTL_TwainAppMgr::ClipboardTransfer( CTL_ITwainSession *pSession,
                                         CTL_ITwainSource *pSource )
{
    if ( pSource->GetSpecialTransferMode() == DTWAIN_USENATIVE )
        return NativeTransfer( pSession, pSource );
    return BufferTransfer( pSession, pSource );
}


int  CTL_TwainAppMgr::FileTransfer( CTL_ITwainSession *pSession,
                                    CTL_ITwainSource  *pSource,
                                    CTL_TwainAcquireEnum AcquireType)
{
    auto& acquireFileStatus = pSource->GetAcquireFileStatusRef();
    // Set the file type
    CTL_StringType sFileName;
    long lFlags = pSource->GetAcquireFileStatus().GetAcquireFileFlags();
    if ( lFlags & DTWAIN_USEPROMPT)
    {
        CTL_StringType szTempPath;
        // Set the temp file name here
        szTempPath = GetDTWAINTempFilePath(pSource->GetDTWAINHandle());
        if ( szTempPath.empty() )
            return 0;

        auto sGUID = StringWrapper::GetGUID();
        szTempPath += sGUID + _T(".IDT");
        StringWrapper::TrimAll(szTempPath);
        pSource->GetAcquireFileStatus().SetAcquireFileName(szTempPath);
    }
    else
    {
        sFileName = pSource->GetCurrentImageFileName();

        // See if name should be changed by sending notification to user
        acquireFileStatus.SetActualFileName(sFileName);
        SendTwainMsgToWindow(pSource->GetTwainSession(), nullptr, DTWAIN_TN_FILENAMECHANGING, reinterpret_cast<LPARAM>(pSource));
        // check if name has changed
        auto sFileNameNew = acquireFileStatus.GetActualFileName();
        if ( sFileName != sFileNameNew )
            SendTwainMsgToWindow(pSource->GetTwainSession(), nullptr, DTWAIN_TN_FILENAMECHANGED, reinterpret_cast<LPARAM>(pSource));
        std::swap(sFileNameNew, sFileName);  // swap the names, even if they may not have changed.
    }

    CTL_SetSetupFileXferTriplet  FileXferSetup(pSession,
                                       pSource,
                                       acquireFileStatus.GetAcquireFileFormat(),
                                       sFileName
                                       );
    // Set the file type and name
    if ( FileXferSetup.Execute() == TWRC_FAILURE )
    {
        auto ccode = GetLastConditionCodeError();
        if ( ccode != TWCC_SUCCESS )
            return 0;
        return 1;
    }

    // If this is a memory file transfer, need to set the buffer
    if ( AcquireType == TWAINAcquireType_MemFile )
    {
        // Check if user has defined a strip size
        auto nSizeStrip = static_cast<TW_UINT32>(pSource->GetUserStripBufSize());

        // User has not defined a buffer.  Let DTWAIN handle the memory here
        if (!pSource->GetUserStripBuffer())
        {
            // Get the buffer strip size
            TW_SETUPMEMXFER pXfer = {};
            if (!GetMemXferValues(pSource, &pXfer))
                return 0;
            nSizeStrip = pXfer.Preferred;
        }
        CTL_ImageMemFileXferTriplet IXfer(pSession, pSource, nSizeStrip, pSource->GetUserStripBufSize()?false:true);
        return StartTransfer(pSession, pSource, &IXfer);
    }

    // If this is an audio file, then we need to execute this triplet
    // once per acquisition
    if (AcquireType == TWAINAcquireType_AudioFile)
    {
        CTL_AudioFileXferTriplet AudioXFer(pSession, pSource);
        // Set the file type and name
        if (AudioXFer.Execute() == TWRC_FAILURE)
        {
            auto ccode = GetLastConditionCodeError();
            if (ccode != TWCC_SUCCESS)
                return 0;
            return 1;
        }
    }


    // Start the transferring of the image
    CTL_ImageXferTriplet IXfer(pSession,
                               pSource,
                               DAT_IMAGEFILEXFER);

    return StartTransfer( pSession, pSource, &IXfer );
}

int  CTL_TwainAppMgr::BufferTransfer( CTL_ITwainSession *pSession,
                                      CTL_ITwainSource  *pSource,
                                      bool bIsMemoryFile)
{
    // Get the source
    auto* pTempSource = static_cast<CTL_ITwainSource*>(pSource);

    // Get the image information
    CTL_ImageInfoTriplet ImageInfo(pSession, pTempSource);
    if ( ImageInfo.Execute() != TWRC_SUCCESS )
        return 0;
    TW_IMAGEINFO *pInfo = ImageInfo.GetImageInfoBuffer();

    // Get the buffer strip size
    TW_SETUPMEMXFER pXfer = {};
    if ( !GetMemXferValues(pTempSource, &pXfer))
        return 0;

    // Get the total size of an image of 256 colors

    // First check if the user has defined their own buffer
    HGLOBAL hGlobAcquire;
    DWORD nTotalSize = 0;
    TW_INT32 nSizeStrip;

    // Check if user has defined a strip size
    nSizeStrip = static_cast<TW_INT32>(pSource->GetUserStripBufSize());

    // User has not defined a buffer.  Let DTWAIN handle the memory here
    if ( !pSource->GetUserStripBuffer())
    {
        // Did user specify a size?  If 0, DTWAIN determines the size
        nSizeStrip = AllocateBufferStrip( pInfo, &pXfer, &hGlobAcquire, &nTotalSize, nSizeStrip, pSource->GetCompressionType());
        if ( nSizeStrip == 0 )
            return 0;
    }
    else
    {
        // User has defined a buffer
        hGlobAcquire = pSource->GetUserStripBuffer();
        nSizeStrip = static_cast<TW_INT32>(pSource->GetUserStripBufSize());
    }
    // Setup the DIB's information for an uncompressed image
    if ( pSource->GetCompressionType() == TWCP_NONE )
        SetupMemXferDIB( pSession, pSource, hGlobAcquire, pInfo, static_cast<TW_INT32>(nTotalSize));

    // hGlobAcquire is a handle to a DIB that will be used as the bitmap to display
    // Set up the transfer triplet
    // Get the default transfer strip
    LONG nFlags = TWMF_APPOWNS | TWMF_POINTER;

    // For tiled mode, the Source owns the memory
    if (pSource->IsTileModeOn())
        nFlags = TWMF_DSOWNS;

    CTL_ImageMemXferTriplet IXfer(pSession, pSource, hGlobAcquire, nFlags,
                                  pInfo->PixelType, nSizeStrip, static_cast<TW_UINT16>(pSource->GetCompressionType()));
    return  StartTransfer( pSession, pSource, &IXfer );
}

TW_UINT16 CTL_TwainAppMgr::GetMemXferValues(CTL_ITwainSource *pSource, TW_SETUPMEMXFER *pXfer)
{
    const auto pSession = pSource->GetTwainSession();
    CTL_SetupMemXferTriplet MemXfer(pSession, pSource);
    if ( MemXfer.Execute() != TWRC_SUCCESS )
        return 0;

    memcpy(pXfer, MemXfer.GetSetupMemXferBuffer(), sizeof(TW_SETUPMEMXFER));
    return 1;
}

TW_INT32 CTL_TwainAppMgr::AllocateBufferStrip(const TW_IMAGEINFO *pImgInfo,
                                              const TW_SETUPMEMXFER *pSetupInfo,
                                              HGLOBAL *pGlobal,
                                              DWORD* pSize,
                                              DWORD SizeToUse,
                                              LONG nCompression)
{
    if ( pSetupInfo->MinBufSize == 0 ||
         pSetupInfo->MaxBufSize == 0 ||
         pSetupInfo->Preferred == 0)
        return 0;

    DWORD nExtra = sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD);
    TW_INT32 nImageSize = (static_cast<TW_INT32>(pImgInfo->ImageWidth) *
            pImgInfo->BitsPerPixel + 31) / 32 *4
        * pImgInfo->ImageLength;
    // No image if compression is used
    if ( nCompression != TWCP_NONE)
    {
        nImageSize = 0;
        nExtra = 0;
    }

    // SizeToUse has the amount to allocate if app specifies the buffer size
    int nBlocks;
    DWORD nAllocSize;
    if ( SizeToUse > 0 )
    {
        nBlocks = static_cast<int>(nImageSize / SizeToUse);
        nAllocSize = (nBlocks + 2) * SizeToUse;
        *pGlobal = ImageMemoryHandler::GlobalAlloc( GHND,   nAllocSize + nExtra);
        *pSize = nAllocSize;
         if ( *pGlobal )
            return static_cast<TW_INT32>(SizeToUse);
        return 0;
    }

    // DTWAIN determines how much to allocate
    // Attempt to allocate preferred size
    nBlocks = static_cast<int>(nImageSize / pSetupInfo->Preferred);
    nAllocSize = (nBlocks + 2) * pSetupInfo->Preferred;
    *pSize = nAllocSize;


    // Try preferred size
    *pGlobal = ImageMemoryHandler::GlobalAlloc( GHND, nAllocSize + nExtra);
     if ( *pGlobal )
        return static_cast<TW_INT32>(pSetupInfo->Preferred);

    // Try the minimum size
    nBlocks = static_cast<int>(nImageSize / pSetupInfo->MinBufSize);
    nAllocSize = (nBlocks + 2) * pSetupInfo->MinBufSize;
    *pSize = nAllocSize;
    *pGlobal = ImageMemoryHandler::GlobalAlloc( GHND,   nAllocSize + nExtra );

    if ( *pGlobal )
        return static_cast<TW_INT32>(pSetupInfo->MinBufSize);
    return 0;
}

bool CTL_TwainAppMgr::SetupMemXferDIB(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource,
                                      HGLOBAL hGlobal, const TW_IMAGEINFO* pImgInfo, TW_INT32 nSize)
{
    DTWAINGlobalHandle_RAII dibHandle(hGlobal);
    const auto pDibInfo = static_cast<LPBITMAPINFO>(ImageMemoryHandler::GlobalLock(hGlobal));

    // fill in the image information
    pDibInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pDibInfo->bmiHeader.biWidth  = pImgInfo->ImageWidth;
    pDibInfo->bmiHeader.biHeight = pImgInfo->ImageLength;
    pDibInfo->bmiHeader.biPlanes = 1;
    pDibInfo->bmiHeader.biBitCount = pImgInfo->BitsPerPixel;
    pDibInfo->bmiHeader.biCompression = 0;
    pDibInfo->bmiHeader.biSizeImage   = nSize;

    // Get Units and calculate PelsPerMeter
    const auto nValue = static_cast<TW_INT16>(GetCurrentUnitMeasure(pSource));
    if ( nValue == -1)
        return false;

    const float XRes = Fix32ToFloat(pImgInfo->XResolution);
    const float YRes = Fix32ToFloat(pImgInfo->YResolution);

    switch( nValue )
    {
        case TWUN_INCHES:
            pDibInfo->bmiHeader.biXPelsPerMeter = static_cast<LONG>((XRes * 39.37008));
            pDibInfo->bmiHeader.biYPelsPerMeter = static_cast<LONG>((YRes * 39.37008));
            break;

        case TWUN_CENTIMETERS:
            pDibInfo->bmiHeader.biXPelsPerMeter = static_cast<LONG>(XRes * 100);
            pDibInfo->bmiHeader.biYPelsPerMeter = static_cast<LONG>(YRes * 100);
            break;

        case TWUN_TWIPS:
            pDibInfo->bmiHeader.biXPelsPerMeter = static_cast<LONG>((XRes * 56692.9152));
            pDibInfo->bmiHeader.biYPelsPerMeter = static_cast<LONG>((YRes * 56692.9152));
            break;

        case TWUN_PICAS:
        case TWUN_POINTS:
        case TWUN_PIXELS:
        default:
            pDibInfo->bmiHeader.biXPelsPerMeter = 0;
            pDibInfo->bmiHeader.biYPelsPerMeter = 0;
            break;
    }

    switch (pImgInfo->PixelType)
    {
        TW_INT16 nPixelFlavor;
        case TWPT_BW:
        {
            pDibInfo->bmiHeader.biClrUsed       = 2;
            pDibInfo->bmiHeader.biClrImportant  = 0;

            // Get Units and calculate PelsPerMeter
            nPixelFlavor = TWPF_CHOCOLATE;
            if ( !GetOneTwainCapValue( pSource, &nPixelFlavor, ICAP_PIXELFLAVOR, MSG_GETCURRENT, TWTY_UINT16))
                nPixelFlavor = TWPF_CHOCOLATE;
            switch ( nPixelFlavor )
            {
                case TWPF_CHOCOLATE:
                {
                    pDibInfo->bmiColors[0].rgbRed = 0x0000;
                    pDibInfo->bmiColors[0].rgbGreen = 0x0000;
                    pDibInfo->bmiColors[0].rgbBlue = 0x0000;
                    pDibInfo->bmiColors[0].rgbReserved = 0;

                    pDibInfo->bmiColors[1].rgbRed = 0x00FF;
                    pDibInfo->bmiColors[1].rgbGreen = 0x00FF;
                    pDibInfo->bmiColors[1].rgbBlue = 0x00FF;
                    pDibInfo->bmiColors[1].rgbReserved = 0;
                }
                break;

                case TWPF_VANILLA:
                {
                    pDibInfo->bmiColors[0].rgbRed = 0x00FF;
                    pDibInfo->bmiColors[0].rgbGreen = 0x00FF;
                    pDibInfo->bmiColors[0].rgbBlue = 0x00FF;
                    pDibInfo->bmiColors[0].rgbReserved = 0;

                    pDibInfo->bmiColors[1].rgbRed = 0x0000;
                    pDibInfo->bmiColors[1].rgbGreen = 0x0000;
                    pDibInfo->bmiColors[1].rgbBlue = 0x0000;
                    pDibInfo->bmiColors[1].rgbReserved = 0;
                }
                break;
            }
        }
        break;

        case TWPT_GRAY:
        {
            if ( pDibInfo->bmiHeader.biBitCount == 4)
            {
                pDibInfo->bmiHeader.biClrUsed = 16;
                for (DWORD i=0; i<pDibInfo->bmiHeader.biClrUsed; i++)
                {
                    BYTE val = static_cast<BYTE>(i << 4 | i);
                    pDibInfo->bmiColors[i].rgbRed = val;
                    pDibInfo->bmiColors[i].rgbGreen = val;
                    pDibInfo->bmiColors[i].rgbBlue = val;
                    pDibInfo->bmiColors[i].rgbReserved = 0;
                }
            }
            else
            {
                pDibInfo->bmiHeader.biClrUsed = 256;
                for (DWORD i=0; i<pDibInfo->bmiHeader.biClrUsed; i++)
                {
                    pDibInfo->bmiColors[i].rgbRed = static_cast<BYTE>(i);
                    pDibInfo->bmiColors[i].rgbGreen = static_cast<BYTE>(i);
                    pDibInfo->bmiColors[i].rgbBlue = static_cast<BYTE>(i);
                    pDibInfo->bmiColors[i].rgbReserved = 0;
                }
            }
        }
        break;

        case TWPT_RGB:
            pDibInfo->bmiHeader.biClrUsed = 0;
        break;

        case TWPT_PALETTE:
        case TWPT_CMY:
        case TWPT_CMYK:
        case TWPT_YUV:
        case TWPT_YUVK:
        case TWPT_CIEXYZ:
        default:
        {
            CTL_GetPalette8Triplet  Palette8(pSession, pSource);

            if ( Palette8.Execute() == TWRC_FAILURE )
            {
                pDibInfo->bmiHeader.biClrImportant = 0;
                pDibInfo->bmiHeader.biClrUsed = 256;
                for (int i=0; i<=255; i++)
                {
                    pDibInfo->bmiColors[i].rgbRed = static_cast<BYTE>(i);
                    pDibInfo->bmiColors[i].rgbGreen = static_cast<BYTE>(i);
                    pDibInfo->bmiColors[i].rgbBlue = static_cast<BYTE>(i);
                    pDibInfo->bmiColors[i].rgbReserved = 0;
                }
            }
            else
            {
                const TW_PALETTE8 *pBuf = Palette8.GetPalette8Buffer();
                pDibInfo->bmiHeader.biClrUsed = pBuf->NumColors;
                pDibInfo->bmiHeader.biClrImportant = 0;
                for (int i=0; i<pBuf->NumColors; i++)
                {
                    pDibInfo->bmiColors[i].rgbRed = pBuf->Colors[i].Channel1;
                    pDibInfo->bmiColors[i].rgbGreen = pBuf->Colors[i].Channel2;
                    pDibInfo->bmiColors[i].rgbBlue = pBuf->Colors[i].Channel3;
                    pDibInfo->bmiColors[i].rgbReserved = 0;
                }
            }
        }
        break;
    }

//    GlobalUnlock(hGlobal);
    return true;
}




int CTL_TwainAppMgr::StartTransfer( CTL_ITwainSession * /*pSession*/,
                                     CTL_ITwainSource *pSource,
                                     CTL_ImageXferTriplet *pTrip)
{
    const int nCurImage = pSource->GetPendingImageNum();
    pSource->SetState(SOURCE_STATE_TRANSFERRING);
    pSource->SetTransferDone(false);
    const TW_UINT16 rc = pTrip->Execute();
    switch (rc )
    {
        case TWRC_FAILURE:
        {
            // A failure occurred.  See if termination is done
            if (pTrip->GetAcquireFailAction() == DTWAIN_PAGEFAIL_TERMINATE)
                return 0; //  No more images pending
            else
                return -1; // Failure, attempt to scan again
        }
        break;

        case TWRC_XFERDONE:
        {
            // Check if File Transfer and using Prompt
            const CTL_TwainAcquireEnum nAcquireType = pSource->GetAcquireType();
            if ( nAcquireType == TWAINAcquireType_FileUsingNative )
            {
                // Check if using Prompt
                long lFlags   = pSource->GetAcquireFileStatusRef().GetAcquireFileFlags();
                if ( lFlags & DTWAIN_USEPROMPT && pSource->IsPromptPending())
                {
                    pTrip->PromptAndSaveImage( nCurImage );
                    if ( pSource->IsDeleteDibOnScan() )
                    {
                        // Get the array of current array of DIBS (this pointer allows changes to Source's internal DIB array)
                        CTL_TwainDibArray* pArray = pSource->GetDibArray();
                        // Let array class handle deleting of the DIB (Global memory will be freed only)
                        pArray->DeleteDibMemory( nCurImage );
                    }
                }
            }
            else
            if ( nAcquireType == TWAINAcquireType_File ) // The TWAIN source is solely responsible for the file handling
            {
                // Send notification that file save was successful
                CTL_TwainAppMgr::SendTwainMsgToWindow(pSource->GetTwainSession(),
                    nullptr, DTWAIN_TN_FILEPAGESAVEOK, reinterpret_cast<LPARAM>(pSource));
            }
        }
        break;
    }
    return pTrip->IsScanPending();  // true = more images, false = no more images
}


bool CTL_TwainAppMgr::GetFileTransferDefaults(CTL_ITwainSource *pSource, int &nFileType)
{
    const auto pTempSource = static_cast<CTL_ITwainSource*>(pSource);
    const auto pSession = pTempSource->GetTwainSession();
    CTL_GetDefaultSetupFileXferTriplet  FileXferGetDef( pSession, pTempSource,
                                            static_cast<CTL_TwainFileFormatEnum>(0),{});
    if ( FileXferGetDef.Execute() == TWRC_SUCCESS )
    {
        nFileType = FileXferGetDef.GetFileFormat();
        return true;
    }
    return false;
}


CTL_TwainAcquireEnum CTL_TwainAppMgr::GetCompatibleFileTransferType( const CTL_ITwainSource *pSource )
{
    const auto& iArray = pSource->GetSupportedTransferMechanisms();
    if ( iArray.empty() )
        return TWAINAcquireType_Invalid;
    if ( std::find(iArray.begin(), iArray.end(), TWSX_FILE) != iArray.end())
        return TWAINAcquireType_File;
    return TWAINAcquireType_FileUsingNative;
}

bool CTL_TwainAppMgr::IsMemFileTransferSupported(const CTL_ITwainSource *pSource)
{
    const auto& iArray = pSource->GetSupportedTransferMechanisms();
    return (std::find(iArray.begin(), iArray.end(), TWSX_MEMFILE) != iArray.end());
}

HINSTANCE CTL_TwainAppMgr::GetAppInstance()
{
    if ( s_pGlobalAppMgr )
        return s_pGlobalAppMgr->m_Instance;
    return nullptr;
}

int CTL_TwainAppMgr::SendTwainMsgToWindow(const CTL_ITwainSession *pSession,
                                           HWND hWndWhich,
                                           WPARAM wParam,
                                           LPARAM lParam)
{
#ifdef _WIN32
    const unsigned int nMsg = GetRegisteredMsg();
    if ( hWndWhich == nullptr)
    {
        if ( pSession )
        {
            const HWND hWnd = *pSession->GetWindowHandlePtr();
            if ( hWnd )
                return static_cast<int>(::SendMessage(hWnd, nMsg, wParam, lParam));
        }
    }
    else
        return static_cast<int>(::SendMessage(hWndWhich, nMsg, wParam, lParam));
#endif
    return 1;
}

bool CTL_TwainAppMgr::CloseSourceManager(CTL_ITwainSession* pSession)
{
    // Close the source manager
    // Use the session
    CTL_TwainDLLHandle* pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    CTL_TwainCloseSMTriplet SM( pSession );
    const TW_UINT16 rc = SM.Execute();
    if ( rc != TWRC_SUCCESS )
        return false;
    pHandle->m_nDSMState = DSM_STATE_LOADED;
    return true;
}


////////// These are static error functions that get errors from the RC file
void CTL_TwainAppMgr::SetError(int nError, std::string_view extraInfo, bool bMustReportGeneralError)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    char szBuffer[1024] = {0};
    int nRealError = nError;
    if ( nRealError > 0 )
        nRealError = -nRealError;

    static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal())->m_lLastError = nRealError;

    if ( nError < 0 )
        nError = -nError;  // Can't have negative error codes
    GetResourceStringA(nError, szBuffer, 1024);
    s_strLastError  = szBuffer;
    s_strLastError += " ";
    s_strLastError += extraInfo;
    s_nLastError    = nError;

    CTL_StaticData::GetExtraErrorInfoMap()[abs(s_nLastError)] = extraInfo;
    if ( CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_USEBUFFER )
    {
        // Push error onto error stack
        const std::deque<int>::size_type nEntries = pHandle->m_vErrorBuffer.size();
        pHandle->m_vErrorBuffer.push_front(-nError);

        // Check if beyond reserve size
        const unsigned int nReserve = pHandle->m_nErrorBufferReserve;

        if ( nEntries > nReserve)
            pHandle->m_vErrorBuffer.resize(pHandle->m_nErrorBufferThreshold);
    }

    const UINT uMsg = CTL_StaticData::GetRegisteredMessage();
    LogDTWAINMessage(nullptr, uMsg, DTWAIN_TN_GENERALERROR, -nError, true);

    // if there is a callback and we want to report the general error, 
    // call it now with the error notifications.  We don't want to overwhelm the non-error
    // related callbacks with all of the general errors we are getting.
    if (bMustReportGeneralError)
    {
        if (pHandle->m_pCallbackFn)
            (*pHandle->m_pCallbackFn)(DTWAIN_TN_GENERALERROR, -nError, static_cast<LPARAM>(0));

        // If there is a 64 bit callback, call it now with the error notifications
        if (pHandle->m_pCallbackFn64)
            (*pHandle->m_pCallbackFn64)(DTWAIN_TN_GENERALERROR, -nError, static_cast<LPARAM>(0));
    }

    // If there is an error-only callback, call it now with the error notification
    if ( pHandle->m_pErrorProcFn )
        (*pHandle->m_pErrorProcFn)(-nError, pHandle->m_lErrorProcUserData);

    if (pHandle->m_pErrorProcFn64)
        (*pHandle->m_pErrorProcFn64)(-nError, pHandle->m_lErrorProcUserData64);
}

int CTL_TwainAppMgr::GetLastError()
{
    return s_nLastError;
}

LPSTR CTL_TwainAppMgr::GetLastErrorString(LPSTR lpszBuffer, int nSize)
{
    return GetErrorString(GetLastError(), lpszBuffer, nSize);
}


LPSTR CTL_TwainAppMgr::GetErrorString(int nError, LPSTR lpszBuffer, int nSize)
{
    if ( nError == s_nLastError )
        StringWrapperA::CopyInfoToCString(s_strLastError, lpszBuffer, nSize);
    else
        GetResourceStringA(nError, lpszBuffer, nSize);
    return lpszBuffer;
}

void CTL_TwainAppMgr::SetAndLogError(int nError, std::string_view extraInfo, bool bMustReportGeneralError)
{
    int nActualError = std::abs(nError);
    CTL_TwainAppMgr::SetError(nActualError, extraInfo.data(), bMustReportGeneralError);
    if (CTL_StaticData::GetLogFilterFlags() != 0)
    {
        char szBuf[DTWAIN_USERRES_MAXSIZE + 1] = {};
        CTL_TwainAppMgr::GetLastErrorString(szBuf, DTWAIN_USERRES_MAXSIZE);
        LogWriterUtils::WriteLogInfoIndentedA(szBuf);
    }
}


////////////////////////////////////////////////////////////////////////////
///////////////////  ******* Capability Code ******* //////////////////////
///////////////////////////////////////////////////////////////////////////
bool CTL_TwainAppMgr::IsCapabilitySupported(const CTL_ITwainSource *pSource, TW_UINT16 nCap, bool bRetest, int nType)
{
    if (!pSource)
        return false;

    if (!IsSourceOpen(pSource))
        return false;

    const auto pTempSource = const_cast<CTL_ITwainSource *>(pSource);

    const bool supported = pSource->IsCapInSupportedList(nCap);
    if (supported)
        return true;

    if (pSource->IsCapInUnsupportedList(nCap))
        return false;

    if (!bRetest)
        return false;

    const auto pSession = pTempSource->GetTwainSession();

    if (!IsValidTwainSession(pSession))
        return false;

    if (!s_pGlobalAppMgr->IsSourceOpen(pSource))
        return false;

    std::unique_ptr<CTL_CapabilityGetTriplet> pTrip;
    switch (nType)
    {
        case MSG_GET:
        case MSG_GETCURRENT:
        case MSG_GETDEFAULT:
            pTrip = std::make_unique<CTL_CapabilityGetTriplet>(pSession, pTempSource, 
                                    static_cast<TW_UINT16>(nType), nCap, TW_UINT16{ 0 });
            break;

        default:
            return false;
    }
    const bool bRet = pTrip->IsCapabilitySupported();
    if (!bRet)
        pTempSource->AddCapToUnsupportedList(nCap);
    return bRet;
}

bool CTL_TwainAppMgr::IsCapabilitySupported(const CTL_ITwainSource *pSource, TW_UINT16 nCap, int nType /*=MSG_GET*/)
{
    return IsCapabilitySupported(pSource, nCap, false, nType);
}

bool CTL_TwainAppMgr::IsSourceOpen( const CTL_ITwainSource *pSource )
{
    const auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);

    if ( !pSource )
        return false;
    const auto pSession = pTempSource->GetTwainSession();
    if ( !IsValidTwainSession( pSession) )
        return false;
    if ( !pTempSource->IsOpened() )
        return false;
    return true;
}


bool CTL_TwainAppMgr::GetMultipleIntValues( const CTL_ITwainSource *pSource, CTL_IntArray & pArray, CTL_CapabilityGetTriplet *pTrip)
{
    return GetMultipleValues<CTL_IntArray, TW_UINT16>(pSource, pArray, pTrip);
}


bool CTL_TwainAppMgr::GetMultipleRealValues( const CTL_ITwainSource *pSource,CTL_RealArray & pArray,CTL_CapabilityGetTriplet *pTrip)
{
    return GetMultipleValues<CTL_RealArray, float>(pSource, pArray, pTrip);
}


bool CTL_TwainAppMgr::GetOneIntValue(const CTL_ITwainSource *pSource,
                                     TW_UINT16* pInt,
                                     CTL_CapabilityGetTriplet *pTrip)
{
    CTL_IntArray Array;
    if ( !GetMultipleIntValues( pSource, Array, pTrip ) )
        return false;
    if ( !Array.empty() )
    {
        *pInt = static_cast<TW_UINT16>(Array[0]);
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
bool CTL_TwainAppMgr::GetOneTwainCapValue( const CTL_ITwainSource *pSource,
                                           void *pValue,
                                           TW_UINT16 Cap,
                                           TW_UINT16 GetType,
                                           TW_UINT16 nDataType )
{
    auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
    // Get the #transfer count
    auto pSession = pTempSource->GetTwainSession();
    CTL_CapabilityGetOneValTriplet GetOne( pSession,
                                           pTempSource,
                                           GetType,
                                           Cap,
                                           nDataType);
    if ( !IsSourceOpen( pSource ) )
        return false;

    const TW_UINT16   rc = GetOne.Execute();
    switch (rc)
    {
        case TWRC_SUCCESS:
        {
            GetOne.GetValue(pValue, 0);
            return true;
        }
        break;
        case TWRC_FAILURE:
            SendTwainMsgToWindow(pSession, nullptr, TWRC_FAILURE, GetLastConditionCodeError());
            return false;
        break;
    }
    return false;
}
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////// Mandatory capabilities //////////////////////////
///////////////////////// Transfer Count ///////////////////////////////////////
int CTL_TwainAppMgr::GetTransferCount( const CTL_ITwainSource *pSource )
{
    TW_UINT16 nValue;
    GetOneTwainCapValue( pSource, &nValue, CAP_XFERCOUNT, MSG_GET, TWTY_UINT16 );
    return nValue;
}

int CTL_TwainAppMgr::SetTransferCount( const CTL_ITwainSource *pSource,
                                       int nCount )
{
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS )
    {
        StringStreamA strm;
        strm << boost::format("Setting Transfer Count.  Transfer Count = %1%") % nCount;
        LogWriterUtils::WriteLogInfoIndentedA(strm.str());
    }

    // If the device supports the CAP_SHEETCOUNT capability, use that to set the number of
    // pages to acquire
    if (IsCapabilitySupported(pSource, CAP_SHEETCOUNT))
    {
        SetOneCapValue(pSource, CAP_XFERCOUNT, MSG_SET, -1, TWTY_INT16);

        if ( nCount == -1 )
            SetOneCapValue(pSource, CAP_SHEETCOUNT, MSG_SET, 0, TWTY_UINT32);
        else
            SetOneCapValue(pSource, CAP_SHEETCOUNT, MSG_SET,  nCount, TWTY_UINT32);
    }
    else
    {
        // If we are in duplex mode, we need to set the transfer count to 2 * the number
        // of pages, since each page will use two transfers
        LONG isDuplex = 0;
        GetOneTwainCapValue(pSource, &isDuplex, CAP_DUPLEXENABLED, MSG_GETCURRENT, TWTY_BOOL);
        if (isDuplex == 1 && nCount != -1)
        {
            if (pSource->IsDoublePageCountOnDuplex())
                nCount *= 2; // double the number of images that may be received
            SetOneCapValue( pSource, CAP_XFERCOUNT, MSG_SET, nCount, TWTY_INT16);
        }
    }
    return nCount;
}


TW_UINT16 CTL_TwainAppMgr::ProcessReturnCodeOneValue(CTL_ITwainSource *pSource, TW_UINT16 rc)
{
    auto pSession = pSource->GetTwainSession();
    switch (rc)
    {
        case TWRC_SUCCESS:
            return 1;

        case TWRC_FAILURE:
            SendTwainMsgToWindow(pSession, nullptr, TWRC_FAILURE, GetLastConditionCodeError());
            return 0;
          break;
    }
    return 0;
}

////////////////////////// Transfer mechanisms ///////////////////////////////////
int CTL_TwainAppMgr::SetTransferMechanism( const CTL_ITwainSource *pSource,CTL_TwainAcquireEnum AcquireType,LONG ClipboardTransferType)
{
    // Set the transfer mechanism
    // Change AcquireType to TWAIN type
    TW_UINT16 uTwainType = static_cast<TW_UINT16>(AcquireType);
    if ( AcquireType == TWAINAcquireType_FileUsingNative )
        uTwainType = TWSX_NATIVE;
    else
    if ( AcquireType == TWAINAcquireType_Clipboard)
        uTwainType = static_cast<TW_UINT16>(ClipboardTransferType);
    else
    if ( AcquireType == TWAINAcquireType_File)
        uTwainType = TWSX_FILE;
    else
    if ( AcquireType == TWAINAcquireType_MemFile)
        uTwainType = TWSX_MEMFILE;

    if (AcquireType != TWAINAcquireType_AudioNative)
        SetOneCapValue(pSource, ICAP_XFERMECH, MSG_SET, uTwainType, TWTY_UINT16);
    else
        SetOneCapValue(pSource, ACAP_XFERMECH, MSG_SET, uTwainType, TWTY_UINT16);
    return 1;
}

CTL_IntArray CTL_TwainAppMgr::EnumTransferMechanisms( const CTL_ITwainSource *pSource)
{
    CTL_IntArray rArray;
    GetMultiValuesImpl<CTL_IntArray, TW_UINT16>::GetMultipleTwainCapValues(pSource, rArray, ICAP_XFERMECH, TWTY_UINT16);
    return rArray;
}

std::vector<TW_UINT32> CTL_TwainAppMgr::EnumSupportedDATS(const CTL_ITwainSource* pSource)
{
    std::vector<TW_UINT32> rArray;
    GetMultiValuesImpl<std::vector<TW_UINT32>, TW_UINT32>::GetMultipleTwainCapValues(pSource, rArray, CAP_SUPPORTEDDATS, TWTY_UINT32);
    return rArray;
}

//////////////////////////////////////////////////////////////////////

////////////////////// Pixel and Bit Depth settings /////////////////
void CTL_TwainAppMgr::SetPixelAndBitDepth(const CTL_ITwainSource * /*pSource*/)
{}

///////////////////////////////////////////////////////////////////////
CTL_TwainUnitEnum CTL_TwainAppMgr::GetCurrentUnitMeasure(const CTL_ITwainSource *pSource)
{
    TW_INT16 nValue;
    if ( !GetOneTwainCapValue(pSource, &nValue, ICAP_UNITS, MSG_GETCURRENT, TWTY_UINT16) )
    {
        return TwainUnit_INCHES;
    }
    return static_cast<CTL_TwainUnitEnum>(nValue);
}
//////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////
void CTL_TwainAppMgr::GetCompressionTypes( const CTL_ITwainSource *pSource, CTL_IntArray & rArray )
{
    GetMultiValuesImpl<CTL_IntArray, TW_UINT16>::GetMultipleTwainCapValues(pSource, rArray, ICAP_COMPRESSION,TWTY_UINT16);
}

void CTL_TwainAppMgr::GetUnitTypes( const CTL_ITwainSource *pSource, CTL_IntArray & rArray )
{
    GetMultiValuesImpl<CTL_IntArray, TW_UINT16>::GetMultipleTwainCapValues(pSource, rArray, ICAP_UNITS,TWTY_UINT16);
}

/////////////////////// End mandatory capabilities /////////////////////////

////////////////// Capabilities that should be supported ///////////////////
/*CAP_XFERCOUNT
Every Source must support DG_CONTROL / DAT_CAPABILITY MSG_GET on:
CAP_SUPPORTEDCAPS
CAP_UICONTROLLABLE
Sources that supply image information must support DG_CONTROL / DAT_CAPABILITY /
MSG_GET, MSG_GETCURRENT, MSG_GETDEFAULT on:
ICAP_COMPRESSION
ICAP_PLANARCHUNKY
ICAP_PHYSICALHEIGHT
ICAP_PHYSICALWIDTH
ICAP_PIXELFLAVOR
Sources that supply image information must support DG_CONTROL / DAT_CAPABILITY /
MSG_GET, MSG_GETCURRENT, MSG_GETDEFAULT, MSG_RESET and MSG_SET on:
ICAP_BITDEPTH
ICAP_BITORDER
ICAP_PIXELTYPE
ICAP_UNITS
ICAP_XFERMECH
ICAP_XRESOLUTION
ICAP_YRESOLUTION
  */
using MandatorySet = std::set<TW_UINT16>;

void CTL_TwainAppMgr::GetCapabilities(const CTL_ITwainSource *pSource, CTL_TwainCapArray & rArray)
{
    // Get all the capabilities of the source
    rArray.clear();
    GetMultiValuesImpl<CTL_TwainCapArray, TW_UINT16>::GetMultipleTwainCapValues(pSource, rArray, CAP_SUPPORTEDCAPS, TWTY_UINT16, TwainContainer_ARRAY);
}

void CTL_TwainAppMgr::GetExtendedCapabilities(const CTL_ITwainSource *pSource, CTL_IntArray & rArray)
{
    // Get the extended capabilities of the source
    rArray.clear();
    GetMultiValuesImpl<CTL_IntArray, TW_UINT16>::GetMultipleTwainCapValues(pSource, rArray, CAP_EXTENDEDCAPS, TWTY_UINT16, TwainContainer_ARRAY);
}

UINT CTL_TwainAppMgr::GetCapOps(const CTL_ITwainSource *pSource, int nCap, bool bCanQuery)
{
    UINT nOps = 0;
    if ( bCanQuery )
        nOps = GetCapabilityOperations(pSource, nCap).GetSupport();

    if ( nOps == 0 )
    {
        const UINT nContainer = GetContainerTypesFromCap( static_cast<TW_UINT16>(nCap), 1 );
        nOps = 0xFFFF;
        if ( !nContainer )
            nOps = 0xFFFF & ~(TWQC_SET | TWQC_RESET);
    }
    return nOps;
}

CTL_CapabilityQueryTriplet CTL_TwainAppMgr::GetCapabilityOperations(const CTL_ITwainSource *pSource, int nCap)
{
    const auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
    if (!pSource)
        return {nullptr, nullptr, 0};

    const auto pSession = pTempSource->GetTwainSession();

    if (!IsValidTwainSession(pSession))
        return { nullptr, nullptr, 0 };

    if (!s_pGlobalAppMgr->IsSourceOpen(pSource))
        return { nullptr, nullptr, 0 };

    CTL_CapabilityQueryTriplet QT(pSession, pTempSource, static_cast<TW_UINT16>(nCap));
    const TW_UINT16 rc = QT.Execute();
    if (rc != TWRC_SUCCESS)
        return { nullptr, nullptr, 0 };
    return QT;
}

////////////////// End Capabilities that should be supported /////////////////

/////////////// Capabilities that do not have to be supported ////////////////
bool CTL_TwainAppMgr::IsFeederLoaded( const CTL_ITwainSource *pSource )
{
    TW_UINT16 nValue;
    GetOneTwainCapValue( pSource, &nValue, CAP_FEEDERLOADED, MSG_GET, TWTY_BOOL);
    return nValue?true:false;
}


bool CTL_TwainAppMgr::IsFeederEnabled( const CTL_ITwainSource *pSource, TW_UINT16& nValue )
{
    if (!GetOneTwainCapValue( pSource, &nValue, CAP_FEEDERENABLED, MSG_GET, TWTY_BOOL))
        return false;
    return true;
}

bool CTL_TwainAppMgr::IsJobControlSupported( const CTL_ITwainSource *pSource, TW_UINT16& nValue )
{
    if (!GetOneTwainCapValue( pSource, &nValue, CAP_JOBCONTROL, MSG_GET, TWTY_UINT16 ))
        return false;
    return true;
}

bool CTL_TwainAppMgr::SetupFeeder( const CTL_ITwainSource *pSource, int /*maxpages*/, bool bSet )
{
    if ( !pSource->IsFeederEnabledMode())
        return false;

    // Determine if there is a document feeder capability
    TW_UINT16 nValue;
    if ( !IsFeederEnabled(pSource, nValue))
        return false;

    TW_BOOL bValue;

    // Check if it needs to be turned off via source or implicitly
    const bool bTurnOffAutoFeed = !pSource->GetAutoFeedMode();

    if ( bSet == false || bTurnOffAutoFeed )
    {
        // Turn off autofeed
        // Enable the CAP_AUTOFEED capability
        // Get a set capability triplet compatible for one value
        bValue = false;
        SetOneCapValue( pSource, CAP_AUTOFEED, MSG_SET, bValue,  TWTY_BOOL );

        // Return, since the autofeed has been turned off and the feeder has been
        // disabled
        if ( bSet == false )
            return true;
    }

    // Set the automatic document feeder mode if present
    nValue = 1;
    SetOneCapValue( pSource, CAP_FEEDERENABLED, MSG_SET, nValue, TWTY_BOOL);

    // Enable the CAP_AUTOFEED capability if the user wants to automatically feed
    // the page
    // Get a set capability triplet compatible for one value
    if ( !bTurnOffAutoFeed)
    {
        bValue = true;
        SetOneCapValue( pSource, CAP_AUTOFEED, MSG_SET, bValue, TWTY_BOOL);
    }

    return true;
}

int CTL_TwainAppMgr::FindConditionCode(TW_UINT16 nCode)
{
    const auto it = dynarithmic::generic_array_finder_if(mapCondCode, [&](const auto& pr) { return pr.first == nCode; });
    if (!it.first)
        return INVALID_CONDITION_CODE;
    return mapCondCode[it.second].second;
}

std::string CTL_TwainAppMgr::GetCapNameFromCap( LONG Cap )
{
    if ( static_cast<UINT>(Cap) >= CAP_CUSTOMBASE )
    {
        StringStreamOutA strm;
        strm << boost::format("CAP_CUSTOMBASE + %1%") % (static_cast<long>(Cap) - static_cast<long>(CAP_CUSTOMBASE));
        return strm.str();
    }
    else
    {
        static constexpr std::array<int, 3> aConstantTypes = { {DTWAIN_CONSTANT_ICAP, DTWAIN_CONSTANT_CAP, DTWAIN_CONSTANT_TWEI} };
        for (auto constantType : aConstantTypes)
        {
            auto pr = CTL_StaticData::GetTwainNameFromConstantA(constantType, Cap);
            if (pr.first)
                return pr.second;
        }
    }
    StringStreamOutA strm;
    strm << std::hex << Cap;
    return "Unknown capability.  Hex value: " + strm.str();
}

int CTL_TwainAppMgr::GetDataTypeFromCap( TW_UINT16 Cap, CTL_ITwainSource *pSource/*=NULL*/ )
{
    const auto nThisCap = static_cast<TW_UINT16>(Cap);
    if (nThisCap >= CAP_CUSTOMBASE)
    {
        if (!pSource)
            return DTWAIN_FAILURE1;
        return GetCustomCapDataType(pSource, nThisCap);
    }
    const CTL_CapStruct cStruct = GetGeneralCapInfo(Cap);
    if ( static_cast<std::string>(cStruct).length() == 0 )
        return (std::numeric_limits<int>::min)();
    return static_cast<int>(cStruct.m_nDataType);
}

CTL_CapStruct CTL_TwainAppMgr::GetGeneralCapInfo(LONG Cap)
{
    if ( Cap >= CAP_CUSTOMBASE )
        Cap = CAP_CUSTOMBASE;

    CTL_CapStruct cStruct;
    bool bFoundCap = false;
    auto& generalInfo = CTL_StaticData::GetGeneralCapInfo();
    const auto it = generalInfo.find( static_cast<short>(Cap) );
    if ( it != generalInfo.end() )
    {
        bFoundCap = true;
        cStruct = (*it).second;
    }

    if ( !bFoundCap )
    {
        cStruct = generalInfo[static_cast<short>(Cap)];
    }
    return cStruct;
}

LONG CTL_TwainAppMgr::GetCapFromCapName(const char* szCapName)
{
    std::string strCap = szCapName;
    StringWrapperA::TrimAll(strCap);
    StringWrapperA::MakeUpperCase(strCap);
    if (strCap.empty())
        return TwainCap_INVALID;

    // Check if the cap name is CAP_CUSTOMBASE
    if (StringWrapperA::StartsWith(strCap, "CAP_CUSTOMBASE"))
    {
        // Extract the integer portion
        StringArray sArray;
        StringWrapperA::Tokenize(StringWrapperA::Mid(strCap, 14), "+ ", sArray);
        const size_t nSize = sArray.size();
        if (nSize > 0)
        {
            std::string sNum = sArray[nSize - 1];
            int nNum = 0;
            try
            {
                nNum = stoi(sNum);
            }
            catch (const std::invalid_argument& /*ex*/)
            {
                return TwainCap_INVALID;
            }
            catch (const std::out_of_range& /*ex*/)
            {
                return TwainCap_INVALID;
            }
            return CAP_CUSTOMBASE + nNum;
        }
    }

    // Handle TWEI_, ICAP_, or CAP_ names
    static constexpr std::array<std::string_view, 3> startPrefix = { {"TWEI_","ICAP_","CAP_"} };
    size_t count = 0;
    for (; count < startPrefix.size(); ++count)
    {
        if (StringWrapperA::StartsWith(strCap, startPrefix[count].data()))
        {
            // Get the id, given the TWAIN name
            auto retVal = CTL_StaticData::GetIDFromTwainName(strCap);
            if (retVal.first)
                return static_cast<LONG>(retVal.second);
            else
                return TwainCap_INVALID;
        }
    }
    return TwainCap_INVALID;
}

UINT CTL_TwainAppMgr::GetContainerTypesFromCap( TW_UINT16 Cap, bool nType )
{
    const CTL_CapStruct cStruct = GetGeneralCapInfo(Cap);

    if ( static_cast<std::string>(cStruct).empty())
        return 0;

    if ( !nType )
        return cStruct.m_nGetContainer;
    return cStruct.m_nSetContainer;
}

CTL_ErrorStruct CTL_TwainAppMgr::GetGeneralErrorInfo(TW_UINT32 nDG, TW_UINT16 nDAT, TW_UINT16 nMSG)
{
    CTL_ErrorStruct eStruct;
    auto& errorInfoMap = CTL_StaticData::GetGeneralErrorInfoMap();
    const auto it = errorInfoMap.find(std::make_tuple(nDG, nDAT, nMSG));
    if ( it != errorInfoMap.end() )
        eStruct = (*it).second;
    return eStruct;
}

bool CTL_TwainAppMgr::IsSourceCompliant( const CTL_ITwainSource *pSource,
                                         CTL_EnumTwainVersion TVersion,
                                         CTL_TwainCapArray& rArray )
{
#if 0
    if ( !s_pGlobalAppMgr )
        return false;

    if ( !s_pGlobalAppMgr->IsSourceOpen( pSource ))
        return false;

    return pSource->IsSourceCompliant( TVersion, rArray );
#else
    return true;
#endif
}

#ifdef _WIN32
#include "winget_twain.inl"
#else
#include "linuxget_twain.inl"
#endif

CTL_StringType CTL_TwainAppMgr::GetTwainDirFullName(LPCTSTR szTwainDLLName,
                                                    LPLONG pWhichSearch,
                                                    bool bLeaveLoaded/*=false*/,
                                                    boost::dll::shared_library *pModule)
{
    return ::GetTwainDirFullName(szTwainDLLName, pWhichSearch, bLeaveLoaded, pModule);
}

CTL_StringType CTL_TwainAppMgr::GetTwainDirFullNameEx(LPCTSTR szTwainDLLName,
                                                      LPLONG pWhichSearch,
                                                      bool bLeaveLoaded/*=false*/,
                                                      boost::dll::shared_library *pModule)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    return ::GetTwainDirFullNameEx(pHandle, szTwainDLLName, pWhichSearch, bLeaveLoaded, pModule);
}

std::pair<bool, CTL_StringType> CTL_TwainAppMgr::CheckTwainExistence(CTL_StringType strTwainDLLName, LPLONG pWhichSearch)
{
    auto str = GetTwainDirFullName(strTwainDLLName.c_str(), pWhichSearch);
    if ( str.empty())
        return { false, str };
    return { true, str };
}

/////////////////////////****************//////////////////////////////////
/////////////// member functions for the CTL_TwainAppMgr///////////////////
CTL_TwainAppMgr::CTL_TwainAppMgr(CTL_TwainDLLHandle *pHandle,
                                 LPCTSTR lpszDLLName,
                                 HINSTANCE hInstance,
                                 HINSTANCE /*hThisInstance*/) : m_nErrorTWRC(0), m_nErrorTWCC(0), m_pCurrentTriplet(nullptr), m_pDLLHandle(pHandle)
{
    if ( !lpszDLLName || lpszDLLName[0] == '\0' )
       m_strTwainDSMPath = GetDefaultDLLName();
    else
       m_strTwainDSMPath = lpszDLLName;

    #ifdef _WIN32
    // Register a twain App message
    m_nTwainMsg = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
    #endif

    // Record the instance
    m_Instance = hInstance;
    m_lpDSMEntry = nullptr;
}

void CTL_TwainAppMgr::OpenLogFile(LPCSTR lpszFile)
{
}


void CTL_TwainAppMgr::WriteToLogFile(int /*rc*/)
{
}

void CTL_TwainAppMgr::CloseLogFile()
{
}

void CTL_TwainAppMgr::DestroyAllTwainSessions()
{
    std::for_each(m_arrTwainSession.begin(), m_arrTwainSession.end(), CTL_ITwainSession::Destroy);
    m_arrTwainSession.clear();
}


void CTL_TwainAppMgr::DestroySession(const CTL_ITwainSession* pSession)
{
    const auto iter = FindSession(pSession);
    if ( iter != s_pGlobalAppMgr->m_arrTwainSession.end() )
    {
        CTL_ITwainSession::Destroy( *iter );
        m_arrTwainSession.erase( iter );
    }
}

CTL_StringType CTL_TwainAppMgr::GetDefaultDLLName()
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( pHandle )
    {
        if ( pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_LATESTVERSION )
            pHandle->m_SessionStruct.DSMName = GetLatestDSMVersion();
        return pHandle->m_SessionStruct.DSMName;
    }
    return TWAINDLLVERSION_1;
}

CTL_StringType CTL_TwainAppMgr::GetLatestDSMVersion()
{
    const bool bRet1 = CheckTwainExistence(TWAINDLLVERSION_1).first;
    const bool bRet2 = CheckTwainExistence(TWAINDLLVERSION_2).first;

    if ( bRet2 )
        return TWAINDLLVERSION_2;
    if ( bRet1 )
        return TWAINDLLVERSION_1;
    return {};
}

template <typename ErrorCodeType>
static int LoadSourceManagerImpl(boost::dll::shared_library& libloader, const CTL_StringType& fNameTotal)
{
    ErrorCodeType ec;
    libloader.load(fNameTotal, ec, boost::dll::load_mode::search_system_folders);
    return ec.value();
}

bool CTL_TwainAppMgr::LoadSourceManager( LPCTSTR pszDLLName )
{
    constexpr int boost_version_minor = (BOOST_VERSION / 100) % 1000;
    constexpr int boost_version_major = BOOST_VERSION / 100000;
    if ( pszDLLName != nullptr && pszDLLName[0] )
    {
        // This is a custom path, so user knows what they're doing.
 
        m_strTwainDSMPath = pszDLLName;
        // Attempt to load TWAIN DLL
        boost::dll::shared_library libloader;
        int loadReturnCode = 0;
        if constexpr (boost_version_major == 1 && boost_version_minor >= 88)
        {
            // Boost has changed return code status for version 1.88 and higher
            loadReturnCode = LoadSourceManagerImpl<std::error_code>(libloader, m_strTwainDSMPath);
        }
        else
        {
            // Use boost 1.87 and below compatible code
            loadReturnCode = LoadSourceManagerImpl<boost::system::error_code>(libloader, m_strTwainDSMPath);
        }

        if ( loadReturnCode != boost::system::errc::success)
        {
            const CTL_StringType dllName = _T(" : ") + m_strTwainDSMPath;
            DTWAIN_ERROR_CONDITION_EX(IDS_ErrTwainDLLNotFound, StringConversion::Convert_Native_To_Ansi(dllName, dllName.length()), false, true)
        }

        // Attempt to load the DSM_Entry point
        bool bLoadedDSM = LoadDSM();
        if (!bLoadedDSM)
        {
            // This DLL cannot be used to communicate with TWAIN due to the lack of a DSM entry point
            libloader.unload();
            return false;
        }
        m_hLibModule = libloader;
    }
    else
    {
        // load the default TWAIN_32.DLL or TWAINDSM.DLL using the
        // normal process of finding these DLL's
        const auto& tempStr = m_strTwainDSMPath;
        m_strTwainDSMPath = GetTwainDirFullName(m_strTwainDSMPath.c_str(), &m_nTwainDSMFoundPath, true, &m_hLibModule);
        if ( m_strTwainDSMPath.empty() )
        {
            m_strTwainDSMPath = tempStr;
            m_strTwainDSMPath = GetTwainDirFullNameEx(m_strTwainDSMPath.c_str(), &m_nTwainDSMFoundPath, true, &m_hLibModule);
            if ( m_strTwainDSMPath.empty())
            {
                const CTL_StringType dllName = _T(" : ") + tempStr;
                DTWAIN_ERROR_CONDITION_EX(IDS_ErrTwainDLLNotFound, StringConversion::Convert_Native_To_Ansi(dllName, dllName.length()), false, true)
            }
        }
        m_strTwainDSMVersionInfo = dynarithmic::GetVersionInfo(m_hLibModule.native(), 0);
        CTL_StringStreamType strm;
        strm << _T("TWAIN DSM \"") + m_strTwainDSMPath + _T("\" is found and will be used for this TWAIN session...\n");
        strm << _T("Version information for \"") << m_strTwainDSMPath << _T("\":\n") << dynarithmic::GetVersionInfo(m_hLibModule.native(), 4);
        LogToDebugMonitor(strm.str());
        if (CTL_StaticData::GetLogFilterFlags() != 0)
            DTWAIN_LogMessageA(StringConversion::Convert_Native_To_Ansi(strm.str()).c_str());

        // Load the entry point for these DLL's
        LoadDSM();
    }
    return true;
}

bool CTL_TwainAppMgr::LoadDSM()
{
    CTL_TwainDLLHandle* pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    m_lpDSMEntry = dtwain_library_loader<DSMENTRYPROC>::get_func_ptr(m_hLibModule.native(), "DSM_Entry");
    if ( !m_lpDSMEntry )
        DTWAIN_ERROR_CONDITION(IDS_ErrTwainDLLInvalid,false, true)
    pHandle->m_nDSMState = DSM_STATE_LOADED;
    return true;   // return success
}

TW_UINT16 CTL_TwainAppMgr::CallDSMEntryProc( TW_IDENTITY *pOrigin, TW_IDENTITY* pDest,
                                             TW_UINT32 dg, TW_UINT16 dat, TW_UINT16 msg,
                                             TW_MEMREF pMemref)
{
    return s_pGlobalAppMgr->CallDSMEntryProc( CTL_TwainTriplet(pOrigin, pDest, dg, dat, msg, pMemref) );
}

void CTL_TwainAppMgr::GatherCapabilityInfo(CTL_ITwainSource* pSource)
{
    const auto pHandle = pSource->GetDTWAINHandle();
    if (!pSource->RetrievedAllCaps())
    {
        // Get the capabilities using TWAIN
        CTL_TwainCapArray rArray;
        CTL_TwainAppMgr::GetCapabilities(pSource, rArray);

        // Report TWAIN compliance issue if retrieving capabilities returns no values
        bool logErrors = (CTL_StaticData::GetLogFilterFlags());
        if ( rArray.empty() && logErrors)
        {
            std::string s1 = GetResourceStringFromMap(DTWAIN_ERR_SUPPORTEDCAPS_COMPLIANCY1);
            s1 += " - " + StringWrapperA::QuoteString(pSource->GetProductNameA());
            LogWriterUtils::WriteLogInfoIndentedA(s1);
        }
        if (!rArray.empty() && logErrors)
        {
            static constexpr std::array<TW_UINT16, 3> mandatoryCaps = { CAP_SUPPORTEDCAPS, ICAP_XFERMECH, ICAP_PIXELTYPE };
            bool bOk = true;
            for (auto cap : mandatoryCaps)
                bOk = bOk && (std::find(rArray.begin(), rArray.end(), cap) != rArray.end());
            if (!bOk)
            {
                std::string s1 = GetResourceStringFromMap(DTWAIN_ERR_SUPPORTEDCAPS_COMPLIANCY2);
                s1 += " - " + StringWrapperA::QuoteString(pSource->GetProductNameA());
                LogWriterUtils::WriteLogInfoIndentedA(s1);
            }
        }
        pSource->SetCapSupportedList(rArray);

        // Get the capabilities from the list in the Source
        CapList& pArray = pSource->GetCapSupportedList();

        // Get all the information about the capability.
        std::for_each(pArray.begin(), pArray.end(), [&](TW_UINT16 val)
        {
            DTWAIN_CacheCapabilityInfo(pSource, pHandle, static_cast<TW_UINT16>(val));
        });

        // Retrieve any custom caps
        auto& customCapSet = pSource->GetCustomCapCache();
        customCapSet.clear();
        for (auto& capInfo : pArray)
        {
            if (capInfo >= CAP_CUSTOMBASE)
                customCapSet.insert(capInfo);
        }

        // We have retrieved all the capability information
        pSource->SetRetrievedAllCaps(true);
    }
}

struct TripletSaveRestore
{
    const CTL_TwainTriplet **pTrip = nullptr;
    TripletSaveRestore(const CTL_TwainTriplet** p) : pTrip(p) {}
    ~TripletSaveRestore()
    {
        if ( pTrip && *pTrip )
            *pTrip = nullptr;
    }
};

TW_UINT16 CTL_TwainAppMgr::CallDSMEntryProc( const CTL_TwainTriplet & pTriplet )
{
    if (!m_lpDSMEntry)
        return 0;

    // This runs on exit to ensure a condition code is always produced for each TWAIN DSM call
    struct ConditionCodeRAII
    {
        TW_IDENTITY* m_pSession = nullptr;
        TW_IDENTITY* m_pIdentity = nullptr;
        bool m_bRunConditionCode = true;
        ConditionCodeRAII(TW_IDENTITY* pSession, TW_IDENTITY* pIdentity, bool bRunConditionCode) : 
                            m_pSession(pSession), m_pIdentity(pIdentity), m_bRunConditionCode(bRunConditionCode) {}
        ~ConditionCodeRAII()
        {
            if (m_bRunConditionCode)
            {
                CTL_ConditionCodeTriplet CC(m_pSession, m_pIdentity);
                CC.Execute();
                ProcessConditionCodeError(CC.GetConditionCode());
            }
        }
    };

    TW_UINT16 retcode = TWRC_SUCCESS;

    CTL_ErrorStruct e;
    std::string s;

    pTW_IDENTITY pOrigin = pTriplet.GetOriginID();
    pTW_IDENTITY pDest   = pTriplet.GetDestinationID();
    TW_UINT32    nDG     = pTriplet.GetDG();
    TW_UINT16    nDAT    = pTriplet.GetDAT();
    TW_UINT16    nMSG    = pTriplet.GetMSG();
    TW_MEMREF    pData   = pTriplet.GetMemRef();

    bool isConditionCode = (nDG == DG_CONTROL && nDAT == DAT_STATUS && nMSG == MSG_GET);

    // To avoid infinite loop, only run the exit condition code triplet for 
    // triplets that are not DG_CONTROL / DAT_STATUS / MSG_GET
    ConditionCodeRAII raii(pOrigin, pDest, !isConditionCode);

    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_LOWLEVELTWAIN)
    {
        e = GetGeneralErrorInfo(nDG, nDAT, nMSG);
        s = e.GetIdentityAndDataInfo(pOrigin, pDest, pData);
        s = GetResourceStringFromMap(IDS_LOGMSG_INPUTTEXT) + ": " + s;
        LogWriterUtils::WriteMultiLineInfoIndentedA(s, "\n");
    }

    TripletSaveRestore tSaveRestore(&m_pCurrentTriplet);
    m_pCurrentTriplet = &pTriplet;

    if ( m_pDLLHandle->m_bNotifyTripletsUsed)
    {
        // Send out that we have begun to process the TWAIN triplet
        SendTwainMsgToWindow(pTriplet.GetSessionPtr(), nullptr, DTWAIN_TN_TWAINTRIPLETBEGIN, 0);
    }

    try
    {
        // This is the actual low-level call to the TWAIN Data Source Manager 
        // (TWAIN_32.DLL or TWAINDSM.DLL)
        retcode = (*m_lpDSMEntry)( pOrigin, pDest, nDG, nDAT, nMSG, pData );
    }
    catch(...)
    {
        if (m_pDLLHandle->m_bNotificationsUsed)
        {
            // Send out that we have ended processing the TWAIN triplet
            SendTwainMsgToWindow(pTriplet.GetSessionPtr(), nullptr, DTWAIN_TN_TWAINTRIPLETEND, 0);
        }
        // An exception occurred.  This is bad!
        // Check what to do when this happens (possibly close DSM and start over?)
        // To do later...
        retcode = -DTWAIN_ERR_EXCEPTION_ERROR;
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_LOWLEVELTWAIN)
        {
            std::string sz;
            std::ostringstream strm;
            sz = e.GetTWAINDSMErrorCC(IDS_TWCC_EXCEPTION);
            s = e.GetIdentityAndDataInfo(pOrigin, pDest, pData);
            strm << boost::format("%1%=%2% (%3%)\n%4%") % GetResourceStringFromMap(IDS_LOGMSG_OUTPUTDSMTEXT) % retcode % sz % s;
            LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
        }
        return retcode;
    }
    if (m_pDLLHandle->m_bNotifyTripletsUsed)
    {
        // Send out that we have ended processing the TWAIN triplet
        SendTwainMsgToWindow(pTriplet.GetSessionPtr(), nullptr, DTWAIN_TN_TWAINTRIPLETEND, 0);
    }
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_LOWLEVELTWAIN)
    {
        std::string sz;
        std::ostringstream strm;
        s =  e.GetIdentityAndDataInfo(pOrigin, pDest, pData);
        sz = e.GetTWAINDSMError(retcode);
        std::string s1 = GetResourceStringFromMap(IDS_LOGMSG_OUTPUTDSMTEXT);
        boost::format fmt("%1%=%2% (%3%)\n%4%\n");
        strm << fmt % s1.c_str() % retcode % sz % s;
        LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
    }
    if (retcode == TWRC_FAILURE)
    {
        SetLastTwainError(retcode, TWRC_Error);
        if (nDG == DG_CONTROL && nDAT == DAT_EVENT && nMSG == MSG_PROCESSEVENT)
            raii.m_bRunConditionCode = false;
    }
    else
        raii.m_bRunConditionCode = false;
    return retcode;
}

void CTL_TwainAppMgr::SetLastTwainError( TW_UINT16 nError,
                                         int nErrorType )
{
    if ( nErrorType == TWRC_Error )
        m_nErrorTWRC = nError;
    else
        m_nErrorTWCC = nError;
}

int CTL_TwainAppMgr::GetLastTwainError()
{
    if (s_pGlobalAppMgr)
        return s_pGlobalAppMgr->m_nErrorTWRC;
    return TWRC_SUCCESS;
}

int CTL_TwainAppMgr::GetLastConditionCodeError()
{
    if (s_pGlobalAppMgr)
        return s_pGlobalAppMgr->m_nErrorTWCC;
    return TWCC_SUCCESS;
}

bool CTL_TwainAppMgr::SetDefaultSource( CTL_ITwainSession *pSession, const CTL_ITwainSource *pSource )
{
    const auto pTemp = const_cast<CTL_ITwainSource*>(pSource);
    CTL_SetDefaultSourceTriplet Trip(pSession, pTemp);
    const TW_UINT16 rc = Trip.Execute();
    if ( rc != TWRC_SUCCESS )
    {
        auto ccode = GetLastConditionCodeError();
        if (ccode != TWCC_SUCCESS)
            return false;
    }
    return true;
}

VOID CALLBACK CTL_TwainAppMgr::TwainTimeOutProc(HWND, UINT, ULONG, DWORD)
{
#if 0
#ifdef _WIN32
    KillTimer(nullptr, CTL_StaticData::GetTimeoutID());

    LogWriterUtils::WriteLogInfoIndentedA("The last TWAIN triplet was not completed due to time out");
    SetError(DTWAIN_ERR_TIMEOUT, "", false);
    throw DTWAINException(DTWAIN_ERR_TIMEOUT);
#endif
#endif
}

CTL_StringType CTL_TwainAppMgr::GetDSMPath()
{
    const auto mgr = GetInstance();
    if ( mgr )
        return mgr->m_strTwainDSMPath;
    return {};
}

CTL_StringType CTL_TwainAppMgr::GetDSMVersionInfo()
{
    const auto mgr = GetInstance();
    if (mgr)
        return mgr->m_strTwainDSMVersionInfo;
    return {};
}

LONG CTL_TwainAppMgr::GetDSMPathLocation()
{
    const auto mgr = GetInstance();
    if (mgr)
        return mgr->m_nTwainDSMFoundPath;
    return -1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// Global app pointer
CTL_TwainAppMgrPtr CTL_TwainAppMgr::s_pGlobalAppMgr;
// Initialize the m_AppID structure
TW_IDENTITY CTL_TwainAppMgr::s_AppId = {};
CTL_ITwainSession* CTL_TwainAppMgr::s_pSelectedSession = nullptr;
int          CTL_TwainAppMgr::s_nLastError = 0;
std::string  CTL_TwainAppMgr::s_strLastError;
HINSTANCE    CTL_TwainAppMgr::s_ThisInstance = static_cast<HINSTANCE>(nullptr);
SourceToXferReadyMap CTL_TwainAppMgr::s_SourceToXferReadyMap;
SourceToXferReadyList CTL_TwainAppMgr::s_SourceToXferReadyList;
SourceFlatbedOnlyList CTL_TwainAppMgr::s_SourceFlatbedOnlyList;
SourcePeekMessageList CTL_TwainAppMgr::s_SourcePeekMessageList;
SourcePaperDetectableMap CTL_TwainAppMgr::s_SourcePaperDetectableMap;