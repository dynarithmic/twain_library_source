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
#include <algorithm>
#include <numeric>
#include <boost/algorithm/hex.hpp>
#include <boost/algorithm/string.hpp>
#include <windows.h>
#include <cctype>
#include <cwctype>

// Widening string macros
#define WIDEN2(x) L##x
#define WIDEN(x)  WIDEN2(x)

#ifdef _MSC_VER
    #define DTWAIN_SPRINTF_FUNC sprintf_s
    #define DTWAIN_SWPRINTF_FUNC swprintf_s
#else
    #define DTWAIN_SPRINTF_FUNC sprintf
    #define DTWAIN_SWPRINTF_FUNC swprintf
#endif

namespace dynarithmic
{
    template <typename CharT>
    struct CharTraits;

    template <>
    struct CharTraits<char>
    {
        static bool IsDigit(char c)
        {
            return std::isdigit(static_cast<unsigned char>(c)) != 0;
        }

        static bool IsSpace(char c)
        {
            return std::isspace(static_cast<unsigned char>(c)) != 0;
        }

        static char ToLower(char c)
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }

        static char ToUpper(char c)
        {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }

        static double ToDouble(const char * s1, double defVal = 0.0)
        {
            return s1 ? strtod(s1, nullptr) : defVal;
        }

        static constexpr char* DoubleQuote() { return "\""; }
        static constexpr char* Space() { return " "; }
        static constexpr char* EmptyString() { return ""; }
        static constexpr char* NewLineString() { return "\n"; }

        static size_t Length(const char* s) { return std::char_traits<char>::length(s); }

    };

    template <>
    struct CharTraits<wchar_t>
    {
        static bool IsDigit(wchar_t c)
        {
            return std::iswdigit(c) != 0;
        }

        static bool IsSpace(wchar_t c)
        {
            return std::iswspace(c) != 0;
        }

        static wchar_t ToLower(wchar_t c)
        {
            return std::towlower(c);
        }

        static wchar_t ToUpper(wchar_t c) 
        { 
            return towupper(c); 
        }

        static double ToDouble(const wchar_t* s1, double defVal = 0.0)
        {
            return s1 ? wcstod(s1, nullptr) : defVal;
        }

        static constexpr wchar_t* DoubleQuote() { return L"\""; }
        static constexpr wchar_t* Space() { return L" "; }
        static constexpr wchar_t* EmptyString() { return L""; }
        static constexpr wchar_t* NewLineString() { return L"\n"; }

        static size_t Length(const wchar_t* s) { return std::char_traits<wchar_t>::length(s); }
    };

    namespace basicstringutils
    {
        template <typename CharType>
        bool IsAllSpace(const CharType* src)
        {
            return std::all_of(src, src + CharTraits<CharType>::Length(src), [](wint_t ch)
                { return CharTraits<CharType>::IsSpace(ch); });
        }

        template <typename CharT, typename Traits = std::char_traits<CharT>>
        bool StartsWith(std::basic_string_view<CharT, Traits> str,
            std::basic_string_view<CharT, Traits> prefix)
        {
            return str.size() >= prefix.size() &&
                str.compare(0, prefix.size(), prefix) == 0;
        }

        template <typename CharT, typename Traits = std::char_traits<CharT>>
        bool EndsWith(std::basic_string_view<CharT, Traits> str,
            std::basic_string_view<CharT, Traits> suffix)
        {
            return str.size() >= suffix.size() &&
                str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        template <typename StringType>
        bool StartsWith(const StringType& str, const StringType& prefix)
        {
            return StartsWith(std::basic_string_view<StringType::value_type>(str),
                std::basic_string_view<StringType::value_type>(prefix));
        }

        template <typename StringType>
        bool EndsWith(const StringType& str, const StringType& suffix)
        {
            return EndsWith(std::basic_string_view<StringType::value_type>(str),
                std::basic_string_view<StringType::value_type>(suffix));
        }

        template <typename StringType>
        StringType QuoteString(const StringType& str,
            typename const StringType::value_type* quoteString =
            CharTraits<StringType::value_type>::DoubleQuote())
        {
            return quoteString + str + quoteString;
        }

        template <typename StringType, typename T>
        StringType ToString(const T& value)
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                if constexpr (std::is_same_v<StringType, std::string>)
                    return std::to_string(value);
                else
                    return std::to_wstring(value);
            }
            else
            {
                using StreamType = std::basic_ostringstream<StringType::value_type>;
                StreamType strm;
                strm << value;
                return strm.str();
            }
            return {};
        }

        template <typename StringType>
        StringType Mid(typename std::basic_string_view<typename StringType::value_type> str, size_t nFirst)
        {
            return str.substr(nFirst).data();
        }

        template <typename StringType>
        StringType Mid(typename std::basic_string_view<typename StringType::value_type> str, size_t nFirst, size_t nNum)
        {
            return { str.substr(nFirst, nNum).data(), nNum };
        }

        template <typename StringType>
        StringType Left(typename std::basic_string_view<typename StringType::value_type> str, size_t nNum)
        {
            return Mid<StringType>(str, 0, nNum);
        }

        template <typename StringType>
        StringType Right(typename std::basic_string_view<typename StringType::value_type> str, size_t nNum)
        {
            const size_t nLen = str.length();
            if (nNum > nLen)
                nNum = nLen;
            return Mid<StringType>(nLen - nNum, nNum);
        }

        template <typename StringType>
        StringType TrimDouble(double value, int numDigitsPrecision = 8)
        {
            StringType::value_type buf[256];
            if constexpr (std::is_same_v<StringType, std::string>)
                DTWAIN_SPRINTF_FUNC(buf, "%.*g", numDigitsPrecision, value);
            else
                DTWAIN_SWPRINTF_FUNC(buf, L"%.*g", numDigitsPrecision, value);
            return buf;
        }

        template <typename CharType>
        CharType* CopyN(CharType* dest, const CharType* src, size_t count)
        {
            return std::char_traits<CharType>::copy(dest, src, count);
        }

        template <typename CharType>
        CharType* Copy(CharType* dest, const CharType* src) 
        { 
            return std::char_traits<CharType>::copy(dest, src, CharTraits<CharType>::Length(src)); 
        }

        template <typename CharType>
        CharType* SafeStrcpy(CharType* pDest, const CharType* pSrc, size_t nMaxChars)
        {
            if (!pSrc || !pDest)
                return pDest;
            size_t nLen = CharTraits<CharType>::Length(pSrc);
            if (nMaxChars < nLen)
                nLen = nMaxChars;
            CopyN(pDest, pSrc, nLen);
            pDest[nLen] = 0;
            return pDest;
        }

        template <typename CharType>
        CharType* SafeStrcpy(CharType* pDest, const CharType* pSrc)
        {
            if (!pSrc || !pDest)
                return pDest;
            return Copy(pDest, pSrc);
        }

        template <typename StringType>
        StringType ReplaceAll(typename std::basic_string_view<typename StringType::value_type> strOrig,
                              typename std::basic_string_view<typename StringType::value_type> findStr,
                              typename std::basic_string_view<typename StringType::value_type> replaceStr)
        {
            if (strOrig.empty())
                return {};
            StringType str = strOrig.data();
            size_t start_pos = 0;
            while ((start_pos = str.find(findStr, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, findStr.length(), replaceStr.data(), replaceStr.length());
                start_pos += replaceStr.length();
            }
            return str;
        }

        template <typename CharT, typename Traits = std::char_traits<CharT>>
        bool IEquals(std::basic_string_view<CharT, Traits> lhs,
                     std::basic_string_view<CharT, Traits> rhs)
        {
            if (lhs.size() != rhs.size())
                return false;

            return std::equal(lhs.begin(), lhs.end(), rhs.begin(),
                [](CharT c1, CharT c2)
                {
                    return CharTraits<CharT>::ToLower(c1) ==
                        CharTraits<CharT>::ToLower(c2);
                });
        }

        template <typename StringType>
        int Compare(typename std::basic_string_view<typename StringType::value_type> str, 
                    const typename StringType::value_type* lpsz)
        {
            return str.compare(lpsz);
        }

        template <typename StringType>
        bool CompareNoCase(typename std::basic_string_view<typename StringType::value_type> str, 
                           const typename StringType::value_type* lpsz)
        {
            using StringView = std::basic_string_view<typename StringType::value_type>;
            StringView vw(lpsz);
            return IEquals(str, vw);
        }

        template <typename CharT>
        class is_any_of_pred
        {
            public:
                using string_type = std::basic_string<CharT>;

                explicit is_any_of_pred(string_type chars)
                    : chars_(std::move(chars)) {
                }

                bool operator()(CharT ch) const
                {
                    return chars_.find(ch) != string_type::npos;
                }

            private:
                string_type chars_;
        };

        template <typename CharT>
        auto is_any_of(const CharT* chars)
        {
            return is_any_of_pred<CharT>(std::basic_string<CharT>(chars));
        }

        template <typename StringType, typename Pred>
        StringType& ltrim_if(StringType& str, Pred pred)
        {
            auto it2 = std::find_if_not(str.begin(), str.end(), pred);
            str.erase(str.begin(), it2);
            return str;
        }

        template <typename StringType, typename Pred>
        StringType& rtrim_if(StringType& str, Pred pred)
        {
            auto it1 = std::find_if_not(str.rbegin(), str.rend(), pred);
            str.erase(it1.base(), str.end());
            return str;
        }

        template <typename StringType, typename Pred>
        StringType ltrim_copy_if(StringType str, Pred pred)
        {
            return ltrim_if(str, pred);
        }

        template <typename StringType, typename Pred>
        StringType rtrim_copy(StringType str, Pred pred)
        {
            return ltrim_if(str, pred);
        }

        template <typename StringType, typename Pred>
        StringType trim_copy_if(StringType str, Pred pred)
        {
            return ltrim_if(rtrim_if(str, pred), pred);
        }

        template <typename StringType, typename Pred>
        StringType& trim_if(StringType& str, Pred pred)
        {
            return ltrim_if(rtrim_if(str, pred), pred);
        }

        template <typename StringType, typename TrimmerFn>
        decltype(auto) string_trimmer(StringType&& str, TrimmerFn fn)
        {
            if constexpr (std::is_same_v <StringType, std::wstring>)
            {
                return fn(str, [](unsigned char ch) { return !iswspace(ch); });
            }
            else
            {
                return fn(str, [](unsigned char ch) { return !isspace(ch); });
            }
            return std::forward<StringType>(str);
        }

        template <typename StringType>
        decltype(auto) ltrim(StringType&& str)
        {
            return string_trimmer(str, &ltrim_if);
        }

        template <typename StringType>
        decltype(auto) rtrim(StringType&& str)
        {
            return string_trimmer(str, &rtrim_if);
        }

        template <typename StringType>
        decltype(auto) trim(StringType&& str)
        {
            return ltrim_copy(rtrim_copy(str));
        }

        template <typename StringType>
        StringType ltrim_copy(StringType str)
        {
            return ltrim(str);
        }

        template <typename StringType>
        StringType rtrim_copy(StringType str)
        {
            return rtrim(str);
        }

        template <typename StringType>
        StringType trim_copy(StringType str)
        {
            return ltrim_copy(rtrim_copy(str));
        }

        template <typename StringType>
        StringType& TrimRight(StringType& str, 
                   const typename StringType::value_type* lpszTrimStr = typename CharTraits<StringType::value_type>::Space())
        {
            return rtrim_if(str, is_any_of(lpszTrimStr));
        }

        template <typename StringType>
        StringType& TrimLeft(StringType& str, 
            const typename StringType::value_type* lpszTrimStr = typename CharTraits<StringType::value_type>::Space())
        {
            return ltrim_if(str, is_any_of(lpszTrimStr));
        }

        template <typename StringType>
        StringType& TrimAll(StringType& str, 
            const typename StringType::value_type* lpszTrimStr = typename CharTraits<StringType::value_type>::Space())
        {
            return trim_if(str, is_any_of(lpszTrimStr));
        }

        template <typename StringType>
        int TokenizeEx(const StringType& str,
                        const typename StringType::value_type* lpszTokStr,
                        std::vector<StringType>& rArray,
                        bool bGetNullTokens,
                        std::vector<unsigned>* positionArray = nullptr)
        {
            using size_type = typename StringType::size_type;

            rArray.clear();
            if (positionArray)
                positionArray->clear();

            if (!lpszTokStr || !*lpszTokStr)
            {
                if (!str.empty() || bGetNullTokens)
                {
                    rArray.push_back(str);
                    if (positionArray)
                        positionArray->push_back(0);
                }
                return static_cast<int>(rArray.size());
            }

            size_type start = 0;

            while (start <= str.size())
            {
                const size_type pos = str.find_first_of(lpszTokStr, start);

                const size_type end =
                    (pos == StringType::npos) ? str.size() : pos;

                if (end != start || bGetNullTokens)
                {
                    rArray.emplace_back(str.substr(start, end - start));

                    if (positionArray)
                        positionArray->push_back(static_cast<unsigned>(start));
                }

                if (pos == StringType::npos)
                    break;

                start = pos + 1;
            }

            return static_cast<int>(rArray.size());
        }

        template <typename StringType>
        int TokenizeQuotedEx(const StringType& str,
                             const typename StringType::value_type* lpszTokStr,
                             std::vector<StringType>& rArray,
                             bool bGetNullTokens,
                             std::vector<unsigned>* positionArray = nullptr)
        {
            using CharT = typename StringType::value_type;
            using size_type = typename StringType::size_type;

            rArray.clear();
            if (positionArray)
                positionArray->clear();

            auto is_delimiter = [lpszTokStr](CharT ch) -> bool
            {
                if (!lpszTokStr)
                    return false;

                for (const CharT* p = lpszTokStr; *p; ++p)
                {
                    if (*p == ch)
                        return true;
                }
                return false;
            };

            auto is_quote = [](CharT ch) -> bool
            {
                return ch == static_cast<CharT>('\'') ||
                    ch == static_cast<CharT>('"');
            };

            auto add_token = [&](size_type tokenStart,
                size_type tokenEnd,
                size_type reportedPosition)
            {
                if (tokenEnd < tokenStart)
                    tokenEnd = tokenStart;

                if (tokenEnd != tokenStart || bGetNullTokens)
                {
                    rArray.emplace_back(str.substr(tokenStart, tokenEnd - tokenStart));

                    if (positionArray)
                        positionArray->push_back(static_cast<unsigned>(reportedPosition));
                }
            };

            const size_type n = str.size();
            size_type tokenStart = 0;
            size_type tokenContentStart = 0;
            bool inQuote = false;
            CharT quoteChar = 0;
            bool tokenStarted = false;
            bool quotedToken = false;

            for (size_type i = 0; i <= n; ++i)
            {
                const bool atEnd = (i == n);
                const CharT ch = atEnd ? CharT{} : str[i];

                if (!atEnd && !tokenStarted)
                {
                    tokenStarted = true;
                    tokenStart = i;
                    tokenContentStart = i;

                    if (is_quote(ch))
                    {
                        quotedToken = true;
                        inQuote = true;
                        quoteChar = ch;
                        tokenContentStart = i + 1;
                        continue;
                    }
                }

                if (!atEnd && inQuote)
                {
                    if (ch == quoteChar)
                    {
                        inQuote = false;
                        continue;
                    }

                    continue;
                }

                if (atEnd || (!inQuote && is_delimiter(ch)))
                {
                    size_type tokenEnd = i;

                    if (quotedToken)
                    {
                        // Strip trailing quote if the token ended after a quote.
                        tokenEnd = i;

                        if (tokenEnd > tokenContentStart &&
                            is_quote(str[tokenEnd - 1]))
                        {
                            --tokenEnd;
                        }
                    }

                    add_token(tokenContentStart, tokenEnd, tokenStart);

                    tokenStarted = false;
                    quotedToken = false;
                    inQuote = false;
                    quoteChar = 0;

                    tokenStart = i + 1;
                    tokenContentStart = i + 1;
                }
            }

            return static_cast<int>(rArray.size());
        }

        template <typename StringType>
        int Tokenize(const StringType& str, const typename StringType::value_type* lpszTokStr,
                           std::vector<StringType>& rArray, bool bGetNullTokens = false)
        {
            return TokenizeEx(str, lpszTokStr, rArray, bGetNullTokens);
        }

        template <typename CharType>
        int Tokenize(const CharType* str, const CharType* lpszTokStr,
                     std::vector<std::basic_string<CharType>>& rArray, bool bGetNullTokens = false)
        {
            using StringType = std::basic_string<CharType>;
            return TokenizeEx((StringType)str, lpszTokStr, rArray, bGetNullTokens);
        }

        template <typename StringType>
        int TokenizeQuoted(const StringType& str, const typename StringType::value_type* lpszTokStr,
                           std::vector<StringType>& rArray, bool bGetNullTokens = false)
        {
            return TokenizeQuotedEx(str, lpszTokStr, rArray, bGetNullTokens);
        }

        template <typename CharType>
        void to_lower_upper(CharType* input, size_t len, bool isLower, const std::locale& loc = std::locale())
        {
            auto const& facet = std::use_facet<std::ctype<CharType>>(loc);
            std::transform(input, input + len, input,
                [&facet, isLower](CharType c) { return isLower ? facet.tolower(c) : facet.toupper(c); });
        }

        template <typename StringType>
        StringType to_lower_upper_copy(typename std::basic_string_view<typename StringType::value_type> input,
                                        bool isLower,
                                        const std::locale& loc = std::locale())
        {
            using CharType = StringType::value_type;
            auto const& facet = std::use_facet<std::ctype<CharType>>(loc);
            StringType out;
            out.reserve(input.size());

            std::transform(input.begin(), input.end(), std::back_inserter(out),
                [&facet, isLower](CharType c) { return isLower ? facet.tolower(c) : facet.toupper(c); });

            return out;
        }

        template <typename StringType>
        StringType& MakeUpperCase(StringType& str)
        {
            to_lower_upper(str.data(), str.size(), false);
            return str;
        }

        template <typename StringType>
        StringType& MakeLowerCase(StringType& str)
        {
            to_lower_upper(str.data(), str.size(), true);
            return str;
        }

        template <typename StringType>
        StringType UpperCase(typename std::basic_string_view<typename StringType::value_type> str)
        {
            return to_lower_upper_copy(str, false);
        }

        template <typename StringType>
        StringType UpperCase(const StringType& str)
        {
            return to_lower_upper_copy<StringType>(std::basic_string_view<typename StringType::value_type>(str), false);
        }

        template <typename StringType>
        StringType LowerCase(typename std::basic_string_view<typename StringType::value_type> str)
        {
            return to_lower_upper_copy(str, true);
        }

        template <typename StringType>
        StringType LowerCase(const StringType& str)
        {
            return to_lower_upper_copy<StringType>(std::basic_string_view<typename StringType::value_type>(str), true);
        }

        template <typename StringType, typename Container>
        StringType Join(const Container& ct, const StringType& seperator = {})
        {
            return Join<StringType, Container>(ct.begin(), ct.end(), seperator);
        }

        template <typename StringType, typename Container>
        StringType Join(const Container& ct, const typename StringType::value_type* seperator = 
                            CharTraits<StringType::value_type>::EmptyString())
        {
            StringType sSep(seperator);
            return Join<StringType, typename Container::const_iterator>(ct.begin(), ct.end(), sSep);
        }

        template <typename StringType, typename val>
        StringType defaultJoinImpl(const StringType& str,
                                    const val& value,
                                    const typename StringType::value_type* separator
                                    = CharTraits<StringType::value_type>::EmptyString())
        {
            using StreamType = std::basic_ostringstream<StringType::value_type>;
            StreamType strm{};
            if (!str.empty())
                strm << str << separator << value;
            else
                strm << value;
            return strm.str();
        }

        template <typename StringType, typename Iter>
        StringType Join(Iter iter1, Iter iter2, 
                        const typename StringType::value_type* separator = 
                            CharTraits<StringType::value_type>::EmptyString())
        {
            return std::accumulate(iter1, iter2, StringType(),
                [&](const auto& str, typename std::iterator_traits<Iter>::value_type val)
                {
                    return defaultJoinImpl<StringType>(str, val, separator);
                });
        }

        template <typename StringType, typename Iter>
        StringType Join(Iter iter1, Iter iter2, const StringType& separator = {})
        {
            return Join<StringType>(iter1, iter2, separator.c_str());
        }

        template <typename StringType, typename Iter, typename Fn>
        StringType JoinEx(Iter iter1, Iter iter2, Fn concatFn)
        {
            return std::accumulate(iter1, iter2, StringType(),
                [&](const auto& str, typename std::iterator_traits<Iter>::value_type val)
                {
                    return concatFn(str, val);
                });
        }
    }
}
#endif


