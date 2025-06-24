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
#ifndef CTLLOGFUNCTIONCALL_H
#define CTLLOGFUNCTIONCALL_H

#include <string.h>
#include <winconst.h>
#include <string>
#include "logwriterutils.h"

namespace dynarithmic
{
    std::string CTL_LogFunctionCallHelper(LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString = nullptr);
    std::string CTL_LogFunctionCallA(LONG logFlags, LPCSTR pFuncName, int nWhich, LPCSTR pOptionalString = nullptr);
    long& GetLogFilterFlags();

    template <typename TupleType>
    std::string CTL_LogFunctionCallAndParamsIn_String(LONG filterFlags, LPCSTR pFuncName, std::string_view paramList, TupleType theArgs)
    {
        if (CTL_StaticData::GetLogFilterFlags() & filterFlags)
        {
            ParamOutputter pm(paramList);
            return CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, pFuncName, LOG_INDENT_IN) +
                            pm.processTupleArguments(theArgs, paramList, true, false); // .outputParam(std::forward<Args>(theArgs)...).getString();
        }
        return {};
    }

    template <typename TupleType>
    void CTL_LogFunctionCallAndParamsIn(LONG filterFlags, LPCSTR pFuncName, std::string_view paramList, TupleType theArgs)
    {
        if (CTL_StaticData::GetLogFilterFlags() & filterFlags)
            LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallAndParamsIn_String(filterFlags, pFuncName, paramList, theArgs));
    }

    template <typename TupleType>
    std::string CTL_LogFunctionCallAndParamsOut_String(LONG filterFlags, LPCSTR pFuncName, TupleType theArgs)
    {
        if (CTL_StaticData::GetLogFilterFlags() & filterFlags)
        {
            ParamOutputter pm("", true);
            return CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, pFuncName, LOG_INDENT_OUT) +
                                        pm.processTupleArguments(theArgs, "", false, false);
        }
        return {};
    }

    template <typename TupleType>
    void CTL_LogFunctionCallAndParamsOut(LONG filterFlags, LPCSTR pFuncName, TupleType theArgs)
    {
        if (CTL_StaticData::GetLogFilterFlags() & filterFlags)
            LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallAndParamsOut_String(filterFlags, pFuncName, theArgs));
    }

    template <typename TupleType>
    void CTL_LogFunctionDerefParams(LPCSTR pFuncName, std::string_view paramList, TupleType theArgs)
    {
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK)
        {
            ParamOutputter pm(paramList);
            std::string s = CTL_LogFunctionCallA(DTWAIN_LOG_CALLSTACK, pFuncName, LOG_INDENT_USELAST) +
                            pm.processTupleArguments(theArgs, paramList, true, true); 
            LogWriterUtils::WriteLogInfoA(s);
        }
    }
}
#endif