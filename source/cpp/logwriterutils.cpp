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
#include "logwriterutils.h"
#include "ctlstringutils.h"
#include "ctlstringutilsx.h"
#include "ctltwainmanager.h"
#include "ctlstaticdata.h"
#include "dtwain_config.h"
#include "dtwainx.h"
#include "ctllogcalls.h"

namespace stringutils = dynarithmic::basicstringutils;

namespace
{
    void LogDTWAINErrorToMsgBox(int nError, LPCSTR func, std::string_view s)
    {
        std::ostringstream strm;
        if (!func)
            func = "(Uninitialized DTWAIN DLL)";
        strm << "DTWAIN Function " << func << " returned error code " << nError << std::endl << std::endl;
        strm << s.data();
        const std::string st = strm.str();
        MessageBoxA(nullptr, st.c_str(), "DTWAIN Error", MB_ICONSTOP);
    }
}

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
        WriteLogInfoA(stringconversion::Convert_Wide_To_Ansi(s.data()), bFlush);
    }

    void LogWriterUtils::WriteLogInfo(CTL_StringViewType s, bool bFlush)
    {
        WriteLogInfoA(stringconversion::Convert_NativePtr_To_Ansi(s.data()));
    }

    void LogWriterUtils::WriteLogInfoIndentedA(std::string_view s)
    {
        #if DTWAIN_BUILD_LOGCALLSTACK == 1
        CTL_LogFunctionCallA(CTL_StaticData::GetLogFilterFlags(), TruncateStringWithMore(s, maxOutput).c_str(), LOG_INDENT_USELAST_NOFUNCTION);
        #endif
    }

    void LogWriterUtils::WriteLogInfoIndentedW(std::wstring_view s)
    {
        WriteLogInfoIndentedA(stringconversion::Convert_WidePtr_To_Ansi(s.data()));
    }

    void LogWriterUtils::WriteLogInfoIndented(CTL_StringViewType s)
    {
        WriteLogInfoIndentedA(stringconversion::Convert_NativePtr_To_Ansi(s.data()));
    }

    void LogWriterUtils::MultiLineWriter(std::string_view s, const char* pszDelim, int nWhich)
    {
        #if DTWAIN_BUILD_LOGCALLSTACK == 1
        std::vector<CTL_StringTypeA> sArray;
        stringutils::Tokenize(s.data(), pszDelim, sArray, true);
        for (auto& oneString : sArray)
            CTL_LogFunctionCallA(CTL_StaticData::GetLogFilterFlags(), oneString.c_str(), nWhich);
        #endif
    }

    void LogWriterUtils::WriteMultiLineInfo(CTL_StringViewType s, const CTL_StringType::traits_type::char_type* pszDelim)
    {
        WriteMultiLineInfoA(stringconversion::Convert_NativePtr_To_Ansi(s.data()), 
                            stringconversion::Convert_NativePtr_To_Ansi(pszDelim).c_str());
    }

    void LogWriterUtils::WriteMultiLineInfoA(std::string_view s, const char* pszDelim)
    {
        MultiLineWriter(s, pszDelim, 0);
    }
    
    void LogWriterUtils::WriteMultiLineInfoW(std::wstring_view s, const wchar_t* pszDelim)
    {
        WriteMultiLineInfoA(stringconversion::Convert_WidePtr_To_Ansi(s.data()),
                            stringconversion::Convert_WidePtr_To_Ansi(pszDelim).c_str());
    }

    void LogWriterUtils::WriteMultiLineInfoIndented(CTL_StringViewType s, const CTL_StringType::traits_type::char_type* pszDelim)
    {
        WriteMultiLineInfoIndentedA(stringconversion::Convert_NativePtr_To_Ansi(s.data()),
                                    stringconversion::Convert_NativePtr_To_Ansi(pszDelim).c_str());
    }
    
    void LogWriterUtils::WriteMultiLineInfoIndentedA(std::string_view s, const char* pszDelim)
    {
        MultiLineWriter(s, pszDelim, LOG_INDENT_USELAST_NOFUNCTION);
    }

    void LogWriterUtils::WriteMultiLineInfoIndentedW(std::wstring_view s, const wchar_t* pszDelim)
    {
        WriteMultiLineInfoIndentedA(stringconversion::Convert_WidePtr_To_Ansi(s.data()),
                                    stringconversion::Convert_WidePtr_To_Ansi(pszDelim).c_str());
    }

    void OutputDTWAINErrorW(const CTL_TwainDLLHandle* pHandle, LPCWSTR pFunc)
    {
        if ( pFunc )
            OutputDTWAINError(pHandle, stringconversion::Convert_Wide_To_Ansi(pFunc).c_str());
        else
            OutputDTWAINError(pHandle);
    }

    void OutputDTWAINErrorA(const CTL_TwainDLLHandle* pHandle, LPCSTR pFunc)
    {
        OutputDTWAINError(pHandle, pFunc);
    }

    void OutputDTWAINError(const CTL_TwainDLLHandle* pHandle, LPCSTR pFunc)
    {
        auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
        if (!(logFilterFlags & DTWAIN_LOG_DTWAINERRORS))
            return;
        static constexpr int MaxMessage = DTWAIN_USERRES_MAXSIZE;
        char szBuf[MaxMessage + 1]{};
        if (!pHandle)
            DTWAIN_GetErrorStringA(DTWAIN_ERR_BAD_HANDLE, szBuf, MaxMessage);
        else
            CTL_TwainAppMgr::GetLastErrorString(szBuf, MaxMessage);
        std::string_view s(szBuf);
        if (pHandle)
            LogWriterUtils::WriteLogInfoIndentedA(s);

        if (logFilterFlags & DTWAIN_LOG_ERRORMSGBOX && pHandle)
            LogDTWAINErrorToMsgBox(pHandle->m_lLastError, pFunc, s);
        else
        if (!pHandle && logFilterFlags & DTWAIN_LOG_INITFAILURE)
            LogDTWAINErrorToMsgBox(DTWAIN_ERR_BAD_HANDLE, nullptr, s);
    }
}
