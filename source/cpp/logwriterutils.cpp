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
#include "logwriterutils.h"
#include "ctliface.h"
#include "cppfunc.h"

namespace dynarithmic
{
    void LogWriterUtils::WriteLogInfoA(const std::string& s, bool bFlush)
    {
        if (!CTL_StaticData::GetLogFilterFlags())
            return;

        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_USECRLF)
            std::string crlf = "\n";

        CTL_StaticData::GetLogger().StatusOutFast(s.c_str());
        if (bFlush)
            CTL_StaticData::GetLogger().Flush();
    }

    void LogWriterUtils::WriteLogInfoW(const std::wstring& s, bool bFlush)
    {
        WriteLogInfoA(StringConversion::Convert_Wide_To_Ansi(s), bFlush);
    }

    void LogWriterUtils::WriteLogInfo(const CTL_StringType& s, bool bFlush)
    {
        WriteLogInfoA(StringConversion::Convert_Native_To_Ansi(s));
    }

    void LogWriterUtils::WriteLogInfoIndentedA(const std::string& s)
    {
        // Truncate if text is too long
        if (s.size() > maxOutput)
        {
            auto tempS = s;
            std::string MoreText = "...(" + GetResourceStringFromMap(IDS_LOGMSG_MORETEXT) + ")...";
            tempS.resize(maxOutput);
            tempS += MoreText;
            if (tempS.size() < s.size())
            {
                CTL_LogFunctionCallA(tempS.c_str(), LOG_INDENT_USELAST_NOFUNCTION);
                return;
            }
        }
        CTL_LogFunctionCallA(s.c_str(), LOG_INDENT_USELAST_NOFUNCTION);
    }

    void LogWriterUtils::WriteLogInfoIndentedW(const std::wstring& s)
    {
        WriteLogInfoIndentedA(StringConversion::Convert_Wide_To_Ansi(s));
    }

    void LogWriterUtils::WriteLogInfoIndented(const CTL_StringType& s)
    {
        WriteLogInfoIndentedA(StringConversion::Convert_Native_To_Ansi(s));
    }

    void LogWriterUtils::MultiLineWriter(const std::string& s, const char* pszDelim, int nWhich)
    {
        StringWrapperA::StringArrayType sArray;
        StringWrapperA::Tokenize(s, pszDelim, sArray);
        for (auto& oneString : sArray)
            CTL_LogFunctionCallA(oneString.c_str(), nWhich);
    }

    void LogWriterUtils::WriteMultiLineInfo(const CTL_StringType& s, const CTL_StringType::traits_type::char_type* pszDelim)
    {
        WriteMultiLineInfoA(StringConversion::Convert_Native_To_Ansi(s), 
                            StringConversion::Convert_NativePtr_To_Ansi(pszDelim).c_str());
    }

    void LogWriterUtils::WriteMultiLineInfoA(const std::string& s, const char* pszDelim)
    {
        MultiLineWriter(s, pszDelim, 0);
    }
    
    void LogWriterUtils::WriteMultiLineInfoW(const std::wstring& s, const wchar_t* pszDelim)
    {
        WriteMultiLineInfoA(StringConversion::Convert_Wide_To_Ansi(s),
                            StringConversion::Convert_WidePtr_To_Ansi(pszDelim).c_str());
    }

    void LogWriterUtils::WriteMultiLineInfoIndented(const CTL_StringType& s, const CTL_StringType::traits_type::char_type* pszDelim)
    {
        WriteMultiLineInfoIndentedA(StringConversion::Convert_Native_To_Ansi(s),
                                    StringConversion::Convert_NativePtr_To_Ansi(pszDelim).c_str());
    }
    
    void LogWriterUtils::WriteMultiLineInfoIndentedA(const std::string& s, const char* pszDelim)
    {
        MultiLineWriter(s, pszDelim, LOG_INDENT_USELAST_NOFUNCTION);
    }

    void LogWriterUtils::WriteMultiLineInfoIndentedW(const std::wstring& s, const wchar_t* pszDelim)
    {
        WriteMultiLineInfoIndentedA(StringConversion::Convert_Wide_To_Ansi(s),
                                    StringConversion::Convert_WidePtr_To_Ansi(pszDelim).c_str());
    }
}