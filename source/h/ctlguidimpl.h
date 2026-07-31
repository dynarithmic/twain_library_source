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
#ifndef CTLGUIDIMPL_H
#define CTLGUIDIMPL_H

#include <string>
#include <random>
#include <array>

namespace dynarithmic
{
    template <typename StringType>
    StringType GenerateUUIDv4Impl()
    {
        using char_type = typename StringType::value_type;

        static_assert(std::is_same_v<StringType, std::string> || std::is_same_v<StringType, std::wstring>,
            "StringType must be std::string or std::wstring");

        std::array<std::uint8_t, 16> bytes{};

        std::random_device rd;
        std::mt19937_64 gen(rd());

        for (std::size_t i = 0; i < bytes.size(); i += 8)
        {
            const auto value = gen();

            for (std::size_t j = 0; j < 8 && i + j < bytes.size(); ++j)
            {
                bytes[i + j] =
                    static_cast<std::uint8_t>((value >> (j * 8)) & 0xFF);
            }
        }

        // UUID version 4
        bytes[6] = static_cast<std::uint8_t>((bytes[6] & 0x0F) | 0x40);

        // RFC 4122 variant
        bytes[8] = static_cast<std::uint8_t>((bytes[8] & 0x3F) | 0x80);

        constexpr char_type hex[] =
        {
            char_type('0'), char_type('1'), char_type('2'), char_type('3'),
            char_type('4'), char_type('5'), char_type('6'), char_type('7'),
            char_type('8'), char_type('9'), char_type('a'), char_type('b'),
            char_type('c'), char_type('d'), char_type('e'), char_type('f')
        };

        std::array<char_type, 37> out{};
        std::size_t pos = 0;

        for (std::size_t i = 0; i < bytes.size(); ++i)
        {
            if (i == 4 || i == 6 || i == 8 || i == 10)
                out[pos++] = char_type('-');

            out[pos++] = hex[(bytes[i] >> 4) & 0x0F];
            out[pos++] = hex[bytes[i] & 0x0F];
        }

        out[pos] = char_type('\0');
        return StringType(out.data());
    }

    template <typename StringType>
    StringType GetGUIDImpl()
    {
        using char_type = typename StringType::value_type;
        return char_type('{') + GenerateUUIDv4Impl<StringType>() + char_type('}');
    }

#ifdef _UNICODE
    inline std::wstring GetGUID() { return GetGUIDImpl<std::wstring>(); }
    inline std::wstring GenerateUUIDv4() { return GenerateUUIDv4Impl<std::wstring>(); }
#else
    inline std::string GetGUID() { return GetGUIDImpl<std::string>(); }
    inline std::string GenerateUUIDv4() { return GenerateUUIDv4Impl<std::string>(); }
#endif
}
#endif