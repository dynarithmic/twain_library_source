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
#ifndef CTLOBSTR_H
#define CTLOBSTR_H

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <numeric>
#include <type_traits>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/hex.hpp>
#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <iomanip>
#include <locale>
#include <iostream>
#include <string_view>
#include <cctype>
#include <cwctype>
#include <random>
#include <boost/lexical_cast.hpp>
#include <dtwain_filesystem.h>
#include "dtwain_standard_defs.h"
#include "ctlstringconversion.h"

#ifndef _MAX_PATH
    #define _MAX_PATH 260
#endif

#ifdef _MSC_VER
    #define DTWAIN_SPRINTF_FUNC sprintf_s
    #define DTWAIN_SWPRINTF_FUNC swprintf_s
#else
    #define DTWAIN_SPRINTF_FUNC sprintf
    #define DTWAIN_SWPRINTF_FUNC swprintf
#endif

namespace dynarithmic
{
    typedef std::vector<std::string> StringArray;
    typedef std::vector<std::wstring> StringArrayW;
    typedef std::stringstream StringStreamA;
    typedef std::wstringstream StringStreamW;
    typedef std::ostringstream StringStreamOutA;
    typedef std::wostringstream StringStreamOutW;
    typedef std::istringstream StringStreamInA;
    typedef std::wistringstream StringStreamInW;
    typedef std::wofstream  OutputFileStreamW;
    typedef std::wostream   OutputBaseStreamW;
    typedef std::ofstream  OutputFileStreamA;
    typedef std::ostream   OutputBaseStreamA;
    typedef std::ifstream InputFileStreamA;
    typedef std::wifstream InputFileStreamW;
    typedef std::wistream InputBaseStreamW;
    typedef std::istream InputBaseStreamA;

    #ifdef UNICODE
        typedef std::wstring        CTL_StringType;
        typedef std::wstring_view   CTL_StringViewType;
        typedef StringArrayW        CTL_StringArrayType;
        typedef std::wostringstream CTL_StringStreamType;
        typedef std::wistringstream CTL_StringStreamInType;
        typedef std::wifstream      CTL_InputFileStreamType;
        typedef std::wistream       CTL_InputBaseStreamType;
        typedef std::wofstream      CTL_OutputFileStreamType;
        typedef std::wostream       CTL_OutputBaseStreamType;
        struct ErrorStream
        {
            template <typename T>
            static void StreamMe(T t ) { std::wcerr << t; }
        };
    #else
        typedef std::string         CTL_StringType;
        typedef std::string_view    CTL_StringViewType;
        typedef StringArray         CTL_StringArrayType;
        typedef std::ostringstream  CTL_StringStreamType;
        typedef std::istringstream  CTL_StringStreamInType;
        typedef std::ifstream       CTL_InputFileStreamType;
        typedef std::istream        CTL_InputBaseStreamType;
        typedef std::ofstream       CTL_OutputFileStreamType;
        typedef std::ostream        CTL_OutputBaseStreamType;
        struct ErrorStream
        {
            template <typename T>
            static void StreamMe(T t) { std::cerr << t; }
        };
#endif

    using CTL_StringTypeA = std::string;
    using CTL_StringTypeW = std::wstring;

    #define LOCAL_STATIC static
    #define STRINGWRAPPER_QUALIFIER StringWrapper::
    #define STRINGWRAPPER_PREFIX StringWrapper::

    struct ANSIStringTraits
    {
        using char_type = char;
        using uchar_type = unsigned char;
        using string_type = std::string;
        using stringview_type = std::string_view;
        using stringarray_type = std::vector<string_type>;
        using outputstream_type = std::ostringstream;
        using inputstream_type = std::istringstream;
        using outputfile_type = std::ofstream;
        using inputfile_type = std::ifstream;
        using baseoutputstream_type = std::ostream;
        using baseinputstream_type = std::istream;

        using FILESYSTEM_PATHTYPE = filesys::path;

        template <typename T>
        static std::string PathGenericString(const T& x) { return x.generic_string(); }

        template <typename T, typename std::enable_if<std::is_arithmetic_v<T>,bool>::type = true>
        static std::string ToString(const T& value)
        {
            return std::to_string(value);
        }

        template <typename T>
        static std::string ToStringEx(const T& value)
        {
            std::ostringstream strm;
            strm << value;
            return strm.str();
        }

        static constexpr char_type GetSpace() { return ' ';}
        static constexpr char_type GetLeftCurlyBrace() { return '{'; }
        static constexpr char_type GetRightCurlyBrace() { return '}'; }
        static constexpr const char_type* GetSpaceString() { return " "; }
        static constexpr const char_type* GetEmptyString() { return ""; }
        static constexpr char_type GetZeroString() { return '\0'; }
        static constexpr char_type EscapeChar() { return '\\';}
        static constexpr char_type BackSlashChar() { return '\\';}
        static constexpr char_type ForwardSlashChar() { return '/'; }
        static constexpr const char_type* EscapeString() { return "\\"; }
        static constexpr const char_type* AllQuoteString() { return "\"'"; }
        static constexpr char_type SingleQuoteChar() { return '\''; }
        static constexpr const char_type* SingleQuoteString() { return "'"; }
        static constexpr char_type DoubleQuoteChar() { return '\"'; }
        static constexpr const char_type* DoubleQuoteString() { return "\""; }
        static constexpr char_type GetZeroNumericString() { return '0'; }
        static const char_type* GetCompatStringLiteral(const char_type* x) { return x; }
        static constexpr char_type GetNewLineChar() { return '\n'; }
        static constexpr const char_type* GetNewLineString() { return "\n"; }
        static constexpr const char_type* GetWindowsNewLineString() { return "\r\n"; }
        static size_t Length(const char_type* s) { return std::char_traits<char_type>::length(s); }
        static char_type* Copy(char_type* dest, const char_type* src) { return std::char_traits<char_type>::copy(dest, src, Length(src)); }
        static char_type* CopyN(char_type* dest, const char_type* src, size_t count) { return std::char_traits<char_type>::copy(dest, src, count); }
        static int Compare(const char_type* dest, const char_type* src, size_t count) { return std::char_traits<char_type>::compare(dest, src, count); }
        static int Compare(const char_type* dest, const char_type* src) { return std::char_traits<char_type>::compare(dest, src, (std::min)(Length(dest), Length(src))); }

        static bool IsAllSpace(const char_type* src)
        {
            return std::all_of(src, src + Length(src), [](char ch) { return std::isspace(static_cast<unsigned char>(ch)); });
        }

        static const char_type* ConvertToOther(wchar_t* dest, const char_type* src)
        {
            auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
            return f.widen(src, src + Length(src), dest);
        }

        static const char_type* ConvertToOther(wchar_t* dest, const char_type* src, int len)
        {
            auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
            return f.widen(src, src + len, dest);
        }

        static constexpr char_type CharToThisType(char ch)
        {
            return ch;
        }

        static int ToUpper(char_type ch) { return toupper(static_cast<int>(ch)); }
        static int ToLower(char_type ch) { return tolower(static_cast<int>(ch)); }
        static bool IsDigit(int ch) { return isdigit(ch)?true:false; }
        static double ToDouble(const char_type* s1)
        { return s1?strtod(s1, nullptr):0.0; }

        static std::string TrimDouble(double value, int numDigitsPrecision)
        {
            char_type buf[256];
            DTWAIN_SPRINTF_FUNC(buf, "%.*g", numDigitsPrecision, value);
            return buf;
        }

        #ifdef _WIN32
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
                    { return GetWindowsDirectoryA(buffer, _MAX_PATH); }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
                    { return GetSystemDirectoryA(buffer, _MAX_PATH); }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
                    { return GetModuleFileNameA(hModule, lpFileName, nSize); }
        static UINT GetWindowsDirectoryImpl(string_type& buffer)
        {
            buffer.resize(_MAX_PATH);
            auto nSize = GetWindowsDirectoryA(&buffer[0], _MAX_PATH);
            buffer.resize(nSize);
            return nSize;
        }
        static UINT GetSystemDirectoryImpl(string_type& buffer)
        {
            buffer.resize(_MAX_PATH);
            auto nSize = GetSystemDirectoryA(&buffer[0], _MAX_PATH);
            buffer.resize(nSize);
            return nSize;
        }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, string_type& lpFileName, DWORD nSize)
        {
            lpFileName.resize(nSize);
            auto actualSize = GetModuleFileNameA(hModule, &lpFileName[0], nSize);
            if (actualSize < nSize)
                lpFileName.resize(actualSize);
            return actualSize;
        }
        #else
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
        { getcwd(buffer, 8096); return 1; }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
        { getcwd(buffer, 8096); return 1; }
        static UINT GetWindowsDirectoryImpl(string_type& buffer)
        {
            buffer.resize(8096);
            getcwd(&buffer[0], 8096); 
            return 1;
        }
        static UINT GetSystemDirectoryImpl(string_type& buffer)
        {
            buffer.resize(8096);
            getcwd(&buffer[0], 8096);
            return 1;
        }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
        {
            return 0; // ::GetModuleFileNameA(hModule, lpFileName, nSize);
        }
        #endif
    };

    struct UnicodeStringTraits
    {
        using char_type = wchar_t;
        using uchar_type = wchar_t;
        using string_type = std::wstring;
        using stringview_type = std::wstring_view;
        using stringarray_type = std::vector<string_type>;
        using outputstream_type = std::wostringstream;
        using inputstream_type = std::wistringstream;
        using outputfile_type = std::wofstream;
        using inputfile_type = std::wifstream;
        using baseoutputstream_type = std::wostream;
        using baseinputstream_type = std::wistream;

        using FILESYSTEM_PATHTYPE = filesys::path;

        template <typename T>
        static std::wstring PathGenericString(const T& x) { return x.generic_wstring(); }

        template <typename T, typename std::enable_if<std::is_arithmetic_v<T>, bool>::type = true>
        static std::wstring ToString(const T& value)
        {
            return std::to_wstring(value);
        }

        template <typename T>
        static std::wstring ToStringEx(const T& value)
        {
            std::wostringstream strm;
            strm << value;
            return strm.str();
        }

        static constexpr char_type GetSpace() { return L' ';}
        static constexpr char_type GetLeftCurlyBrace() { return L'{'; }
        static constexpr char_type GetRightCurlyBrace() { return L'}'; }
        static constexpr const char_type* GetSpaceString() { return L" "; }
        static constexpr const char_type* GetEmptyString() { return L""; }
        static constexpr char_type GetZeroString() { return L'\0'; }
        static constexpr char_type EscapeChar() { return L'\\'; }
        static constexpr char_type BackSlashChar() { return L'\\'; }
        static constexpr char_type ForwardSlashChar() { return L'/'; }
        static constexpr const char_type* EscapeString() { return L"\\"; }
        static constexpr const char_type* AllQuoteString() { return L"\"'"; }
        static constexpr char_type SingleQuoteChar() { return L'\''; }
        static constexpr const char_type* SingleQuoteString() { return L"'"; }
        static constexpr char_type DoubleQuoteChar() { return L'\"'; }
        static constexpr const char_type* DoubleQuoteString() { return L"\""; }
        static constexpr char_type GetZeroNumericString() { return L'0'; }
        static constexpr char_type GetNewLineChar() { return L'\n'; }
        static constexpr const char_type* GetNewLineString() { return L"\n"; }
        static constexpr const char_type* GetWindowsNewLineString() { return L"\r\n"; }
        static const char_type* GetCompatStringLiteral(const char_type* x) { return x; }
        static size_t Length(const char_type* s) { return std::char_traits<char_type>::length(s); }
        static int Compare(const char_type* dest, const char_type* src, size_t count) { return std::char_traits<char_type>::compare(dest, src, count); }
        static int Compare(const char_type* dest, const char_type* src) { return std::char_traits<char_type>::compare(dest, src, (std::min)(Length(dest), Length(src))); }

        static bool IsAllSpace(const char_type* src)
        {
            return std::all_of(src, src + Length(src), [](wint_t ch) { return std::iswspace(static_cast<wint_t>(ch)); });
        }

        static const char_type* ConvertToOther(char* dest, const char_type* src)
        {
            auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
            return f.narrow(src, src + Length(src), '\0', dest);
        }

        static const char_type* ConvertToOther(char* dest, const char_type* src, int len)
        {
            auto& f = std::use_facet<std::ctype<wchar_t>>(std::locale());
            return f.narrow(src, src + len, '\0', dest);
        }

        static const char_type CharToThisType(char ch)
        {
            char_type dest {};
            ANSIStringTraits::ConvertToOther(&dest, &ch, 1);
            return dest;
        }

        static char_type* Copy(char_type* dest, const char_type* src) { return std::char_traits<char_type>::copy(dest, src, Length(src)); }
        static char_type* CopyN(char_type* dest, const char_type* src, size_t count) { return std::char_traits<char_type>::copy(dest, src, count); }
        static wint_t ToUpper(char_type ch) { return towupper(static_cast<wint_t>(ch)); }
        static wint_t ToLower(char_type ch) { return towlower(static_cast<wint_t>(ch)); }
        static bool IsDigit(wint_t ch) { return iswdigit(ch)?true:false; }
        static double ToDouble(const char_type* s1)
        { return s1 ? wcstod(s1, nullptr) : 0.0; }

        static std::wstring TrimDouble(double value, int numDigitsPrecision)
        {
            char_type buf[255];
            DTWAIN_SWPRINTF_FUNC(buf, L"%.*g", numDigitsPrecision, value);
            return buf;
        }

        #ifdef _WIN32
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
        { return GetWindowsDirectoryW(buffer, _MAX_PATH); }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
        { return GetSystemDirectoryW(buffer, _MAX_PATH); }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
        { return GetModuleFileNameW(hModule, lpFileName, nSize); }

        static UINT GetWindowsDirectoryImpl(string_type& buffer)
        {
            buffer.resize(_MAX_PATH);
            auto nSize = GetWindowsDirectoryW(&buffer[0], _MAX_PATH);
            buffer.resize(nSize);
            return nSize;
        }

        static UINT GetSystemDirectoryImpl(string_type& buffer)
        {
            buffer.resize(_MAX_PATH);
            auto nSize = GetSystemDirectoryW(&buffer[0], _MAX_PATH);
            buffer.resize(nSize);
            return nSize;
        }

        static DWORD GetModuleFileNameImpl(HMODULE hModule, string_type& lpFileName, DWORD nSize)
        {
            lpFileName.resize(nSize);
            auto actualSize = GetModuleFileNameW(hModule, &lpFileName[0], nSize);
            if (actualSize < nSize)
                lpFileName.resize(actualSize);
            return actualSize;
        }
        #else
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
        {
            std::vector<char> buffer_temp(buffer, buffer + 8096);
            getcwd(buffer_temp.data(), 8096);
            std::transform(buffer_temp.begin(), buffer_temp.end(), buffer, [&](char ch) { return ch; });
            return 1;
        }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
        {
            return GetWindowsDirectoryImpl(buffer);
        }

        static UINT GetWindowsDirectoryImpl(string_type& buffer)
        {
            buffer.resize(8096);
            return GetWindowsDirectoryImpl(&buffer[0]);
        }
        static UINT GetSystemDirectoryImpl(string_type& buffer)
        {
            buffer.resize(8096);
            return GetSystemDirectoryImpl(&buffer[0]);
        }

        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
        {
            return 0; // ::GetModuleFileNameA(hModule, lpFileName, nSize);
        }
        #endif
    };

    template <typename StringType, typename CharType, typename StringTraits>
    struct StringWrapper_Impl
    {
        typedef StringTraits traits_type;
        enum { DRIVE_POS, DRIVE_PATH, DIRECTORY_POS, NAME_POS, EXTENSION_POS };
        typedef std::vector<StringType> StringArrayType;

        static traits_type GetTraits() { return {}; }

        static StringType Mid(typename StringTraits::stringview_type str, size_t nFirst)
        {
            return str.substr(nFirst).data();
        }

        static StringType Mid(typename StringTraits::stringview_type str, size_t nFirst, size_t nNum)
        {
            return { str.substr(nFirst, nNum).data(), nNum };
        }

        static StringType Left(typename StringTraits::stringview_type str, size_t nNum)
        {
            return Mid(str, 0, nNum);
        }

        // define string helper functions here
        static StringType Right(typename StringTraits::stringview_type str, size_t nNum)
        {
            const size_t nLen = str.length();
            if (nNum > nLen)
                nNum = nLen;
            return Mid(nLen - nNum, nNum);
        }

        static CharType GetAt(typename StringTraits::stringview_type str, size_t nPos)
        {
            assert(nPos < str.length());
            return str[nPos];
        }

        static void SetAt(StringType& str, size_t nPos, CharType c)
        {
            str.replace(nPos, 1, 1, c);
        }

        static bool IsEmpty(typename StringTraits::stringview_type str)
        {
            return str.empty();
        }

        static bool IsAllSpace(typename StringTraits::stringview_type str)
        {
            return StringTraits::IsAllSpace(str.data());
        }

        static void Empty(StringType &str )
        {
            str = StringTraits::GetEmptyString();
        }

        static StringType ReplaceAll(typename StringTraits::stringview_type strOrig, 
                                     typename StringTraits::stringview_type findStr, 
                                     typename StringTraits::stringview_type replaceStr)
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

        private:
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
        static auto is_any_of(const CharT* chars)
        {
            return is_any_of_pred<CharT>(std::basic_string<CharT>(chars));
        }

        template <typename StringType, typename Pred>
        static StringType& ltrim_if(StringType& str, Pred pred)
        {
            auto it2 = std::find_if_not(str.begin(), str.end(), pred);
            str.erase(str.begin(), it2);
            return str;
        }

        template <typename StringType, typename Pred>
        static StringType& rtrim_if(StringType& str, Pred pred)
        {
            auto it1 = std::find_if_not(str.rbegin(), str.rend(), pred);
            str.erase(it1.base(), str.end());
            return str;
        }

        template <typename StringType, typename Pred>
        static StringType ltrim_copy_if(StringType str, Pred pred)
        {
            return ltrim_if(str, pred);
        }

        template <typename StringType, typename Pred>
        static StringType rtrim_copy(StringType str, Pred pred)
        {
            return ltrim_if(str, pred);
        }

        template <typename StringType, typename Pred>
        static StringType trim_copy_if(StringType str, Pred pred)
        {
            return ltrim_if(rtrim_if(str, pred), pred);
        }

        template <typename StringType, typename Pred>
        static StringType& trim_if(StringType& str, Pred pred)
        {
            return ltrim_if(rtrim_if(str, pred), pred);
        }

        template <typename StringType, typename TrimmerFn>
        static decltype(auto) string_trimmer(StringType&& str, TrimmerFn fn)
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

        static decltype(auto) ltrim(StringType&& str)
        {
            return string_trimmer(str, &ltrim_if);
        }

        static decltype(auto) rtrim(StringType&& str)
        {
            return string_trimmer(str, &rtrim_if);
        }

        static decltype(auto) trim(StringType&& str)
        {
            return ltrim_copy(rtrim_copy(str));
        }

        static StringType ltrim_copy(StringType str)
        {
            return ltrim(str);
        }

        static StringType rtrim_copy(StringType str)
        {
            return rtrim(str);
        }

        static StringType trim_copy(StringType str)
        {
            return ltrim_copy(rtrim_copy(str));
        }


        public:
        static StringType& TrimRight(StringType& str, const CharType *lpszTrimStr = StringTraits::GetSpaceString())
        {
            return rtrim_if(str, is_any_of(lpszTrimStr));
        }

        static StringType& TrimLeft(StringType& str, const CharType* lpszTrimStr = StringTraits::GetSpaceString())
        {
            return ltrim_if(str, is_any_of(lpszTrimStr));
        }

        static StringType& TrimAll(StringType& str, const CharType *lpszTrimStr = StringTraits::GetSpaceString())
        {
            return trim_if(str, is_any_of(lpszTrimStr));
        }

        template <typename Container>
        static StringType Join(const Container& ct, const StringType& seperator = StringTraits::GetEmptyString())
        {
            return Join(ct.begin(), ct.end(), seperator);
        }

        template <typename strtraits, typename val>
        static typename strtraits::string_type defaultJoinImpl(const typename strtraits::string_type& str, 
                                                                        const val& value,
                                                                        const typename strtraits::char_type* separator
                                                                        = typename strtraits::GetEmptyString())
        {
            typename strtraits::outputstream_type strm{};
            if (!str.empty())
                strm << str << separator << value;
            else
                strm << value;
            return strm.str();
        }

        template <typename Iter>
        static StringType Join(Iter iter1, Iter iter2, const StringType& separator = StringTraits::GetEmptyString())
        {
            return std::accumulate(iter1, iter2, StringType(),
                [&](const auto& str, typename std::iterator_traits<Iter>::value_type val)
                {
                    return defaultJoinImpl<traits_type>(str, val, separator.c_str());
                });
        }

        template <typename Iter, typename Fn>
        static StringType JoinEx(Iter iter1, Iter iter2, Fn concatFn)
        {
            return std::accumulate(iter1, iter2, StringType(),
                [&](const auto& str, typename std::iterator_traits<Iter>::value_type val)
                {
                    return concatFn(str, val);
                });
        }

        static int Tokenize(const StringType& str, const CharType *lpszTokStr,
                            StringArrayType &rArray, bool bGetNullTokens=false)
        {
            return TokenizeEx(str, lpszTokStr, rArray, bGetNullTokens);
        }

        static bool StartsWith(typename StringTraits::stringview_type str, 
                               typename StringTraits::stringview_type sub)
        {
            return boost::algorithm::starts_with(str, sub);
        }

        static bool EndsWith(typename StringTraits::stringview_type str, 
                             typename StringTraits::stringview_type sub)
        {
            return boost::algorithm::ends_with(str, sub);
        }

        static StringType QuoteString(typename StringTraits::stringview_type str,
                                      const StringType& quoteString = typename traits_type::DoubleQuoteString())
        {
            return quoteString + str.data() + quoteString;
        }

        static int TokenizeQuoted(const StringType& str, const CharType *lpszTokStr,
                                  StringArrayType &rArray, bool bGetNullTokens = false)
        {
            return TokenizeQuotedEx(str, lpszTokStr, rArray, bGetNullTokens);
        }

        static int Compare(typename StringTraits::stringview_type str, const CharType* lpsz)
        {
            return str.compare(lpsz);
        }

        static bool CompareNoCase(typename StringTraits::stringview_type str, const CharType* lpsz)
        {
            return boost::iequals(str, lpsz);
        }

        static void to_lower_upper(CharType* input, size_t len,
                                   bool isLower,
                                   const std::locale& loc = std::locale())
        {
            auto const& facet = std::use_facet<std::ctype<CharType>>(loc);
            std::transform(input, input + len, input,
                [&facet, isLower](CharType c) { return isLower ? facet.tolower(c) : facet.toupper(c); });
        }


        static StringType to_lower_upper_copy(typename StringTraits::stringview_type input, 
                                              bool isLower,
                                              const std::locale& loc = std::locale())
        {
            auto const& facet = std::use_facet<std::ctype<CharType>>(loc);
            StringType out;
            out.reserve(input.size());

            std::transform(input.begin(), input.end(), std::back_inserter(out),
                [&facet, isLower](CharType c) { return isLower?facet.tolower(c):facet.toupper(c); });

            return out;
        }

        static StringType&  MakeUpperCase(StringType& str)
        {
            to_lower_upper(str.data(), str.size(), false);
            return str;
        }

        static StringType&  MakeLowerCase(StringType& str)
        {
            to_lower_upper(str.data(), str.size(), true);
            return str;
        }

        static StringType UpperCase(typename StringTraits::stringview_type str)
        {
            return to_lower_upper_copy(str, false);
        }

        static StringType LowerCase(typename StringTraits::stringview_type str)
        {
            return to_lower_upper_copy(str, true);
        }

        template <typename T>
        static StringType ToString(const T& value)
        {
            return StringTraits::ToString(value);
        }

        static double ToDouble(typename StringTraits::stringview_type s1)
        {
            return StringTraits::ToDouble(s1.data());
        }

        static StringType TrimDouble(double val, int numDigits = 8)
        {
            return StringTraits::TrimDouble(val, numDigits);
        }

        static double ToDouble(const CharType* s1, double defVal = 0.0)
        {
            return s1?StringTraits::ToDouble(s1):defVal;
        }

        static StringType StringFromUChars(const typename StringTraits::uchar_type* val, size_t nSize)
        {
            return StringType(val, val + nSize);
        }

        static StringType HexStringFromUChars(const typename StringTraits::uchar_type* val, size_t nSize)
        {
            StringType hex_output_vector;
            boost::algorithm::hex_lower(val, val + nSize, std::back_inserter(hex_output_vector));
            return hex_output_vector;
        }

        static std::vector<typename StringTraits::uchar_type> UCharsFromString(typename StringTraits::stringview_type str)
        {
            return std::vector<typename StringTraits::uchar_type>(str.begin(), str.end());
        }

        static int ReverseFind(typename StringTraits::stringview_type str, CharType ch)
        {
            return static_cast<int>(str.rfind(ch));
        }

        static CharType* GetBuffer(const StringType& str)
        {
            return str.c_str();
        }

        static CharType* SafeStrcpy( CharType *pDest,
                                     const CharType* pSrc,
                                     size_t nMaxChars)
        {
            if ( !pSrc || !pDest)
                return pDest;
            size_t nLen = StringTraits::Length( pSrc );
            if ( nMaxChars < nLen )
                nLen = nMaxChars;
            StringTraits::CopyN( pDest, pSrc, nLen );
            pDest[nLen] = 0;
            return pDest;
        }

        static CharType* SafeStrcpy( CharType *pDest,
                                     const CharType* pSrc)
        {
            if ( !pSrc || !pDest)
                return pDest;
            return StringTraits::Copy( pDest, pSrc );
        }

        static void SplitPath(typename StringTraits::stringview_type str, StringArrayType & rArray)
        {
            static constexpr int numComponents = 5;
            typename StringTraits::FILESYSTEM_PATHTYPE p(str);
            rArray.clear();
            if (str.empty())
            {
                rArray.resize(numComponents);
                return;
            }
            rArray.emplace_back(StringTraits::PathGenericString(p.root_name()));
            rArray.emplace_back(StringTraits::PathGenericString(p.root_directory()));
            rArray.emplace_back(StringTraits::PathGenericString(p.parent_path()));
            rArray.emplace_back(StringTraits::PathGenericString(p.stem()));
            rArray.emplace_back(StringTraits::PathGenericString(p.extension()));
            for (auto& name : rArray)
            {
                if (!name.empty())
                {
                    if constexpr (std::is_same_v<StringType, std::string>)
                        name = std::filesystem::path(name).make_preferred().string();
                    else
                        name = std::filesystem::path(name).make_preferred().native();
                }
            }
        }

        static StringArrayType SplitPath(typename StringTraits::stringview_type str)
        {
            StringArrayType sArrType;
            SplitPath(str, sArrType);
            return sArrType;
        }

        static StringType GetFileNameFromPath(typename StringTraits::stringview_type str)
        {
            StringArrayType rArray;
            SplitPath(str, rArray);
            return rArray[NAME_POS] + StringType(".") + rArray[EXTENSION_POS];
        }

        static StringType MakePath(const StringArrayType & rArray)
        {
            if ( rArray.size() < 5 )
                return StringTraits::GetEmptyString();
            StringType s = rArray[NAME_POS] + rArray[EXTENSION_POS];
            const filesys::path dir(rArray[DIRECTORY_POS]);
            const filesys::path file = s; 
            filesys::path full_path = dir / file;
            s = StringTraits::PathGenericString(full_path);
            return s;
        }

        static StringType GetWindowsDirectory()
        {
            StringType buffer;
            const UINT retValue = StringTraits::GetWindowsDirectoryImpl(buffer);
            if ( retValue != 0 )
                return buffer;
            return StringTraits::GetEmptyString();
        }

        static StringType GetSystemDirectory()
        {
            StringType buffer;
            const UINT retValue = StringTraits::GetSystemDirectoryImpl(buffer);
            if ( retValue != 0 )
                return buffer;
            return StringTraits::GetEmptyString();
        }

        static StringType AddBackslashToDirectory(typename StringTraits::stringview_type pathName)
        {
            std::filesystem::path fsPath(pathName);
            fsPath /= StringTraits::GetEmptyString();
            if constexpr (std::is_same_v<std::string_view, StringTraits::stringview_type>)
                return fsPath.string();
            else
                return fsPath.native();
        }

        static StringType RemoveBackslashFromDirectory(StringType pathName)
        {
            if (!pathName.empty())
            {
                if (pathName.back() == filesys::path::preferred_separator)
                    pathName.pop_back();
            }
            return pathName;
        }

        static StringType GenerateUUIDv4()
        {
            using char_type = typename StringType::value_type;

            static_assert(std::is_same_v<StringType, std::string> ||std::is_same_v<StringType, std::wstring>,
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

        static StringType GetGUID()
        {
            return StringTraits::GetLeftCurlyBrace() + GenerateUUIDv4() + StringTraits::GetRightCurlyBrace();
        }

        static StringType GetModuleFileName(HMODULE hModule)
        {
            // Try 1024 bytes for the app name
            StringType szName(1024,0);
            DWORD nBytes = StringTraits::GetModuleFileNameImpl(hModule, szName, 1024);

            // Get the file name safely
            if (nBytes > 1024)
            {
                szName.resize(nBytes + 1, 0);
                StringTraits::GetModuleFileNameImpl(hModule, szName, nBytes);
            }
            return szName;
        }

        static StringType ConvertToAPIString(const StringType& origString)
        {
            return boost::algorithm::replace_all_copy(origString, StringTraits::GetNewLineString(), StringTraits::GetWindowsNewLineString());
        }

        static HANDLE ConvertToAPIStringEx(typename StringTraits::stringview_type origString)
        {
            StringType newString = ConvertToAPIString(origString.data());
            HANDLE newHandle = GlobalAlloc(GHND | GMEM_ZEROINIT, newString.size() * sizeof(StringTraits::char_type) + sizeof(StringTraits::char_type));
            if (newHandle)
            {
                typename StringTraits::char_type* pData = (typename StringTraits::char_type*)GlobalLock(newHandle);
                memcpy(pData, newString.data(), newString.size() * sizeof(StringTraits::char_type));
                GlobalUnlock(newHandle);
                return newHandle;
            }
            return NULL;
        }

        static int TokenizeEx(const StringType& str,
                              const typename StringType::value_type* lpszTokStr,
                              StringArrayType& rArray,
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

        static int TokenizeQuotedEx(const StringType& str,
                                    const typename StringType::value_type* lpszTokStr,
                                    StringArrayType& rArray,
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

        // If szInfo is nullptr, only the computed length is returned.
        // The length includes trailing null character.
        static int32_t CopyInfoToCString(typename StringTraits::stringview_type strInfo, CharType* szInfo, int32_t nMaxLen)
        {
            if (strInfo.empty())
            {
                if ( szInfo && nMaxLen > 0 )
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
            int32_t nRealLen;
            if (szInfo != nullptr && nMaxLen >= 0)
            {
                const size_t nLen = strInfo.size();
                nRealLen = static_cast<int32_t>((std::min)(static_cast<size_t>(nMaxLen), nLen));
                CharType* pEnd = std::copy(strInfo.begin(), strInfo.begin() + static_cast<size_t>(nRealLen), szInfo);
                *pEnd = StringTraits::GetZeroString();
            }
            else
                nRealLen = static_cast<int32_t>(strInfo.size());
            ++nRealLen;
            return nRealLen;
        }

        static typename StringTraits::string_type CreateFileNameFromNumber(typename StringTraits::string_type sFileName, int num, int nDigits)
        {
            typename StringTraits::stringarray_type rArray = {};
            SplitPath(sFileName, rArray);

            // Adjust the file name
            typename StringTraits::outputstream_type strm{};
            strm << std::setfill(StringTraits::GetZeroNumericString()) << std::setw(nDigits) << num;
            typename StringTraits::string_type szBuf = strm.str();
            typename StringTraits::string_type& sTemp = rArray[NAME_POS];
            sTemp = sTemp.substr(0, sTemp.length() - nDigits) + szBuf;
            return MakePath(rArray);
        }

        static int GetInitialFileNumber(typename StringTraits::string_type sFileName, size_t &nDigits)
        {
            typename StringTraits::stringarray_type rArray = {};
            SplitPath(sFileName, rArray);
            nDigits = 0;
            typename StringTraits::string_type sTemp;
            const size_t nLen = rArray[NAME_POS].length();
            for (size_t i = nLen - 1; ; --i)
            {
                if (StringTraits::IsDigit(rArray[NAME_POS][i]))
                {
                    sTemp = rArray[NAME_POS][i] + sTemp;
                    nDigits++;
                }
                else
                    break;
                if (i == 0)
                    break;
            }

            // now loop until we get a good cast from the string we have
            while (!sTemp.empty())
            {
                try
                {
                    return boost::lexical_cast<int>(sTemp);
                }
                catch (boost::bad_lexical_cast&)
                {
                    sTemp.erase(sTemp.begin());
                }
            }
            return 0;
        }

        ////////////////////////////////////////////////////////////////////////////
        static typename StringTraits::string_type GetPageFileName(typename StringTraits::string_type strBase, int nCurImage, bool bUseLongNames)
        {
            typename StringTraits::string_type strFormat;
            typename StringTraits::outputstream_type strm{};
            strm << nCurImage;
            strFormat = strm.str();
            const int nLenFormat = static_cast<int>(strFormat.length());

            typename StringTraits::stringarray_type rName = {};
            SplitPath(strBase, rName);

            auto strName = rName[NAME_POS];

            if (bUseLongNames)
                strName += strFormat;
            else
            {
                if ((strName + strFormat).length() > 8)
                {
                    int nBase = 8 - nLenFormat;
                    strName = Left(strName, nBase) + strFormat;
                }
                else
                    strName += strFormat;
            }
            rName[NAME_POS] = strName;
            return MakePath(rName);
        }
    };

    typedef StringWrapper_Impl<std::string, char, ANSIStringTraits> StringWrapperA;
    typedef StringWrapper_Impl<std::wstring, wchar_t, UnicodeStringTraits> StringWrapperW;
    typedef UnicodeStringTraits StringTraitsW;
    typedef ANSIStringTraits    StringTraitsA;

    #define GET_NUM_CHARS_NATIVE(x) (sizeof(x) / sizeof((x)[0])
    #define GET_NUM_BYTES_NATIVE(x) sizeof(x)

    #ifdef UNICODE
        typedef StringWrapperW StringWrapper;
        typedef StringTraitsW StringTraits;
    #else
        typedef StringWrapperA StringWrapper;
        typedef StringTraitsA   StringTraits;
    #endif
}
#endif
