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
#include <boost/format.hpp>

#include "cppfunc.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "dtwain_resource_constants.h"

using namespace dynarithmic;

static void LogExceptionToConsole(LPCSTR fname, const char* sAdditionalText=nullptr);

std::string dynarithmic::CTL_LogFunctionCallA(LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString/* = NULL*/)
{
    std::string ret;
    if (pOptionalString)
        ret = CTL_LogFunctionCallHelper(pFuncName, nWhich, pOptionalString);
    else
        ret = CTL_LogFunctionCallHelper(pFuncName, nWhich );
    return ret;
}

std::string dynarithmic::CTL_LogFunctionCallHelper(LPCSTR pFuncName, int nWhich, LPCSTR pString/*=NULL*/)
{
    if (CTL_StaticData::GetLogFilterFlags() == 0 )
         return {};
    static int nIndent = 0;
    std::string s;
    std::string s2;
    if ( pString )
        s2 = pString;
    if ( nWhich != LOG_NO_INDENT )
    {
        if ( nWhich == 0 || nWhich == LOG_INDENT_IN)
        {
            const std::string sTemp(nIndent, ' ');
            auto resText = GetResourceStringFromMap(IDS_LOGMSG_ENTERTEXT);
            if (resText.empty())
                resText = "Entering";
            s = sTemp + static_cast<std::string>("===>>>") + resText + " ";
            nIndent += 3;
        }
        else
        {
            nIndent -= 3;
            nIndent = (std::max)(0, nIndent);
            const std::string sTemp(nIndent, ' ');
            auto resText = GetResourceStringFromMap(IDS_LOGMSG_EXITTEXT);
            if (resText.empty())
                resText = "Exiting";
            s = sTemp + static_cast<std::string>("<<<===") + resText + " ";
        }
    }
    else
    {
        s = std::string(nIndent, ' ');
    }
    if ( !pString )
        s += pFuncName;
    else
        s += s2;
    if ( nWhich != LOG_INDENT_IN && nWhich != LOG_INDENT_OUT)
    {
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_USEFILE)
        {
            if (!CTL_StaticData::GetLogger().StatusOutFast( s.c_str() ) )
                CTL_StaticData::GetLogger().OutputDebugStringFull(s);
        }
        else
        {
           CTL_StaticData::GetLogger().OutputDebugStringFull(s);
        }
    }
    return s;
}

void dynarithmic::LogExceptionErrorA(LPCSTR fname, const char* sAdditionalText)
{
    if ( !(CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_SHOWEXCEPTIONS) )
         return;
    try
    {
       const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
       if (pHandle)
           pHandle->m_lLastError = DTWAIN_ERR_EXCEPTION_ERROR;
       else
           return;
       std::ostringstream output;

       output << "**** DTWAIN " << GetResourceStringFromMap(IDS_LOGMSG_EXCEPTERRORTEXT) <<  " ****.  " <<
                                   GetResourceStringFromMap(IDS_LOGMSG_MODULETEXT) << ": " <<  fname;
       if ( sAdditionalText )
          output << "\nAdditional Information: " << sAdditionalText;

       std::string s = output.str();
       if (!(CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_USEFILE))
            s += "\n";
       CTL_TwainAppMgr::WriteLogInfoA(s, true);  // flush all writes to the log file
       LogExceptionToConsole(fname, sAdditionalText);
    }
    catch(...)
    {
        LogExceptionToConsole(fname, sAdditionalText);
    }
}

void LogExceptionToConsole(LPCSTR fname, const char* sAdditionalText)
{
    try
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        if (pHandle)
            pHandle->m_lLastError = DTWAIN_ERR_EXCEPTION_ERROR;
        else
            return;
        std::ostringstream strm;
        strm << boost::format("**** DTWAIN %1% ****.  %2%: %3%\n") %
            GetResourceStringFromMap(IDS_LOGMSG_EXCEPTERRORTEXT).c_str() %
            GetResourceStringFromMap(IDS_LOGMSG_MODULETEXT).c_str() % fname;
        if (sAdditionalText)
            strm << "\nAdditional Information: " << sAdditionalText;
        #ifdef _WIN32
        MessageBoxA(nullptr, strm.str().c_str(), GetResourceStringFromMap(IDS_LOGMSG_EXCEPTERRORTEXT).c_str(), MB_ICONSTOP);
        #else
           std::cout << strm.str() << '\n';
        #endif
    }
    catch (...) {}  // can't really do anything
}
