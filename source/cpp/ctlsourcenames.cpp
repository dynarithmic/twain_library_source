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
#include "ctltwmgr.h"
#include "errorcheck.h"
#include "../h/cppfunc.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

LONG DLLENTRY_DEF  DTWAIN_GetCapFromName(LPCTSTR szName)
{
    LOG_FUNC_ENTRY_PARAMS((szName))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, 0, FUNC_MACRO);

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !szName; }, DTWAIN_ERR_INVALID_PARAM, 0L, FUNC_MACRO);

    const LONG Cap = CTL_TwainAppMgr::GetCapFromCapName(StringConversion::Convert_NativePtr_To_Ansi(szName).c_str());
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return Cap == -1L; }, DTWAIN_ERR_BAD_CAP, 0L, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS((long)Cap)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetNameFromCap(LONG nCapValue, LPTSTR szValue, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((nCapValue, szValue, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    const LONG nTotalBytes = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(CTL_TwainAppMgr::GetCapNameFromCap(nCapValue)), szValue, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(nTotalBytes)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetExtCapFromName(LPCTSTR szName)
{
    LOG_FUNC_ENTRY_PARAMS((szName))
    const LONG Cap = DTWAIN_GetCapFromName(szName);
    LOG_FUNC_EXIT_PARAMS((long)Cap)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetExtNameFromCap(LONG nValue, LPTSTR szValue, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((nValue, szValue, nMaxLen))
    const LONG bRet = DTWAIN_GetNameFromCap(nValue + 1000, szValue, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(-1)
}

static void GetGenericTwainName(LONG id, LPTSTR szName, const CTL_TwainLongToStringMap& twainMap)
{
    const auto iter = twainMap.find(id);
    std::string twainName;
    if (iter != twainMap.end())
        twainName = iter->second;
    StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(twainName), szName, 32);
}

static LONG GetGenericTwainValue(LPCTSTR name, const CTL_TwainLongToStringMap& twainMap)
{
    const std::string s = StringConversion::Convert_Native_To_Ansi(name);
    const auto iter = std::find_if(twainMap.begin(), twainMap.end(), [&](const CTL_TwainLongToStringMap::value_type& vt)
                                   {return vt.second == s; });
    if (iter != twainMap.end())
        return iter->first;
    return -1L;
}

BOOL DLLENTRY_DEF DTWAIN_GetTwainCountryName(LONG countryId, LPTSTR szName)
{
    LOG_FUNC_ENTRY_PARAMS((countryId, szName))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    GetGenericTwainName(countryId, szName, CTL_TwainDLLHandle::GetTwainCountryMap());
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainCountryValue(LPCTSTR country)
{
    LOG_FUNC_ENTRY_PARAMS((country))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    const auto value = GetGenericTwainValue(country, CTL_TwainDLLHandle::GetTwainCountryMap());
    LOG_FUNC_EXIT_PARAMS(value)
    CATCH_BLOCK(-1L)
}

BOOL DLLENTRY_DEF DTWAIN_GetTwainLanguageName(LONG nameId, LPTSTR szName)
{
    LOG_FUNC_ENTRY_PARAMS((nameId, szName))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    GetGenericTwainName(nameId, szName, CTL_TwainDLLHandle::GetTwainLanguageMap());
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainLanguageValue(LPCTSTR szName)
{
    LOG_FUNC_ENTRY_PARAMS((szName))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    const auto value = GetGenericTwainValue(szName, CTL_TwainDLLHandle::GetTwainLanguageMap());
    LOG_FUNC_EXIT_PARAMS(value)
    CATCH_BLOCK(-1L)
}
