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
#include "arrayfactory.h"
#include "errorcheck.h"
#include "../wildcards/wildcards.hpp"
#include "dtwain_paramlogger.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

static void TestAndCachePixelTypes(CTL_ITwainSource *p);
static void DetermineIfSpecialXfer(CTL_ITwainSource* p);
static void DetermineIfPeekMessage(CTL_ITwainSource* p);
static void DetermineIfPaperDetectable(CTL_ITwainSource* p);
static std::pair<bool, int> PerformPixelTypeCompliancyTest(CTL_ITwainSource * p);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_OpenSourcesOnSelect(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    pHandle->m_bOpenSourceOnSelect = bSet ? true : false;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsOpenSourcesOnSelect(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const bool retVal = pHandle->m_bOpenSourceOnSelect;
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_OpenSource(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);

    // If source already opened, just return TRUE.
    if (pSource->IsOpened())
        LOG_FUNC_EXIT_NONAME_PARAMS(true)

    // Set up the opening of the source

    // Go through TWAIN to open the source
    bool bRetval = CTL_TwainAppMgr::OpenSource(pHandle->m_pTwainSession, pSource);
    if (bRetval)
    {
        // Set up status of the source 
        auto& sourcemap = CTL_StaticData::GetSourceStatusMap();
        auto iter = sourcemap.insert({ pSource->GetProductNameA(), {} }).first;
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_OPEN, true);
        iter->second.SetStatus(SourceStatus::SOURCE_STATUS_UNKNOWN, false);
    }

    // Check for failure to open the source
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return bRetval == false; }, DTWAIN_ERR_SOURCE_COULD_NOT_OPEN, false, FUNC_MACRO);

    // Get all the caps supported
    DTWAIN_ARRAY arr = nullptr;
    DTWAINArrayPtr_RAII raii(pHandle, &arr);
    CTL_TwainAppMgr::GatherCapabilityInfo(pSource);

    // See if there is an override for paper detectable
    DetermineIfPaperDetectable(pSource);

    // If this source has a feeder, check the status of whether we should check.
    // If the check is on, add it to the feeder sources container.
    //
    // Since this operation may rely on the device hardware to respond to CAP_PAPERDETECTABLE
    // there is an optional check done that can be set in the DTWAIN INI file(s).
    if (pHandle->m_OnSourceOpenProperties.m_bCheckFeederStatusOnOpen && DTWAIN_IsPaperDetectable(Source))
        pHandle->m_aFeederSources.insert(Source);

    // Get the supported transfer types
    pSource->SetSupportedTransferMechanisms(CTL_TwainAppMgr::EnumTransferMechanisms(pSource));

    // Get the supported DAT types
    pSource->SetSupportedDATS(CTL_TwainAppMgr::EnumSupportedDATS(pSource));

    // See if the source is one that has a bug in the MSG_XFERREADY sending on the 
    // TWAIN message queue
    DetermineIfSpecialXfer(pSource);

    // See if the source uses PeekMessage processing for the TWAIN message loop
    DetermineIfPeekMessage(pSource);

    // Cache the pixel types and bit depths
    TestAndCachePixelTypes(pSource);

    // get the list of caps created
    CapList& theCapList = pSource->GetCapSupportedList();

    // See if extended image info is supported and cache the results
    pSource->SetExtendedImageInfoSupported(theCapList.count(static_cast<TW_UINT16>(ICAP_EXTIMAGEINFO))?true:false);

    // Cache the supported TWEI_x values for extended image information
    if (pSource->IsExtendedImageInfoSupported())
    {
        DTWAIN_ARRAY extArray = {};
        DTWAINArrayPtr_RAII temp_(pHandle, &extArray);
        bool bOk = DTWAIN_GetCapValuesEx2(pSource, ICAP_SUPPORTEDEXTIMAGEINFO, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &extArray);
        if (bOk && extArray)
        {
            auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(extArray);
            pSource->SetSupportedExtImageInfos(vValues);
            pSource->SetSupportedExtImageInfoCap(true);
        }
        else
            pSource->SetSupportedExtImageInfoCap(false);
    }

    // See if audio transfers are supported
    pSource->SetAudioTransferSupported(DTWAIN_IsAudioXferSupported(Source, DTWAIN_ANYSUPPORT)?true:false);

    // if any logging is turned on, then get the capabilities and log the values
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
    {
        CTL_StringType msg = _T("Source: ") + pSource->GetProductName() + _T(" has been opened successfully");
        LogWriterUtils::WriteLogInfoIndented(msg);

        // Log the caps if logging is turned on
        CTL_StringType sName;

        std::vector<std::string> VecString(theCapList.size());

        // copy the names
        std::transform(theCapList.begin(), theCapList.end(), VecString.begin(), [](LONG n) { return CTL_TwainAppMgr::GetCapNameFromCap(n); });

        // Sort the names
        std::sort(VecString.begin(), VecString.end());
        CTL_StringStreamType strm;
        strm << theCapList.size();
        sName = _T("\n\n");
        sName += GetResourceStringFromMap_Native(IDS_LOGMSG_CAPABILITYLISTING);
        sName += _T(" (") + pSource->GetProductName() + _T(")");
        sName += _T(" (") + strm.str() + _T("):\n{\n");
        if (theCapList.empty())
            sName += _T(" No capabilities:\n");
        else
        {
            sName += _T("    ");
            sName += StringConversion::Convert_Ansi_To_Native(StringWrapperA::Join(VecString, "\n    "));
        }
        sName += _T("\n}");

        LogWriterUtils::WriteMultiLineInfoIndented(sName, _T("\n"));
    }

    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceOpen(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if ( !Source )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    auto [pHandle, pSource] = VerifyHandles(Source);
    const DTWAIN_BOOL bRet = CTL_TwainAppMgr::IsSourceOpen(pSource);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

void TestAndCachePixelTypes(CTL_ITwainSource* p)
{
    struct ProcessingRAII
    {
        CTL_ITwainSource* m_pSource;
        ProcessingRAII(CTL_ITwainSource* pSource) : m_pSource(pSource) {}
        ~ProcessingRAII() { m_pSource->SetCurrentlyProcessingPixelInfo(false); }
    };

    static constexpr int MaxMessage = 1024;
    if (p->PixelTypesRetrieved())
        return;

    p->SetCurrentlyProcessingPixelInfo(true);
    ProcessingRAII raii(p);

    // Do pixel type compliancy and retrieval test.
    auto& compliancyTester = p->GetCompliancyTester();
    auto pr = compliancyTester.TestPixelTypeCompliancy();
    if (!pr.first)
    {
        // TWAIN source is not compliant with respect to pixel type
        if (CTL_StaticData::GetLogFilterFlags())
        {
            std::string s1 = GetResourceStringFromMap(pr.second);
            s1 += " - " + StringWrapperA::QuoteString(p->GetProductNameA());
            LogWriterUtils::WriteLogInfoIndentedA(s1);
        }
    }

    // Print information on the pixel types found
    const auto& pixelBitDepthMap = p->GetPixelTypeMap();

    StringStreamA strm;
    std::string sBitDepths;
    LONG oldflags = CTL_StaticData::GetLogFilterFlags();
    for (auto& mapPr : pixelBitDepthMap)
    {
        strm.str("");
        strm << "\nFor source \"" << p->GetProductNameA() << "\", there are (is) " <<
            mapPr.second.size() << " available bit depth(s) for pixel type " <<
            CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_TWPT, mapPr.first).second << "\n";
        size_t j = 0;
        for (auto& val : mapPr.second)
        {
            strm << "Bit depth[" << j << "] = " << val << "\n";
            ++j;
        }
        sBitDepths += strm.str();
    }
    if (oldflags)
        LogWriterUtils::WriteMultiLineInfoIndentedA(sBitDepths, "\n");
}

void DetermineIfSpecialXfer(CTL_ITwainSource* p)
{
    using wildcards::match;
    auto& xfer_map = CTL_TwainAppMgr::GetSourceToXferReadyMap();
    auto& xfer_list= CTL_TwainAppMgr::GetSourceToXferReadyList();
    std::string sourceName = p->GetProductNameA();
    auto iter = xfer_map.find(sourceName);

    // Already in map
    if (iter != xfer_map.end())
        return;

    // Search vector for a matching name
    auto iterSearch = xfer_list.begin();
    while (iterSearch != xfer_list.end())
    {
        bool matches = match(sourceName, iterSearch->first);
        if (matches)
        {
            // Add this source as one that will require special MSG_XFERREADY processing
            auto insertPr = xfer_map.insert({ sourceName, {} });
            insertPr.first->second.m_MaxThreshold = iterSearch->second;
            return;
        }
        ++iterSearch;
    }
}

void DetermineIfPaperDetectable(CTL_ITwainSource* p)
{
    using wildcards::match;
    auto& paperdetectable_map = CTL_TwainAppMgr::GetSourcePaperDetectionMap();
    std::string sourceName = p->GetProductNameA();

    // Search map for a matching name
    auto iterSearch = paperdetectable_map.begin();
    while (iterSearch != paperdetectable_map.end())
    {
        bool matches = match(sourceName, iterSearch->first);
        if (matches)
        {
            p->SetFeederSensitive(iterSearch->second);
            return;
        }
        ++iterSearch;
    }
}

void DetermineIfPeekMessage(CTL_ITwainSource* pSource)
{
    using wildcards::match;
    auto& peekmsg_list = CTL_TwainAppMgr::GetSourcePeekMessageList();
    std::string sourceName = pSource->GetProductNameA();
    
    // Search vector for a matching name
    auto iterSearch = peekmsg_list.begin();
    while (iterSearch != peekmsg_list.end())
    {
        bool matches = match(sourceName, *iterSearch);
        if (matches)
        {
            pSource->SetUsePeekMessage(true);
            return;
        }
        ++iterSearch;
    }
}