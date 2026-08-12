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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <array>

#include "cppfunc.h"
#include "ctlstringutils.h"
#include "ctlstringutilsx.h"
#include "ctlconstexprfind.h"
#include "ctlglobalhandletraits.h"

using namespace dynarithmic;

namespace
{
    template <typename StringType, typename PointerTypeIn = StringType::value_type*,
              typename PointerTypeOut = PointerTypeIn>
    LONG ConvertToAPIString_InternalEx(const PointerTypeIn lpOrigString, PointerTypeOut outString, LONG nLength)
    {
        if (!lpOrigString)
            return 0;
        auto retval = ConvertToAPIStringEx<StringType>(lpOrigString);
        if (retval)
        {
            HandleRAII raii(retval);
            PointerTypeIn ptrData = (PointerTypeIn)raii.getData();
            auto len = CharTraits<StringType::value_type>::Length(ptrData);
            StringType str(ptrData, len);
            return CopyInfoToCString(str, outString, nLength);
        }
        return 0;
    }

    template <typename StringType>
    StringType ConvertToAPIString(const StringType& origString)
    {
        using CharType = StringType::value_type;

        constexpr CharType CR = static_cast<CharType>('\r');
        constexpr CharType LF = static_cast<CharType>('\n');

        StringType result;
        result.reserve(origString.size());

        for (std::size_t i = 0; i < origString.size(); ++i)
        {
            if (origString[i] == LF &&
                (i == 0 || origString[i - 1] != CR))
            {
                result.push_back(CR);
            }

            result.push_back(origString[i]);
        }

        return result;
    }

    template <typename StringType>
    HANDLE ConvertToAPIStringEx(typename std::basic_string_view<typename StringType::value_type> origString)
    {
        constexpr size_t cSize = sizeof(typename StringType::value_type);
        StringType newString = ConvertToAPIString<StringType>(origString.data());
        HANDLE newHandle = GlobalAlloc(GHND | GMEM_ZEROINIT, newString.size() * cSize + cSize);
        if (newHandle)
        {
            auto pData = (typename StringType::value_type*)GlobalLock(newHandle);
            memcpy(pData, newString.data(), newString.size() * cSize);
            GlobalUnlock(newHandle);
            return newHandle;
        }
        return nullptr;
    }
}

extern "C"
{
    HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIString(LPCTSTR lpOrigString)
    {
        LOG_FUNC_ENTRY_PARAMS((lpOrigString))
            auto retval = ConvertToAPIStringEx<CTL_StringType>(lpOrigString);
        LOG_FUNC_EXIT_NONAME_PARAMS(retval)
            CATCH_BLOCK(nullptr)
    }

    HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIStringA(LPCSTR lpOrigString)
    {
        LOG_FUNC_ENTRY_PARAMS((lpOrigString))
        auto retval = ConvertToAPIStringEx<std::string>(lpOrigString);
        LOG_FUNC_EXIT_NONAME_PARAMS(retval)
        CATCH_BLOCK(nullptr)
    }


    HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIStringW(LPCWSTR lpOrigString)
    {
        LOG_FUNC_ENTRY_PARAMS((lpOrigString))
        auto retval = ConvertToAPIStringEx<std::wstring>(lpOrigString);
        LOG_FUNC_EXIT_NONAME_PARAMS(retval)
        CATCH_BLOCK(nullptr)
    }

    LONG DLLENTRY_DEF DTWAIN_ConvertToAPIStringEx(LPCTSTR lpOrigString, LPTSTR lpOutString, LONG nSize)
    {
        LOG_FUNC_ENTRY_PARAMS((lpOrigString, lpOutString, nSize))
        LONG retval = ConvertToAPIString_InternalEx<CTL_StringType>(lpOrigString, lpOutString, nSize);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpOutString))
        LOG_FUNC_EXIT_NONAME_PARAMS(retval)
        CATCH_BLOCK(0)
    }
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


    // Function to convert a two-character hex string to a byte
    static constexpr unsigned char HexCharToByte(char c) noexcept 
    {
        // create lookup table
        constexpr std::array<std::pair<char, unsigned int>, 22> hexMap =
        { {
            {'0',0},{'1',1},{'2',2},{'3',3},{'4',4},{'5',5},{'6',6},{'7', 7},{'8',8},{'9',9},
            {'A',10},{'B',11},{'C',12},{'D',13},{'E',14},{'F',15},
            {'a',10},{'b',11},{'c',12},{'d',13},{'e',14},{'f',15}
        } };

        const auto foundVal = generic_array_finder_if(hexMap, [&](const auto& pr) 
                                                            { return pr.first == c; });
        if (foundVal.first)
            return static_cast<unsigned char>(foundVal.second);
        return 0;
    }

    // convert a hex string to a byte array
    std::vector<unsigned char> HexStringToByteArray(std::string_view hexString)
    {
        std::vector<unsigned char> byteArray;
        if (hexString.size() % 2 != 0)
        {
            return byteArray;
        }
        for (size_t i = 0; i < hexString.length(); i += 2)
        {
            unsigned char highNibble = HexCharToByte(hexString[i]);
            unsigned char lowNibble = HexCharToByte(hexString[i + 1]);
            byteArray.push_back((highNibble << 4) | lowNibble);
        }
        return byteArray;
    }
}
