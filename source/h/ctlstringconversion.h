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
#include <string_view>

namespace dynarithmic
{
#ifdef WIN32
#include "ansiwideconverter_win32.h"
#include "ansiwideconverter_win32ex.h"
#else
#include "ansiwideconverter_generic.h"
#endif
    namespace stringconversion
    {
#ifdef UNICODE
        inline std::wstring          Convert_Ansi_To_Native(std::string_view x) { return dynarithmic::ANSIToWide(x); }
        inline std::wstring          Convert_AnsiPtr_To_Native(const char* x) { return dynarithmic::ANSIToWide(x ? x : ""); }

        inline const std::wstring&   Convert_Wide_To_Native(const std::wstring& x) { return x; }
        inline std::wstring          Convert_WidePtr_To_Native(const wchar_t* x) { return x ? x : L""; }

        inline std::string           Convert_Native_To_Ansi(std::wstring_view x) { return dynarithmic::WideToANSI(x); }
        inline std::string           Convert_NativePtr_To_Ansi(const wchar_t* x) { return dynarithmic::WideToANSI(x ? x : L""); }

        inline const std::wstring& Convert_Native_To_Wide(const std::wstring& x) { return x; }
        inline std::wstring          Convert_NativePtr_To_Wide(const wchar_t* x) { return x ? x : L""; }

        inline std::wstring          Convert_Ansi_To_Native(std::string_view x, size_t len) { return dynarithmic::ANSIToWide(x, len); }
        inline std::wstring          Convert_AnsiPtr_To_Native(const char* x, size_t len) { return dynarithmic::ANSIToWide(x ? x : "", len); }

        inline const std::wstring& Convert_Wide_To_Native(const std::wstring& x, size_t len) { return x; }
        inline std::wstring          Convert_WidePtr_To_Native(const wchar_t* x, size_t len) { if (x) return { x, len }; return {}; }

        inline std::string           Convert_Native_To_Ansi(std::wstring_view x, size_t len) { return dynarithmic::WideToANSI(x, len); }
        inline std::string           Convert_NativePtr_To_Ansi(const wchar_t* x, size_t len) { return dynarithmic::WideToANSI(x ? x : L"", len); }

        inline const std::wstring& Convert_Native_To_Wide(const std::wstring& x, size_t) { return x; }
        inline std::wstring          Convert_NativePtr_To_Wide(const wchar_t* x, size_t) { return x ? x : L""; }

#else
        inline const std::string& Convert_Ansi_To_Native(const std::string& x) { return x; }
        inline std::string    Convert_AnsiPtr_To_Native(const char* x) { return x ? x : ""; }

        inline std::string    Convert_Wide_To_Native(std::wstring_view x) { return dynarithmic::WideToANSI(x); }
        inline std::string    Convert_WidePtr_To_Native(const wchar_t* x) { return dynarithmic::WideToANSI(x ? x : L""); }

        inline const std::string& Convert_Native_To_Ansi(const std::string& x) { return x; }
        inline std::string    Convert_NativePtr_To_Ansi(const char* x) { return x ? x : ""; }

        inline std::wstring   Convert_Native_To_Wide(std::string_view x) { return dynarithmic::ANSIToWide(x); }
        inline std::wstring   Convert_NativePtr_To_Wide(const char* x) { return dynarithmic::ANSIToWide(x ? x : ""); }

        inline const std::string& Convert_Ansi_To_Native(const std::string& x, size_t len) { return x; }
        inline std::string    Convert_AnsiPtr_To_Native(const char* x, size_t len) { if (x) return { x, len }; return {}; }

        inline std::string    Convert_Wide_To_Native(std::wstring_view x, size_t len) { return dynarithmic::WideToANSI(x, len); }
        inline std::string    Convert_WidePtr_To_Native(const wchar_t* x, size_t len) { return dynarithmic::WideToANSI(x ? x : L"", len); }

        inline const std::string& Convert_Native_To_Ansi(const std::string& x, size_t) { return x; }
        inline std::string    Convert_NativePtr_To_Ansi(const char* x, size_t) { return x ? x : ""; }

        inline std::wstring   Convert_Native_To_Wide(std::string_view x, size_t len) { return dynarithmic::ANSIToWide(x, len); }
        inline std::wstring   Convert_NativePtr_To_Wide(const char* x, size_t len) { return dynarithmic::ANSIToWide(x ? x : "", len); }
#endif
        inline std::string     Convert_Wide_To_Ansi(std::wstring_view x) { return dynarithmic::WideToANSI(x); }
        inline std::wstring    Convert_Ansi_To_Wide(std::string_view x) { return dynarithmic::ANSIToWide(x); }
        inline std::string     Convert_WidePtr_To_Ansi(const wchar_t* x) { return x ? dynarithmic::WideToANSI(x) : ""; }
        inline std::wstring     Convert_AnsiPtr_To_Wide(const char* x) { return x ? dynarithmic::ANSIToWide(x) : L""; }

        inline std::string     Convert_Wide_To_Ansi(std::wstring_view x, size_t len) { return dynarithmic::WideToANSI(x, len); }
        inline std::wstring    Convert_Ansi_To_Wide(std::string_view x, size_t len) { return dynarithmic::ANSIToWide(x, len); }
        inline std::string     Convert_WidePtr_To_Ansi(const wchar_t* x, size_t len) { return x ? dynarithmic::WideToANSI(x, len) : ""; }
        inline std::wstring    Convert_AnsiPtr_To_Wide(const char* x, size_t len) { return x ? dynarithmic::ANSIToWide(x, len) : L""; }

#ifdef _WIN32
        using utf16strType = std::wstring;
        using utf16viewType = std::wstring_view;
#else
        using utf16strType = std::u16string;
        using utf16viewType = std::u16string_view;
#endif
        inline std::pair<utf16strType, bool> Convert_UTF8_To_UTF16(std::string_view utf8, bool bMakeWideIfError = true)
        {
            auto pr = dynarithmic::utfconverter::UTF8ToUTF16(utf8);
            if (!pr.second && bMakeWideIfError)
                return { dynarithmic::ANSIToWide(utf8), false }; // Right now, only works for Windows
            return pr;
        }

        inline std::pair<std::string, bool> Convert_UTF16_To_UTF8(utf16viewType utf16, bool bMakeAnsiIfError = true)
        {
            auto pr = dynarithmic::utfconverter::UTF16ToUTF8(utf16);
            if (!pr.second && bMakeAnsiIfError)
                return { dynarithmic::WideToANSI(utf16), false }; // Right now, only works for Windows
            return pr;
        }
    }
}
#endif


