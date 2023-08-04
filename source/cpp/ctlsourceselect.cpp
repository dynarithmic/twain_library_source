/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#include "ctltmpl5.h"
#include "dtwstrfn.h"
#include "sourceselectopts.h"
#include "errorcheck.h"
#include "../simpleini/simpleini.h"
#include "ctlfileutils.h"
#include "ctldefsource.h"
#include "ctlthreadutils.h"
#include <boost/logic/tribool.hpp>

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;
using namespace boost::logic;

LONG DLLENTRY_DEF DTWAIN_SetTwainDialogFont(HFONT font)
{
    LOG_FUNC_ENTRY_PARAMS((font))
    CTL_StaticData::s_DialogFont = font;
    LOG_FUNC_EXIT_PARAMS(1)
    CATCH_BLOCK(0)
}

static LRESULT CALLBACK DisplayTwainDlgProc(HWND, UINT, WPARAM, LPARAM);
static std::string GetTwainDlgTextFromResource(int nID, size_t& status);
static void DisplayLocalString(HWND hWnd, int nID, int ResID);

typedef DTWAIN_SOURCE(*SourceFn)(const SourceSelectionOptions&);
static std::unordered_map<int, SourceFn> SourcefnMap = {{SELECTSOURCE, DTWAIN_LLSelectSource},
                                                        {SELECTDEFAULTSOURCE, DTWAIN_LLSelectDefaultSource},
                                                        {SELECTSOURCEBYNAME, DTWAIN_LLSelectSourceByName},
                                                        {SELECTSOURCE2, DTWAIN_LLSelectSource2} 
                                                        };

static LONG OpenSourceInternal(DTWAIN_SOURCE Source, const SourceSelectionOptions& opts)
{
    auto* pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const auto p = static_cast<CTL_ITwainSource *>(Source);
    if (p)
        p->SetSelected(true);
    else
        return DTWAIN_ERR_BAD_SOURCE;
    if (opts.nOptions & DTWAIN_DLG_OPENONSELECT) // pHandle->m_bOpenSourceOnSelect)
    {
        const DTWAIN_BOOL retval = DTWAIN_OpenSource(Source);
        if (retval != TRUE)
        {
            const LONG err = DTWAIN_ERR_SOURCE_COULD_NOT_OPEN;
            CTL_StringType sProdName = StringWrapper::QuoteString(p->GetProductName());
            DTWAIN_Check_Error_Condition_3_Ex(pHandle, [&]{return !retval; }, err, 
                                              StringConversion::Convert_Native_To_Ansi(sProdName), err, FUNC_MACRO);
        }
    }
    return DTWAIN_NO_ERROR;
}

static DTWAIN_SOURCE SelectAndOpenSource(const SourceSelectionOptions& opts)
{
    const DTWAIN_SOURCE Source = SourceSelect(opts);
    auto& sourcemap = CTL_StaticData::GetSourceStatusMap();
    if (Source)
    {
        CTL_ITwainSource* pSource = reinterpret_cast<CTL_ITwainSource*>(Source);
        auto iter = sourcemap.insert({ pSource->GetProductNameA(), {} }).first;
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_SELECECTED, true);
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, false);
        const LONG retVal = OpenSourceInternal(Source, opts);
        if (retVal != DTWAIN_NO_ERROR)
        {
            if ( opts.nWhich == SELECTSOURCEBYNAME )
                CTL_TwainAppMgr::SetError(retVal, StringConversion::Convert_NativePtr_To_Ansi(opts.szProduct));
            return nullptr;
        }
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_OPEN, CTL_TwainAppMgr::IsSourceOpen(pSource));
        iter->second.SetThreadID(dynarithmic::getThreadIdAsString());
        iter->second.SetSourceHandle(pSource);
    }

    if ( !Source && opts.nWhich == SELECTSOURCEBYNAME )
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
        if (pHandle)
            CTL_TwainAppMgr::SetError(pHandle->m_lLastError, StringConversion::Convert_NativePtr_To_Ansi(opts.szProduct));
    }
    return Source;
}

static DTWAIN_SOURCE SelectSourceHelper(SourceSelectionOptions opts, tribool bOpen)
{
    auto* pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);
    if ( indeterminate(bOpen))
    {
        if (pHandle->m_bOpenSourceOnSelect)
            opts.nOptions |= DTWAIN_DLG_OPENONSELECT;
    }
    else
    {
        opts.nOptions |= DTWAIN_DLG_OPENONSELECTOVERRIDE;
        if ( bOpen )
            opts.nOptions |= DTWAIN_DLG_OPENONSELECT;
    }
    return SelectAndOpenSource(opts);
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource()
{
    LOG_FUNC_ENTRY_PARAMS(())
    DTWAIN_SOURCE Source = SelectSourceHelper(SourceSelectionOptions(), {indeterminate});
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectDefaultSource()
{
    LOG_FUNC_ENTRY_PARAMS(())
    DTWAIN_SOURCE Source = SelectSourceHelper(SourceSelectionOptions(SELECTDEFAULTSOURCE), {indeterminate});
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByName(LPCTSTR szProduct)
{
    LOG_FUNC_ENTRY_PARAMS((szProduct))
    const CTL_StringType sProduct = szProduct;
    DTWAIN_SOURCE Source = SelectSourceHelper(SourceSelectionOptions(SELECTSOURCEBYNAME, sProduct.c_str()), { indeterminate });
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceWithOpen(DTWAIN_BOOL bOpen)
{
    LOG_FUNC_ENTRY_PARAMS((bOpen))
    DTWAIN_SOURCE Source = SelectSourceHelper(SourceSelectionOptions(), bOpen?true:false);
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectDefaultSourceWithOpen(DTWAIN_BOOL bOpen)
{
    LOG_FUNC_ENTRY_PARAMS((bOpen))
    DTWAIN_SOURCE Source = SelectSourceHelper(SourceSelectionOptions(SELECTDEFAULTSOURCE), bOpen ? true : false);
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameWithOpen(LPCTSTR szProduct, DTWAIN_BOOL bOpen)
{
    LOG_FUNC_ENTRY_PARAMS((szProduct, bOpen))
    const CTL_StringType sProduct = szProduct;
    DTWAIN_SOURCE Source = SelectSourceHelper(SourceSelectionOptions(SELECTSOURCEBYNAME, sProduct.c_str()), bOpen ? true : false);
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2(HWND hWndParent, 
                                                LPCTSTR szTitle, 
                                                LONG xPos, 
                                                LONG yPos, 
                                                LONG nOptions)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nOptions))
    const DTWAIN_SOURCE Source = SelectAndOpenSource(SourceSelectionOptions(SELECTSOURCE2, nullptr, hWndParent,
                                                                            szTitle, 
                                                                            xPos, yPos, 
                                                                            nullptr, 
                                                                            nullptr, 
                                                                            nullptr, 
                                                                            nOptions));
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2Ex(HWND hWndParent,
                                                  LPCTSTR szTitle,
                                                  LONG xPos,
                                                  LONG yPos,
                                                  LPCTSTR szIncludeFilter,
                                                  LPCTSTR szExcludeFilter,
                                                  LPCTSTR szNameMapping,
                                                  LONG nOptions)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, szIncludeFilter, szExcludeFilter, szNameMapping, nOptions))
    const CTL_StringType sInclude = szIncludeFilter? szIncludeFilter : _T("");
    const CTL_StringType sExclude = szExcludeFilter ? szExcludeFilter : _T("");
    const CTL_StringType sMapping = szNameMapping ? szNameMapping : _T("");
    const DTWAIN_SOURCE Source = SelectAndOpenSource(SourceSelectionOptions(SELECTSOURCE2, nullptr, hWndParent,
                                                                            szTitle,
                                                                            xPos, yPos,
                                                                            sInclude.c_str(),
                                                                            sExclude.c_str(),
                                                                            sMapping.c_str(), nOptions));
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceSelected(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, FALSE, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);
    DTWAIN_BOOL bRet = FALSE;
    if ( p )
        bRet = p->IsSelected();
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(FALSE)
}

DTWAIN_SOURCE dynarithmic::SourceSelect(const SourceSelectionOptions& options)
{
    LOG_FUNC_ENTRY_PARAMS((options))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);

    // Start a session if not already started by app
    if (!pHandle->m_bSessionAllocated)
    {
        if (!DTWAIN_StartTwainSession(nullptr, nullptr))
            LOG_FUNC_EXIT_PARAMS((DTWAIN_SOURCE)NULL)
    }

    // Do a minimal check for TWAIN here
    const DTWAIN_SOURCE pSource = SourcefnMap[options.nWhich] (options);

    if (!pSource)
        LOG_FUNC_EXIT_PARAMS((DTWAIN_SOURCE)NULL)

    // Open and close the source to initialize capability structure
    // Return a dead source.  This allows closing of the source without
    // destroying the source info
    auto pRealSource = static_cast<CTL_ITwainSource*>(pSource);
    DTWAIN_SOURCE pDead = nullptr;

    DTWAIN_ARRAY pDTWAINArray = nullptr;
    bool bFound = false;

    if (!DTWAIN_EnumSources(&pDTWAINArray))
    {
        DTWAIN_EndTwainSession();
        LOG_FUNC_EXIT_PARAMS(NULL)
    }

    DTWAINArrayLL_RAII arr(pDTWAINArray);
    const auto& vSources = pHandle->m_ArrayFactory->underlying_container_t<CTL_ITwainSource*>(pDTWAINArray);

    if (!vSources.empty())
    {
        const auto sName = pRealSource->GetProductName();
        auto pDeadIt = std::find_if(vSources.begin(), vSources.end(),
            [&](const CTL_ITwainSource* pS){ return pS->GetProductName() == sName; });

        if (pDeadIt != vSources.end())
        {
            pDead = *pDeadIt;
            bFound = true;
        }
        if (bFound)
        {
            if (pRealSource != pDead)
            {
                const auto pSession = CTL_TwainAppMgr::GetCurrentSession();
                pSession->DestroyOneSource(pRealSource);
            }
            DTWAIN_SetDefaultSource(pDead);
            LOG_FUNC_EXIT_PARAMS(pDead)
        }
    }
    DTWAIN_EndTwainSession();
    LOG_FUNC_EXIT_PARAMS((DTWAIN_SOURCE)NULL)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectSource(const SourceSelectionOptions& /*opt*/)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, NULL, FUNC_MACRO);
    // Select a source from the source dialog
    // Bring TWAIN window to top
    const CTL_ITwainSource *pSource = CTL_TwainAppMgr::SelectSourceDlg( pHandle->m_pTwainSession );
    // Check if a source was selected
    LOG_FUNC_EXIT_PARAMS((DTWAIN_SOURCE)pSource)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectSource2(const SourceSelectionOptions& opts)
{
    #ifndef _WIN32
    return DTWAIN_LLSelectSource(opts);
    #else
    LOG_FUNC_ENTRY_PARAMS((opts))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // Get the resource for the Twain dialog
    const HGLOBAL hglb = LoadResource(CTL_StaticData::s_DLLInstance,
                                      static_cast<HRSRC>(FindResource(CTL_StaticData::s_DLLInstance,
                                                                      MAKEINTRESOURCE(10000), RT_DIALOG)));
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{ return !hglb;}, DTWAIN_ERR_NULL_WINDOW, NULL, FUNC_MACRO);

    const auto lpTemplate = static_cast<LPDLGTEMPLATE>(LockResource(hglb));

    SelectStruct selectStruct;
    selectStruct.CS.xpos = opts.xPos;
    selectStruct.CS.ypos = opts.yPos;
    selectStruct.CS.nOptions = opts.nOptions;
    selectStruct.CS.hWndParent = opts.hWndParent;
    selectStruct.nItems = 0;
    if ( opts.szTitle )
        selectStruct.CS.sTitle = opts.szTitle;
    else
    {
        size_t status = 0;
        CTL_TwainAppMgr::WriteLogInfoA("Retrieving TWAIN Dialog Resources...\n");
        selectStruct.CS.sTitle = StringConversion::Convert_Ansi_To_Native(GetTwainDlgTextFromResource(IDS_SELECT_SOURCE_TEXT, status));
        CTL_TwainAppMgr::WriteLogInfoA("Retrieved TWAIN Dialog Resources successfully...\n");
        if ( !status )
            selectStruct.CS.sTitle = _T("Select Source");
    }

    if (opts.szIncludeNames)
        StringWrapper::Tokenize(opts.szIncludeNames, _T("|"), selectStruct.CS.aIncludeNames);
    if (opts.szExcludeNames)
        StringWrapper::Tokenize(opts.szExcludeNames, _T("|"), selectStruct.CS.aExcludeNames);
    if (opts.szNameMapping)
    {
        std::vector<CTL_StringType> mapPairs;
        StringWrapper::Tokenize(opts.szNameMapping, _T("|"), mapPairs);
        for (auto& m : mapPairs)
        {
            std::vector<CTL_StringType> onePair;
            StringWrapper::Tokenize(m, _T("="), onePair);
            if (onePair.size() == 2)
                selectStruct.CS.mapNames.insert({ onePair.front(), onePair.back() });
        }
    }
    if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)
        CTL_TwainAppMgr::WriteLogInfoA("Displaying TWAIN Dialog...\n");
    const INT_PTR bRet = DialogBoxIndirectParam(CTL_StaticData::s_DLLInstance, lpTemplate, opts.hWndParent,
                                                reinterpret_cast<DLGPROC>(DisplayTwainDlgProc), reinterpret_cast<LPARAM>(&selectStruct));
    if ( bRet == -1 )
        LOG_FUNC_EXIT_PARAMS(NULL)

    // See if cancel was selected
    if (selectStruct.SourceName.empty() || selectStruct.nItems == 0 )
    {
        CTL_TwainAppMgr::SetError(DTWAIN_ERR_SOURCESELECTION_CANCELED);
        LOG_FUNC_EXIT_PARAMS(NULL)
    }

    // Could be a mapped name, so need to check
    auto actualSourceName = selectStruct.SourceName;
    if (opts.nOptions & DTWAIN_DLG_USENAMEMAPPING)
    {
        const auto iter = std::find_if(selectStruct.CS.mapNames.begin(), selectStruct.CS.mapNames.end(),
                                       [&](auto& pr)
                                       { return pr.second == actualSourceName;}
        );
        if (iter != selectStruct.CS.mapNames.end())
            actualSourceName = iter->first;
    }
    DTWAIN_SOURCE Source = nullptr;
    if ( opts.nOptions & DTWAIN_DLG_OPENONSELECTOVERRIDE )
        Source = DTWAIN_SelectSourceByNameWithOpen(actualSourceName.c_str(), opts.nOptions & DTWAIN_DLG_OPENONSELECT);
    else
        Source = DTWAIN_SelectSourceByName(actualSourceName.c_str());

    // Set the default Source
    DTWAIN_SetDefaultSource(Source);

    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
    #endif
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectSourceByName( const SourceSelectionOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, NULL, FUNC_MACRO);
    // Select a source from the source dialog
    const CTL_ITwainSource *pSource = CTL_TwainAppMgr::SelectSource( pHandle->m_pTwainSession, opts.szProduct);
    // Check if a source was selected
    LOG_FUNC_EXIT_PARAMS(static_cast<DTWAIN_SOURCE>(const_cast<CTL_ITwainSource *>(pSource)))
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectDefaultSource(const SourceSelectionOptions& /*opts*/)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, NULL, FUNC_MACRO);
    const CTL_ITwainSource* pSource = CTL_TwainAppMgr::GetDefaultSource(pHandle->m_pTwainSession);
    const DTWAIN_SOURCE Source = static_cast<DTWAIN_SOURCE>(const_cast<CTL_ITwainSource *>(pSource));
    LOG_FUNC_EXIT_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDefaultSource(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    CTL_ITwainSource *p = VerifySourceHandle(GetDTWAINHandle_Internal(), Source);
    if (p)
    {
        // Load the resources
        CSimpleIniA customProfile;
        CTL_StringType fullDirectory = dynarithmic::GetDTWAININIPath().c_str();
        customProfile.LoadFile(fullDirectory.c_str());
        customProfile.SetValue("Sources", "Default", p->GetProductNameA().c_str());
        customProfile.SaveFile(fullDirectory.c_str());
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

static CTL_StringType GetDefaultSource()
{
    // Load the resources
    CSimpleIniA customProfile;
    CTL_StringType fullDirectory = dynarithmic::GetDTWAININIPath();
    customProfile.LoadFile(fullDirectory.c_str());
    const char *defSource = customProfile.GetValue("Sources", "Default");
    return StringConversion::Convert_AnsiPtr_To_Native(defSource);
}
#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////////
/// TWAIN Dialog procedure
std::string GetTwainDlgTextFromResource(int nID, size_t& status)
{
    status = 0;
    const size_t resSize = GetResourceStringA(nID, nullptr, 0);
    if (resSize > 0)
    {
        std::string buffer(resSize, '\0');
        status = GetResourceStringA(nID, &buffer[0], static_cast<LONG>(resSize));
        return buffer;
    }
    return {};
}

static bool ByCX(SIZE sz1, SIZE sz2)
{ return sz1.cx > sz2.cx; }

static BOOL CALLBACK ChildEnumFontProc(HWND hWnd, LPARAM lParam)
{
    SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
    return TRUE;
}

static std::vector<CTL_StringType> AdjustSourceNames(std::vector<CTL_StringType>& vSourceNames, CustomPlacement CS)
{
    if (vSourceNames.empty())
        return {};

    const bool doExclude = CS.nOptions & DTWAIN_DLG_USEEXCLUDENAMES?true:false;
    const bool doInclude = CS.nOptions & DTWAIN_DLG_USEINCLUDENAMES?true : false;
    const bool doMapping = CS.nOptions & DTWAIN_DLG_USENAMEMAPPING?true : false;

    if (!doInclude && !doExclude && !doMapping)
        return vSourceNames;

    for (auto& sName : vSourceNames)
        sName = StringWrapper::TrimAll(sName);

    if (doInclude)
    {
        for (auto& sName : CS.aIncludeNames)
            sName = StringWrapper::TrimAll(sName);

        std::vector<CTL_StringType> vReturn2;
        std::sort(vSourceNames.begin(), vSourceNames.end());
        std::sort(CS.aIncludeNames.begin(), CS.aIncludeNames.end());
        std::set_intersection(vSourceNames.begin(), vSourceNames.end(),
                              CS.aIncludeNames.begin(), CS.aIncludeNames.end(), std::back_inserter(vReturn2));
        if (!vReturn2.empty())
            vSourceNames = vReturn2;
    }

    auto vReturn = vSourceNames;

    if (doExclude)
    {
        for (auto& sName : CS.aExcludeNames)
            sName = StringWrapper::TrimAll(sName);

        std::vector<CTL_StringType> vReturn2;
        std::sort(vSourceNames.begin(), vSourceNames.end());
        std::sort(CS.aExcludeNames.begin(), CS.aExcludeNames.end());
        std::set_difference(vSourceNames.begin(), vSourceNames.end(),
                            CS.aExcludeNames.begin(), CS.aExcludeNames.end(), std::back_inserter(vReturn2));
        if (!vReturn2.empty())
            vReturn = vReturn2;
    }

    if (doMapping)
    {
        std::vector<CTL_StringType> vMapped;
        for (auto& sName : vReturn)
        {
            auto iter = CS.mapNames.find(sName);
            if (iter != CS.mapNames.end())
                vMapped.push_back(iter->second);
            else
                vMapped.push_back(sName);
        }
        vReturn = vMapped;
    }
    return vReturn;
}

struct openSourceSaver
{
    bool m_bSaved;
    openSourceSaver(bool bSaved) : m_bSaved(bSaved) {}
    ~openSourceSaver() { DTWAIN_OpenSourcesOnSelect(m_bSaved); }
};

struct closeSourceRAII
{
    DTWAIN_SOURCE m_Source;
    closeSourceRAII(DTWAIN_SOURCE source) : m_Source(source) {}
    ~closeSourceRAII() { DTWAIN_CloseSource(m_Source); }
};

LRESULT CALLBACK DisplayTwainDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static SelectStruct *pS;
    bool bLogMessages = (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)?true:false;
    switch (message)
    {
        case WM_INITDIALOG:
        {
            DTWAINDeviceContextRelease_RAII contextRAII;
            if (CTL_StaticData::s_DialogFont)
            {
                SendMessage(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(CTL_StaticData::s_DialogFont), 0);
                EnumChildWindows(hWnd, ChildEnumFontProc, reinterpret_cast<LPARAM>(CTL_StaticData::s_DialogFont));
            }

            HWND lstSources;
            if ( bLogMessages) 
                CTL_TwainAppMgr::WriteLogInfoA("Initializing TWAIN Dialog...\n");
            pS = reinterpret_cast<SelectStruct*>(lParam);

            if (pS->CS.nOptions & DTWAIN_DLG_CENTER_SCREEN)
                CenterWindow(hWnd, nullptr);
            else
            if (pS->CS.nOptions & DTWAIN_DLG_CENTER)
                CenterWindow(hWnd, GetParent(hWnd));
            else
                SetWindowPos(hWnd, nullptr, pS->CS.xpos, pS->CS.ypos, 0, 0, SWP_NOSIZE);
            lstSources = GetDlgItem(hWnd, IDC_LSTSOURCES);

            // Set the title
            ::SetWindowText(hWnd, pS->CS.sTitle.c_str());

            // Fill the list box with the sources
            DTWAIN_ARRAY Array = nullptr;
            DTWAIN_EnumSources(&Array);
            DTWAINArrayLL_RAII arr(Array);
            const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<CTL_ITwainSource*>(Array);

            int nCount;
            if (vValues.empty())
                nCount = 0;
            else
                nCount = static_cast<int>(vValues.size());
            pS->nItems = nCount;
            if (nCount == 0)
            {
                HWND hWndSelect = GetDlgItem(hWnd, IDOK);
                if (hWndSelect)
                    EnableWindow(hWndSelect, FALSE);
                if (bLogMessages)
                    CTL_TwainAppMgr::WriteLogInfoA("Finished Adding names to TWAIN dialog...\n");

                // Display the local strings if they are available:
                DisplayLocalString(hWnd, IDOK, IDS_SELECT_TEXT);
                DisplayLocalString(hWnd, IDCANCEL, IDS_CANCEL_TEXT);
                DisplayLocalString(hWnd, IDC_SOURCETEXT, IDS_SOURCES_TEXT);
                SetFocus(hWnd);
                if (bLogMessages)
                    CTL_TwainAppMgr::WriteLogInfoA("Finished Initializing TWAIN Dialog...\n");
                return TRUE;
            }

            // Get the default Source
            DTWAIN_SOURCE DefSource = nullptr;
            std::vector<TCHAR> DefName;
            if (bLogMessages)
                CTL_TwainAppMgr::WriteLogInfoA("Initializing TWAIN Dialog -- Retrieving default TWAIN Source...\n");
            {
                // Try the INI setting first
                CTL_StringType sourceName = GetDefaultSource();
                if (!sourceName.empty())
                {
                    DefName = std::vector<TCHAR>(sourceName.begin(), sourceName.end());
                    DefName.push_back(0);
                }
                else
                {
                    {
                        // Turn off default open temporarily
                        openSourceSaver sourceSaver(DTWAIN_IsOpenSourcesOnSelect() ? true : false);
                        DTWAIN_OpenSourcesOnSelect(false);
                        DefSource = DTWAIN_SelectDefaultSource();
                    }
                    if (DefSource)
                    {
                        closeSourceRAII cs(DefSource);
                        LONG nCharacters = DTWAIN_GetSourceProductName(DefSource, nullptr, 0);
                        if (nCharacters > 0)
                        {
                            DefName.resize(nCharacters);
                            DTWAIN_GetSourceProductName(DefSource, DefName.data(), nCharacters);
                            if (bLogMessages)
                                CTL_TwainAppMgr::WriteLogInfoA("Initializing TWAIN Dialog -- Retrieved default TWAIN Source name...\n");
                        }
                    }
                }
            }
            if (bLogMessages)
            {
                if (DefName.empty())
                        CTL_TwainAppMgr::WriteLogInfoA("The TWAIN default name has no characters...\n");
                else
                {
                    StringStreamA strm;
                    strm << "The default TWAIN source is \"" <<
                        StringConversion::Convert_NativePtr_To_Ansi(DefName.data()).c_str() << "\" ...\n";
                    CTL_TwainAppMgr::WriteLogInfoA(strm.str());
                }
            }

            std::vector<SIZE> TextExtents;
            HDC hdcList = nullptr;
            if (pS->CS.nOptions & DTWAIN_DLG_HORIZONTALSCROLL)
            {
                hdcList = GetDC(lstSources);
                auto pr = std::make_pair(lstSources, hdcList);
                contextRAII.reset(&pr);
            }
            std::vector<CTL_StringType> vNewSourceNames;
            if (!vValues.empty())
            {
                std::vector<CTL_StringType> vSourceNames;
                TCHAR ProdName[256];

                std::transform(vValues.begin(), vValues.end(), std::back_inserter(vSourceNames),
                               [&](CTL_ITwainSourcePtr ptr)
                    {
                        DTWAIN_GetSourceProductName(ptr, ProdName, 255);
                        return ProdName;
                    });

                // Remove and rename sources depending on the options
                vNewSourceNames = AdjustSourceNames(vSourceNames, pS->CS);

                for (auto& sName : vNewSourceNames )
                {
                    SIZE szType;
                    if (hdcList)
                    {
                        auto cstr = sName.c_str();
                        ::GetTextExtentPoint32(hdcList, cstr, static_cast<int>(StringWrapper::traits_type::Length(cstr)), &szType);
                        TextExtents.push_back(szType);
                    }
                }

                if (hdcList)
                {
                    sort(TextExtents.begin(), TextExtents.end(), ByCX);
                }

                // Sort the names
                if (pS->CS.nOptions & DTWAIN_DLG_SORTNAMES)
                {
                    if (bLogMessages)
                        CTL_TwainAppMgr::WriteLogInfoA("Initializing TWAIN Dialog -- Sorting TWAIN Source names...\n");
                    sort(vNewSourceNames.begin(), vNewSourceNames.end());
                }
            }
            LRESULT index;
            LRESULT DefIndex = 0;
            if (bLogMessages)
            {
                StringStreamA strm;
                CTL_TwainAppMgr::WriteLogInfoA("Initializing TWAIN Dialog -- Adding names to dialog...\n");
                strm << "TWAIN found " << vNewSourceNames.size() << " source names to add to TWAIN dialog...\n";
                strm << "The TWAIN dialog window handle to add names is " << lstSources << "\n";
                CTL_TwainAppMgr::WriteLogInfoA(strm.str());
            }
            CTL_StringStreamType strm2;
            for (auto& sName : vNewSourceNames)
            {
                if ( bLogMessages )
                {
                    strm2.str(_T(""));
                    strm2 << _T("The TWAIN name being added is ") << sName << _T("\n");
                    CTL_TwainAppMgr::WriteLogInfo(strm2.str());
                }
                index = SendMessage(lstSources, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(sName.c_str()));
                if ( bLogMessages )
                {
                    CTL_TwainAppMgr::WriteLogInfoA("LB_ADDSTRING was sent to TWAIN dialog...\n");
                    CTL_TwainAppMgr::WriteLogInfoA("TWAIN now comparing names...\n");
                }
                if (!DefName.empty())
                {
                    if (StringWrapper::traits_type::Compare(sName.c_str(), static_cast<LPCTSTR>(&DefName[0])) == 0)
                        DefIndex = index;
                }
                if ( bLogMessages)
                    CTL_TwainAppMgr::WriteLogInfoA("TWAIN now finished comparing names...\n");
            }
            if (DefName.empty())
                DefIndex = 0;

            if (!TextExtents.empty())
            {
                SendMessage(lstSources, LB_SETCURSEL, DefIndex, 0);
                if (pS->CS.nOptions & DTWAIN_DLG_HORIZONTALSCROLL)
                    SendMessage(lstSources, LB_SETHORIZONTALEXTENT, TextExtents[0].cx, 0);
            }
            else
                SendMessage(lstSources, LB_SETCURSEL, DefIndex, 0);

            if (bLogMessages)
                CTL_TwainAppMgr::WriteLogInfoA("Finished Adding names to TWAIN dialog...\n");

            // Display the local strings if they are available:
            DisplayLocalString(hWnd, IDOK, IDS_SELECT_TEXT);
            DisplayLocalString(hWnd, IDCANCEL, IDS_CANCEL_TEXT);
            DisplayLocalString(hWnd, IDC_SOURCETEXT, IDS_SOURCES_TEXT);

            // Display the window as topmost if topmost flag is on
            if ( pS->CS.nOptions & DTWAIN_DLG_TOPMOSTWINDOW )
            { 
                ::SetForegroundWindow(hWnd);
                ::SetWindowPos(hWnd,       // handle to window
                               HWND_TOPMOST,  // placement-order handle
                               0,     // horizontal position
                               0,      // vertical position
                               0,  // width
                               0, // height
                               SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE// window-positioning options
                                );
            }
            SetFocus(hWnd);
            if (bLogMessages)
                CTL_TwainAppMgr::WriteLogInfoA("Finished Initializing TWAIN Dialog...\n");
            return TRUE;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK)
            {
                HWND lstSources = GetDlgItem(hWnd, IDC_LSTSOURCES);
                TCHAR sz[255];
                LRESULT nSel = SendMessage(lstSources, LB_GETCURSEL, 0, 0);
                SendMessage(lstSources, LB_GETTEXT, nSel, reinterpret_cast<LPARAM>(sz));
                pS->SourceName = sz;
                EndDialog(hWnd, LOWORD(wParam));
                return TRUE;
            }
            else
                if (LOWORD(wParam) == IDCANCEL)
                {
                    pS->SourceName.clear();
                    EndDialog(hWnd, LOWORD(wParam));
                    return TRUE;
                }
            break;
    }
    return FALSE;
}

void DisplayLocalString(HWND hWnd, int nID, int resID)
{
    std::string sText;
    size_t status = 0;
    sText = GetTwainDlgTextFromResource(resID, status);
    if (status)
    {
        const HWND hWndControl = GetDlgItem(hWnd, nID);
        if (hWndControl)
            SetWindowTextA(hWndControl, sText.c_str());
    }
}
#endif
