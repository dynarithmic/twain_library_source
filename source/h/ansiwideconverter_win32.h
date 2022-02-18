/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
class ConvertW2A
{
    std::string m_sz;
    UINT nConvertCodePage;

public:
    ConvertW2A(LPCWSTR psz) : nConvertCodePage(CP_THREAD_ACP)
    {
        Init(psz);
    }

    operator LPCSTR() const
    {
        return(m_sz.c_str());
    }

private:
    void Init(LPCWSTR psz)
    {
        if (psz == nullptr)
            return;
        const int nLengthW = static_cast<int>(wcslen(psz)) + 1;
        int nLengthA = nLengthW * 4;
        std::vector<char> szBuffer(nLengthA);
        bool bFailed = (0 == ::WideCharToMultiByte(nConvertCodePage, 0, psz, nLengthW, szBuffer.data(), nLengthA, nullptr, nullptr)) ? true : false;
        if (bFailed)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                nLengthA = ::WideCharToMultiByte(nConvertCodePage, 0, psz, nLengthW, nullptr, 0, nullptr, nullptr);
                szBuffer.resize(nLengthA);
                bFailed = (0 == ::WideCharToMultiByte(nConvertCodePage, 0, psz, nLengthW, szBuffer.data(), nLengthA, nullptr, nullptr)) ? true : false;
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
    ConvertA2W(LPCSTR psz) : nConvertCodePage(CP_THREAD_ACP)
    {
        Init(psz);
    }

    operator LPCWSTR() const
    {
        return(m_sz.c_str());
    }

private:
    void Init(LPCSTR psz)
    {
        if (psz == nullptr)
            return;
        const int nLengthA = static_cast<int>(strlen(psz)) + 1;
        int nLengthW = nLengthA;
        std::vector<wchar_t> szBuffer(nLengthW);
        bool bFailed = (0 == ::MultiByteToWideChar(nConvertCodePage, 0, psz, nLengthA, szBuffer.data(), nLengthW)) ? true : false;
        if (bFailed)
        {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                nLengthW = ::MultiByteToWideChar(nConvertCodePage, 0, psz, nLengthA, nullptr, 0);
                szBuffer.resize(nLengthW);
                bFailed = (0 == ::MultiByteToWideChar(nConvertCodePage, 0, psz, nLengthA, szBuffer.data(), nLengthW)) ? true : false;
            }
        }
        if (bFailed)
            return;
        m_sz = std::wstring(szBuffer.data(), szBuffer.size());
    }
};
#endif // ANSIWIDECONVERTER_WIN32_H
