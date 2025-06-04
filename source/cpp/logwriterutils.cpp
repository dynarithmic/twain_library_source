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
#include "logwriterutils.h"
#include "ctliface.h"
#include "cppfunc.h"
#include "ctlstringutils.h"

namespace dynarithmic
{
    void LogWriterUtils::WriteLogInfoExA(long filterFlags, std::string_view s, bool bFlush)
    {
        if (CTL_StaticData::GetLogFilterFlags() & filterFlags)
        {
            CTL_StaticData::GetLogger().StatusOutFast(s.data());
            if (bFlush)
                CTL_StaticData::GetLogger().Flush();
        }
    }

    void LogWriterUtils::WriteLogInfoA(std::string_view s, bool bFlush)
    {
        if (!CTL_StaticData::GetLogFilterFlags())
            return;

        CTL_StaticData::GetLogger().StatusOutFast(s.data());
        if (bFlush)
            CTL_StaticData::GetLogger().Flush();
    }

    void LogWriterUtils::WriteLogInfoW(std::wstring_view s, bool bFlush)
    {
        WriteLogInfoA(StringConversion::Convert_Wide_To_Ansi(s.data()), bFlush);
    }

    void LogWriterUtils::WriteLogInfo(CTL_StringViewType s, bool bFlush)
    {
        WriteLogInfoA(StringConversion::Convert_NativePtr_To_Ansi(s.data()));
    }

    void LogWriterUtils::WriteLogInfoIndentedA(std::string_view s)
    {
        CTL_LogFunctionCallA(TruncateStringWithMore(s, maxOutput).c_str(), LOG_INDENT_USELAST_NOFUNCTION);
    }

    void LogWriterUtils::WriteLogInfoIndentedW(std::wstring_view s)
    {
        WriteLogInfoIndentedA(StringConversion::Convert_WidePtr_To_Ansi(s.data()));
    }

    void LogWriterUtils::WriteLogInfoIndented(CTL_StringViewType s)
    {
        WriteLogInfoIndentedA(StringConversion::Convert_NativePtr_To_Ansi(s.data()));
    }

    void LogWriterUtils::MultiLineWriter(std::string_view s, const char* pszDelim, int nWhich)
    {
        StringWrapperA::StringArrayType sArray;
        StringWrapperA::Tokenize(s.data(), pszDelim, sArray, true);
        for (auto& oneString : sArray)
            CTL_LogFunctionCallA(oneString.c_str(), nWhich);
    }

    void LogWriterUtils::WriteMultiLineInfo(CTL_StringViewType s, const CTL_StringType::traits_type::char_type* pszDelim)
    {
        WriteMultiLineInfoA(StringConversion::Convert_NativePtr_To_Ansi(s.data()), 
                            StringConversion::Convert_NativePtr_To_Ansi(pszDelim).c_str());
    }

    void LogWriterUtils::WriteMultiLineInfoA(std::string_view s, const char* pszDelim)
    {
        MultiLineWriter(s, pszDelim, 0);
    }
    
    void LogWriterUtils::WriteMultiLineInfoW(std::wstring_view s, const wchar_t* pszDelim)
    {
        WriteMultiLineInfoA(StringConversion::Convert_WidePtr_To_Ansi(s.data()),
                            StringConversion::Convert_WidePtr_To_Ansi(pszDelim).c_str());
    }

    void LogWriterUtils::WriteMultiLineInfoIndented(CTL_StringViewType s, const CTL_StringType::traits_type::char_type* pszDelim)
    {
        WriteMultiLineInfoIndentedA(StringConversion::Convert_NativePtr_To_Ansi(s.data()),
                                    StringConversion::Convert_NativePtr_To_Ansi(pszDelim).c_str());
    }
    
    void LogWriterUtils::WriteMultiLineInfoIndentedA(std::string_view s, const char* pszDelim)
    {
        MultiLineWriter(s, pszDelim, LOG_INDENT_USELAST_NOFUNCTION);
    }

    void LogWriterUtils::WriteMultiLineInfoIndentedW(std::wstring_view s, const wchar_t* pszDelim)
    {
        WriteMultiLineInfoIndentedA(StringConversion::Convert_WidePtr_To_Ansi(s.data()),
                                    StringConversion::Convert_WidePtr_To_Ansi(pszDelim).c_str());
    }
}