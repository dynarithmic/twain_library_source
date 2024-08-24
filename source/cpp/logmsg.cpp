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
#include "ctlthreadutils.h"

using namespace dynarithmic;
using namespace date;

/////////////////////////////////////////////////////////////////////////////
std::mutex CLogSystem::s_logMutex;

namespace dynarithmic
{
    std::string CBaseLogger::getTime()
    {
        const auto currentDateTime = std::chrono::system_clock::now();
        const auto currentDateTimeTimeT = std::chrono::system_clock::to_time_t(currentDateTime);
        const auto currentDateTimeLocalTime = *std::localtime(&currentDateTimeTimeT);
        const auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(currentDateTime).time_since_epoch().count() % 1000;
        std::ostringstream strm;
        strm << std::put_time(&currentDateTimeLocalTime, "[%Y-%m-%d %X")
            << "." << std::setfill('0') << std::setw(3) << ms << "] ";
        return strm.str();
    }

    std::string CBaseLogger::getThreadID()
    {
        std::string result;
        auto str = dynarithmic::getThreadIdAsString();
        result = "Thread [" + str + "] ";
        return result;
    }

    std::string CBaseLogger::applyDecoration(const std::string& msg)
    {
        std::string total = getTime() + getThreadID() + msg;
        if (total.back() != '\n')
            total += '\n';
        return total;
    }

    void CBaseLogger::generic_outstream(std::ostream& os, const std::string& msg)
    {
        os << msg << '\n';
    }

    void StdCout_Logger::trace(const std::string& msg)
    {
        std::cout << applyDecoration(msg).c_str();
    }

    #ifdef _WIN32
    void DebugMonitor_Logger::trace(const std::string& msg) 
    { 
        OutputDebugStringA(applyDecoration(msg).c_str());
    }
    #else
    void DebugMonitor_Logger::trace(const std::string& msg) { generic_outstream(std::cout, applyDecoration()); }
    #endif

    File_Logger::File_Logger(const LPCSTR filename, const FileLoggingTraits& fTraits)
    {
        if (fTraits.m_bCreateDirectory)
        {
            // auto-create the directory
            const auto dirCreated = dynarithmic::create_directory(dynarithmic::get_parent_directory(fTraits.m_filename.c_str(), false).c_str());
            if (!dirCreated.first)
            {
                m_bFileCreated = false;
                return;
            }
        }
        if (fTraits.m_bAppend)
            m_ostr.open(filename, std::ios::app);
        else
            m_ostr.open(filename);
        m_bFileCreated = m_ostr?true:false;
    }

    File_Logger::~File_Logger()
    {
        if (m_ostr.is_open())
            m_ostr.close();
    }

    void File_Logger::trace(const std::string& msg)
    {
        if (m_ostr)
            generic_outstream(m_ostr, getTime() + getThreadID() + msg);
    }

    StdCout_Logger::~StdCout_Logger()
    {
        ::FreeConsole();
    }
}

void Callback_Logger::trace(const std::string& msg)
{
    // We have to convert the string to native format, since the user-defined logger handles both wide and non-wide
    // character strings
    if (UserDefinedLoggerExists())
    {
        auto fullMessage = getTime() + getThreadID() + msg;
        WriteUserDefinedLogMsgA(fullMessage.c_str());
    }
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

bool CLogSystem::InitLogger(int loggerType, LPCTSTR pOutputFilename, HINSTANCE hInst, const FileLoggingTraits& fTraits)
{
    bool loggerSet = false;
    GetModuleName(hInst);
    auto iter = app_logger_map.find(loggerType);
    if (iter == app_logger_map.end())
    {
        switch (loggerType)
        {
            case CONSOLE_LOGGING:
                app_logger_map[CONSOLE_LOGGING] = std::make_shared<StdCout_Logger>();
                loggerSet = true;
            break;
            case DEBUG_WINDOW_LOGGING:
                app_logger_map[DEBUG_WINDOW_LOGGING] = std::make_shared<DebugMonitor_Logger>();
                loggerSet = true;
            break;
            case FILE_LOGGING:
            {
                auto filelogging = std::make_shared<File_Logger>(StringConversion::Convert_NativePtr_To_Ansi(pOutputFilename).c_str(), fTraits);
                if (filelogging->IsFileCreated())
                {
                    app_logger_map[FILE_LOGGING] = filelogging;
                loggerSet = true;
                }
                else
                    loggerSet = false;
            }
            break;
            case CALLBACK_LOGGING:
                app_logger_map[CALLBACK_LOGGING] = std::make_shared<Callback_Logger>();
                loggerSet = true;
            break;
            default:;
        }
    }
    else
        loggerSet = true;
    m_bEnable = loggerSet;
    return m_bEnable;
}

void CLogSystem::DisableLogger(int loggerType)
{
    app_logger_map.erase(loggerType);
    m_bEnable = !app_logger_map.empty();
}

void CLogSystem::DisableAllLoggers()
{
    app_logger_map.clear();
    m_bEnable = false;
}

bool CLogSystem::InitConsoleLogging(HINSTANCE hInst)
{
#ifdef _WIN32
    FILE* fDummy;
    BOOL bRet = AllocConsole();
    if (bRet)
    {
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
#endif
        InitLogger(CONSOLE_LOGGING, nullptr, hInst);
    }
    return bRet ? true : false;
}

bool CLogSystem::InitDebugWindowLogging(HINSTANCE hInst)
{
    InitLogger(DEBUG_WINDOW_LOGGING, nullptr, hInst);
    return true;
}

bool  CLogSystem::InitCallbackLogging(HINSTANCE hInst)
{
    InitLogger(CALLBACK_LOGGING, nullptr, hInst);
    return true;
}

bool CLogSystem::InitFileLogging(LPCTSTR pOutputFilename, HINSTANCE hInst, const FileLoggingTraits& fTraits)
{
    bool bLogOpen = false;
    if (pOutputFilename)
        bLogOpen = InitLogger(FILE_LOGGING, pOutputFilename, hInst, fTraits);
    return bLogOpen;
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
    std::lock_guard<std::mutex> g(s_logMutex);
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
