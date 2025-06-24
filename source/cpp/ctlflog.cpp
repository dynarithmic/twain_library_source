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
#include <boost/format.hpp>

#include "cppfunc.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "dtwain_resource_constants.h"
#include "dtwain_exception.h"

using namespace dynarithmic;

static void LogExceptionToConsole(LPCSTR fname, const char* sAdditionalText=nullptr);

std::string dynarithmic::CTL_LogFunctionCallA(int32_t logFlags, const char *pFuncName, int nWhich, const char *pOptionalString/* = NULL*/)
{
    if (!(CTL_StaticData::GetLogFilterFlags() & logFlags))
        return {};
    std::string ret;
    if (pOptionalString)
        ret = CTL_LogFunctionCallHelper(pFuncName, nWhich, pOptionalString);
    else
        ret = CTL_LogFunctionCallHelper(pFuncName, nWhich );
    return ret;
}

std::string dynarithmic::CTL_LogFunctionCallHelper(const char *pFuncName, int nWhich, const char *pString)
{
    if (CTL_StaticData::GetLogFilterFlags() == 0 )
         return {};
    auto& theLogger = CTL_StaticData::GetLogger();
    int nIndent = theLogger.GetCurrentIndentLevel();
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
            theLogger.IndentLine();
        }
        else
        if ( nWhich == LOG_INDENT_OUT )
        {
            theLogger.OutdentLine();
            nIndent = theLogger.GetCurrentIndentLevel();
            const std::string sTemp(nIndent, ' ');
            auto resText = GetResourceStringFromMap(IDS_LOGMSG_EXITTEXT);
            if (resText.empty())
                resText = "Exiting";
            s = sTemp + static_cast<std::string>("<<<===") + resText + " ";
        }
        else
        if (nWhich == LOG_INDENT_USELAST)
        {
            const std::string sTemp(nIndent, ' '); 
            std::ostringstream strm;
            strm << sTemp << pFuncName << ": " << GetResourceStringFromMap(IDS_LOGMSG_RETURNSETVALUES) << ": ";
            s = strm.str();
        }
        else
        if (nWhich == LOG_INDENT_USELAST_NOFUNCTION)
        {
            const std::string sTemp(nIndent + theLogger.GetIndentSize(), ' '); 
            std::ostringstream strm;
            strm << sTemp << pFuncName;
            s = strm.str();
        }
    }
    else
    {
        s = std::string(nIndent, ' ');
    }
    if ( !pString && nWhich != LOG_INDENT_USELAST && nWhich != LOG_INDENT_USELAST_NOFUNCTION)
        s += pFuncName;
    else
        s += s2;
    if ( nWhich != LOG_INDENT_IN && nWhich != LOG_INDENT_OUT && nWhich != LOG_INDENT_USELAST)
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

void dynarithmic::LogExceptionErrorA(const char * fname, bool bIsCatchAll, const char* sAdditionalText)
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
       if (bIsCatchAll)
           output << "\n(Catch all invoked)";
       std::string s = output.str();
       if (!(CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_USEFILE))
            s += "\n";
       LogWriterUtils::WriteLogInfoIndentedA(s);  // flush all writes to the log file
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

long& dynarithmic::GetLogFilterFlags()
{
    return CTL_StaticData::GetLogFilterFlags();
}
