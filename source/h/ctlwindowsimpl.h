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
#ifndef CTLWINDOWSIMPL_H
#define CTLWINDOWSIMPL_H

#include <windows.h>
#include <string>
#include <string_view>
#include <algorithm>
#include "dtwain_filesystem.h"

namespace dynarithmic
{
    struct WindowsWideFuncImpl
    {
        static UINT GetWindowsDirectory(LPWSTR lpBuffer, UINT uSize)
        {
            return ::GetWindowsDirectoryW(&lpBuffer[0], uSize);
        }

        static UINT GetSystemDirectory(LPWSTR lpBuffer, UINT uSize)
        {
            return ::GetSystemDirectoryW(&lpBuffer[0], uSize);
        }

        static DWORD GetModuleFileName(HMODULE hModule, LPWSTR lpBuffer, DWORD nSize)
        {
            return ::GetModuleFileNameW(hModule, &lpBuffer[0], nSize);
        }
    };

    struct WindowsAnsiFuncImpl
    {
        static UINT GetWindowsDirectory(LPSTR lpBuffer, UINT uSize)
        {
            return ::GetWindowsDirectoryA(&lpBuffer[0], uSize);
        }

        static UINT GetSystemDirectory(LPSTR lpBuffer, UINT uSize)
        {
            return ::GetSystemDirectoryA(&lpBuffer[0], uSize);
        }

        static DWORD GetModuleFileName(HMODULE hModule, LPSTR lpBuffer, DWORD nSize)
        {
            return ::GetModuleFileNameA(hModule, &lpBuffer[0], nSize);
        }
    };

    template <typename string_type, typename WinAPITraits>
    struct WindowsAPIImpl
    {
        static UINT GetWindowsDirectoryImpl(string_type& buffer)
        {
            buffer.resize(_MAX_PATH);
            auto nSize = WinAPITraits::GetWindowsDirectory(&buffer[0], _MAX_PATH);
            buffer.resize(nSize);
            return nSize;
        }
        static UINT GetSystemDirectoryImpl(string_type& buffer)
        {
            buffer.resize(_MAX_PATH);
            auto nSize = WinAPITraits::GetSystemDirectory(&buffer[0], _MAX_PATH);
            buffer.resize(nSize);
            return nSize;
        }
        static DWORD GetModuleFileNameImpl(HMODULE hModule, string_type& lpFileName, DWORD nSize)
        {
            lpFileName.resize(nSize);
            auto actualSize = WinAPITraits::GetModuleFileName(hModule, &lpFileName[0], nSize);
            lpFileName.resize(std::min(nSize, actualSize));
            return actualSize;
        }

        static string_type GetWindowsDirectory()
        {
            string_type buffer;
            const UINT retValue = WinAPITraits::GetWindowsDirectoryImpl(buffer);
            if (retValue != 0)
                return buffer;
            return {};
        }

        static string_type GetSystemDirectory()
        {
            string_type buffer;
            const UINT retValue = WinAPITraits::GetSystemDirectoryImpl(buffer);
            if (retValue != 0)
                return buffer;
            return {};
        }

        static string_type GetModuleFileName(HMODULE hModule)
        {
            string_type buffer;
            const UINT retValue = WinAPITraits::GetModuleFileNameImpl(hModule, buffer, 32767);
            if (retValue != 0)
                return buffer;
            return {};
        }

        static string_type AddBackslashToDirectory(std::basic_string_view<typename string_type::value_type> pathName)
        {
            using stringview_type = std::basic_string_view<typename string_type::value_type>;
            std::filesystem::path fsPath(pathName);
            fsPath /= {};
            if constexpr (std::is_same_v<std::string_view, stringview_type>)
                return fsPath.string();
            else
                return fsPath.native();
        }

        static string_type RemoveBackslashFromDirectory(string_type pathName)
        {
            if (!pathName.empty())
            {
                if (pathName.back() == filesys::path::preferred_separator)
                    pathName.pop_back();
            }
            return pathName;
        }
    };

    using WindowsAPIImplA = WindowsAPIImpl<std::string, WindowsAPIImpl<std::string, WindowsAnsiFuncImpl>>;
    using WindowsAPIImplW = WindowsAPIImpl<std::wstring, WindowsAPIImpl<std::wstring, WindowsWideFuncImpl>>;
#ifdef _UNICODE
    using WindowsAPIImplDef = WindowsAPIImplW;
#else
    using WindowsAPIImplDef = WindowsAPIImplA;
#endif

}
#endif