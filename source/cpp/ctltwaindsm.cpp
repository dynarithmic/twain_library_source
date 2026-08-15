/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

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
#include <algorithm>
#include "cppfunc.h"
#include "ctldtwainhandle.h"
#include "ctlstaticdata.h"
#include "ctlstringutils.h"
#include "ctlstringutilsx.h"
#include "ctltwainmanager.h"
#include "errorcheck.h"
using namespace dynarithmic;

namespace
{
    CTL_StringType CheckSearchOrderString(CTL_StringType str)
    {
        static std::set<TCHAR> setValidChars = {_T('C'),_T('W'),_T('O'),_T('U'), _T('S')};
        std::set<TCHAR> setDuplicates;
        CTL_StringType strOut;
        basicstringutils::MakeUpperCase(str);
        std::copy_if(str.begin(), str.end(), std::back_inserter(strOut), [&](TCHAR ch)
            {
                bool isValidChar = false;
                if (!setDuplicates.count(ch))
                    isValidChar = setValidChars.count(ch);
                setDuplicates.insert(ch);
                return isValidChar;
            });
        return strOut;
    }
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainDSM(LONG DSMType)
    {
        LOG_FUNC_ENTRY_PARAMS((DSMType))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        BOOL sessionEnabled = pHandle->m_bSessionAllocated;
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] { return sessionEnabled; }, 
                                                  DTWAIN_ERR_ACTIVE_TWAINSESSION, nullptr, FUNC_MACRO);
        #ifndef _WIN64
        if ( DSMType == DTWAIN_TWAINDSM_LEGACY || DSMType == DTWAIN_TWAINDSM_LATESTVERSION)
        {
            pHandle->m_SessionStruct.nSessionType = DSMType;
            pHandle->m_SessionStruct.DSMName = TWAINDLLVERSION_1;
        }
        else
        if ( DSMType == DTWAIN_TWAINDSM_VERSION2)
        {
            pHandle->m_SessionStruct.nSessionType = DTWAIN_TWAINDSM_VERSION2;
            pHandle->m_SessionStruct.DSMName = TWAINDLLVERSION_2;
        }
        #else
        pHandle->m_SessionStruct.nSessionType = DTWAIN_TWAINDSM_VERSION2;
        pHandle->m_SessionStruct.DSMName = TWAINDLLVERSION_2;
        #endif
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrder(LONG SearchOrder)
    {
        LOG_FUNC_ENTRY_PARAMS((SearchOrder))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        pHandle->m_TwainDSMSearchOrder = SearchOrder;
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    LONG DLLENTRY_DEF DTWAIN_GetDSMSearchOrder(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
        const LONG SearchOrder = pHandle->m_TwainDSMSearchOrder;
        LOG_FUNC_EXIT_NONAME_PARAMS(SearchOrder)
        CATCH_BLOCK(0)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderEx(LPCTSTR SearchOrder, LPCTSTR UserDirectory)
    {
        LOG_FUNC_ENTRY_PARAMS((SearchOrder, UserDirectory))
        if (!SearchOrder)
            LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        auto strValidString = CheckSearchOrderString(SearchOrder);
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
        if (!pHandle)
        {
            if (!strValidString.empty())
                CTL_StaticData::GetStartupDSMSearchOrder() = strValidString;
            CTL_StaticData::GetStartupDSMSearchOrderDir() = UserDirectory ? UserDirectory : _T("");
            LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
        }
        else
        {
            if (!strValidString.empty())
            {

                pHandle->m_TwainDSMSearchOrderStr = strValidString;
                pHandle->m_TwainDSMUserDirectory = UserDirectory ? UserDirectory : _T("");
                pHandle->m_TwainDSMSearchOrder = -1;
                CTL_StaticData::GetStartupDSMSearchOrder() =  strValidString;
                CTL_StaticData::GetStartupDSMSearchOrderDir() = UserDirectory ? UserDirectory : _T("");
                LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
            }
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        CATCH_BLOCK(false)
    }

    LONG DLLENTRY_DEF DTWAIN_GetDSMSearchOrderEx(LPTSTR SearchOrder, LPTSTR UserDirectory)
    {
        LOG_FUNC_ENTRY_PARAMS((SearchOrder, UserDirectory))
        if (SearchOrder)
            CopyInfoToCString(CTL_StaticData::GetStartupDSMSearchOrder(), SearchOrder, 6);

        LONG nSize = static_cast<LONG>(CTL_StaticData::GetStartupDSMSearchOrderDir().size() + 1);
        LONG retSize = CopyInfoToCString(CTL_StaticData::GetStartupDSMSearchOrderDir(), UserDirectory, nSize);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((SearchOrder, UserDirectory))
        LOG_FUNC_EXIT_NONAME_PARAMS(retSize)
        CATCH_BLOCK(DTWAIN_FAILURE1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetDSMFullName(LONG DSMType, LPTSTR szDLLName, LONG nMaxLen, LPLONG pWhichSearch)
    {
        LOG_FUNC_ENTRY_PARAMS((DSMType, szDLLName, nMaxLen, pWhichSearch))
        CTL_StringType sName;
        if ( DSMType == DTWAIN_TWAINDSM_LEGACY )
            sName = TWAINDLLVERSION_1;
        else
            sName = TWAINDLLVERSION_2;

        CTL_StringType sPath;
        CTL_StringType* strToSet = &sPath;
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
        if (pHandle)
        {
            if (DSMType == DTWAIN_TWAINDSM_LEGACY &&
                pHandle->m_bSessionAllocated &&
                pHandle->m_SessionStruct.nSessionType == DSMType)
            {
                strToSet = &pHandle->m_strTWAINPath;
            }
            else
            if (IsTwainDSM2(DSMType) && 
                IsTwainDSM2(pHandle->m_SessionStruct.nSessionType) &&
                pHandle->m_bSessionAllocated)
            {
                strToSet = &pHandle->m_strTWAINPath2;
            }
        
            if ( pWhichSearch )
                *pWhichSearch = pHandle->m_nTwainPathLocation;
        }
        if ( strToSet->empty() )
            *strToSet = CTL_TwainAppMgr::GetTwainDirFullName(sName.c_str(), pWhichSearch);

        const size_t nBytes = strToSet->length();
        if ( nBytes == 0)
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
        LONG nTotalBytes;
        if ( !szDLLName )
        {
            nTotalBytes = static_cast<LONG>(strToSet->length() + 1);
            LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
        }

        CTL_StringType sDLLName;
        std::copy(strToSet->begin(), strToSet->end(), std::back_inserter(sDLLName));
        nTotalBytes = CopyInfoToCString(sDLLName, szDLLName, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szDLLName, pWhichSearch))
        LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
        CATCH_BLOCK(0)
    }

    LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPath(LPTSTR szDLLName, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((szDLLName, nMaxLen))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto retVal = CopyInfoToCString(CTL_TwainAppMgr::GetDSMPath(), szDLLName, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szDLLName))
        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetActiveDSMVersionInfo(LPTSTR szDLLInfo, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((szDLLInfo, nMaxLen))
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto retVal = CopyInfoToCString(CTL_TwainAppMgr::GetDSMVersionInfo(), szDLLInfo, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szDLLInfo))
        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        CATCH_BLOCK(-1)
    }
}