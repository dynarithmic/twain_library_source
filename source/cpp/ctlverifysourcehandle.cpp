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
#include "ctltwainmanager.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_CheckHandles(DTWAIN_BOOL bCheck)
{
    LOG_FUNC_ENTRY_PARAMS((bCheck))
    CTL_StaticData::SetCheckHandles(bCheck ? true : false);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

namespace dynarithmic
{

    static bool DTWAIN_Check_Bad_Handle_Ex2(CTL_TwainDLLHandle* pHandle, const std::string::value_type* fnName)
    {
        if (CTL_StaticData::IsCheckHandles() && !IsDLLHandleValid(pHandle, false))
        {
            OutputDTWAINErrorA(nullptr, fnName);
            return false;
        }
        return true;
    }

    static void ThrowBadHandle()
    {
        std::error_code ec(DTWAIN_ERR_BAD_HANDLE, std::system_category());
        std::system_error err(ec, "Invalid DTWAIN Handle");
        // Since the handle is bad, the only way to report the
        // error visually to the app is to write to whatever debug
        // logger may be attached to the program.
        OutputDebugStringA(err.what());
        throw DTWAINException(DTWAIN_ERR_BAD_HANDLE);
    }

    static std::pair<CTL_TwainDLLHandle*, CTL_ITwainSource*> ThrowSourceError(CTL_TwainDLLHandle* pHandle, bool setLastError, bool doThrow)
    {
        if (setLastError)
            pHandle->m_lLastError = DTWAIN_ERR_BAD_SOURCE;
        if (doThrow)
            throw DTWAINException(DTWAIN_ERR_BAD_SOURCE);
        return { nullptr, nullptr };
    }

    std::pair<CTL_TwainDLLHandle*, CTL_ITwainSource*> VerifyHandles(DTWAIN_SOURCE Source, int Testing/* = DTWAIN_TEST_DLLHANDLE | DTWAIN_TEST_SOURCE*/)
    {
        CTL_ITwainSource* pSource = nullptr;
        CTL_TwainDLLHandle* pHandle = nullptr;
        bool doThrow = !(Testing & DTWAIN_TEST_NOTHROW);
        bool setLastError = Testing & DTWAIN_TEST_SETLASTERROR;

        if (!CTL_StaticData::IsCheckHandles())
        {
            pSource = static_cast<CTL_ITwainSource*>(Source);
            return { static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal()), pSource };
        }
        else
        {
            bool bHandleGood = true;
            if (Testing & DTWAIN_VERIFY_DLLHANDLE)
            {
                pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
                bHandleGood = DTWAIN_Check_Bad_Handle_Ex2(pHandle, FUNC_MACRO);
            }
            if (!bHandleGood)
            {
                if (doThrow)
                    ThrowBadHandle();
                return { nullptr, nullptr };
            }

            if ((Testing & DTWAIN_VERIFY_SOURCEHANDLE) || (Testing & DTWAIN_TEST_SOURCEOPEN))
            {
                if (!pHandle)
                    pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

                if (!pHandle)
                {
                    if (doThrow)
                        ThrowBadHandle();
                    return { nullptr, nullptr };
                }

                pSource = static_cast<CTL_ITwainSource*>(Source);
                if (!pSource || !CTL_TwainAppMgr::IsValidTwainSource(pHandle->m_pTwainSession, pSource))
                    return ThrowSourceError(pHandle, setLastError, doThrow);

                if (Testing & DTWAIN_TEST_SOURCEOPEN)
                {
                    if (!CTL_TwainAppMgr::IsSourceOpen(pSource))
                        return ThrowSourceError(pHandle, setLastError, doThrow);
                }
            }
        }
        // no error
        if (Testing & DTWAIN_TEST_SETLASTERROR)
            pHandle->m_lLastError = DTWAIN_NO_ERROR;
        return { pHandle, pSource };
    }
}
