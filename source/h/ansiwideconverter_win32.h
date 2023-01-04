/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#ifndef ANSIWIDECONVERTER_WIN32_H
#define ANSIWIDECONVERTER_WIN32_H
#include <string>
#include <vector>
#include <limits>
class ConvertW2A
{
    std::string m_sz;
    UINT nConvertCodePage;

public:
    explicit ConvertW2A(LPCWSTR psz) : nConvertCodePage(CP_THREAD_ACP)
    {
        Init(psz);
    }

    ConvertW2A(LPCWSTR psz, size_t len) : nConvertCodePage(CP_THREAD_ACP)
    {
        Init(psz, len);
    }

    explicit operator LPCSTR() const noexcept
    {
        return m_sz.c_str();
    }

private:
    void Init(LPCWSTR psz, size_t len = (std::numeric_limits<size_t>::max)())
    {
        if (psz == nullptr)
            return;
        size_t nLengthW;
        if (len == (std::numeric_limits<size_t>::max)())
            nLengthW = wcslen(psz) + 1;
        else
            nLengthW = len;
        int nLengthA = static_cast<int>(nLengthW * 4);
        std::vector<char> szBuffer(nLengthA);
        bool bFailed = 0 == WideCharToMultiByte(nConvertCodePage, 0, psz, static_cast<int>(nLengthW), szBuffer.data(), nLengthA, nullptr, nullptr) ? true : false;
        if (bFailed)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                nLengthA = WideCharToMultiByte(nConvertCodePage, 0, psz, static_cast<int>(nLengthW), nullptr, 0, nullptr, nullptr);
                szBuffer.resize(nLengthA);
                bFailed = 0 == WideCharToMultiByte(nConvertCodePage, 0, psz, static_cast<int>(nLengthW), szBuffer.data(), nLengthA, nullptr, nullptr) ? true : false;
            }
        }
        if (bFailed)
            return;
        m_sz = std::string(szBuffer.data(), szBuffer.size());
    }
};

class ConvertA2W
{
    std::wstring m_sz;
    UINT nConvertCodePage;

public:
    explicit ConvertA2W(LPCSTR psz) : nConvertCodePage(CP_THREAD_ACP)
    {
        Init(psz);
    }

    ConvertA2W(LPCSTR psz, size_t len) : nConvertCodePage(CP_THREAD_ACP)
    {
        Init(psz, len);
    }

    explicit operator LPCWSTR() const noexcept
    {
        return m_sz.c_str();
    }

private:
    void Init(LPCSTR psz, size_t len = (std::numeric_limits<size_t>::max)())
    {
        if (psz == nullptr)
            return;
        size_t nLengthA;
        if (len == (std::numeric_limits<size_t>::max)())
            nLengthA = strlen(psz) + 1;
        else
            nLengthA = len;
        int nLengthW = static_cast<int>(nLengthA);
        std::vector<wchar_t> szBuffer(nLengthW);
        bool bFailed = 0 == MultiByteToWideChar(nConvertCodePage, 0, psz, static_cast<int>(nLengthA), szBuffer.data(), nLengthW) ? true : false;
        if (bFailed)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                nLengthW = MultiByteToWideChar(nConvertCodePage, 0, psz, static_cast<int>(nLengthA), nullptr, 0);
                szBuffer.resize(nLengthW);
                bFailed = 0 == MultiByteToWideChar(nConvertCodePage, 0, psz, static_cast<int>(nLengthA), szBuffer.data(), nLengthW) ? true : false;
            }
        }
        if (bFailed)
            return;
        m_sz = std::wstring(szBuffer.data(), szBuffer.size());
    }
};
#endif // ANSIWIDECONVERTER_WIN32_H
