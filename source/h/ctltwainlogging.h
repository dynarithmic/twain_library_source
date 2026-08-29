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
#ifndef CTLTWAINLOGGING_H
#define CTLTWAINLOGGING_H

#include "dtwain_standard_defs.h"
#include "ctlstringdefs.h"
#include "ctlstaticdata.h"

namespace dynarithmic
{
    class CTL_TwainDLLHandle;

    struct LogTraitsOff
    {
        static long Apply(long turnOff);
    };

    struct LogTraitsOn
    {
        static long Apply(long turnOn);
    };

    // RAII Class for turning on/off logging locally
    struct DTWAINScopedLogController
    {
        long m_ErrorFilterFlags;
        DTWAINScopedLogController(long newFilter) : m_ErrorFilterFlags(CTL_StaticData::GetLogFilterFlags())
        { 
            CTL_StaticData::GetLogFilterFlags() = newFilter; 
        }
        ~DTWAINScopedLogController() { CTL_StaticData::GetLogFilterFlags() = m_ErrorFilterFlags; }
        DTWAINScopedLogController(DTWAINScopedLogController&) = delete;
        DTWAINScopedLogController& operator=(DTWAINScopedLogController&) = delete;
    };

    template <typename LogTraits>
    struct DTWAINScopedLogControllerEx
    {
        DTWAINScopedLogController m_controller;
        DTWAINScopedLogControllerEx(long newValue) : m_controller(LogTraits::Apply(newValue)) {}
        DTWAINScopedLogControllerEx(DTWAINScopedLogControllerEx&) = delete;
        DTWAINScopedLogControllerEx& operator=(DTWAINScopedLogControllerEx&) = delete;
    };

    using DTWAINScopedLogControllerExclude = DTWAINScopedLogControllerEx<LogTraitsOff>;
    using DTWAINScopedLogControllerInclude = DTWAINScopedLogControllerEx<LogTraitsOn>;

    bool AnyLoggerExists(CTL_TwainDLLHandle* pHandle);
    void WriteUserDefinedLogMsg(CTL_TwainDLLHandle* pHandle, LPCTSTR sz);
    void WriteUserDefinedLogMsgA(CTL_TwainDLLHandle* pHandle, LPCSTR sz);
    void WriteUserDefinedLogMsgW(CTL_TwainDLLHandle* pHandle, LPCWSTR sz);
    bool UserDefinedLoggerExists(CTL_TwainDLLHandle* pHandle);
    std::string LogWin32Error(DWORD lastError);
    void LogToDebugMonitorA(std::string sMsg);
    void LogToDebugMonitorW(std::wstring sMsg);
    void LogToDebugMonitor(CTL_StringType sMsg);
    void LogDTWAINMessage(HWND, UINT, WPARAM, LPARAM, bool bCallback=false);

    struct LoggingTraits;
    std::pair<bool, std::vector<uint16_t>> OpenLogging(LPCTSTR pFileName, LONG logFlags, const LoggingTraits& fTraits = {});
    void WriteVersionToLog(CTL_TwainDLLHandle* pHandle);

}
#endif
