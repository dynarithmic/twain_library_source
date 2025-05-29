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
/*********************************************************************/

#ifndef CLogSystem_H
#define CLogSystem_H
#include "ctlobstr.h"
#include <memory>
#include <unordered_map>
#include <fstream>
#include <mutex>
#include <string>

/////////////////////////////////////////////////////////////////////////////
namespace dynarithmic
{
    class CTL_TwainDLLHandle;
    struct LoggingTraits
    {
        CTL_StringType m_filename;
        bool m_bAppend = false;
        bool m_bCreateDirectory = false;
        bool m_bSetConsoleHandler = false;
    };

    class CBaseLogger
    {
        public:
            virtual ~CBaseLogger() = default;
            CBaseLogger(const CBaseLogger&) = default;
            CBaseLogger() = default;
            CBaseLogger& operator=(const CBaseLogger&) = default;
            virtual void trace(std::string_view msg) = 0;
            static std::string applyDecoration(std::string_view msg);
            static void generic_outstream(std::ostream& os, std::string_view msg);
            static std::string getTime();
            static std::string getThreadID();
    };

    class StdCout_Logger final : public CBaseLogger
    {
        public:
            void trace(std::string_view msg) override;
            static BOOL WINAPI ConsoleCtrlHandler(DWORD dwCtrlType);
            StdCout_Logger(const LoggingTraits& lTraits);
            ~StdCout_Logger();
    };

    class DebugMonitor_Logger final : public CBaseLogger
    {
        public:
            void trace(std::string_view msg) override;
    };

    class File_Logger final : public CBaseLogger
    {
        std::string m_fileName;
        std::ofstream m_ostr;
        bool m_bFileCreated = false;
        public:
            File_Logger(const LPCSTR filename, const LoggingTraits& fTraits);
            ~File_Logger();
            bool IsFileCreated() const { return m_bFileCreated; }
            void trace(std::string_view msg) override;
    };

    class Callback_Logger final : public CBaseLogger
    {
        CTL_TwainDLLHandle* m_pHandle = {};
        public:
            Callback_Logger(CTL_TwainDLLHandle* pHandle) : m_pHandle(pHandle) {}
            void trace(std::string_view msg) override;
    };

    class CLogSystem
    {
    public:
        enum {FILE_LOGGING, DEBUG_WINDOW_LOGGING, CONSOLE_LOGGING, CALLBACK_LOGGING};

       /////////////////////////////////////////////////////////////////////////////
        std::unordered_map<int, std::shared_ptr<CBaseLogger>> app_logger_map;
        CLogSystem();
       ~CLogSystem() = default;

       /////////////////////////////////////////////////////////////////////////////
       bool     InitFileLogging(LPCTSTR pOutputFilename, HINSTANCE hInst, const LoggingTraits& fTraits);
       bool     InitConsoleLogging(HINSTANCE hInst, const LoggingTraits& lTraits); // adds console.
       bool     InitDebugWindowLogging(HINSTANCE hInst); // adds win debug logging.
       bool     InitCallbackLogging(HINSTANCE hInst);


       /////////////////////////////////////////////////////////////////////////////
       // output text, just like TRACE or printf
       bool     StatusOutFast(LPCSTR fmt);


       /////////////////////////////////////////////////////////////////////////////
       // turn it on or off
       void     Enable(bool bEnable);

       /////////////////////////////////////////////////////////////////////////////
       // timestamp each line?
       void     PrintTime(bool b) {m_bPrintTime = b;}

       /////////////////////////////////////////////////////////////////////////////
       // print the application name?
       void     PrintAppName(bool b) {m_bPrintAppName = b;}

       /////////////////////////////////////////////////////////////////////////////
       // override the default app name, which is the name the EXE (minus the ".exe")
       void     SetAppName(LPCSTR pName) {m_csAppName = pName;}

       bool     Flush();

       void     PrintBanner(bool bStarted = true);

       void DisableLogger(int loggerType);
       void DisableAllLoggers();

       std::string GetAppName() const {return m_csAppName;}
       void OutputDebugStringFull(std::string_view s);
       std::string GetDebugStringFull(std::string_view s);
       void SetDLLHandle(CTL_TwainDLLHandle* pHandle);
       int GetCurrentIndentLevel() const { return m_nCurrentIndentLevel; }
       void SetCurrentIndentLevel(int nLevel) { m_nCurrentIndentLevel = nLevel; }
       void IndentLine() { m_nCurrentIndentLevel += m_nIndentSize; }
       void OutdentLine() { m_nCurrentIndentLevel -= m_nIndentSize; m_nCurrentIndentLevel = std::max(0, m_nCurrentIndentLevel); }
       int GetIndentSize() const { return m_nIndentSize; }
       void SetIndentSize(int nSize) { m_nIndentSize = nSize; }

    protected:
       CTL_TwainDLLHandle* m_pDLLHandle;
       std::string m_csAppName;
       std::string m_csFileName;
       int m_nCurrentIndentLevel;
       int m_nIndentSize;

       /////////////////////////////////////////////////////////////////////////////
       // controlling stuff
       bool     m_bEnable;
       bool     m_bPrintTime;
       bool     m_bPrintAppName;
       bool     m_bFileOpenedOK;
       bool     m_bErrorDisplayed;

       /////////////////////////////////////////////////////////////////////////////
       // string utils
       std::string GetBaseDir(std::string_view path) const;
       std::string GetBaseName(std::string_view path) const;
       void GetModuleName(HINSTANCE hInst);
       bool WriteOnDemand(std::string_view fmt);

       private:
           bool InitLogger(int loggerType, LPCTSTR pOutputFilename, HINSTANCE hInst, const LoggingTraits& fTraits = {});
           static std::mutex s_logMutex;
    };
}
#endif

