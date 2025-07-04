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
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include <algorithm>

#include "cppfunc.h"
#include "errorcheck.h"
#include "ctlsetgetcaps.h"

using namespace dynarithmic;

typedef void (CTL_ITwainSource::*SetFunc)(bool);
typedef bool (CTL_ITwainSource::*IsEnabledFunc)() const;

static bool EnableFeederFunc(DTWAIN_SOURCE Source, LONG lCap, CTL_ITwainSource* p, SetFunc Func, bool bSet);
static bool IsFeederEnabledFunc(DTWAIN_SOURCE Source, IsEnabledFunc Func);
static bool ExecuteFeederState5Func(DTWAIN_SOURCE Source, LONG lCap);
static VOID CALLBACK ThisTimerProc(HWND hwnd, UINT uMsg, ULONG idEvent,DWORD dwTime);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsFeederSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))

    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto getSupport = pSource->IsFeederSupported();

    // If status of feeder support already determined, return result.
    if (getSupport.value != boost::tribool::indeterminate_value)
        LOG_FUNC_EXIT_NONAME_PARAMS(getSupport ? true : false)

    BOOL bRet = false;

    // Check if feeder has been enabled.  If so, then device has a feeder
    DTWAIN_ARRAY arr = nullptr;
    const BOOL bOk = GetCapValuesEx2_Internal(pSource, CAP_FEEDERENABLED, 
                                              DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &arr);
    if (!bOk)
    {
        pSource->SetFeederSupported(false);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    DTWAINArrayLowLevel_RAII a(pHandle, arr);

    auto& vFeeder = pHandle->m_ArrayFactory->underlying_container_t<LONG>(arr);
    if (vFeeder.empty())
    {
        pSource->SetFeederSupported(false);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    if (vFeeder[0] == 1)
    {
        pSource->SetFeederSupported(true);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    // Enable the feeder temporarily to test if setting it will work.
    vFeeder[0] = 1;
    bRet = SetCapValuesEx2_Internal(pSource, CAP_FEEDERENABLED, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, arr);
    if (!bRet)
    {
        pSource->SetFeederSupported(false);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    // Disable the feeder
    vFeeder[0] = 0; 
    bRet = SetCapValuesEx2_Internal(pSource, CAP_FEEDERENABLED, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, arr);
    pSource->SetFeederSupported(bRet);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsFeederLoaded(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if ( DTWAIN_IsFeederSupported(Source) )
    {
        CTL_ITwainSource* pSource = static_cast<CTL_ITwainSource*>(Source);
        const auto pHandle = pSource->GetDTWAINHandle();
        DTWAIN_ARRAY a = nullptr;
        const DTWAIN_BOOL bReturn = GetCapValuesEx2_Internal(pSource, CAP_FEEDERLOADED, 
                                                             DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &a);
        DTWAINArrayLowLevel_RAII arr(pHandle, a);
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !bReturn;}, DTWAIN_ERR_NO_FEEDER_QUERY, false, FUNC_MACRO);
        auto& vFeeder = pHandle->m_ArrayFactory->underlying_container_t<LONG>(a);
        LONG Val = 0;
        if ( !vFeeder.empty() )
            Val = vFeeder.front();
        LOG_FUNC_EXIT_NONAME_PARAMS(Val)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

//*********************************************************************************************
DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_IsFeederEnabled(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if (DTWAIN_IsFeederSupported(Source))
    {
        const DTWAIN_BOOL bRet = IsFeederEnabledFunc(Source, &CTL_ITwainSource::IsFeederEnabledMode);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_IsAutoFeedEnabled(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if (DTWAIN_IsAutoFeedSupported(Source))
    {
        const DTWAIN_BOOL bRet = IsFeederEnabledFunc(Source, &CTL_ITwainSource::GetAutoFeedMode);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

bool IsFeederEnabledFunc(DTWAIN_SOURCE Source, IsEnabledFunc Func)
{
    CTL_ITwainSource* p = static_cast<CTL_ITwainSource*>(Source);
    return (p->*Func)();
}
//*********************************************************************************************
DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_EnableFeeder(DTWAIN_SOURCE Source, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bSet))
    // Check if feeder supported
    if ( !DTWAIN_IsFeederSupported(Source))
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    auto pSource = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = pSource->GetDTWAINHandle();

    // Call general function to enable feeder
    const bool bRet = EnableFeederFunc(Source, CAP_FEEDERENABLED, pSource,
                                       &CTL_ITwainSource::SetFeederEnableMode, bSet?true:false);

    // Enable the feeder state 5 and 6 functionality here
    if ( bSet )
    {
        DTWAIN_ARRAY aExtendedCaps = nullptr;
        CHECK_IF_CAP_SUPPORTED(pSource, pHandle, CAP_EXTENDEDCAPS, bRet)
        const DTWAIN_BOOL bOk = GetCapValuesEx2_Internal(pSource, CAP_EXTENDEDCAPS, 
                                                DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aExtendedCaps);
        if ( bOk && aExtendedCaps )
        {
            LONG Caps[] = { CAP_FEEDPAGE, CAP_REWINDPAGE, CAP_CLEARPAGE };
            DTWAINArrayLowLevel_RAII arr(pHandle, aExtendedCaps);
            auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aExtendedCaps);
            const size_t oldSize = vCaps.size();
            std::for_each(std::begin(Caps), std::end(Caps), [&](LONG n)
            {
                if (pSource->IsExtendedCapNegotiable(n) && std::find(vCaps.begin(), vCaps.end(), n) == vCaps.end())
                    vCaps.push_back(n);
            });
            if (vCaps.size() > oldSize)
                SetCapValuesEx2_Internal( pSource, CAP_EXTENDEDCAPS, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, aExtendedCaps);
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnableAutoFeed(DTWAIN_SOURCE Source, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bSet))
    // Check if feeder supported
    if (!DTWAIN_IsAutoFeedSupported(Source))
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    CTL_ITwainSource* p = static_cast<CTL_ITwainSource*>(Source);
    const bool bRet = EnableFeederFunc(Source, CAP_AUTOFEED, p,
                                       &CTL_ITwainSource::SetAutoFeedMode, bSet ? true : false);
    // Call general function to enable feeder
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

bool EnableFeederFunc(DTWAIN_SOURCE Source, LONG lCap, CTL_ITwainSource* pSource, SetFunc Func, bool bSet)
{
    // Set the capability value
    DTWAIN_ARRAY aValues = nullptr;

    // Check the current value
    const auto pHandle = pSource->GetDTWAINHandle();
    DTWAIN_BOOL bReturn = GetCapValuesEx2_Internal(pSource, lCap, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aValues);
    if (!bReturn)
        return false;
    DTWAINArrayLowLevel_RAII arr(pHandle, aValues);
    auto& vFeeder = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);

    if ( vFeeder.empty() )
        return false;

    const bool Val = vFeeder[0]?true:false;
    bool bValue=false;
    if ( bSet != 0 )
        bValue = true;

    // Already set, so return
    if ( bValue == Val )
    {
        (pSource->*Func)(bValue);
        return true;
    }

    // Set here
    vFeeder[0] = bValue;
    bReturn = SetCapValuesEx2_Internal( pSource, lCap, DTWAIN_CAPSETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, aValues)?true:false;
    if ( bReturn )
        (pSource->*Func)(bValue);
    else
        (pSource->*Func)(FALSE);
    return true;
}
//*********************************************************************************************
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsFeederSensitive(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto getSupport = pSource->IsFeederSensitive();

    // If status of feeder paper sensor support already determined, return result.
    if (getSupport.value != boost::tribool::indeterminate_value)
        LOG_FUNC_EXIT_NONAME_PARAMS(getSupport ? true : false)

    bool bRet = false;

    // Check if paper detectable
    bRet = DTWAIN_IsPaperDetectable(Source);
    pSource->SetFeederSensitive(bRet);

    // We will see if the source is compliant
    if (!bRet && !pSource->IsCapInSupportedList(CAP_PAPERDETECTABLE))
    {
        BOOL bSupported = DTWAIN_IsFeederSupported(Source);
        if ( bSupported )
        {
            CTL_TwainAppMgr::SetError(DTWAIN_ERR_FEEDER_COMPLIANCY, pSource->GetProductNameA().c_str(), false);
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

//*********************************************************************************************
// Only called in State 5
DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_FeedPage(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if ( !DTWAIN_IsFeederEnabled(Source) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    const DTWAIN_BOOL bRet = ExecuteFeederState5Func(Source, CAP_FEEDPAGE)?1:0;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_RewindPage(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if ( !DTWAIN_IsFeederEnabled(Source) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    const DTWAIN_BOOL bRet = ExecuteFeederState5Func(Source, CAP_REWINDPAGE)?1:0;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_ClearPage(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    if ( !DTWAIN_IsFeederEnabled(Source) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    const DTWAIN_BOOL bRet = ExecuteFeederState5Func(Source, CAP_CLEARPAGE)?1:0;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetFeederFuncs(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    constexpr LONG Caps[] = { CAP_FEEDPAGE, CAP_CLEARPAGE, CAP_REWINDPAGE };
    LONG ReturnVal = 0;
    for ( int i = 0; i < 3; i++ )
    {
        if (DTWAIN_IsCapSupported(Source,Caps[i]))
            ReturnVal |= i << 1;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(ReturnVal)
    CATCH_BLOCK(0)
}

bool ExecuteFeederState5Func(DTWAIN_SOURCE Source, LONG lCap)
{
    CTL_ITwainSource* pSource = static_cast<CTL_ITwainSource*>(Source);
    if (!pSource->IsCapInSupportedList(lCap))
        return false;

    const auto pHandle = pSource->GetDTWAINHandle();
    DTWAIN_ARRAY aValues = CreateArrayFromCap(pHandle, nullptr, lCap, 0);
    if ( !aValues )
        return false;
    DTWAINArrayLowLevel_RAII aRAII(pHandle, aValues);
    auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
    vCaps.push_back(true);
    SetCapValuesEx2_Internal(pSource, lCap, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, aValues);
    return false;
}

///////////////////////////////////////////////////////////
VOID CALLBACK ThisTimerProc(HWND, UINT, ULONG idEvent, DWORD)
{
    return;
    // This is experimental code that unfortunately does not work
    // consistently, thus it is commented out.
    #if 0
    #ifdef _WIN32
    // Make sure that user set a callback
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const auto pFn = DTWAIN_GetCallback();
    if ( !pFn )
        return;

    if ( idEvent == CTL_TwainDLLHandle::s_nTimerID )
    {
        // Check if any open source supports feeder
        auto it = pHandle->m_aFeederSources.begin();
        const auto it2 = pHandle->m_aFeederSources.end();

        while ( it != it2 )
        {
            const auto pSource = static_cast<CTL_ITwainSource*>(*it);
            const auto sourceState = pSource->GetState();
            if ( sourceState != SOURCE_STATE_XFERREADY &&
                 sourceState != SOURCE_STATE_TRANSFERRING)
            {
                if (DTWAIN_IsFeederLoaded( *it ))
                    (*pFn)(DTWAIN_TN_FEEDERLOADED, reinterpret_cast<LPARAM>(*it), pHandle->m_lCallbackData);
                else
                    (*pFn)(DTWAIN_TN_FEEDERNOTLOADED, reinterpret_cast<LPARAM>(*it), pHandle->m_lCallbackData);
            }
            ++it;
        }
    }
    #endif
    #endif
}
