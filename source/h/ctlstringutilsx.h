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
#ifndef CTLSTRINGUTILSX_H
#define CTLSTRINGUTILSX_H

#include <string>
#include <vector>
#include <string_view>

namespace dynarithmic
{
    ////////////////////////////////////////////////////
    // Truncate a string if it goes over maxLen, and replace truncated contents with
    // the text "More".
    std::string TruncateStringWithMore(std::string_view origString, size_t maxLen);

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
            UCharsFromString(std::basic_string_view<typename StringType::value_type> str)
    {
        return std::vector<std::make_unsigned_t<typename StringType::value_type>>(str.begin(), str.end());
    }


    template <typename StringType, typename ByteType>
    StringType BytesToHex(const ByteType* data, std::size_t size)
    {
        using CharType = typename StringType::value_type;

        static constexpr CharType hexDigits[] =
        {
            CharType('0'), CharType('1'), CharType('2'), CharType('3'),
            CharType('4'), CharType('5'), CharType('6'), CharType('7'),
            CharType('8'), CharType('9'), CharType('a'), CharType('b'),
            CharType('c'), CharType('d'), CharType('e'), CharType('f')
        };

        StringType result;
        result.reserve(size * 2);

        for (std::size_t i = 0; i < size; ++i)
        {
            unsigned char ch = static_cast<unsigned char>(data[i]);

            result.push_back(hexDigits[ch >> 4]);
            result.push_back(hexDigits[ch & 0x0F]);
        }

        return result;
    }
    template <typename StringType>
    StringType HexStringFromUChars(const std::make_unsigned_t<typename StringType::value_type>* val, 
                                    size_t nSize)
    {
        using CharType = typename StringType::value_type;

        static constexpr CharType hexDigits[] =
        {
            CharType('0'), CharType('1'), CharType('2'), CharType('3'),
            CharType('4'), CharType('5'), CharType('6'), CharType('7'),
            CharType('8'), CharType('9'), CharType('a'), CharType('b'),
            CharType('c'), CharType('d'), CharType('e'), CharType('f')
        };

        StringType result;
        result.reserve(nSize * 2);

        for (std::size_t i = 0; i < nSize; ++i)
        {
            unsigned char ch = static_cast<unsigned char>(val[i]);

            result.push_back(hexDigits[ch >> 4]);
            result.push_back(hexDigits[ch & 0x0F]);
        }

        return result;
    }

    // If szInfo is nullptr, only the computed length is returned.
    // The length includes trailing null character.
    template <typename StringType>
    int32_t CopyInfoToCString(const StringType& strInfo, 
                              typename StringType::value_type* szInfo, 
                              int32_t nMaxLen)
    {
        using CharType = typename StringType::value_type;
        if (strInfo.empty())
        {
            if (szInfo && nMaxLen > 0)
            {
                szInfo[0] = {};
                return 1;
            }
            return 0;
        }
        if (szInfo && nMaxLen <= 0)
            return 0;

        if (nMaxLen > 0)
            --nMaxLen;
        int32_t nRealLen = 0;
        if (szInfo != nullptr && nMaxLen >= 0)
        {
            const size_t nLen = strInfo.size();
            nRealLen = static_cast<int32_t>((std::min)(static_cast<size_t>(nMaxLen), nLen));
            CharType* pEnd = std::copy(strInfo.begin(), strInfo.begin() + static_cast<size_t>(nRealLen), szInfo);
            *pEnd = CharType('\0');
        }
        else
            nRealLen = static_cast<int32_t>(strInfo.size());
        ++nRealLen;
        return nRealLen;
    }
}
#endif