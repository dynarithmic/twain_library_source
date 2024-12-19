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
#ifndef ERRORCHECK_H
#define ERRORCHECK_H
#include "cppfunc.h"
#include "ctliface.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    template <typename Func, typename RetType, bool doThrow>
    void DTWAIN_Check_Error_Condition_0_Impl(CTL_TwainDLLHandle* Handle,
                                            Func f,
                                            int32_t Err,
                                            const std::string& extraInfo,
                                            RetType retErr,
                                            const std::string::value_type* fnName,
                                            bool logError=true)
    {
        Handle->m_lLastError = 0;
        const bool bRet = f() ? true : false;
        if (bRet)
        {
            Handle->m_lLastError = Err;
            CTL_TwainAppMgr::SetError(Err, extraInfo, false);
            OutputDTWAINErrorA(Handle, fnName);
            #if DTWAIN_BUILD_LOGCALLSTACK == 1
            if (logError && (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK))
            {
                LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallA(fnName, LOG_INDENT_OUT) +
                    ParamOutputter("", true).outputParam(retErr).getString());
            }
            #endif
            if (doThrow)
            throw retErr;
        }
    }

    template <typename Func, typename RetType>
    void DTWAIN_Check_Error_Condition_0_Ex(CTL_TwainDLLHandle* Handle,
                                            Func f,
                                            int32_t Err,
                                            RetType retErr,
                                            const std::string::value_type* fnName,
                                            bool logError=true)
    {
        DTWAIN_Check_Error_Condition_0_Impl<Func,RetType,true>(Handle,f,Err, "", retErr,fnName,logError);
    }

    template <typename Func, typename RetType>
    void DTWAIN_Check_Error_Condition_1_Ex(CTL_TwainDLLHandle* Handle,
                                            Func f,
                                            int32_t Err,
                                            RetType retErr,
                                            const std::string::value_type* fnName)
    { DTWAIN_Check_Error_Condition_0_Ex(Handle, f, Err, retErr, fnName, false); }

    template <typename Func, typename RetType>
    void DTWAIN_Check_Error_Condition_2_Ex(CTL_TwainDLLHandle* Handle,Func f,int32_t Err,RetType retErr,
                                           const std::string::value_type* fnName,bool logError = true)
    {
        DTWAIN_Check_Error_Condition_0_Impl<Func, RetType, false>(Handle, f, Err, "", retErr, fnName, logError);
    }

    template <typename Func, typename RetType>
    void DTWAIN_Check_Error_Condition_3_Ex(CTL_TwainDLLHandle* Handle,
                                           Func f,
                                           int32_t Err,
                                           const std::string& errInfo, 
                                           RetType retErr,
                                           const std::string::value_type* fnName)
    {
        DTWAIN_Check_Error_Condition_0_Impl<Func, RetType, true>(Handle, f, Err, errInfo, retErr, fnName, false);
    }

    template <typename RetType>
    bool DTWAIN_Check_Bad_Handle_Ex(CTL_TwainDLLHandle* pHandle, RetType retErr, const std::string::value_type* fnName, bool willThrow=true)
    {
        if (CTL_StaticData::IsCheckHandles() && !IsDLLHandleValid(pHandle, false))
        {
            OutputDTWAINErrorA(nullptr, fnName);
            if ( willThrow )
                throw retErr;
            return false;
        }
        return true;
    }

#define CHECK_IF_CAP_SUPPORTED(pSource, pHandle, nCap, retValue) \
    if (!pSource->IsCapInSupportedList(nCap)) \
    { \
        pHandle->m_lLastError = DTWAIN_ERR_CAP_NO_SUPPORT; \
        LOG_FUNC_EXIT_NONAME_PARAMS(retValue) \
    }
}
#endif
