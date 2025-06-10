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
#include <chrono>
#include <iomanip>
#include <sstream>
#include "ctltwainmanager.h"
#include "ctliface.h"
#include "cppfunc.h"
#include "ctlstringutils.h"

using namespace dynarithmic;

template <typename WrapperToUse, typename PointerType>
static HANDLE ConvertToAPIString_Internal(PointerType lpOrigString)
{
    if (!lpOrigString)
        return NULL;
    return WrapperToUse::ConvertToAPIStringEx(lpOrigString);
}

template <typename WrapperToUse, typename PointerTypeIn, typename PointerTypeOut>
static LONG ConvertToAPIString_InternalEx(PointerTypeIn lpOrigString, PointerTypeOut outString, LONG nLength)
{
    if (!lpOrigString)
        return 0;
    auto retval = WrapperToUse::ConvertToAPIStringEx(lpOrigString);
    if (retval)
    {
        HandleRAII raii(retval);
        PointerTypeIn ptrData = (PointerTypeIn)raii.getData();
        auto len = WrapperToUse::traits_type::Length(ptrData);
        typename WrapperToUse::traits_type::string_type str(ptrData, len);
        return StringWrapper::CopyInfoToCString(str, outString, nLength);
    }
    return 0;
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIString(LPCTSTR lpOrigString)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString))
    auto retval = ConvertToAPIString_Internal<StringWrapper,LPCTSTR>(lpOrigString);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK((HANDLE)NULL)
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIStringA(LPCSTR lpOrigString)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString))
    auto retval = ConvertToAPIString_Internal<StringWrapperA,LPCSTR>(lpOrigString);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK((HANDLE)NULL)
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIStringW(LPCWSTR lpOrigString)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString))
    auto retval = ConvertToAPIString_Internal<StringWrapperW,LPCWSTR>(lpOrigString);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK((HANDLE)NULL)
}

LONG DLLENTRY_DEF DTWAIN_ConvertToAPIStringEx(LPCTSTR lpOrigString, LPTSTR lpOutString, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString, lpOutString, nSize))
    LONG retval = ConvertToAPIString_InternalEx<StringWrapper>(lpOrigString, lpOutString, nSize);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpOutString))
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK(0)
}

namespace dynarithmic
{
    std::string TruncateStringWithMore(std::string_view origString, size_t maxLen)
    {
        // Truncate if text is too long
        if (origString.size() > maxLen)
        {
            // Get the "More" text
            std::string MoreText = "...(" + GetResourceStringFromMap(IDS_LOGMSG_MORETEXT) + ")...";
            std::string origStringS = origString.data();

            // Get original string and resize it
            std::string tempS = origStringS.substr(0, maxLen);

            // Add the "More" text
            tempS += MoreText;
            if (tempS.size() < origString.size())
                return tempS;
        }

        // Just return the original string
        return origString.data();
    }

    std::string CreateFileNameWithDateTime(std::string_view prefix, std::string_view ext, bool useUTC)
    {
        auto now = std::chrono::system_clock::now();
        if (useUTC)
        {
            auto UTC = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
            return prefix.data() + std::to_string(UTC) + "." + ext.data();
        }
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream datetime;
        datetime << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        std::string outVal = datetime.str();
        outVal = StringWrapperA::ReplaceAll(outVal, ":", "_");
        return prefix.data() + outVal + "." + ext.data();
    }
}