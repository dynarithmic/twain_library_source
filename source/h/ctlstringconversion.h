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
#ifndef CTLSTRINGCONVERSION_H
#define CTLSTRINGCONVERSION_H

#include <string>
#include <vector>
#include <string_view>

namespace dynarithmic
{
#ifdef WIN32
#include "ansiwideconverter_win32.h"
#else
#include "ansiwideconverter_generic.h"
#endif
    struct StringConversion
    {
#ifdef UNICODE
        static std::wstring          Convert_Ansi_To_Native(std::string_view x) { return ANSIToWide(x); }
        static std::wstring          Convert_AnsiPtr_To_Native(const char* x) { return ANSIToWide(x ? x : ""); }

        static const std::wstring&   Convert_Wide_To_Native(const std::wstring& x) { return x; }
        static std::wstring          Convert_WidePtr_To_Native(const wchar_t* x) { return x ? x : L""; }

        static std::string           Convert_Native_To_Ansi(std::wstring_view x) { return WideToANSI(x); }
        static std::string           Convert_NativePtr_To_Ansi(const wchar_t* x) { return WideToANSI(x ? x : L""); }

        static const std::wstring& Convert_Native_To_Wide(const std::wstring& x) { return x; }
        static std::wstring          Convert_NativePtr_To_Wide(const wchar_t* x) { return x ? x : L""; }

        static std::wstring          Convert_Ansi_To_Native(std::string_view x, size_t len) { return ANSIToWide(x, len); }
        static std::wstring          Convert_AnsiPtr_To_Native(const char* x, size_t len) { return ANSIToWide(x ? x : "", len); }

        static const std::wstring& Convert_Wide_To_Native(const std::wstring& x, size_t len) { return x; }
        static std::wstring          Convert_WidePtr_To_Native(const wchar_t* x, size_t len) { if (x) return { x, len }; return {}; }

        static std::string           Convert_Native_To_Ansi(std::wstring_view x, size_t len) { return WideToANSI(x, len); }
        static std::string           Convert_NativePtr_To_Ansi(const wchar_t* x, size_t len) { return WideToANSI(x ? x : L"", len); }

        static const std::wstring& Convert_Native_To_Wide(const std::wstring& x, size_t) { return x; }
        static std::wstring          Convert_NativePtr_To_Wide(const wchar_t* x, size_t) { return x ? x : L""; }

#else
        static const std::string& Convert_Ansi_To_Native(const std::string& x) { return x; }
        static std::string    Convert_AnsiPtr_To_Native(const char* x) { return x ? x : ""; }

        static std::string    Convert_Wide_To_Native(std::wstring_view x) { return WideToANSI(x); }
        static std::string    Convert_WidePtr_To_Native(const wchar_t* x) { return WideToANSI(x ? x : L""); }

        static const std::string& Convert_Native_To_Ansi(const std::string& x) { return x; }
        static std::string    Convert_NativePtr_To_Ansi(const char* x) { return x ? x : ""; }

        static std::wstring   Convert_Native_To_Wide(std::string_view x) { return ANSIToWide(x); }
        static std::wstring   Convert_NativePtr_To_Wide(const char* x) { return ANSIToWide(x ? x : ""); }

        static const std::string& Convert_Ansi_To_Native(const std::string& x, size_t len) { return x; }
        static std::string    Convert_AnsiPtr_To_Native(const char* x, size_t len) { if (x) return { x, len }; return {}; }

        static std::string    Convert_Wide_To_Native(std::wstring_view x, size_t len) { return WideToANSI(x, len); }
        static std::string    Convert_WidePtr_To_Native(const wchar_t* x, size_t len) { return WideToANSI(x ? x : L"", len); }

        static const std::string& Convert_Native_To_Ansi(const std::string& x, size_t) { return x; }
        static std::string    Convert_NativePtr_To_Ansi(const char* x, size_t) { return x ? x : ""; }

        static std::wstring   Convert_Native_To_Wide(std::string_view x, size_t len) { return ANSIToWide(x, len); }
        static std::wstring   Convert_NativePtr_To_Wide(const char* x, size_t len) { return ANSIToWide(x ? x : "", len); }
#endif
        static std::string     Convert_Wide_To_Ansi(std::wstring_view x) { return WideToANSI(x); }
        static std::wstring    Convert_Ansi_To_Wide(std::string_view x) { return ANSIToWide(x); }
        static std::string     Convert_WidePtr_To_Ansi(const wchar_t* x) { return x ? WideToANSI(x) : ""; }
        static std::wstring     Convert_AnsiPtr_To_Wide(const char* x) { return x ? ANSIToWide(x) : L""; }

        static std::string     Convert_Wide_To_Ansi(std::wstring_view x, size_t len) { return WideToANSI(x, len); }
        static std::wstring    Convert_Ansi_To_Wide(std::string_view x, size_t len) { return ANSIToWide(x, len); }
        static std::string     Convert_WidePtr_To_Ansi(const wchar_t* x, size_t len) { return x ? WideToANSI(x, len) : ""; }
        static std::wstring    Convert_AnsiPtr_To_Wide(const char* x, size_t len) { return x ? ANSIToWide(x, len) : L""; }

#ifdef _WIN32
        using utf16strType = std::wstring;
        using utf16viewType = std::wstring_view;
#else
        using utf16strType = std::u16string;
        using utf16viewType = std::u16string_view;
#endif
        static std::pair<utf16strType, bool> Convert_UTF8_To_UTF16(std::string_view utf8, bool bMakeWideIfError = true)
        {
            auto pr = UTF8_UTF16_Converter::UTF8ToUTF16(utf8);
            if (!pr.second && bMakeWideIfError)
                return { StringConversion::Convert_Ansi_To_Wide(utf8), false }; // Right now, only works for Windows
            return pr;
        }

        static std::pair<std::string, bool> Convert_UTF16_To_UTF8(utf16viewType utf16, bool bMakeAnsiIfError = true)
        {
            auto pr = UTF8_UTF16_Converter::UTF16ToUTF8(utf16);
            if (!pr.second && bMakeAnsiIfError)
                return { StringConversion::Convert_Wide_To_Ansi(utf16), false }; // Right now, only works for Windows
            return pr;
        }

        static std::string WideToANSI(std::wstring_view wstr)
        {
            return static_cast<LPCSTR>(ConvertW2A(wstr.data()));
        }

        static std::wstring ANSIToWide(std::string_view str)
        {
            return static_cast<LPCWSTR>(ConvertA2W(str.data()));
        }

        static std::string WideToANSI(std::wstring_view wstr, size_t len)
        {
            return static_cast<LPCSTR>(ConvertW2A(wstr.data(), len));
        }

        static std::wstring ANSIToWide(std::string_view str, size_t len)
        {
            return static_cast<LPCWSTR>(ConvertA2W(str.data(), len));
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
            {}

            typename T::value_type* getBuffer()
            {
                return m_vChar.data();
            }
        };

        static CTL_StringVector<std::string> WideToANSIWriteable(std::wstring_view wstr)
        {
            const ConvertW2A conv(wstr.data());
            CTL_StringVector<std::string> ret(static_cast<LPCSTR>(conv));
            return ret;
        }

        static CTL_StringVector<std::wstring> ANSIToWideWriteable(std::string_view str)
        {
            const ConvertA2W conv(str.data());
            CTL_StringVector<std::wstring> ret(static_cast<LPCWSTR>(conv));
            return ret;
        }
    };
}
#endif


