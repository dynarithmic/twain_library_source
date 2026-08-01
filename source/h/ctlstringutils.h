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
#ifndef CTLSTRINGUTILS_H
#define CTLSTRINGUTILS_H

#include <string>
#include <vector>
#include <string_view>
#include <iterator>
#include <boost/algorithm/hex.hpp>
#include <windows.h>

// Widening string macros
#define WIDEN2(x) L##x
#define WIDEN(x)  WIDEN2(x)

namespace dynarithmic
{
    // Truncate a string if it goes over maxLen, and replace truncated contents with
    // the text "More".
    std::string TruncateStringWithMore(std::string_view origString, size_t maxLen);

    // Create a file using the data and time within the file name
    std::string CreateFileNameWithDateTime(std::string_view prefix, std::string_view ext, bool useUTC=false);

    // Convert a string into a byte array
    std::vector<unsigned char> HexStringToByteArray(std::string_view hexString);

    // Search and replace %1, %2, etc. placeholders with data
    template <typename StringType, typename Container=std::vector<StringType>>
    StringType ReplacePlaceHolders(const StringType& fmt, const Container& values)
    {
        using char_type = typename StringType::value_type;

        StringType result;
        result.reserve(fmt.size());

        for (std::size_t i = 0; i < fmt.size(); ++i)
        {
            if (fmt[i] == char_type('%') && i + 1 < fmt.size())
            {
                std::size_t j = i + 1;
                std::size_t index = 0;

                while (j < fmt.size() &&
                    fmt[j] >= char_type('0') &&
                    fmt[j] <= char_type('9'))
                {
                    index = (index * 10) + static_cast<std::size_t>(fmt[j] - char_type('0'));
                    ++j;
                }

                if (index >= 1 && index <= values.size())
                {
                    result += values[index - 1];
                    i = j - 1;
                    continue;
                }
            }

            result += fmt[i];
        }

        return result;
    }

    template <typename StringType>
    StringType StringFromUChars(const std::make_unsigned_t<typename StringType::value_type>* val, std::size_t nSize)
    {
        if (!val || nSize == 0)
            return {};

        return StringType(val, val + nSize);
    }

    template <typename StringType>
    std::vector<std::make_unsigned_t<typename StringType::value_type>> 
            UCharsFromString(typename std::basic_string_view<typename StringType::value_type> str)
    {
        return std::vector<std::make_unsigned_t<typename StringType::value_type>>(str.begin(), str.end());
    }

    template <typename StringType>
    StringType HexStringFromUChars(const std::make_unsigned_t<typename StringType::value_type>* val, size_t nSize)
    {
        StringType hex_output_vector;
        boost::algorithm::hex_lower(val, val + nSize, std::back_inserter(hex_output_vector));
        return hex_output_vector;
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
#endif


