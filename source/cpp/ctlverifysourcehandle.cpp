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
#include "ctltwmgr.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_CheckHandles(DTWAIN_BOOL bCheck)
{
    LOG_FUNC_ENTRY_PARAMS((bCheck))
    CTL_StaticData::s_bCheckHandles = bCheck ? true : false;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

std::pair<CTL_TwainDLLHandle*, CTL_ITwainSource*> dynarithmic::VerifySourceHandle(DTWAIN_SOURCE Source, int Testing/* = DTWAIN_TEST_HANDLE | DTWAIN_TEST_SOURCE*/)
{
    CTL_ITwainSource *pSource = nullptr;
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
        if (Testing & DTWAIN_TEST_HANDLE)
        {
            pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            bHandleGood = DTWAIN_Check_Bad_Handle_Ex<std::pair<CTL_TwainDLLHandle*, CTL_ITwainSource*>>(pHandle, { nullptr, nullptr }, FUNC_MACRO, false);
        }
        if ( !bHandleGood )
        {
            if (doThrow )
                throw DTWAIN_ERR_BAD_HANDLE;
            return { nullptr, nullptr };
        }
        if ( Testing & DTWAIN_TEST_SOURCE )
        {
            if ( !pHandle )
                pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

            if (!pHandle)
            {
                if ( doThrow )
                    throw DTWAIN_ERR_BAD_HANDLE;
                return { nullptr, nullptr };
            }

            pSource = static_cast<CTL_ITwainSource*>(Source);
            if (!pSource || !CTL_TwainAppMgr::IsValidTwainSource(pHandle->m_pTwainSession, pSource))
            {
                if (setLastError)
                    pHandle->m_lLastError = DTWAIN_ERR_BAD_SOURCE;
                if ( doThrow )
                    throw DTWAIN_ERR_BAD_SOURCE;
                return { nullptr, nullptr };
            }
        }
    }
    return { pHandle, pSource };
}

