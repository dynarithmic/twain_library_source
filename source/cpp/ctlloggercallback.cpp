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
#include "cppfunc.h"
#include "ctltwmgr.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif
using namespace dynarithmic;

#define CREATE_SETLOGGER_CALLBACK(loggerproc) \
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLoggerCallback##loggerproc(DTWAIN_LOGGER_PROC##loggerproc logProc, DTWAIN_LONG64 UserData) \
{ \
    LOG_FUNC_ENTRY_PARAMS((logProc)) \
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal()); \
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO); \
    pHandle->s_pLoggerCallback##loggerproc = logProc; \
    pHandle->s_pLoggerCallback_UserData##loggerproc = UserData; \
    LOG_FUNC_EXIT_PARAMS(true) \
    CATCH_BLOCK(false); \
}

#define CREATE_GETLOGGER_CALLBACK(loggerproc) \
DTWAIN_LOGGER_PROC##loggerproc DLLENTRY_DEF DTWAIN_GetLoggerCallback##loggerproc(VOID_PROTOTYPE) \
{ \
    LOG_FUNC_ENTRY_PARAMS(()) \
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal()); \
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO); \
    DTWAIN_LOGGER_PROC##loggerproc theProc = pHandle->m_LoggerCallbackInfo.m_pLoggerCallback##loggerproc; \
    LOG_FUNC_EXIT_PARAMS(theProc) \
    CATCH_BLOCK(DTWAIN_LOGGER_PROC##loggerproc(0)) \
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLoggerCallback(DTWAIN_LOGGER_PROC logProc, DTWAIN_LONG64 UserData)
{
    LOG_FUNC_ENTRY_PARAMS((logProc))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    pHandle->m_LoggerCallbackInfo.m_pLoggerCallback = logProc;
    pHandle->m_LoggerCallbackInfo.m_pLoggerCallback_UserData = UserData;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_LOGGER_PROC DLLENTRY_DEF DTWAIN_GetLoggerCallback(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(pHandle->m_LoggerCallbackInfo.m_pLoggerCallback)
    CATCH_BLOCK(DTWAIN_LOGGER_PROC(0))
}

template <typename LoggerProc, typename CallBackType, typename UserDataType>
static void LoggerCallBackSetter(LoggerProc lprocIn, LoggerProc& lProcOut, UserDataType& userOut, DTWAIN_LONG64 UserData)
{
    lProcOut = lprocIn;
    userOut = UserData;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLoggerCallbackA(DTWAIN_LOGGER_PROCA logProc, DTWAIN_LONG64 UserData)
{
    LOG_FUNC_ENTRY_PARAMS((logProc)) 
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal()); 
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO); 
    pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA = logProc;
    pHandle->m_LoggerCallbackInfo.m_pLoggerCallback_UserDataA = UserData;
    LOG_FUNC_EXIT_PARAMS(true) 
    CATCH_BLOCK(false) 
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLoggerCallbackW(DTWAIN_LOGGER_PROCW logProc, DTWAIN_LONG64 UserData)
{
    LOG_FUNC_ENTRY_PARAMS((logProc))
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW = logProc;
    pHandle->m_LoggerCallbackInfo.m_pLoggerCallback_UserDataW = UserData;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


CREATE_GETLOGGER_CALLBACK(A)
CREATE_GETLOGGER_CALLBACK(W)
