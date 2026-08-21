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
#include "dtwainc.h"
#include "dtwainx.h"
#include "ctlstringutils.h"
#include "ctlstringutilsx.h"
#include "cppfunc.h"
#include <errorcheck.h>
#include "ctldtwainhandle.h"

using namespace dynarithmic;
namespace stringutils = basicstringutils;

namespace
{
    bool GenericResourceLoader(CTL_TwainDLLHandle* pHandle, LPCTSTR sLangDLL, bool bClear)
    {
        const std::string sLangDLLString = stringconversion::Convert_NativePtr_To_Ansi(sLangDLL);
        // Add the resource to the registry.
        const auto exists = pHandle->AddResourceToRegistry(sLangDLLString.c_str(), bClear).second;
        bool bRet = false;
        if (exists)
            bRet = LoadLanguageResourceA(sLangDLLString.c_str(), pHandle->GetResourceRegistry(), bClear);
        return bRet;
    }

    LONG GetResourceStringInternal(LONG resourceID, LPTSTR lpszBuffer, LONG nMaxLen)
    {
        auto actualResourceID = std::abs(resourceID);
        CTL_StringType sCopy;
        size_t nBytes = GetResourceStringA(static_cast<UINT>(actualResourceID), nullptr, DTWAIN_USERRES_MAXSIZE);
        if (nBytes == 0)
        {
            // Copy the error number to the buffer if we haven't been able to find the 
            // resource string
            sCopy = stringutils::ToString(resourceID);
            if (resourceID != DTWAIN_ERR_WIN32_ERROR)
                return CopyInfoToCString(sCopy, lpszBuffer, nMaxLen);
        }
        nBytes = DTWAIN_USERRES_MAXSIZE;
        resourceID = actualResourceID;

        size_t nAdditionalBytes = 0;
        auto& extraInfoMap = CTL_StaticData::GetExtraErrorInfoMap();
        auto iter = extraInfoMap.find(resourceID);
        if (iter != extraInfoMap.end())
            nAdditionalBytes += iter->second.size();

        std::vector<char> szTemp(nBytes, 0);
        GetResourceStringA(static_cast<UINT>(resourceID), szTemp.data(), static_cast<LONG>(nBytes));
        if (nAdditionalBytes > 0)
        {
            while (!szTemp.empty() && szTemp.back() == 0)
                szTemp.pop_back();
            szTemp.push_back(' ');
            szTemp.push_back('-');
            szTemp.push_back(' ');
            szTemp.insert(szTemp.end(), iter->second.begin(), iter->second.end());
            szTemp.push_back(0);
        }
        #if _UNICODE
        // We need to convert the error string to UTF16 for wide buffer
        auto utf16String = stringconversion::Convert_UTF8_To_UTF16(szTemp.data()).first;
        if (!utf16String.empty())
            sCopy += utf16String;
        else
            sCopy += stringconversion::Convert_Ansi_To_Native(szTemp.data(), szTemp.size());
        #else
        sCopy += stringconversion::Convert_Ansi_To_Native(szTemp.data(), szTemp.size());
        #endif
        return CopyInfoToCString(sCopy, lpszBuffer, nMaxLen);
    }
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResources(LPCTSTR sLangDLL)
    {
        LOG_FUNC_ENTRY_PARAMS((sLangDLL))
        auto bRet = DTWAIN_LoadCustomStringResourcesEx(sLangDLL, false);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesEx(LPCTSTR sLangDLL, DTWAIN_BOOL bClear)
    {
        LOG_FUNC_ENTRY_PARAMS((sLangDLL, bClear))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] { return !sLangDLL; }, DTWAIN_ERR_BLANKNAMEDETECTED, false, FUNC_MACRO);
        bool bRet = GenericResourceLoader(pHandle, sLangDLL, bClear);
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] {return !bRet; }, DTWAIN_ERR_FILEOPEN, false, FUNC_MACRO);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadLanguageResource(LONG nLanguage)
    {
        LOG_FUNC_ENTRY_PARAMS((nLanguage))
        LPCTSTR pLangDLL;
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        switch(nLanguage)
        {
            case DTWAIN_RES_ENGLISH:
                pLangDLL = _T("english");
                break;
            case DTWAIN_RES_FRENCH:
                pLangDLL = _T("french");
                break;
            case DTWAIN_RES_SPANISH:
                pLangDLL = _T("spanish");
                break;
            case DTWAIN_RES_GERMAN:
                pLangDLL = _T("german");
                break;
            case DTWAIN_RES_DUTCH:
                pLangDLL = _T("dutch");
                break;
            case DTWAIN_RES_ITALIAN:
                pLangDLL = _T("italian");
                break;
            default:
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }

        // Add the resource to the registry.
        const DTWAIN_BOOL bRet = DTWAIN_LoadCustomStringResources(pLangDLL);
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
        CATCH_BLOCK(false)
    }

    LONG DLLENTRY_DEF  DTWAIN_GetResourceString(LONG ResourceID, LPTSTR lpszBuffer, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((ResourceID, lpszBuffer, nMaxLen))
        auto nTotalBytes = GetResourceStringInternal(ResourceID, lpszBuffer, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
        LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
        CATCH_BLOCK(0)
    }

    LONG DLLENTRY_DEF  DTWAIN_GetErrorString(LONG lError, LPTSTR lpszBuffer, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((lError, lpszBuffer, nMaxLen))
        auto nTotalBytes = GetResourceStringInternal(lError, lpszBuffer, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
        LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
        CATCH_BLOCK(0)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePath(LPCTSTR ResourcePath)
    {
        LOG_FUNC_ENTRY_PARAMS((ResourcePath))
        if ( ResourcePath )
            CTL_StaticData::GetResourcePath() = ResourcePath;
        else
            CTL_StaticData::GetResourcePath() = _T("");
        LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
        CATCH_BLOCK(false)
    }

    LONG DLLENTRY_DEF DTWAIN_GetConstantFromTwainName(LPCTSTR lpszBuffer)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszBuffer))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto badValue = std::numeric_limits<LONG>::min();
        auto retVal = CTL_StaticData::GetIDFromTwainName(stringconversion::Convert_NativePtr_To_Ansi(lpszBuffer));
        LOG_FUNC_EXIT_NONAME_PARAMS(retVal.first ? static_cast<LONG>(retVal.second) : badValue);
        CATCH_BLOCK(std::numeric_limits<LONG>::min())
    }

    LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstant(LONG lConstantType, LONG lTwainConstant, LPTSTR lpszOut, LONG nSize)
    {
        LOG_FUNC_ENTRY_PARAMS((lConstantType, lTwainConstant, lpszOut, nSize))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto ret = CTL_StaticData::GetTwainNameFromConstant(lConstantType, lTwainConstant);
        if (!ret.first)
        {
            LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszOut))
            LOG_FUNC_EXIT_NONAME_PARAMS(DTWAIN_FAILURE1)
        }
        auto numChars = CopyInfoToCString(ret.second, lpszOut, nSize);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszOut))
        LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstantEx(LONG lConstantType, LONG lTwainConstant, LPTSTR lpszOut, LONG nSize)
    {
        LOG_FUNC_ENTRY_PARAMS((lConstantType, lTwainConstant, lpszOut, nSize))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto ret = CTL_StaticData::GetTwainNameFromConstant(lConstantType, lTwainConstant);
        auto numChars = CopyInfoToCString(ret.second, lpszOut, nSize);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszOut))
        LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
        CATCH_BLOCK(0)
    }
}