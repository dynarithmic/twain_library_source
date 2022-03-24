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
#ifndef ANSIWIDECONVERTER_GENERIC_H
#define ANSIWIDECONVERTER_GENERIC_H
#include <string>
class ConvertW2A
{
    std::string m_sz;

public:
    ConvertW2A(LPCWSTR psz)
    {
        Init(psz);
    }

    operator LPCSTR() const
    {
        return m_sz.c_str();
    }

private:
    void Init(LPCWSTR psz)
    {
        if (!psz)
            return;
        m_sz = std::string(psz, psz + wcslen(psz));
    }
};

class ConvertA2W
{
    std::wstring m_sz;

public:
    ConvertA2W(LPCSTR psz)
    {
        Init(psz);
    }

    operator LPCWSTR() const
    {
        return m_sz.c_str();
    }

private:
    void Init(LPCSTR psz)
    {
        if (!psz)
            return;
        m_sz = std::wstring(psz, psz + strlen(psz));
    }
};
#endif // ANSIWIDECONVERTER_GENERIC_H
