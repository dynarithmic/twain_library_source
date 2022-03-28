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
#undef min
#undef max
#include "date/date.h"
#include <ctime>
#include <cstdio>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#include <tchar.h>
#else
#include <dlfcn.h>
#endif
#include <string>
#include "logmsg.h"
#include "ctlobstr.h"
#include "ctlfileutils.h"

using namespace dynarithmic;
using namespace date;

/////////////////////////////////////////////////////////////////////////////

namespace dynarithmic
{
    std::string CBaseLogger::getTime()
    {
        const time_t now = time(nullptr);
        struct tm  tstruct{};
        char       buf[80];
        localtime_s(&tstruct, &now);
        strftime(buf, sizeof buf, "[%Y-%m-%d %X] ", &tstruct);
        return buf;
    }

    void CBaseLogger::generic_outstream(std::ostream& os, const std::string& msg)
    {
        os << msg << '\n';
    }

    void StdCout_Logger::trace(const std::string& msg)
    {
        std::string total = getTime() + msg;
        if (total.back() != '\n')
            total += '\n';
        std::cout << total.c_str();
    }

    #ifdef _WIN32
    void DebugMonitor_Logger::trace(const std::string& msg) { OutputDebugStringA((getTime() + msg).c_str()); }
    #else
    void DebugMonitor_Logger::trace(const std::string& msg) { generic_outstream(std::cout, getTime() + msg + "\n"); }
    #endif

    File_Logger::File_Logger(const LPCSTR filename, bool bAppend/* = false*/)
    {
        if (bAppend)
            m_ostr.open(filename, std::ios::app);
        else
            m_ostr.open(filename);
    }

    void File_Logger::trace(const std::string& msg)
    {
        if (m_ostr)
            generic_outstream(m_ostr, getTime() + msg);
    }

}

void Callback_Logger::trace(const std::string& msg)
{
    // We have to convert the string to native format, since the user-defined logger handles both wide and non-wide
    // character strings
    if (UserDefinedLoggerExists())
        WriteUserDefinedLogMsgA(msg.c_str());
}

CLogSystem::CLogSystem() : m_bEnable(false), m_bPrintTime(false), m_bPrintAppName(false), m_bFileOpenedOK(false), m_bErrorDisplayed(false)
{}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

void CLogSystem::GetModuleName(HINSTANCE hInst)
{
    // get application path and name
    #ifdef WIN32
    char buf[_MAX_PATH+1];
    GetModuleFileNameA(hInst, buf, _MAX_PATH);
    m_csAppName = GetBaseName(buf);
#else
    // code for Linux using dladdr
    #endif
}

void CLogSystem::InitLogger(int loggerType, LPCTSTR pOutputFilename, HINSTANCE hInst, bool bAppend)
{
    GetModuleName(hInst);
    switch (loggerType )
    {
        case CONSOLE_LOGGING:
            app_logger_map[CONSOLE_LOGGING] = std::make_shared<StdCout_Logger>();
        break;
        case DEBUG_WINDOW_LOGGING:
            app_logger_map[DEBUG_WINDOW_LOGGING] = std::make_shared<DebugMonitor_Logger>();
        break;
        case FILE_LOGGING:
            app_logger_map[FILE_LOGGING] = std::make_shared<File_Logger>(StringConversion::Convert_NativePtr_To_Ansi(pOutputFilename).c_str(), bAppend);
        break;
        case CALLBACK_LOGGING:
            app_logger_map[CALLBACK_LOGGING] = std::make_shared<Callback_Logger>();
        break;
        default: ;
    }
    m_bEnable = true;
}

void  CLogSystem::InitConsoleLogging(HINSTANCE hInst)
{
#ifdef _WIN32
    FILE* fDummy;
    AllocConsole();
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
#endif
    InitLogger(CONSOLE_LOGGING, nullptr, hInst, false);
}

void  CLogSystem::InitDebugWindowLogging(HINSTANCE hInst)
{
    InitLogger(DEBUG_WINDOW_LOGGING, nullptr, hInst, false);
}

void  CLogSystem::InitCallbackLogging(HINSTANCE hInst)
{
    InitLogger(CALLBACK_LOGGING, nullptr, hInst, false);
}

void CLogSystem::InitFileLogging(LPCTSTR pOutputFilename, HINSTANCE hInst, bool bAppend)
{
    if (pOutputFilename)
        InitLogger(FILE_LOGGING, pOutputFilename, hInst, bAppend);
}

void CLogSystem::PrintBanner(bool bStarted)
{
    if ( bStarted )
    StatusOutFast("****** Log started ******\n");
    else
        StatusOutFast("****** Log ended ******\n");
}

/////////////////////////////////////////////////////////////////////////////

void CLogSystem::Enable(bool bEnable)
{
    m_bEnable = bEnable;
}

/////////////////////////////////////////////////////////////////////////////
bool CLogSystem::StatusOutFast(LPCSTR fmt)
{
    if (!m_bEnable)
        return true;

    WriteOnDemand(fmt);
    return true;
}

bool CLogSystem::WriteOnDemand(const std::string& fmt)
{
    for (const auto& m : app_logger_map)
        m.second->trace(fmt);
    return true;
}

bool CLogSystem::Flush()
{
    return WriteOnDemand("");
}
/////////////////////////////////////////////////////////////////////////////

std::string CLogSystem::GetBaseName(const std::string& path) const
{
    StringArray rArray;
    StringWrapperA::SplitPath(path, rArray);
    return rArray[StringWrapper::NAME_POS];
}

/////////////////////////////////////////////////////////////////////////////

std::string CLogSystem::GetBaseDir(const std::string& path) const
{
    StringArray rArray;
    StringWrapperA::SplitPath(path, rArray);
    return rArray[StringWrapper::DIRECTORY_POS];
}

void CLogSystem::OutputDebugStringFull(const std::string& s)
{
    for (const auto& m : app_logger_map)
        m.second->trace(s);
}

std::string CLogSystem::GetDebugStringFull(const std::string& s)
{
    std::ostringstream strm;
    if ( m_csAppName.empty() )
        m_csAppName = "Unknown App";
    strm << m_csAppName << " : " << CBaseLogger::getTime() << " : " << s;
    return strm.str();
}
