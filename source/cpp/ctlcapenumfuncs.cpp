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
#include "errorcheck.h"
#include "ctltmpl5.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

/////////////////////////////////////////////////////////////////////////
DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumCustomCapsEx2(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    DTWAIN_ARRAY pArray = nullptr;
    DTWAIN_EnumCustomCaps(Source, &pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(pArray)
    CATCH_BLOCK(DTWAIN_ARRAY(nullptr))
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSupportedCaps(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Array))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);

    // Check if Array is nullptr
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return Array == nullptr; },
                                      DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    auto& factory = pHandle->m_ArrayFactory;

    if (factory->is_valid(*Array))
        factory->clear(*Array);

    DTWAIN_ARRAY ThisArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 0);
    DTWAINArrayLowLevel_RAII arr(pHandle, ThisArray);
    auto& vCaps = factory->underlying_container_t<LONG>(ThisArray);

    if (ThisArray)
    {
        if (pSource->RetrievedAllCaps())
        {
            // Check if this source has had capabilities negotiated and tested
            const auto strProdName = pSource->GetProductName();
            int nWhere;
            FindFirstValue(strProdName, &pHandle->m_aSourceCapInfo, &nWhere);

            if (nWhere != -1) // Already negotiated
            {
                // Get the cap array values
                const CTL_SourceCapInfo Info = pHandle->m_aSourceCapInfo[nWhere];
                CTL_CapInfoMap *pCapInfoArray = std::get<1>(Info).get();
                std::for_each(pCapInfoArray->begin(), pCapInfoArray->end(), [&vCaps](const CTL_CapInfoMap::value_type& CapInfo)
                                { vCaps.push_back(static_cast<int>(std::get<0>(CapInfo))); });
                *Array = CreateArrayCopyFromFactory(pHandle, ThisArray);
                LOG_FUNC_EXIT_NONAME_PARAMS(true)
            }
        }

        // First time, so let's go
        const DTWAIN_ARRAY pDTWAINArray = ThisArray;
        factory->clear(pDTWAINArray);
        CTL_TwainCapArray rArray;

        // loop through all capabilities
        CTL_TwainAppMgr::GetCapabilities(pSource, rArray);

        // copy caps to our DTWAIN array
        std::copy(rArray.begin(), rArray.end(), std::back_inserter(vCaps));
        vCaps.erase(std::remove(vCaps.begin(), vCaps.end(), 0), vCaps.end());

        // Cache this information and set source's flag that all caps were retrieved
        DTWAIN_CacheCapabilityInfo(pSource, pHandle, &vCaps);
        pSource->SetRetrievedAllCaps(true);
        const bool bFound = !vCaps.empty();
        *Array = CreateArrayCopyFromFactory(pHandle, ThisArray);
        if (bFound)
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK_LOG_PARAMS(false)
}

template <typename CacheFn>
static void CopyCapsToUserArray(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, LPDTWAIN_ARRAY Array, CacheFn fn)
{
    // Create the container to copy the caps to
    DTWAIN_ARRAY ThisArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 0);

    // If anything goes wrong, make sure the array is deleted from memory
    DTWAINArrayLowLevel_RAII arr(pHandle, ThisArray);

    auto& factory = pHandle->m_ArrayFactory;

    // Now copy the caps to to the container
    auto& vCaps = factory->underlying_container_t<LONG>(ThisArray);
    auto& capCache = (pSource->*fn)();
    vCaps.insert(vCaps.begin(), capCache.begin(), capCache.end());

    // Copy the temp array to the user's copy
    *Array = CreateArrayCopyFromFactory(pHandle, ThisArray);
}

struct EnumCustomTraits
{
    static bool EnumAllCaps(CTL_ITwainSource* pSource, LPDTWAIN_ARRAY Array)
    {
        if (!pSource->RetrievedAllCaps())
        {
            if (!DTWAIN_EnumSupportedCaps(pSource, Array))
                return false;
        }
        return true;
    }
};

struct EnumExtendedTraits
{
    static bool EnumAllCaps(CTL_ITwainSource* pSource, LPDTWAIN_ARRAY /*Array*/)
    {
        pSource->RetrieveExtendedCaps();
        return true;
    }
};

template <typename EnumFn, typename CacheFn>
static int32_t EnumCaps(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array, CacheFn fn)
{
    auto pSource = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = pSource->GetDTWAINHandle();

    bool bRet = EnumFn::EnumAllCaps(pSource, Array);
    if (bRet)
        CopyCapsToUserArray(pHandle, pSource, Array, fn);
    else
        return DTWAIN_ERR_NO_CAPS_DEFINED;
    return DTWAIN_NO_ERROR;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumExtendedCaps(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Array))
    auto [pHandle, pSource] = VerifyHandles(Source);

    // Enumerate the extended caps
    auto retVal = EnumCaps<EnumExtendedTraits>(Source, Array, &CTL_ITwainSource::GetExtendedCapCache);

    // Check for any error return code
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return retVal != DTWAIN_NO_ERROR; },
                                       retVal, false, FUNC_MACRO);

    // Everything is ok
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumCustomCaps(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Array))
    auto [pHandle, pSource] = VerifyHandles(Source);

    // Enumerate the custom caps
    auto retVal = EnumCaps<EnumCustomTraits>(Source, Array, &CTL_ITwainSource::GetCustomCapCache);

    // Check for any error return code
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return retVal != DTWAIN_NO_ERROR; },
                                      retVal, false, FUNC_MACRO);

    // Everything is ok.
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCapOperations(DTWAIN_SOURCE Source, LONG lCapability, LPLONG  lpOps)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCapability, lpOps))
    if (!DTWAIN_IsCapSupported(Source, lCapability))
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    CTL_ITwainSource* pSource = static_cast<CTL_ITwainSource*> (Source);
    const auto pHandle = pSource->GetDTWAINHandle();

    CTL_CapInfo* CapInfo = GetCapInfo(pHandle, pSource, static_cast<TW_UINT16>(lCapability));
    if (!CapInfo)
    {
        *lpOps = 0;
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    *lpOps = std::get<CAPINFO_IDX_SUPPORTEDOPS>(*CapInfo);
    if (*lpOps == 0)
    {
        // Try and get the operations now from TWAIN
        *lpOps = CTL_TwainAppMgr::GetCapOps(pSource, lCapability, true);
        if (*lpOps != 0)
            // Replace 0 with what TWAIN found out about the supported operations
            std::get<CAPINFO_IDX_SUPPORTEDOPS>(*CapInfo) = *lpOps;
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpOps))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAllCapsToDefault(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    CTL_ITwainSource* pTheSource = pSource;
    DTWAIN_ARRAY a = nullptr;
    DTWAINArrayPtr_RAII arr(pHandle, &a);
    DTWAIN_EnumSupportedCaps(Source, &a);

    const CTL_CapInfoMapPtr pArray = GetCapInfoArray(pHandle, pSource);

    std::vector<int> Array;
    Array.push_back(0);
    for_each(pArray->begin(), pArray->end(), [&](const CTL_CapInfoMap::value_type& InfoVal)
    {
        const CTL_CapInfo Info = InfoVal.second;
        if (pTheSource->IsCapNegotiableInState(static_cast<TW_UINT16>(std::get<0>(Info)), pTheSource->GetState()))
        {
            if (!SetCapabilityValues(pTheSource, std::get<0>(Info), MSG_RESET, static_cast<UINT>(TwainContainer_ONEVALUE), 0,Array)) {}
        }
    });

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

