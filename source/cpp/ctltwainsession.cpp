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
#include <cstring>
#include <algorithm>
#include <utility>
#include "ctltwainsession.h"

#include "ctltr001.h"
#include "ctltwainmanager.h"
#include "ctltwainsource.h"
#include "ctlguidimpl.h"
#include <cppfunc.h>
#include <errorcheck.h>
#include "ctldtwainhandle.h"
#include "ctltwaindllpath.h"
#include "ctltwainlogging.h"
#include "dtwainx.h"
#include "windowsinit_impl.h"

using namespace dynarithmic;
namespace stringutils = basicstringutils;

namespace
{
#ifdef _WIN32
    HWND CreateTwainWindow(CTL_TwainDLLHandle* /*pHandle*/, HINSTANCE hInstance/*=NULL*/, HWND hWndParent)
    {
        if (hInstance == nullptr)
            hInstance = CTL_StaticData::GetDLLInstanceHandle();
        HWND hWndP;
        if (!hWndParent)
            hWndP = GetDesktopWindow();
        else
            hWndP = hWndParent;

        RECT rect;

        GetWindowRect(hWndP, &rect);
        const HWND hwnd = CreateWindow(_T("DTWAINWindowClass"),              // class
            _T("Twain Window"),                 // title
            WS_OVERLAPPED | WS_POPUP | WS_CAPTION | WS_EX_TOOLWINDOW,    // style
            0, 0,   // x, y
            100, 100,   // width, height
            hWndParent ? hWndP : NULL,
            NULL,                            // hmenu
            hInstance,
            NULL);                          // lpvparam
        return hwnd;
    }

    ////////// Function to subclass the window ////////////////////////

    #define     TWSubclassWindow(hwnd, lpfn)  \
          (reinterpret_cast<WNDPROC>(SetWindowLongPtr((hwnd), GWLP_WNDPROC, (LONG_PTR)(WNDPROC)(lpfn))))

    WNDPROC SubclassTwainMsgWindow(HWND hWnd, WNDPROC wProcIn = nullptr)
    {
        WNDPROC wProc = nullptr;
        WNDPROC wProcToUse = nullptr;
        if (wProcIn == nullptr)
            wProcToUse = static_cast<WNDPROC>(DTWAIN_WindowProc);
        else
            wProcToUse = wProcIn;
        if (IsWindow(hWnd))
        {
            if (reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hWnd, GWLP_WNDPROC)) != wProcToUse)
            {
                wProc = TWSubclassWindow(hWnd, wProcToUse);
                if (!wProc)
                    LogWin32Error(GetLastError());
            }
        }
        // Already equal, so return the original
        else
            wProc = wProcToUse;
        return wProc;
    }
#endif
}

//////////////////// CTL_ITwainSession functions /////////////////////////////
CTL_ITwainSession*  CTL_ITwainSession::Create(CTL_TwainDLLHandle *pHandle,
                                            LPCTSTR pAppName,
                                            HWND* hAppWnd)
{
    return new CTL_ITwainSession(pHandle, pAppName, hAppWnd);
}


CTL_ITwainSession::CTL_ITwainSession(CTL_TwainDLLHandle *pHandle,
                                     LPCTSTR pAppName,
                                     HWND* hAppWnd) : 
                                    m_AppId{}, m_AppWnd{}, m_pTwainDLLHandle{}, m_pSelectedSource{}
{
    if ( pAppName )
        m_AppName = pAppName;
    if ( hAppWnd )
        m_AppWnd = *hAppWnd;
    m_bTwainWindowCreated = false;
    SetTwainDLLHandle(pHandle);

    if ( !hAppWnd )
    {
        m_AppWnd = CreateTwainWindow();
        m_bTwainWindowCreated = true;
    }

    TW_IDENTITY m_AppIdTemp = {};
    m_AppIdTemp.Id = 0;
    m_AppIdTemp.Version.MajorNum = pHandle->m_SessionStruct.nMajorNum;
    m_AppIdTemp.Version.MinorNum = pHandle->m_SessionStruct.nMinorNum;
    m_AppIdTemp.Version.Language = static_cast<TW_UINT16>(pHandle->m_SessionStruct.nLanguage);
    m_AppIdTemp.Version.Country  = static_cast<TW_UINT16>(pHandle->m_SessionStruct.nCountry);

    stringutils::SafeStrcpy( m_AppIdTemp.Version.Info,
                                stringconversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szVersion).c_str(),
                                sizeof m_AppIdTemp.Version.Info - 1 );

    m_AppIdTemp.ProtocolMajor =    TWON_PROTOCOLMAJOR;
    m_AppIdTemp.ProtocolMinor =    TWON_PROTOCOLMINOR;
    m_AppIdTemp.SupportedGroups =  DG_IMAGE | DG_CONTROL | DG_AUDIO | DF_APP2 | DF_DSM2 ;

    stringutils::SafeStrcpy( m_AppIdTemp.Manufacturer,  stringconversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szManufact).c_str(), sizeof m_AppIdTemp.Manufacturer - 1 );
    stringutils::SafeStrcpy( m_AppIdTemp.ProductFamily, stringconversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szFamily).c_str(), sizeof m_AppIdTemp.ProductFamily - 1 );
    stringutils::SafeStrcpy( m_AppIdTemp.ProductName,   stringconversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szProduct).c_str(),sizeof m_AppIdTemp.ProductName - 1 );
    m_AppId = m_AppIdTemp;
    m_pSelectedSource = nullptr;
    m_bTwainMessageFlag = false;
    m_bAllSourcesRetrieved = false;
}

CTL_ITwainSource* CTL_ITwainSession::CreateTwainSource( LPCTSTR pProduct )
{
    // check if source with this product name has been selected
    CTL_ITwainSource* pSource = IsSourceSelected(pProduct);
    if (!pSource )
    {
        pSource = CTL_ITwainSource::Create( this, pProduct );
        AddTwainSource(pSource);
    }
    return pSource;
}


HWND CTL_ITwainSession::CreateTwainWindow()
{
#ifdef _WIN32
    return  CreateWindow(_T("STATIC"), // class
                         _T("Twain Window"), // title
                         WS_POPUPWINDOW | WS_VISIBLE, // style
                         CW_USEDEFAULT, CW_USEDEFAULT, // x, y
                         CW_USEDEFAULT, CW_USEDEFAULT, // width, height
                         HWND_DESKTOP, // parent window
                         NULL, // hmenu
                         CTL_TwainAppMgr::GetAppInstance(), // hinst
                         NULL);                          // lpvparam
#else
    return 0;
#endif
}

void CTL_ITwainSession::Destroy( CTL_ITwainSessionPtr& pSession )
{
    pSession.reset();
}


bool CTL_ITwainSession::IsTwainWindowActive() const
{
    return m_bTwainWindowCreated;
}

void CTL_ITwainSession::SetTwainMessageFlag(bool bSet)
{
    m_bTwainMessageFlag = bSet;
}


bool CTL_ITwainSession::AddTwainSource( CTL_ITwainSource *pSource )
{
    const TW_IDENTITY* pId = pSource->GetSourceIDPtr();
    const std::string strProduct = pId->ProductName;

    struct SourceFinder
    {
        std::string m_str;
        SourceFinder(std::string str) : m_str(std::move(str)) {}
        bool operator()(CTL_ITwainSource* ptr) const
            { return ptr->GetSourceIDPtr()->ProductName == m_str; }
    };

    auto& sourceStatusMap = CTL_StaticData::GetSourceStatusMap();
    auto iterFound = std::find_if(m_arrTwainSource.begin(), m_arrTwainSource.end(), SourceFinder(strProduct));
    if ( iterFound == m_arrTwainSource.end())
    {
        auto uid = GenerateUUIDv4Impl<std::string>();
        pSource->GetDTWAINHandle()->GetGUIDMap(GUID_SOURCES).Insert( uid, pSource );
        m_arrTwainSource.insert( pSource );
        auto iter = sourceStatusMap.insert({ pSource->GetProductNameA(), {} }).first;
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, true);
        return true;
    }
    else
    {
        // The source has already been selected, so update the info in the twain source array
        // and destroy the previous instance.  Keep the UUID and status
        CTL_ITwainSource::Destroy(*iterFound);
        m_arrTwainSource.erase(iterFound);
        m_arrTwainSource.insert(pSource);
        return true;
    }
    return false;
}

bool CTL_ITwainSession::IsValidSource(const CTL_ITwainSource* pSource) const
{
    return std::find(m_arrTwainSource.begin(), 
                     m_arrTwainSource.end(), 
                     pSource) != m_arrTwainSource.end();
}

bool CTL_ITwainSession::SelectSource( const CTL_ITwainSource* pSource )
{
    if ( !pSource )  // Choose the default source
    {
        // Get default source
        CTL_GetDefaultSourceTriplet ST( this );
        if ( ST.Execute() == TWRC_SUCCESS )
            m_pSelectedSource = ST.GetSourceIDPtr();
        else
            return false;
    }
    else
    {
        // Select the source given by pSource
        if ( !IsSourceSelected( pSource->GetProductName().c_str() ))
        {
            return false;
        }
        m_pSelectedSource = const_cast<CTL_ITwainSource*>(pSource);
        m_pSelectedSource->SetTwainVersion2(m_pSelectedSource->GetSourceIDPtr()->SupportedGroups & DF_DS2 ? true : false);
    }
    return true;
}

bool CTL_ITwainSession::SelectSource( LPCTSTR strName )
{
    if ( m_arrTwainSource.empty() )
        EnumSources();
    CTL_ITwainSource* pSource =  IsSourceSelected( strName );
    if ( pSource )
        return SelectSource( pSource );
    return false;
}


bool CTL_ITwainSession::OpenSource( const CTL_ITwainSource* pSource )
{
    CTL_ITwainSource *pTemp;
    // Open the source
    if ( !pSource )
        pTemp = m_pSelectedSource;
    else
        pTemp = const_cast<CTL_ITwainSource*>(pSource);

    if ( !pTemp )
        return false;

    if ( !IsSourceSelected( pTemp->GetProductName().c_str()) )
        return false;

    if ( !pTemp->IsOpened() )
    {
        // see if this is a DS 2.x source
        pTemp->SetTwainVersion2(pTemp->GetSupportedGroups() & DF_DS2 ? true : false);

        // Not opened, so open it.
        CTL_OpenSourceTriplet ST( this, pTemp );
        if ( ST.Execute() != TWRC_SUCCESS )
        {
            return false;
        }
        pTemp->SetState(SOURCE_STATE_OPENED);
        pTemp->SetOpenFlag(true);
    }

    // Make this the selected source
    m_pSelectedSource = pTemp;

    return true;
}


bool CTL_ITwainSession::CloseSource( const CTL_ITwainSource* pSource, bool bForce )
{
    CTL_ITwainSource *pTemp;

    // Close the source
    if ( !pSource )
        pTemp = m_pSelectedSource;
    else
        pTemp = const_cast<CTL_ITwainSource*>(pSource);
    pTemp->CloseSource(bForce);
    pTemp->SetOpenFlag(false);
    m_pSelectedSource = nullptr;
    return true;
}

CTL_ITwainSession::~CTL_ITwainSession()
{
    // Only destroy if the DTWAIN internal handle is still
    // valid for this thread
    if (GetDTWAINHandle_Internal())
    {
        DestroyAllSources();
        DestroyTwainWindow();
    }
}


void CTL_ITwainSession::DestroyTwainWindow()
{
#ifdef _WIN32
    if ( m_bTwainWindowCreated )
    {
        DestroyWindow( m_AppWnd );
        m_bTwainWindowCreated = false;
    }
#endif
}

void CTL_ITwainSession::DestroyOneSource(CTL_ITwainSource *pSource)
{
    const auto found = find(m_arrTwainSource.begin(),
                                            m_arrTwainSource.end(),
                                            pSource);
    if ( found != m_arrTwainSource.end())
    {
        CTL_ITwainSource::Destroy( pSource );
        m_arrTwainSource.erase(found);
    }
}

void CTL_ITwainSession::DestroyAllSources()
{
    std::for_each(m_arrTwainSource.begin(), m_arrTwainSource.end(), CTL_ITwainSource::Destroy);
    m_arrTwainSource.clear();
    m_pSelectedSource = nullptr;
}

const CTL_TwainSourceSet& CTL_ITwainSession::GetTwainSources() 
{ 
    if (m_arrTwainSource.empty())
        EnumSources();
    return m_arrTwainSource; 
}


void CTL_ITwainSession::EnumSources()
{
    // Get first source
    CTL_GetFirstSourceTriplet ST1( this );
    if ( ST1.Execute() == TWRC_SUCCESS )
    {
        CTL_ITwainSource *pSource = ST1.GetSourceIDPtr();
        if ( !AddTwainSource( pSource ) )
            CTL_ITwainSource::Destroy( pSource );
    }
    else
    {
        CTL_ITwainSource::Destroy( ST1.GetSourceIDPtr() );
        return;
    }
    while ( true )
    {
        CTL_GetNextSourceTriplet STn( this );
        CTL_ITwainSource* pSource = STn.GetSourceIDPtr();

        if ( STn.Execute() == TWRC_SUCCESS )
        {
            if ( !AddTwainSource( pSource ) )
                CTL_ITwainSource::Destroy( pSource );
        }
        else
        {
            CTL_ITwainSource::Destroy( pSource );
            m_bAllSourcesRetrieved = true;
            break;
        }
    }
    UpdateStatusMap();
}

void CTL_ITwainSession::UpdateStatusMap()
{
    // Modify source status map
    auto& status_map = CTL_StaticData::GetSourceStatusMap();

    std::for_each(m_arrTwainSource.begin(), m_arrTwainSource.end(), [&](const CTL_ITwainSource* pSourceInner)
        {
            std::string sname = pSourceInner->GetProductNameA();
            auto iter = status_map.find(sname);
            if (iter == status_map.end())
            {
                auto mapIter = status_map.insert({ sname, {} }).first;
                mapIter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, true);
            }
        });
}

void CTL_ITwainSession::CopyAllSources( CTL_TwainSourceSet & rArray )
{
    GetNumSources();
    rArray = m_arrTwainSource;
}

int CTL_ITwainSession::GetNumSources()
{
    if ( m_arrTwainSource.empty() || !m_bAllSourcesRetrieved )
        EnumSources();
    m_bAllSourcesRetrieved = true;
    return static_cast<int>(m_arrTwainSource.size());
}

CTL_ITwainSource* CTL_ITwainSession::GetSelectedSource() const
{
    return m_pSelectedSource;
}


void CTL_ITwainSession::SetSelectedSource(CTL_ITwainSource* pSource)
{
    m_pSelectedSource = pSource;
}

CTL_ITwainSource* CTL_ITwainSession::Find(const CTL_ITwainSource* pSource)
{
    return IsSourceSelected( pSource->GetProductName().c_str());
}

CTL_ITwainSource* CTL_ITwainSession::IsSourceSelected(LPCTSTR pSourceName)
{
    struct ProductNameFinder
    {
        CTL_StringType m_strProduct;
        ProductNameFinder(CTL_StringType s) : m_strProduct(std::move(s)) {}
        bool operator()(CTL_ITwainSource* pSource) const
        {
            const TW_IDENTITY* pIdentity = pSource->GetSourceIDPtr();
            CTL_StringType strTemp = stringconversion::Convert_AnsiPtr_To_Native(pIdentity->ProductName);
            stringutils::MakeUpperCase(stringutils::TrimAll(strTemp));
            return strTemp == m_strProduct;
        }
    };

    CTL_StringType strProduct;
    if (pSourceName)
        strProduct = pSourceName;
    strProduct = stringutils::TrimAll(stringutils::MakeUpperCase(strProduct));
    const auto it =
        std::find_if(m_arrTwainSource.begin(), m_arrTwainSource.end(), ProductNameFinder(strProduct));
    if (it != m_arrTwainSource.end())
        return *it;
    return nullptr;
}

CTL_ITwainSource* CTL_ITwainSession::GetDefaultSource()
{
    // Get first source
    CTL_GetDefaultSourceTriplet ST( this );
    if ( ST.Execute() == TWRC_SUCCESS )
    {
        AddTwainSource(ST.GetSourceIDPtr());
        return  ST.GetSourceIDPtr();
    }
    return nullptr;
}

SessionCloserRAII::~SessionCloserRAII()
{
    try
    {
        if (bMustClose)
            DTWAIN_EndTwainSession();
    }
    catch (...) {}
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSessionEnabled()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if ( pHandle->m_bSessionAllocated )
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
        CATCH_BLOCK(false)
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSession(HWND hWndMsgNotify, LPCTSTR lpszDLLName)
    {
        LOG_FUNC_ENTRY_PARAMS((hWndMsgNotify, lpszDLLName))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if (pHandle->m_bSessionAllocated)
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CTL_StringType sDLLName;
        if (lpszDLLName)
            sDLLName = lpszDLLName;
    #ifdef _WIN32
        HWND hWndMsg;
        HINSTANCE hInstance;

        // Create a proxy if no window handle is given
        if ( !hWndMsgNotify )
        {
            // Create the window
            hWndMsg = CreateTwainWindow(pHandle,nullptr,hWndMsgNotify);

            // This is the window's instance handle
            hInstance = CTL_StaticData::GetDLLInstanceHandle();

            pHandle->m_bUseProxy = true;
        }
        else
        {
            // Set the TWAIN window to the user's window
            hWndMsg = hWndMsgNotify;

            // Record that we need to subclass this guy
            pHandle->m_bUseProxy = false;

            // Get the instance handle of the user's window
            #ifdef DTWAIN_LIB
            hInstance = CTL_StaticData::s_DLLInstance;
            #else
            hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWndMsg, GWLP_HINSTANCE));
            #endif
        }
        // Error if the window does not exist
        DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&]{ return !hWndMsg;}, DTWAIN_ERR_NULL_WINDOW, false, FUNC_MACRO);
        // Set the callback window
        pHandle->m_hWndTwain = hWndMsg;
    #else
        HINSTANCE hInstance;
        // This is the window's instance handle
        hInstance = CTL_StaticData::s_DLLInstance;
        HWND hWndMsg;
    #endif

        const CTL_StringType szName = GetDTWAINExecutionPath(); // ::dll::symbol_location(DTWAIN_DLLNAME).wstring();

        // See if we need to create a TWAIN application manager
        if ( !CTL_TwainAppMgr::GetInstance() )
        {
            // Create it with the parameters shown
            if ( !CTL_TwainAppMgr::Create(pHandle,
                                          hInstance,
                                          CTL_StaticData::GetDLLInstanceHandle(),
                                          lpszDLLName?sDLLName.c_str():nullptr) )
            {
                if ( pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_LATESTVERSION ||
                     pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_VERSION2 )
                   DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [] { return 1;}, DTWAIN_ERR_TWAINOPENSOURCEDSMNOTFOUND, false, FUNC_MACRO);
                else
                   DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_TWAIN32DSMNOTFOUND, false, FUNC_MACRO);
            }
        }

        // Create a session
        auto Session = CTL_TwainAppMgr::CreateTwainSession( pHandle, szName.c_str(), &hWndMsg);

        if ( Session == nullptr)
        {
            DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, []{return 1;}, DTWAIN_ERR_TWAIN, false, FUNC_MACRO);
        }
        #ifdef DTWAIN_LIB
        CTL_TwainAppMgr::SetDLLInstance( CTL_StaticData::s_DLLInstance );
        #else
        CTL_TwainAppMgr::SetDLLInstance( static_cast<HINSTANCE>(GetDLLInstance()) );
        #endif
        auto appInstance = CTL_TwainAppMgr::GetInstance();
        pHandle->m_pAppMgr = appInstance.get();
        pHandle->m_pTwainSession = Session;
        pHandle->m_hInstance   = hInstance;

        if (pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_LATESTVERSION ||
            pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_VERSION2)
        {
            pHandle->m_strTWAINPath2 = CTL_TwainAppMgr::GetDSMPath();
        }
        else
        {
            pHandle->m_strTWAINPath = CTL_TwainAppMgr::GetDSMPath();
        }

        pHandle->m_nTwainPathLocation = CTL_TwainAppMgr::GetDSMPathLocation();
        // We may need to route the messages to the user.  The user window
        // must be aware that if it is subclassed again, the message will
        // appear twice.
        pHandle->m_hNotifyWnd = hWndMsgNotify;

        // Destroy the pointer
        pHandle->m_bSessionAllocated = true;

        // Subclass the user's window (if necessary)
        #ifdef _WIN32
        if ( !pHandle->m_bUseProxy )
            pHandle->m_hOrigProc = SubclassTwainMsgWindow(hWndMsgNotify);
        else
            pHandle->m_hOrigProc = nullptr;
        #endif
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndTwainSession()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        // Delete it
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&]{return DTWAIN_IsAcquiring()==1;}, DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

        // Check if any source is still acquiring
        if ( pHandle->m_nSourceCloseMode == DTWAIN_SourceCloseModeFORCE )
        {
            // Close any sources
            DTWAIN_CloseAllSources();
        }
        else
            DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return DTWAIN_IsAcquiring()==1;}, DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

        if ( !pHandle->m_bSessionAllocated )
            LOG_FUNC_EXIT_NONAME_PARAMS(true)

        CTL_StringTypeA sClosingDSM = GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_DSM) + "\n";
        CTL_StringTypeA sClosingTwainSession = GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_TWAIN_SESSION) + "\n";

        // Close any sources
        pHandle->m_pTwainSession->DestroyAllSources();

        // Close the source manager
        try
        {
            CTL_TwainAppMgr::CloseSourceManager(pHandle->m_pTwainSession);
        }
        catch(...)
        {
            try
            {
                LogWriterUtils::WriteLogInfoIndentedA(sClosingDSM);
            }
            catch (...)
            {
                OutputDebugStringA("Could not close TWAIN DSM!");
            }
        }

        // close the general TWAIN session
        try
        {
            CTL_TwainAppMgr::DestroyTwainSession(pHandle->m_pTwainSession);
        }
        catch(...)
        {
            LogWriterUtils::WriteLogInfoIndentedA(sClosingTwainSession);
        }
        if ( CTL_StaticData::GetThreadToDLLHandleMap().size() == 1 )
        {
            auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
            try
            {
                CTL_TwainAppMgr::Destroy();
            }
            catch(...)
            {
                if (logFilterFlags)
                {
                    CTL_StringTypeA sClosingManager = GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_DTWAIN_MANAGER);
                    LogWriterUtils::WriteLogInfoIndentedA(sClosingManager);
                }
            }
            if (logFilterFlags)
            {
                LogWriterUtils::WriteLogInfoIndentedA(GetResourceStringFromMap(IDS_CLOSING_DTWAIN));
            }
        }

        // Close the window (Dummy window may have been created)
        pHandle->m_bSessionAllocated = false;
        #ifdef _WIN32
        if ( pHandle->m_bUseProxy )
        {
            pHandle->m_CallbackMsg = nullptr;
            if (IsWindow(pHandle->m_hWndTwain))
            {
                try
                {
                    DestroyWindow(pHandle->m_hWndTwain);
                }
                catch (...) {}
            }
        }
        else
        {
            try
            {
                // Remove subclass from the user's window
                if ( pHandle->m_hOrigProc && pHandle->m_hWndTwain )
                    SubclassTwainMsgWindow(pHandle->m_hWndTwain, pHandle->m_hOrigProc);
            }
            catch(...)
            {
                if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
                {
                    LogWriterUtils::WriteLogInfoIndentedA(GetResourceStringFromMap(IDS_DTWAIN_ERROR_REMOVE_WINDOW));
                }
            }
        }
        pHandle->m_hWndTwain = nullptr;
        pHandle->m_hOrigProc = nullptr;
        #endif
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }
}
