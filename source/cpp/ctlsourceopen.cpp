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

static void LogAndCachePixelTypes(CTL_ITwainSource *p);
static void DetermineIfSpecialXfer(CTL_ITwainSource* p);

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

    // If this source has a feeder, check the status of whether we should check.
    // If the check is on, add it to the feeder sources container.
    //
    // Since this operation may rely on the device hardware to respond to CAP_FEEDERLOADED
    // there is an optional check done that can be set in the DTWAIN INI file(s).
    if (pHandle->m_OnSourceOpenProperties.m_bCheckFeederStatusOnOpen && DTWAIN_IsFeederSensitive(Source))
        pHandle->m_aFeederSources.insert(Source);

    // Get the supported transfer types
    pSource->SetSupportedTransferMechanisms(CTL_TwainAppMgr::EnumTransferMechanisms(pSource));

    // See if the source is one that has a bug in the MSG_XFERREADY sending on the 
    // TWAIN message queue
    DetermineIfSpecialXfer(pSource);

    // Get all the caps supported
    DTWAIN_ARRAY arr = nullptr;
    DTWAINArrayPtr_RAII raii(pHandle, &arr);
    CTL_TwainAppMgr::GatherCapabilityInfo(pSource);

    // Cache the pixel types and bit depths
    LogAndCachePixelTypes(pSource);

    // get the list of caps created
    CapList& theCapList = pSource->GetCapSupportedList();

    // See if extended image info is supported and cache the results
    pSource->SetExtendedImageInfoSupported(theCapList.count(static_cast<TW_UINT16>(ICAP_EXTIMAGEINFO))?true:false);

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

void LogAndCachePixelTypes(CTL_ITwainSource *p)
{
    static constexpr int MaxMessage = 1024;
    if (p->PixelTypesRetrieved())
        return;

    p->SetCurrentlyProcessingPixelInfo(true);
    TCHAR szName[MaxMessage + 1];
    LONG oldflags = CTL_StaticData::GetLogFilterFlags();

    GetSourceInfo(p, &CTL_ITwainSource::GetProductName, szName, MaxMessage);

    std::string sName = StringConversion::Convert_NativePtr_To_Ansi(szName);
    std::string sBitDepths;
    DTWAIN_ARRAY PixelTypes;

    // enumerate all of the pixel types
    DTWAIN_BOOL bOK = DTWAIN_GetCapValues(p, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPGET, &PixelTypes);
    if (bOK)
    {
        const auto pHandle = p->GetDTWAINHandle();
        DTWAINArrayLowLevel_RAII arrP(pHandle, PixelTypes);
        auto& vPixelTypes = pHandle->m_ArrayFactory->underlying_container_t<LONG>(PixelTypes);

        LONG nCount = static_cast<LONG>(vPixelTypes.size());
        if (nCount > 0)
        {
            // create an array of 1
            DTWAIN_ARRAY vCurPixType = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 1);
            DTWAINArrayLowLevel_RAII raii(pHandle, vCurPixType);

            // get pointer to internals of the array
            auto& vCurPixTypePtr = pHandle->m_ArrayFactory->underlying_container_t<LONG>(vCurPixType);

            for (LONG i = 0; i < nCount; ++i)
            {
                // current pixel type
                vCurPixTypePtr[0] = vPixelTypes[i];
                LONG& curPixType = vCurPixTypePtr[0];
                // Set the pixel type temporarily
                if (DTWAIN_SetCapValues(p, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPSET, vCurPixType))
                {
                    // Add to source list
                    // Now get the bit depths for this pixel type
                    DTWAIN_ARRAY BitDepths = nullptr;
                    if (DTWAIN_GetCapValues(p, DTWAIN_CV_ICAPBITDEPTH, DTWAIN_CAPGET, &BitDepths))
                    {
                        DTWAINArrayLowLevel_RAII arr(pHandle, BitDepths);

                        // Get the total number of bit depths.
                        auto& vBitDepths = pHandle->m_ArrayFactory->underlying_container_t<LONG>(BitDepths);

                        LONG nCountBPP = static_cast<LONG>(vBitDepths.size());
                        if (oldflags & DTWAIN_LOG_MISCELLANEOUS)
                        {
                            StringStreamA strm;
                            strm << "\nFor source \"" << sName << "\", there are (is) " <<
                                nCountBPP << " available bit depth(s) for pixel type " <<
                                curPixType << "\n";
                            sBitDepths += strm.str();
                        }
                        LONG nCurBitDepth;
                        for (LONG j = 0; j < nCountBPP; ++j)
                        {
                            nCurBitDepth = vBitDepths[j];
                            p->AddPixelTypeAndBitDepth(curPixType, nCurBitDepth);
                            if (oldflags & DTWAIN_LOG_MISCELLANEOUS)
                            {
                                StringStreamA strm;
                                strm << "Bit depth[" << j << "] = " << nCurBitDepth << "\n";
                                sBitDepths += strm.str();
                            }
                        }
                    }
                }
            }
            DTWAIN_SetCapValues(p, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPRESET, nullptr);
        }
    }
    if (oldflags && bOK )
        LogWriterUtils::WriteMultiLineInfoIndentedA(sBitDepths, "\n");
    else
    if (!bOK)
        LogWriterUtils::WriteLogInfoIndentedA("Could not retrieve bit depth information");
    p->SetCurrentlyProcessingPixelInfo(false);
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