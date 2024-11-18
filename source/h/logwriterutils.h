/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

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
/*********************************************************************/

#ifndef LOGWRITERUTILS_H
#define LOGWRITERUTILS_H

#include "ctlobstr.h"

namespace dynarithmic
{
    struct LogWriterUtils
    {
        static constexpr int maxOutput = 256;
        static void  WriteLogInfo(const CTL_StringType& s, bool bFlush = false);
        static void  WriteLogInfoA(const std::string& s, bool bFlush = false);
        static void  WriteLogInfoW(const std::wstring& s, bool bFlush = false);
        static void  WriteLogInfoIndentedA(const std::string& s);
        static void  WriteLogInfoIndentedW(const std::wstring& s);
        static void  WriteLogInfoIndented(const CTL_StringType& s);
        static void  WriteMultiLineInfo(const CTL_StringType& s, const CTL_StringType::traits_type::char_type* pszDelim);
        static void  WriteMultiLineInfoA(const std::string& s, const char *pszDelim);
        static void  WriteMultiLineInfoW(const std::wstring& s, const wchar_t* pszDelim);
        static void  WriteMultiLineInfoIndented(const CTL_StringType& s, const CTL_StringType::traits_type::char_type* pszDelim);
        static void  WriteMultiLineInfoIndentedA(const std::string& s, const char* pszDelim);
        static void  WriteMultiLineInfoIndentedW(const std::wstring& s, const wchar_t* pszDelim);

        private:
            static void MultiLineWriter(const std::string& s, const char* pszDelim, int nWhich);
    };
}
#endif

