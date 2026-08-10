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
#include "ctliface.h"
#include "cppfunc.h"
#include "errorcheck.h"
#include "ctllogsourcecaps.h"
#include "ctltwainlogging.h"
#include "ctldtwainhandle.h"

using namespace dynarithmic;

namespace dynarithmic
{
    bool UserDefinedLoggerExists(CTL_TwainDLLHandle* pHandle)
    {
        return pHandle->m_LoggerCallbackInfo.m_pLoggerCallback ||
            pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA ||
            pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW;
    }

    bool AnyLoggerExists(CTL_TwainDLLHandle* pHandle)
    {
        return UserDefinedLoggerExists(pHandle) || CTL_StaticData::GetLogFilterFlags() != 0;
    }

    void WriteUserDefinedLogMsg(CTL_TwainDLLHandle* pHandle, LPCTSTR sz)
    {
        if (!pHandle)
            return;
        if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallback)
            pHandle->m_LoggerCallbackInfo.m_pLoggerCallback(sz, pHandle->m_LoggerCallbackInfo.m_pLoggerCallback_UserData);
        if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA)
        #ifdef _UNICODE
            WriteUserDefinedLogMsgA(pHandle, stringconversion::Convert_NativePtr_To_Ansi(sz).c_str());
        #else
            WriteUserDefinedLogMsgA(pHandle, sz);
        #endif
        if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW)
        #ifdef _UNICODE
            WriteUserDefinedLogMsgW(pHandle, sz);
        #else
            WriteUserDefinedLogMsgW(pHandle, stringconversion::Convert_NativePtr_To_Wide(sz).c_str());
        #endif
    }

    void WriteUserDefinedLogMsgA(CTL_TwainDLLHandle* pHandle, LPCSTR sz)
    {
        if (!pHandle)
            return;
        auto& loggerRef = pHandle->m_LoggerCallbackInfo;
        if (loggerRef.m_pLoggerCallbackA)
            loggerRef.m_pLoggerCallbackA(sz, loggerRef.m_pLoggerCallback_UserDataA);

        if (loggerRef.m_pLoggerCallback)
        #ifdef _UNICODE
            loggerRef.m_pLoggerCallback(stringconversion::Convert_Ansi_To_Native(sz).c_str(),
                loggerRef.m_pLoggerCallback_UserData);
        #else
            loggerRef.m_pLoggerCallback(sz, loggerRef.m_pLoggerCallback_UserData);
        #endif
        if (loggerRef.m_pLoggerCallbackW)
        #ifdef _UNICODE
            loggerRef.m_pLoggerCallbackW(stringconversion::Convert_Ansi_To_Native(sz).c_str(),
                loggerRef.m_pLoggerCallback_UserDataW);
        #else
            loggerRef.m_pLoggerCallbackW(stringconversion::Convert_Native_To_Wide(sz).c_str(),
                loggerRef.m_pLoggerCallback_UserDataW);
        #endif
    }

    void WriteUserDefinedLogMsgW(CTL_TwainDLLHandle* pHandle, LPCWSTR sz)
    {
        if (!pHandle)
            return;
        auto& loggerRef = pHandle->m_LoggerCallbackInfo;
        if (loggerRef.m_pLoggerCallbackW)
            loggerRef.m_pLoggerCallbackW(sz, loggerRef.m_pLoggerCallback_UserDataW);

        if (loggerRef.m_pLoggerCallback)
        #ifdef _UNICODE
            loggerRef.m_pLoggerCallback(sz, loggerRef.m_pLoggerCallback_UserData);
        #else
            loggerRef.m_pLoggerCallback(stringconversion::Convert_Wide_To_Native(sz).c_str(),
                loggerRef.m_pLoggerCallback_UserData);
        #endif
        if (loggerRef.m_pLoggerCallbackA)
        #ifdef _UNICODE
            loggerRef.m_pLoggerCallbackA(stringconversion::Convert_Native_To_Ansi(sz).c_str(),
                loggerRef.m_pLoggerCallback_UserDataA);
        #else
            loggerRef.m_pLoggerCallbackA(stringconversion::Convert_Wide_To_Native(sz).c_str(),
                loggerRef.m_pLoggerCallback_UserDataA);
        #endif
    }

    struct LogWin_DestroyTraits
    {
        static void Destroy(LPSTR p)
        {
            if (p)
                LocalFree(p);
        }
    };

    using LogMsg_RAII = DTWAIN_RAII<LPSTR, LogWin_DestroyTraits>;

    std::string LogWin32Error(DWORD lastError)
    {
        LPSTR lpMsgBuf = nullptr;

        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            nullptr,
            lastError,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&lpMsgBuf,
            0,
            nullptr
        );

        LogMsg_RAII raiiFree(lpMsgBuf);

        // Display the string.
        std::string sError = lpMsgBuf;
        while (!sError.empty())
        {
            if (std::iscntrl(sError.back()))
                sError.pop_back();
            else
                break;
        }
        StringStreamA strm;
        strm << "Win32 Error: " << lastError << " (" << sError << ")";
        LogWriterUtils::WriteLogInfoIndentedA(strm.str());

        return strm.str();
    }

    void LogToDebugMonitorA(std::string sMsg)
    {
        if (sMsg.back() != '\n')
            sMsg.push_back('\n');
        OutputDebugStringA(sMsg.c_str());
    }

    void LogToDebugMonitorW(std::wstring sMsg)
    {
        if (sMsg.back() != L'\n')
            sMsg.push_back(L'\n');
        OutputDebugStringW(sMsg.c_str());
    }

    void LogToDebugMonitor(CTL_StringType sMsg)
    {
#ifdef _UNICODE
        LogToDebugMonitorW(sMsg);
#else
        LogToDebugMonitorA(sMsg);
#endif
    }

}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLog(DWORD LogFlags, LPCTSTR lpszLogFile)
{
    LOG_FUNC_ENTRY_PARAMS((LogFlags, lpszLogFile))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    CTL_StaticData::GetLogger().SetDLLHandle(pHandle);

    // If the log flags have not specified what to log
    // then log call stack and general TWAIN send/receive info.
    LONG allFlags = DTWAIN_LOG_ALL;
    if ( (LogFlags != 0) && (LogFlags & allFlags) == 0)  
        LogFlags |= (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_DECODE_SOURCE | DTWAIN_LOG_DECODE_DEST | DTWAIN_LOG_MISCELLANEOUS);
    bool logFailed = false; 

    bool bLoggerExists = AnyLoggerExists(pHandle);
    auto& logFilterFlags = CTL_StaticData::GetLogFilterFlags();
    if (LogFlags == 0 && bLoggerExists)
    {
        CTL_StaticData::GetLogger().PrintBanner(false);
        CTL_StaticData::GetLogger().DisableAllLoggers();
        logFilterFlags = LogFlags;
    }
    else
    {
        logFilterFlags = LogFlags;
        if (LogFlags && !UserDefinedLoggerExists(pHandle))
            logFilterFlags &= ~DTWAIN_LOG_USECALLBACK;

        LoggingTraits fTraits;
        fTraits.m_bAppend = LogFlags & DTWAIN_LOG_FILEAPPEND ? true : false;
        fTraits.m_bCreateDirectory = LogFlags & DTWAIN_LOG_CREATEDIRECTORY ? true : false;
        fTraits.m_filename = lpszLogFile;
        fTraits.m_bSetConsoleHandler = LogFlags & DTWAIN_LOG_CONSOLEWITHHANDLER ? true : false;
        auto isLogOpen = OpenLogging(lpszLogFile, LogFlags, fTraits);

        // Write the version info
        // Write to all the loggers that were created
        if (LogFlags > 0)
            WriteVersionToLog(pHandle);
        logFailed = (LogFlags > 0 && !isLogOpen.first);
        if (logFailed)
        {
            // Indicate that there is at least one logger that failed
            DTWAIN_Check_Error_Condition_NoThrow_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_LOG_CREATE_ERROR, false, FUNC_MACRO, false);
        }

        // If there are opened sources, log the capabilities for each
        if (logFilterFlags)
        {
            auto pOpenedSources = GetOpenSources(pHandle);
            for (auto* pCurSource : pOpenedSources)
                LogSourceCapabilities(pCurSource, false);
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(!logFailed)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLogSaveThreshold(LONG64 lineCount)
{
    LOG_FUNC_ENTRY_PARAMS((lineCount))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if (lineCount <= 0)
        lineCount = -1LL;
    CTL_StaticData::GetLogFileSaveThreshold() = lineCount;
    CTL_StaticData::GetLogger().SetLogSaveThreshold(lineCount);
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(false)
}
