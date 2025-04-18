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
#ifndef CTLOBSTR_H
#define CTLOBSTR_H

#include <string>
#include <vector>
#include <sstream>
#include <cctype>
#include <numeric>
#include <type_traits>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/format.hpp>
#include <assert.h>
#include <algorithm>
#include <stdlib.h>
#include <iomanip>
#include <locale>
#include <iostream>
#include <boost/lexical_cast.hpp>
#include <dtwain_filesystem.h>
#include "dtwain_standard_defs.h"
#ifndef _MAX_PATH
#define _MAX_PATH 260
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
        using string_type = std::string;
        using stringarray_type = std::vector<string_type>;
        using outputstream_type = std::ostringstream;
        using inputstream_type = std::istringstream;
        using outputfile_type = std::ofstream;
        using inputfile_type = std::ifstream;
        using baseoutputstream_type = std::ostream;
        using baseinputstream_type = std::istream;

        using BOOST_FORMAT = boost::format;
        using FILESYSTEM_PATHTYPE = filesys::path;

        template <typename T>
        static std::string PathGenericString(const T& x) { return x.generic_string(); }

        template <typename T>
        static std::string ConvertToBoostUUIDString(const T& x)  { return boost::uuids::to_string(x); }

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
        #ifdef _WIN32
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
                    { return GetWindowsDirectoryA(buffer, _MAX_PATH); }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
                    { return GetSystemDirectoryA(buffer, _MAX_PATH); }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
                    { return GetModuleFileNameA(hModule, lpFileName, nSize); }
        #else
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
        { getcwd(buffer, 8096); return 1; }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
        { getcwd(buffer, 8096); return 1; }

        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
        {
            return 0; // ::GetModuleFileNameA(hModule, lpFileName, nSize);
        }
        #endif
    };

    struct UnicodeStringTraits
    {
        using char_type = wchar_t;
        using string_type = std::wstring;
        using stringarray_type = std::vector<string_type>;
        using outputstream_type = std::wostringstream;
        using inputstream_type = std::wistringstream;
        using outputfile_type = std::wofstream;
        using inputfile_type = std::wifstream;
        using baseoutputstream_type = std::wostream;
        using baseinputstream_type = std::wistream;

        using BOOST_FORMAT = boost::wformat;
        using FILESYSTEM_PATHTYPE = filesys::path;

        template <typename T>
        static std::wstring PathGenericString(const T& x) { return x.generic_wstring(); }

        template <typename T>
        static std::wstring ConvertToBoostUUIDString(const T& x) { return boost::uuids::to_wstring(x); }

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
        #ifdef _WIN32
        static UINT GetWindowsDirectoryImpl(char_type* buffer)
        { return GetWindowsDirectoryW(buffer, _MAX_PATH); }
        static UINT GetSystemDirectoryImpl(char_type* buffer)
        { return GetSystemDirectoryW(buffer, _MAX_PATH); }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
        { return GetModuleFileNameW(hModule, lpFileName, nSize); }
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

        static DWORD GetModuleFileNameImpl(HMODULE hModule, char_type* lpFileName, DWORD nSize)
        {
            return 0; // ::GetModuleFileNameA(hModule, lpFileName, nSize);
        }
        #endif
    };

    #ifdef WIN32
    #include "ansiwideconverter_win32.h"
    #else
    #include "ansiwideconverter_generic.h"
    #endif
    struct StringConversion
    {

        #ifdef UNICODE
        static std::wstring          Convert_Ansi_To_Native(const std::string& x) { return ANSIToWide(x); }
        static std::wstring          Convert_AnsiPtr_To_Native(const char *x) { return ANSIToWide(x?x:""); }

        static const std::wstring&   Convert_Wide_To_Native(const std::wstring& x) { return x; }
        static std::wstring          Convert_WidePtr_To_Native(const wchar_t* x) { return x?x:L""; }

        static std::string           Convert_Native_To_Ansi(const std::wstring& x) { return WideToANSI(x); }
        static std::string           Convert_NativePtr_To_Ansi(const wchar_t *x) { return WideToANSI(x?x:L""); }

        static const std::wstring&   Convert_Native_To_Wide(const std::wstring& x) { return x; }
        static std::wstring          Convert_NativePtr_To_Wide(const wchar_t *x) { return x?x:L""; }

        static std::wstring          Convert_Ansi_To_Native(const std::string& x, size_t len) { return ANSIToWide(x, len); }
        static std::wstring          Convert_AnsiPtr_To_Native(const char* x, size_t len) { return ANSIToWide(x ? x : "", len); }

        static const std::wstring&   Convert_Wide_To_Native(const std::wstring& x, size_t len) { return x; }
        static std::wstring          Convert_WidePtr_To_Native(const wchar_t* x, size_t len) { if (x) return { x, len }; return {}; }

        static std::string           Convert_Native_To_Ansi(const std::wstring& x, size_t len) { return WideToANSI(x, len); }
        static std::string           Convert_NativePtr_To_Ansi(const wchar_t* x, size_t len) { return WideToANSI(x ? x : L"", len); }

        static const std::wstring&   Convert_Native_To_Wide(const std::wstring& x, size_t) { return x; }
        static std::wstring          Convert_NativePtr_To_Wide(const wchar_t* x, size_t) { return x?x:L""; }

        #else
        static const std::string&   Convert_Ansi_To_Native(const std::string& x) { return x; }
        static std::string    Convert_AnsiPtr_To_Native(const char *x) { return x?x:""; }

        static std::string    Convert_Wide_To_Native(const std::wstring& x) {return WideToANSI(x);}
        static std::string    Convert_WidePtr_To_Native(const wchar_t *x) { return WideToANSI(x?x:L""); }

        static const std::string&   Convert_Native_To_Ansi(const std::string& x) { return x; }
        static std::string    Convert_NativePtr_To_Ansi(const char *x)  { return x?x:""; }

        static std::wstring   Convert_Native_To_Wide(const std::string& x) { return ANSIToWide(x); }
        static std::wstring   Convert_NativePtr_To_Wide(const char *x) { return ANSIToWide(x?x:""); }

        static const std::string& Convert_Ansi_To_Native(const std::string& x, size_t len) { return x; }
        static std::string    Convert_AnsiPtr_To_Native(const char* x, size_t len) { if (x) return {x, len}; return {}; }

        static std::string    Convert_Wide_To_Native(const std::wstring& x, size_t len) { return WideToANSI(x, len); }
        static std::string    Convert_WidePtr_To_Native(const wchar_t* x, size_t len) { return WideToANSI(x ? x : L"", len); }

        static const std::string& Convert_Native_To_Ansi(const std::string& x, size_t) { return x; }
        static std::string    Convert_NativePtr_To_Ansi(const char* x, size_t) { return x?x:""; }

        static std::wstring   Convert_Native_To_Wide(const std::string& x, size_t len) { return ANSIToWide(x, len); }
        static std::wstring   Convert_NativePtr_To_Wide(const char* x, size_t len) { return ANSIToWide(x ? x : "", len); }

#endif
        static std::string     Convert_Wide_To_Ansi(const std::wstring& x) { return WideToANSI(x); }
        static std::wstring    Convert_Ansi_To_Wide(const std::string& x) { return ANSIToWide(x); }
        static std::string     Convert_WidePtr_To_Ansi(const wchar_t *x) { return x ? WideToANSI(x) : ""; }
        static std::wstring     Convert_AnsiPtr_To_Wide(const char *x) { return x ? ANSIToWide(x) : L""; }

        static std::string     Convert_Wide_To_Ansi(const std::wstring& x, size_t len) { return WideToANSI(x, len); }
        static std::wstring    Convert_Ansi_To_Wide(const std::string& x, size_t len) { return ANSIToWide(x, len); }
        static std::string     Convert_WidePtr_To_Ansi(const wchar_t* x, size_t len) { return x ? WideToANSI(x, len) : ""; }
        static std::wstring    Convert_AnsiPtr_To_Wide(const char* x, size_t len) { return x ? ANSIToWide(x, len) : L""; }

        static std::string WideToANSI(const std::wstring& wstr)
        {
            return static_cast<LPCSTR>(ConvertW2A(wstr.c_str()));
        }

        static std::wstring ANSIToWide(const std::string& str)
        {
            return static_cast<LPCWSTR>(ConvertA2W(str.c_str()));
        }

        static std::string WideToANSI(const std::wstring& wstr, size_t len)
        {
            return static_cast<LPCSTR>(ConvertW2A(wstr.c_str(), len));
        }

        static std::wstring ANSIToWide(const std::string& str, size_t len)
        {
            return static_cast<LPCWSTR>(ConvertA2W(str.c_str(), len));
        }

        template <typename T>
        struct CTL_StringVector
        {
            std::vector<typename T::value_type> m_vChar;
            CTL_StringVector(const T& s) : m_vChar(s.begin(), s.end())
            {
                m_vChar.push_back(0);
            }

            CTL_StringVector(size_t nSize) : m_vChar(nSize, 0)
            { }

            typename T::value_type* getBuffer()
            {
                return m_vChar.data();
            }
        };

        static CTL_StringVector<std::string> WideToANSIWriteable(const std::wstring& wstr)
        {
            const ConvertW2A conv(wstr.c_str());
            CTL_StringVector<std::string> ret(static_cast<LPCSTR>(conv));
            return ret;
        }

        static CTL_StringVector<std::wstring> ANSIToWideWriteable(const std::string& str)
        {
            const ConvertA2W conv(str.c_str());
            CTL_StringVector<std::wstring> ret(static_cast<LPCWSTR>(conv));
            return ret;
        }
    };

    template <typename StringType, typename CharType, typename StringTraits>
    struct StringWrapper_Impl
    {
        typedef StringTraits traits_type;
        enum { DRIVE_POS, DRIVE_PATH, DIRECTORY_POS, NAME_POS, EXTENSION_POS };
        typedef std::vector<StringType> StringArrayType;

        // define string helper functions here
        static StringType Right(const StringType& str, size_t nNum)
        {
            const size_t nLen = str.length();
            if (nNum == 0)
                return StringTraits::GetEmptyString();
            if (nNum > nLen)
                nNum = nLen;
            return str.substr(nLen - nNum, nNum);
        }

        static StringType Mid(const StringType& str, size_t nFirst)
        {
            if (nFirst == 0)
                return str;
            return str.substr(nFirst);
        }

        static StringType Mid(const StringType& str, size_t  nFirst, size_t nNum)
        {
            return str.substr(nFirst, nNum);
        }

        static StringType Left(const StringType& str, size_t nNum)
        {
            return Mid(str, 0, nNum);
        }

        static CharType GetAt(const StringType &str, size_t nPos)
        {
            assert(nPos < str.length());
            return str[nPos];
        }

        static void SetAt(StringType& str, size_t nPos, CharType c)
        {
            str.replace(nPos, 1, 1, c);
        }

        static bool IsEmpty(const StringType& str)
        {
            return str.empty();
        }

        static void Empty(StringType &str )
        {
            str = StringTraits::GetEmptyString();
        }

        static StringType ReplaceAll(const StringType& str, const StringType& findStr, const StringType& replaceStr)
        {
            return boost::algorithm::replace_all_copy(str, findStr, replaceStr);
        }

        static StringType&  TrimRight(StringType& str, const CharType *lpszTrimStr)
        {
            boost::trim_right_if(str, boost::is_any_of(lpszTrimStr));
            return str;
        }

        static StringType& TrimRight(StringType &str, CharType ch= StringTraits::GetSpace() )
        {
            CharType sz[2] = {};
            sz[0]=ch; sz[1] = 0;
            return TrimRight(str, sz);
        }

        static StringType& TrimLeft(StringType& str, const CharType * lpszTrimStr)
        {
            boost::trim_left_if(str, boost::is_any_of(lpszTrimStr));
            return str;
        }

        static StringType& TrimLeft(StringType& str, CharType ch= StringTraits::GetSpace() )
        {
            CharType sz[2] = {};
            sz[0]=ch; sz[1] = 0;
            return TrimLeft(str, sz);
        }

        static StringType& TrimAll(StringType& str, CharType ch = StringTraits::GetSpace())
        {
            TrimRight( str, ch );
            TrimLeft( str, ch );
            return str;
        }

        static StringType& TrimAll(StringType& str, const CharType *lpszTrimStr)
        {
            TrimRight( str, lpszTrimStr );
            TrimLeft( str, lpszTrimStr );
            return str;
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
            typename strtraits::outputstream_type strm;
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

        static bool StartsWith(const StringType& str, const StringType& sub)
        {
            return boost::algorithm::starts_with(str, sub);
        }

        static bool StartsWith(const StringType& str, const CharType* sub)
        {
            return boost::algorithm::starts_with(str, sub);
        }

        static bool EndsWith(const StringType& str, const StringType& sub)
        {
            return boost::algorithm::ends_with(str, sub);
        }

        static bool EndsWith(const StringType& str, const CharType* sub)
        {
            return boost::algorithm::ends_with(str, sub);
        }

        static StringType QuoteString(const StringType& str, 
                                      const StringType& quoteString = typename traits_type::DoubleQuoteString())
        {
            return quoteString + str + quoteString;
        }

        static int TokenizeQuoted(const StringType& str, const CharType *lpszTokStr,
                                  StringArrayType &rArray, bool bGetNullTokens = false)
        {
            return TokenizeQuotedEx(str, lpszTokStr, rArray, bGetNullTokens);
        }

        static int Compare(const StringType& str, const CharType* lpsz)
        {
            return str.compare(lpsz);
        }

        static bool CompareNoCase(const StringType& str, const CharType* lpsz)
        {
            return boost::iequals(str, lpsz);
        }

        static StringType&  MakeUpperCase(StringType& str)
        {
            boost::algorithm::to_upper(str);
            return str;
        }

        static StringType&  MakeLowerCase(StringType& str)
        {
            boost::algorithm::to_lower(str);
            return str;
        }

        static StringType UpperCase(const StringType& str)
        {
            return boost::algorithm::to_upper_copy(str);
        }

        static StringType LowerCase(const StringType& str)
        {
            return boost::algorithm::to_lower_copy(str);
        }

        template <typename T>
        static StringType ToString(const T& value)
        {
            return StringTraits::ToString(value);
        }

        static double ToDouble(const StringType& s1)
        {
            return StringTraits::ToDouble(s1.c_str());
        }

        static int ReverseFind(const StringType& str, CharType ch)
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

        static void SplitPath(const StringType& str, StringArrayType & rArray)
        {
            static constexpr int numComponents = 5;
            typename StringTraits::FILESYSTEM_PATHTYPE p(str.c_str());
            rArray.clear();
            if (str.empty())
            {
                rArray.resize(numComponents);
                return;
            }
            rArray.push_back(StringTraits::PathGenericString(p.root_name()));
            rArray.push_back(StringTraits::PathGenericString(p.root_directory()));
            rArray.push_back(StringTraits::PathGenericString(p.parent_path()));
            rArray.push_back(StringTraits::PathGenericString(p.stem()));
            rArray.push_back(StringTraits::PathGenericString(p.extension()));
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

        static StringArrayType SplitPath(const StringType& str)
        {
            StringArrayType sArrType;
            SplitPath(str, sArrType);
            return sArrType;
        }

        static StringType GetFileNameFromPath(const StringType& str)
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
            const filesys::path file = s; //rArray[NAME_POS] + StringType(".") + rArray[EXTENSION_POS];
            filesys::path full_path = dir / file;
            s = StringTraits::PathGenericString(full_path);
            return s;
            // std::vector<CharType> retStr(MAX_PATH, 0);
            /*_tmakepath( &retStr[0], rArray[DRIVE_POS].c_str(),
                                    rArray[DIRECTORY_POS].c_str(),
                                    rArray[NAME_POS].c_str(),
                                    rArray[EXTENSION_POS].c_str());*/

            // return &retStr[0];
        }

        static StringType GetWindowsDirectory()
        {
            CharType buffer[_MAX_PATH] = {};
            const UINT retValue = StringTraits::GetWindowsDirectoryImpl(buffer);
            if ( retValue != 0 )
                return buffer;
            return StringTraits::GetEmptyString();
        }

        static StringType GetSystemDirectory()
        {
            CharType buffer[_MAX_PATH] = {};
            const UINT retValue = StringTraits::GetSystemDirectoryImpl(buffer);
            if ( retValue != 0 )
                return buffer;
            return StringTraits::GetEmptyString();
        }

        static StringType AddBackslashToDirectory(const StringType& pathName)
        {
            std::filesystem::path fsPath(pathName);
            fsPath /= StringTraits::GetEmptyString();
            if constexpr (std::is_same_v<std::string, StringType>)
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

        static StringType GetGUID()
        {
            const boost::uuids::uuid u = boost::uuids::random_generator()();
            std::ostringstream strm;
            strm << "{" + to_string(u) + "}";
            StringType sTemp;
            std::string sTempIn = strm.str();
            std::copy(sTempIn.begin(), sTempIn.end(), std::back_inserter(sTemp));
            return sTemp;
        }

        static StringType GetModuleFileName(HMODULE hModule)
        {
            // Try 1024 bytes for the app name
            std::vector<CharType> szName(1024,0);
            DWORD nBytes = StringTraits::GetModuleFileNameImpl(hModule, &szName[0], 1024);

            // Get the file name safely
            if ( nBytes > 1024 )
            {
                szName.resize(nBytes+1,0);
                StringTraits::GetModuleFileNameImpl( hModule, &szName[0], nBytes );
            }
            return &szName[0];
        }

        static StringType ConvertToAPIString(const StringType& origString)
        {
            return boost::algorithm::replace_all_copy(origString, StringTraits::GetNewLineString(), StringTraits::GetWindowsNewLineString());
        }

        static HANDLE ConvertToAPIStringEx(const StringType& origString)
        {
            StringType newString = ConvertToAPIString(origString);
            HANDLE newHandle = GlobalAlloc(GHND, newString.size() * sizeof(StringTraits::char_type) + sizeof(StringTraits::char_type));
            if (newHandle)
            {
                StringTraits::char_type* pData = (StringTraits::char_type*)GlobalLock(newHandle);
                memset(pData, 0, GlobalSize(newHandle));
                memcpy(pData, newString.data(), newString.size() * sizeof(StringTraits::char_type));
                GlobalUnlock(newHandle);
                return newHandle;
            }
            return NULL;
        }

        static int TokenizeEx(const StringType& str,
                              const CharType *lpszTokStr,
                              StringArrayType &rArray,
                              bool bGetNullTokens,
                              std::vector<unsigned>* positionArray= nullptr)
        {
            rArray.clear();
            if (!lpszTokStr)
                return 0;
            typedef boost::tokenizer<boost::char_separator<CharType>,
                                     typename StringType::const_iterator,
                                     StringType> tokenizer;
            boost::empty_token_policy tokenPolicy = bGetNullTokens?boost::keep_empty_tokens : boost::drop_empty_tokens;
            boost::char_separator<CharType> sepr(lpszTokStr, StringTraits::GetEmptyString(), tokenPolicy);
            tokenizer tokens(str, sepr);
            for (typename tokenizer::const_iterator tok_iter = tokens.begin();
                tok_iter != tokens.end(); ++tok_iter)
            {
                rArray.push_back(*tok_iter);
                if ( positionArray )
                {
                    const std::ptrdiff_t offset = tok_iter.base() - str.begin() - tok_iter->size();
                    positionArray->push_back(static_cast<unsigned>(offset));
                }
            }
            return static_cast<int>(rArray.size());
        }

        static int TokenizeQuotedEx(const StringType& str,
                                    const CharType *lpszTokStr,
                                    StringArrayType &rArray,
                                    bool bGetNullTokens,
                                    std::vector<unsigned>* positionArray = nullptr)
        {
            rArray.clear();
            typedef boost::tokenizer<boost::escaped_list_separator<CharType>,
                                    typename StringType::const_iterator,
                                    StringType> tokenizer;

            boost::escaped_list_separator<CharType> sepr(StringTraits::GetEmptyString(), lpszTokStr, StringTraits::AllQuoteString());
            tokenizer tokens(str, sepr);
            for (auto tok_iter = tokens.begin(); tok_iter != tokens.end(); ++tok_iter)
            {
                rArray.push_back(*tok_iter);
                if (positionArray)
                {
                    const std::ptrdiff_t offset = tok_iter.base() - str.begin() - tok_iter->size();
                    positionArray->push_back(static_cast<unsigned>(offset));
                }
            }
            if ( !bGetNullTokens )
            {
                std::vector<size_t> removed_pos;
                for (size_t idx = 0; idx < rArray.size(); ++idx)
                {
                    if ( rArray[idx].empty() )
                        removed_pos.push_back(idx);
                }

                for (auto i : removed_pos)
                {
                    rArray.erase(rArray.begin() + i);
                    if ( positionArray )
                        positionArray->erase(positionArray->begin() + i);
                }
            }
            return static_cast<int>(rArray.size());
        }

        // If szInfo is nullptr, only the computed length is returned.
        // The length includes trailing null character.
        static int32_t CopyInfoToCString(const StringType& strInfo, CharType* szInfo, int32_t nMaxLen)
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
            typename StringTraits::outputstream_type strm;
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
            typename StringTraits::outputstream_type strm;
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
