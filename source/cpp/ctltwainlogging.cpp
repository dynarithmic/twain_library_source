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

#include "cppfunc.h"
#include "errorcheck.h"
#include "ctllogsourcecaps.h"
#include "ctltwainlogging.h"
#include "ctldtwainhandle.h"
#include "ctlgetversion.h"
#include "dtwinverex.h"

using namespace dynarithmic;

namespace
{
    struct LogWin_DestroyTraits
    {
        static void Destroy(LPSTR p)
        {
            if (p)
                LocalFree(p);
        }
    };

    using LogMsg_RAII = DTWAIN_RAII<LPSTR, LogWin_DestroyTraits>;
}

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

    void LogDTWAINMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool bToCallback)
    {
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_NOTIFICATIONS)
        {
            CTL_TWAINDecoderStruct e;
            std::string s;
            if ( bToCallback )
                s = "To callback: ";
            s += e.GetDTWAINMessageAndDataInfo(hWnd, uMsg, wParam, lParam);
            LogWriterUtils::WriteMultiLineInfoIndentedA(s, "\n");
        }
    }

    std::pair<bool, std::vector<uint16_t>> OpenLogging(LPCTSTR pFileName, LONG logFlags, const LoggingTraits& lTraits)
    {
        uint16_t nWhichLogging = 0;
        uint16_t totalLoggingOptions = 0;
        std::vector<uint16_t> vBadLogs;
        if (pFileName && pFileName[0])
        {
            bool bLogOpen = false;
            bLogOpen = CTL_StaticData::GetLogger().InitFileLogging(pFileName, CTL_StaticData::GetDLLInstanceHandle(), lTraits);
            if (!bLogOpen)
                vBadLogs.push_back(nWhichLogging);
            ++totalLoggingOptions;
        }
        ++nWhichLogging;
        std::array<std::function<bool(HINSTANCE, const LoggingTraits&)>, 3> vLoggingFuncs = {
                                [&](HINSTANCE hinst, const LoggingTraits& theTraits) { return CTL_StaticData::GetLogger().InitConsoleLogging(hinst, theTraits); },
                                [&](HINSTANCE hinst, const LoggingTraits&) { return CTL_StaticData::GetLogger().InitDebugWindowLogging(hinst); },
                                [&](HINSTANCE hinst, const LoggingTraits&) { return CTL_StaticData::GetLogger().InitCallbackLogging(hinst); }};
        static constexpr std::array<long, 4> aLogFlags = { 0, DTWAIN_LOG_CONSOLE, DTWAIN_LOG_DEBUGMONITOR, DTWAIN_LOG_USECALLBACK };
        for (auto& fn : vLoggingFuncs)
        {
            if (logFlags & aLogFlags[nWhichLogging])
            {
                bool bRet = true;
                ++totalLoggingOptions;
                bRet = fn(CTL_StaticData::GetDLLInstanceHandle(), lTraits);
                if (!bRet)
                    vBadLogs.push_back(nWhichLogging);
            }
            ++nWhichLogging;
        }
        bool bAnyLogsOpen = vBadLogs.size() < totalLoggingOptions;
        if (bAnyLogsOpen && logFlags)
        {
            CTL_StaticData::GetLogger().PrintTime(true);
            CTL_StaticData::GetLogger().PrintAppName(true);
            CTL_StaticData::GetLogger().PrintBanner();
        }
        return { vBadLogs.empty(), vBadLogs };
    }

    void WriteVersionToLog(CTL_TwainDLLHandle *pHandle)
    {
        std::string ansiVer;
        if (CTL_StaticData::GetLogFilterFlags())
        {
            auto sVer = GetVersionString();
            const auto sWinVer = GetWinVersion();
            auto sDSMPath = CTL_TwainAppMgr::GetDSMPath();
            CTL_StringType sDSMVersionInfo;
            if (sDSMPath.empty())
            {
                sDSMPath = _T("(unknown or not queried)");
                sDSMVersionInfo.clear();
                sDSMVersionInfo = _T("\nDSM Version Information: ");
                sDSMVersionInfo += _T("(unknown or not queried)");
            }
            else
            {
                sDSMVersionInfo = _T("\nDSM Version Information:\n");
                if ( CTL_TwainAppMgr::GetInstance() )
                    sDSMVersionInfo += GetVersionInfo(CTL_TwainAppMgr::GetInstance()->GetDSMModuleHandle(), 4, _T("\n"));
                else
                    sDSMVersionInfo += _T("(unknown or not queried)");
            }
            sDSMPath = _T("Active DSM Path: ") + sDSMPath;
            sVer += _T("\n") + sWinVer + sDSMPath + sDSMVersionInfo + _T("\n");
            std::string sTwainSessionInfo;
            if ( pHandle->m_bSessionAllocated )
                sTwainSessionInfo = CTL_TWAINTypeDecoder::DecodeTWAINIdentity(pHandle->m_pTwainSession->GetAppIDPtr());
            sVer += _T("Current TWAIN Session Info:");
            if (sTwainSessionInfo.empty())
                sVer += _T(" (No Active TWAIN Session)");
            else
                sVer += _T("\n") + stringconversion::Convert_Ansi_To_Native(sTwainSessionInfo);
            #ifdef _WIN32
            // All log messages must be ANSI
            ansiVer = stringconversion::Convert_Native_To_Ansi(sVer);
            auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
            if (logFilterFlags & (DTWAIN_LOG_USEFILE | DTWAIN_LOG_CONSOLE))
            {
                if (!CTL_StaticData::GetLogger().StatusOutFast(ansiVer.c_str()))
                {
                    ansiVer += "\n";
                    LogToDebugMonitorA(ansiVer);
                }
            }
            if (logFilterFlags & DTWAIN_LOG_DEBUGMONITOR)
            {
                sVer += _T("\n");
                LogToDebugMonitor(sVer);
            }
            if ( UserDefinedLoggerExists(pHandle) )
                WriteUserDefinedLogMsgA(pHandle, ansiVer.c_str());
            #endif
        }
    }

    long LogTraitsOff::Apply(long turnOff) { return CTL_StaticData::GetLogFilterFlags() &~turnOff; }
    long LogTraitsOn::Apply(long turnOn) { return CTL_StaticData::GetLogFilterFlags() | turnOn; } 
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLog(DWORD LogFlags, LPCTSTR lpszLogFile)
    {
        LOG_FUNC_ENTRY_PARAMS((LogFlags, lpszLogFile))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);

        // Clear errors.
        pHandle->m_lLastError = DTWAIN_NO_ERROR;
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
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if (lineCount <= 0)
            lineCount = -1LL;
        CTL_StaticData::GetLogFileSaveThreshold() = lineCount;
        CTL_StaticData::GetLogger().SetLogSaveThreshold(lineCount);
        LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
        CATCH_BLOCK(false)
    }
}