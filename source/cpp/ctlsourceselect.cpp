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
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "ctltmpl5.h"
#include "dtwstrfn.h"
#include "sourceselectopts.h"
#include "errorcheck.h"
#include "../simpleini/simpleini.h"
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
    CTL_StaticData::GetDialogFont() = font;
    LOG_FUNC_EXIT_NONAME_PARAMS(1)
    CATCH_BLOCK(0)
}

static std::vector<TCHAR> GetDefaultName(SelectStruct& selectTraits);
static std::vector<CTL_StringType> GetNameList(SelectStruct& pS);

typedef DTWAIN_SOURCE(*SourceFn)(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions&);
static constexpr std::array<std::pair<int, SourceFn>, 4> SourcefnMap = { {{SELECTSOURCE, DTWAIN_LLSelectSource},
                                                        {SELECTDEFAULTSOURCE, DTWAIN_LLSelectDefaultSource},
                                                        {SELECTSOURCEBYNAME, DTWAIN_LLSelectSourceByName},
                                                        {SELECTSOURCE2, DTWAIN_LLSelectSource2}}
                                                        };

static LONG OpenSourceInternal(DTWAIN_SOURCE Source, const SourceSelectionOptions& opts)
{
    const auto p = static_cast<CTL_ITwainSource *>(Source);
    if (p)
        p->SetSelected(true);
    else
        return DTWAIN_ERR_BAD_SOURCE;
    auto* pHandle = p->GetDTWAINHandle();
    if (opts.nOptions & DTWAIN_DLG_OPENONSELECT)
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

static DTWAIN_SOURCE SelectAndOpenSource(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions opts)
{
    const DTWAIN_SOURCE Source = SourceSelect(pHandle, opts);
    auto& sourcemap = CTL_StaticData::GetSourceStatusMap();
    if (Source)
    {
        auto pSource = static_cast<CTL_ITwainSource*>(Source);
        auto iter = sourcemap.insert({ pSource->GetProductNameA(), {} }).first;
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_SELECECTED, true);
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, false);
        const LONG retVal = OpenSourceInternal(Source, opts);
        if (retVal != DTWAIN_NO_ERROR)
        {
            if ( opts.nWhich == SELECTSOURCEBYNAME )
                CTL_TwainAppMgr::SetError(retVal, StringConversion::Convert_NativePtr_To_Ansi(opts.szProduct), false);
            return nullptr;
        }
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_OPEN, CTL_TwainAppMgr::IsSourceOpen(pSource));
        iter->second.SetThreadID(dynarithmic::getThreadIdAsString());
        iter->second.SetSourceHandle(pSource);
        CTL_StringToSourcePtrMap& m_mapTemp = pHandle->m_mapStringToSource;
        m_mapTemp[pSource->GetProductName()] = pSource;
    }

    if ( !Source && opts.nWhich == SELECTSOURCEBYNAME )
        CTL_TwainAppMgr::SetError(pHandle->m_lLastError, StringConversion::Convert_NativePtr_To_Ansi(opts.szProduct), false);
    return Source;
}

static DTWAIN_SOURCE SelectSourceHelper(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions opts, tribool bOpen)
{
    if ( indeterminate(bOpen))
    {
        if (pHandle->m_bOpenSourceOnSelect)
            opts.nOptions |= DTWAIN_DLG_OPENONSELECT;
    }
    else
    {
        if ( bOpen )
            opts.nOptions |= DTWAIN_DLG_OPENONSELECT;
    }
    return SelectAndOpenSource(pHandle, opts);
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_SOURCE Source = SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTSOURCE, IDS_SELECT_SOURCE_TEXT), {indeterminate});
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectDefaultSource()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_SOURCE Source = SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTDEFAULTSOURCE, IDS_SELECT_SOURCE_TEXT), {indeterminate});
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByName(LPCTSTR szProduct)
{
    LOG_FUNC_ENTRY_PARAMS((szProduct))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const CTL_StringType sProduct = szProduct;
    DTWAIN_SOURCE Source = SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTSOURCEBYNAME, IDS_SELECT_SOURCE_TEXT, sProduct.c_str()), { indeterminate });
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceWithOpen(DTWAIN_BOOL bOpen)
{
    LOG_FUNC_ENTRY_PARAMS((bOpen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_SOURCE Source = SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTSOURCE, IDS_SELECT_SOURCE_TEXT), bOpen?true:false);
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectDefaultSourceWithOpen(DTWAIN_BOOL bOpen)
{
    LOG_FUNC_ENTRY_PARAMS((bOpen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_SOURCE Source = SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTDEFAULTSOURCE, IDS_SELECT_SOURCE_TEXT), bOpen ? true : false);
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSourceByNameWithOpen(LPCTSTR szProduct, DTWAIN_BOOL bOpen)
{
    LOG_FUNC_ENTRY_PARAMS((szProduct, bOpen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const CTL_StringType sProduct = szProduct;
    DTWAIN_SOURCE Source = SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTSOURCEBYNAME, IDS_SELECT_SOURCE_TEXT, sProduct.c_str()), bOpen ? true : false);
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE DLLENTRY_DEF DTWAIN_SelectSource2(HWND hWndParent, 
                                                LPCTSTR szTitle, 
                                                LONG xPos, 
                                                LONG yPos, 
                                                LONG nOptions)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nOptions))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const DTWAIN_SOURCE Source = SelectAndOpenSource(pHandle, SourceSelectionOptions(SELECTSOURCE2, IDS_SELECT_SOURCE_TEXT, nullptr, hWndParent,
                                                                            szTitle, 
                                                                            xPos, yPos, 
                                                                            nullptr, 
                                                                            nullptr, 
                                                                            nullptr, 
                                                                            nOptions));
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
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
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const CTL_StringType sInclude = szIncludeFilter? szIncludeFilter : _T("");
    const CTL_StringType sExclude = szExcludeFilter ? szExcludeFilter : _T("");
    const CTL_StringType sMapping = szNameMapping ? szNameMapping : _T("");
    const DTWAIN_SOURCE Source = SelectAndOpenSource(pHandle, SourceSelectionOptions(SELECTSOURCE2, IDS_SELECT_SOURCE_TEXT, nullptr, hWndParent,
                                                                            szTitle,
                                                                            xPos, yPos,
                                                                            sInclude.c_str(),
                                                                            sExclude.c_str(),
                                                                            sMapping.c_str(), nOptions));
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceSelected(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    bool bRet = pSource->IsSelected();
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(FALSE)
}

DTWAIN_SOURCE dynarithmic::SourceSelect(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& options)
{
    LOG_FUNC_ENTRY_PARAMS((options))

    // Start a session if not already started by app
    if (!pHandle->m_bSessionAllocated)
    {
        if (!DTWAIN_StartTwainSession(nullptr, nullptr))
            LOG_FUNC_EXIT_NONAME_PARAMS((DTWAIN_SOURCE)NULL)
    }

    // Call the internal functions to select the source
    auto fnToCall = dynarithmic::generic_array_finder_if(SourcefnMap, [&](const auto& pr) { return pr.first == options.nWhich; }).second;
    const DTWAIN_SOURCE pSource = SourcefnMap[fnToCall].second(pHandle, options);

    if (!pSource)
        LOG_FUNC_EXIT_NONAME_PARAMS((DTWAIN_SOURCE)NULL)

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
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }

    DTWAINArrayLowLevel_RAII arr(pHandle, pDTWAINArray);
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
                if ( pSession )
                    pSession->DestroyOneSource(pRealSource);
            }
            CTL_TwainAppMgr::SetDefaultSource(static_cast<CTL_ITwainSource*>(pDead));
            LOG_FUNC_EXIT_NONAME_PARAMS(pDead)
        }
    }
    DTWAIN_EndTwainSession();
    LOG_FUNC_EXIT_NONAME_PARAMS((DTWAIN_SOURCE)NULL)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectSource(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& /*opt*/)
{
    LOG_FUNC_ENTRY_PARAMS(())
    // Select a source from the source dialog
    const CTL_ITwainSource *pSource = CTL_TwainAppMgr::SelectSourceDlg( pHandle->m_pTwainSession );
    // Check if a source was selected
    LOG_FUNC_EXIT_NONAME_PARAMS((DTWAIN_SOURCE)pSource)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectSource2(CTL_TwainDLLHandle* pHandle,  SourceSelectionOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    opts.getDefaultFunc = &GetDefaultName;
    opts.getNameListFunc = &GetNameList;
    CTL_StringType actualSourceName = LLSelectionDialog(pHandle, opts);
    if ( actualSourceName.empty() )
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    bool openWhenSelected = !(opts.nOptions & DTWAIN_DLG_NOOPENONSELECT)?true:false;
    DTWAIN_SOURCE Source = 
            SelectSourceHelper(pHandle, SourceSelectionOptions(SELECTSOURCEBYNAME, IDS_SELECT_SOURCE_TEXT, actualSourceName.c_str()), openWhenSelected);
    // Set the default Source
    if ( Source )
        CTL_TwainAppMgr::SetDefaultSource(static_cast<CTL_ITwainSource*>(Source));
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectSourceByName(CTL_TwainDLLHandle* pHandle,  SourceSelectionOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((pHandle, opts))
    // Select a source from the source dialog
    const CTL_ITwainSource *pSource = CTL_TwainAppMgr::SelectSource( pHandle->m_pTwainSession, opts.szProduct);
    // Check if a source was selected
    LOG_FUNC_EXIT_NONAME_PARAMS(static_cast<DTWAIN_SOURCE>(const_cast<CTL_ITwainSource *>(pSource)))
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_SOURCE dynarithmic::DTWAIN_LLSelectDefaultSource(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& /*opts*/)
{
    LOG_FUNC_ENTRY_PARAMS((pHandle))
    const CTL_ITwainSource* pSource = CTL_TwainAppMgr::GetDefaultSource(pHandle->m_pTwainSession);
    const DTWAIN_SOURCE Source = static_cast<DTWAIN_SOURCE>(const_cast<CTL_ITwainSource *>(pSource));
    LOG_FUNC_EXIT_NONAME_PARAMS(Source)
    CATCH_BLOCK(DTWAIN_SOURCE(0))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDefaultSource(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    bool bRet = CTL_TwainAppMgr::SetDefaultSource(pSource);
    // Load the resources
    auto* customProfile = CTL_StaticData::GetINIInterface();
    if (customProfile)
    {
        customProfile->SetValue("Sources", "Default", pSource->GetProductNameA().c_str());
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

static CTL_StringType GetDefaultSource()
{
    // Load the resources
    auto* customProfile = CTL_StaticData::GetINIInterface();
    if (customProfile)
    {
        const char* defSource = customProfile->GetValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCES_KEY).data(), 
                                                        CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_DEFAULT_ITEM).data());
        return StringConversion::Convert_AnsiPtr_To_Native(defSource);
    }
    return {};
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

static std::vector<TCHAR> GetDefaultName(SelectStruct& selectTraits)
{
    bool bLogMessages = (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS) ? true : false;
    bool bAlwaysHighlightFirst = selectTraits.CS.nOptions & DTWAIN_DLG_HIGHLIGHTFIRST;
    std::vector<TCHAR> DefName;
    DTWAIN_SOURCE DefSource = nullptr;
    // Try the INI setting first
    CTL_StringType sourceName = GetDefaultSource();
    if (!sourceName.empty())
    {
        DefName = std::vector<TCHAR>(sourceName.begin(), sourceName.end());
        DefName.push_back(0);
    }
    else
    {
        if (!bAlwaysHighlightFirst)
        {
            // Turn off default open temporarily
            openSourceSaver sourceSaver(selectTraits.pHandle->m_bOpenSourceOnSelect ? true : false);
            selectTraits.pHandle->m_bOpenSourceOnSelect = false;

            // Select the default source
            DefSource = SelectSourceHelper(selectTraits.pHandle, SourceSelectionOptions(SELECTDEFAULTSOURCE, IDS_SELECT_SOURCE_TEXT), { indeterminate });
        }
        if (DefSource)
        {
            closeSourceRAII cs(DefSource);
            LONG nCharacters = GetSourceInfo(static_cast<CTL_ITwainSource*>(DefSource), &CTL_ITwainSource::GetProductName, nullptr, 0);
            if (nCharacters > 0)
            {
                DefName.resize(nCharacters);
                GetSourceInfo(static_cast<CTL_ITwainSource*>(DefSource), &CTL_ITwainSource::GetProductName, DefName.data(), nCharacters);
                if (bLogMessages)
                    LogWriterUtils::WriteLogInfoIndentedA("Initializing TWAIN Dialog -- Retrieved default TWAIN Source name...");
            }
        }
    }
    return DefName;
}

static std::vector<CTL_StringType> GetNameList(SelectStruct& pS)
{
    std::vector<CTL_StringType> vSourceNames;
    // Fill the list box with the sources
    DTWAIN_ARRAY Array = nullptr;
    const auto pHandle = pS.pHandle;
    DTWAIN_EnumSources(&Array);
    DTWAINArrayLowLevel_RAII arr(pHandle, Array);
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<CTL_ITwainSource*>(Array);
    if (!vValues.empty())
    {
        TCHAR ProdName[256];

        std::transform(vValues.begin(), vValues.end(), std::back_inserter(vSourceNames),
            [&](CTL_ITwainSourcePtr ptr)
            {
                GetSourceInfo(ptr, &CTL_ITwainSource::GetProductName, ProdName, 255);
                return ProdName;
            });
    }
    return vSourceNames;
}