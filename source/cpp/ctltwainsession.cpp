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
#include <cstring>
#include <algorithm>
#include <utility>
#include "ctltwainsession.h"
#include "ctltrall.h"
#include "ctltwainmanager.h"
#include "ctltwainsource.h"

using namespace dynarithmic;

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

    StringWrapperA::SafeStrcpy( m_AppIdTemp.Version.Info,
                                StringConversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szVersion).c_str(),
                                sizeof m_AppIdTemp.Version.Info - 1 );

    m_AppIdTemp.ProtocolMajor =    TWON_PROTOCOLMAJOR;
    m_AppIdTemp.ProtocolMinor =    TWON_PROTOCOLMINOR;
    m_AppIdTemp.SupportedGroups =  DG_IMAGE | DG_CONTROL | DG_AUDIO | DF_APP2 | DF_DSM2 ;

    StringWrapperA::SafeStrcpy( m_AppIdTemp.Manufacturer,  StringConversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szManufact).c_str(), sizeof m_AppIdTemp.Manufacturer - 1 );
    StringWrapperA::SafeStrcpy( m_AppIdTemp.ProductFamily, StringConversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szFamily).c_str(), sizeof m_AppIdTemp.ProductFamily - 1 );
    StringWrapperA::SafeStrcpy( m_AppIdTemp.ProductName,   StringConversion::Convert_Native_To_Ansi(pHandle->m_SessionStruct.szProduct).c_str(),sizeof m_AppIdTemp.ProductName - 1 );
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
    if ( std::find_if(m_arrTwainSource.begin(), m_arrTwainSource.end(), SourceFinder(strProduct)) == m_arrTwainSource.end())
    {
        m_arrTwainSource.insert( pSource );
        auto iter = sourceStatusMap.insert({ pSource->GetProductNameA(), {} }).first;
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, true);
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
            CTL_StringType strTemp = StringConversion::Convert_AnsiPtr_To_Native(pIdentity->ProductName);
            StringWrapper::MakeUpperCase(StringWrapper::TrimAll(strTemp));
            return strTemp == m_strProduct;
        }
    };

    CTL_StringType strProduct;
    if (pSourceName)
        strProduct = pSourceName;
    strProduct = StringWrapper::TrimAll(StringWrapper::MakeUpperCase(strProduct));
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
