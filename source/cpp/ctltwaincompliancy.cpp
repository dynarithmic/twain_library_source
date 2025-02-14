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
#include "ctltwaincompliancy.h"
#include "ctltwainsource.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"

using namespace dynarithmic;

// Resets the pixel type to the original setting before the compliancy tests were run
struct ResetPixelType
{
    LONG origValue;
    CTL_ITwainSource* m_pSourceRAII;
    ResetPixelType(CTL_ITwainSource* pSource, LONG oValue) : m_pSourceRAII(pSource), origValue(oValue) {}
    ~ResetPixelType()
    {
        try
        {
            DTWAIN_ARRAY arr = CreateArrayFromFactory(m_pSourceRAII->GetDTWAINHandle(), DTWAIN_ARRAYLONG, 1);
            DTWAINArrayLowLevelPtr_RAII raii(m_pSourceRAII->GetDTWAINHandle(), &arr);
            // get pointer to internals of the array
            auto& vCurPtr = m_pSourceRAII->GetDTWAINHandle()->m_ArrayFactory->underlying_container_t<LONG>(arr);
            vCurPtr[0] = origValue;
            DTWAIN_SetCapValuesEx2(m_pSourceRAII, ICAP_PIXELTYPE, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, arr);
        }
        catch (...)
        {
        }
    }
};

std::pair<bool, int> TWAINCompliancyTester::TestPixelTypeCompliancy()
{
    if (!m_pSource)
        return { false, DTWAIN_NO_ERROR };
    auto& results = m_TestResults[TWAIN_PIXELTYPE_TESTS];
    if (results != boost::logic::indeterminate)
        return { results.value, DTWAIN_ERR_ICAPPIXELTYPE_COMPLIANCY1 };

    if (m_pSource->GetState() != SOURCE_STATE_OPENED)
        return { false, DTWAIN_ERR_INVALID_STATE };

    std::pair<bool, int> returnPair = { true, DTWAIN_NO_ERROR };

    DTWAIN_ARRAY PixelTypes = {};
    auto pHandle = m_pSource->GetDTWAINHandle();
    DTWAINArrayLowLevelPtr_RAII arrP(pHandle, &PixelTypes);
    DTWAIN_BOOL bOK = DTWAIN_GetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &PixelTypes);
    if (!bOK || !PixelTypes)
    {
        returnPair = { false, DTWAIN_ERR_ICAPPIXELTYPE_COMPLIANCY1 };
    }
    else
    {
        // get pointer to internals of the array
        auto& vCurPixTypePtr = pHandle->m_ArrayFactory->underlying_container_t<LONG>(PixelTypes);

        if (vCurPixTypePtr.empty())
            returnPair = { false, DTWAIN_ERR_ICAPPIXELTYPE_COMPLIANCY2 };

        // Get the current pixel type
        DTWAIN_ARRAY CurrentPixelType = {};
        DTWAINArrayLowLevelPtr_RAII arr2(pHandle, &CurrentPixelType);
        bOK = DTWAIN_GetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &CurrentPixelType);
        if (!bOK)
            returnPair = { false, DTWAIN_ERR_ICAPPIXELTYPE_COMPLIANCY1 };

        // get pointer to internals of the array
        auto& vCurPixTypePtr2 = pHandle->m_ArrayFactory->underlying_container_t<LONG>(CurrentPixelType);
        if (vCurPixTypePtr2.empty())
            returnPair = { false, DTWAIN_ERR_ICAPPIXELTYPE_COMPLIANCY1 };

        // Make sure we reset the pixel type on return
        ResetPixelType resetter(m_pSource, vCurPixTypePtr2[0]);

        // Test 3, 4, 5 as per TWAIN Specification Standard Capability Compliancy tests
        DTWAIN_ARRAY aBitDepths = {};
        DTWAINArrayLowLevelPtr_RAII raiiBitDepths(pHandle, &aBitDepths);

        // Pixel types to check
        static constexpr std::array<TW_UINT16, 3> aPixelTypes = { TWPT_BW, TWPT_GRAY, TWPT_RGB };

        // Invalid bit depths to check
        std::vector<std::set<int>> setBitDepths = { {24}, {1,24}, {1} };

        // Get any pixel types that do not need to go through compliancy checks
        std::vector<LONG> aOtherPixelTypes = vCurPixTypePtr;

        aOtherPixelTypes.erase(std::remove_if(aOtherPixelTypes.begin(), aOtherPixelTypes.end(),
            [&](LONG val) { return std::find(aPixelTypes.begin(), aPixelTypes.end(), val) != aPixelTypes.end(); }),
            aOtherPixelTypes.end());

        size_t curSet = 0;
        for (auto val : aPixelTypes)
        {
            vCurPixTypePtr2[0] = val;
            // Set the pixel type
            bOK = DTWAIN_SetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, CurrentPixelType);
            if (bOK)
            {
                // Get the current bit depths
                bOK = DTWAIN_GetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPBITDEPTH, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aBitDepths);
                auto& vCurBitDepths = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aBitDepths);
                if (vCurBitDepths.empty())
                    returnPair = { false, DTWAIN_ERR_ICAPBITDEPTH_COMPLIANCY1 };

                // Check if list of bit depths do not contain the invalid bit depth
                if (std::find_if(vCurBitDepths.begin(), vCurBitDepths.end(),
                    [&](LONG val) { return setBitDepths[curSet].count(val); }) != vCurBitDepths.end())
                    returnPair = { false, DTWAIN_ERR_ICAPBITDEPTH_COMPLIANCY1 };
                for (auto curBitDepth : vCurBitDepths)
                    m_pSource->AddPixelTypeAndBitDepth(val, curBitDepth);
            }
            ++curSet;
        }

        // Process other bit depths
        for (auto val : aOtherPixelTypes)
        {
            vCurPixTypePtr2[0] = val;
            // Set the pixel type
            bOK = DTWAIN_SetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, CurrentPixelType);
            if (bOK)
            {
                // Get the current bit depths
                bOK = DTWAIN_GetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPBITDEPTH, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aBitDepths);
                auto& vCurBitDepths = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aBitDepths);
                for (auto curBitDepth : vCurBitDepths)
                    m_pSource->AddPixelTypeAndBitDepth(val, curBitDepth);
            }
        }
    }
    m_ResultCode[TWAIN_PIXELTYPE_TESTS] = returnPair.second;
    m_TestResults[TWAIN_PIXELTYPE_TESTS] = returnPair.first;
    return returnPair;
}

std::pair<bool, int> TWAINCompliancyTester::TestXfermechCompliancy()
{
    if (!m_pSource)
        return { false, DTWAIN_NO_ERROR };
    auto& results = m_TestResults[TWAIN_XFERMECH_TESTS];
    if (results != boost::logic::indeterminate)
        return { results.value, m_ResultCode[TWAIN_XFERMECH_TESTS]};

    if (m_pSource->GetState() != SOURCE_STATE_OPENED)
        return { false, DTWAIN_ERR_INVALID_STATE };

    std::pair<bool, int> returnPair = { true, DTWAIN_NO_ERROR };

    DTWAIN_ARRAY XFerMechs = {};
    auto pHandle = m_pSource->GetDTWAINHandle();
    DTWAINArrayLowLevelPtr_RAII arrP(pHandle, &XFerMechs);
    DTWAIN_BOOL bOK = DTWAIN_GetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPXFERMECH, DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &XFerMechs);
    if (!bOK || !XFerMechs)
    {
        returnPair = { false, DTWAIN_ERR_XFERMECH_COMPLIANCY };
    }
    else
    {
        // get pointer to internals of the array
        auto& vCurXFerMechs = pHandle->m_ArrayFactory->underlying_container_t<LONG>(XFerMechs);

        if (vCurXFerMechs.size() < 2)
            returnPair = { false, DTWAIN_ERR_XFERMECH_COMPLIANCY };
    }
    m_ResultCode[TWAIN_XFERMECH_TESTS] = returnPair.second;
    m_TestResults[TWAIN_XFERMECH_TESTS] = returnPair.first;
    return returnPair;
}

std::pair<bool, int> TWAINCompliancyTester::TestStandardCapabilitiesCompliancy()
{
    if (!m_pSource)
        return { false, DTWAIN_NO_ERROR };
    auto& results = m_TestResults[TWAIN_CAPABILITY_TESTS];
    if (results != boost::logic::indeterminate)
        return { results.value, m_ResultCode[TWAIN_CAPABILITY_TESTS] };

    if (m_pSource->GetState() != SOURCE_STATE_OPENED)
        return { false, DTWAIN_ERR_INVALID_STATE };

    std::pair<bool, int> returnPair = { true, DTWAIN_NO_ERROR };

    auto& pixelTypeMap = m_pSource->GetPixelTypeMap();
    auto pHandle = m_pSource->GetDTWAINHandle();

    // Get the current pixel type
    DTWAIN_ARRAY CurrentPixelType = {};
    DTWAINArrayLowLevelPtr_RAII arr2(pHandle, &CurrentPixelType);
    bool bOK = DTWAIN_GetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &CurrentPixelType);
    if (!bOK)
        returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };

    // get pointer to internals of the array
    auto& vCurPixTypePtr2 = pHandle->m_ArrayFactory->underlying_container_t<LONG>(CurrentPixelType);
    if (vCurPixTypePtr2.empty())
        returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };

    // Make sure we reset the pixel type on return
    ResetPixelType resetter(m_pSource, vCurPixTypePtr2[0]);

    for (auto& pr : pixelTypeMap)
    {
        vCurPixTypePtr2[0] = pr.first;
        bOK = DTWAIN_SetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, CurrentPixelType);
        if (!bOK)
        {
            returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
            break;
        }
        bOK = DTWAIN_SetCapValuesEx2(m_pSource, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPRESETALL, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, CurrentPixelType);
        if (!bOK)
        {
            auto conditionCode = CTL_TwainAppMgr::GetLastConditionCodeError();
            if (conditionCode == TWCC_BADPROTOCOL)
                continue;
            returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
            break;
        }
    }
    if (returnPair.first)
    {
        // Try each supported cap
        auto& allCaps = m_pSource->GetCapSupportedList();
        for (auto& oneCap : allCaps)
        {
            if (oneCap >= CAP_CUSTOMBASE)
                continue;
            const auto& ops = CTL_TwainAppMgr::GetCapabilityOperations(m_pSource, oneCap);
            if ( ops.GetSupport() == 0 )
            { 
                returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                break;
            }
            // check MSG_GET, MSG_GETCURRENT, MSG_GETDEFAULT
            if (ops.IsGet() || ops.IsGetCurrent() || ops.IsGetDefault())
            {
                bOK = (ops.IsGet() && ops.IsGetCurrent() && ops.IsGetDefault());
                if (!bOK)
                {
                    returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                    break;
                }
            }
            if (ops.IsSet() || ops.IsReset())
            {
                bOK = (ops.IsSet() && ops.IsReset() && ops.IsGet() && ops.IsGetCurrent() && ops.IsGetDefault());
                if (!bOK)
                {
                    returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                    break;
                }
            }
            std::array<bool, 3> getops = { ops.IsGet(), ops.IsGetCurrent(), ops.IsGetDefault() };
            static constexpr std::array<LONG, 3> optsToUse = { DTWAIN_CAPGET, DTWAIN_CAPGETCURRENT, DTWAIN_CAPGETDEFAULT };
            if (returnPair.first)
            {
                int curOp = 0;
                for (auto op : getops)
                {
                    if (!op)
                    {
                        ++curOp;
                        continue;
                    }
                    DTWAIN_ARRAY arrTest = {};
                    DTWAINArrayLowLevelPtr_RAII arrTestRAII(pHandle, &arrTest);
                    bOK = DTWAIN_GetCapValuesEx2(m_pSource, oneCap, optsToUse[curOp], DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arrTest);
                    if (!bOK)
                    {
                        auto conditionCode = CTL_TwainAppMgr::GetLastConditionCodeError();
                        if (conditionCode == TWCC_SEQERROR)
                            continue;
                        else
                        {
                            returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                            break;
                        }
                    }
                    ++curOp;
                }

                // Test the reset
                std::array<bool, 1> resetops = { ops.IsReset() };
                static constexpr std::array<LONG, 1> resetoptsToUse = { DTWAIN_CAPRESET };
                curOp = 0;
                for (auto op : resetops)
                {
                    if (!op)
                    {
                        ++curOp;
                        continue;
                    }
                    bOK = DTWAIN_SetCapValuesEx2(m_pSource, oneCap, resetoptsToUse[curOp], DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, NULL);
                    if (!bOK)
                    {
                        returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                        break;
                    }
                    ++curOp;
                }

                // Test get and set in combination
                if (ops.IsSet() && ops.IsGet())
                {
                    curOp = 0;
                    for (auto op : getops)
                    {
                        DTWAIN_ARRAY arrTest = {};
                        DTWAINArrayLowLevelPtr_RAII arrTestRAII(pHandle, &arrTest);
                        bOK = DTWAIN_GetCapValuesEx2(m_pSource, oneCap, op, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arrTest);
                        if (!bOK)
                        {
                            returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                            break;
                        }
                        // Test the set
                        bOK = DTWAIN_SetCapValuesEx2(m_pSource, oneCap, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, arrTest);
                        if (!bOK)
                        {
                            auto lastTwainError = CTL_TwainAppMgr::GetLastTwainError();
                            auto conditionCode = CTL_TwainAppMgr::GetLastConditionCodeError();
                            if (conditionCode == TWCC_SEQERROR || conditionCode == TWCC_SUCCESS || lastTwainError == TWRC_CHECKSTATUS)
                                continue;
                            returnPair = { false, DTWAIN_ERR_STANDARDCAPS_COMPLIANCY };
                            break;
                        }
                    }
                }
            }
        }
    }
    m_ResultCode[TWAIN_CAPABILITY_TESTS] = returnPair.second;
    m_TestResults[TWAIN_CAPABILITY_TESTS] = returnPair.first;
    return returnPair;
}
