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
#include "cppfunc.h"
#include "ctldtwainhandle.h"
#include "ctltwaindllhandle.h"
#include "ctltwainsession.h"

using namespace dynarithmic;
extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfo(LPCTSTR szVerStr, LPCTSTR szManu, LPCTSTR szProdFam, LPCTSTR szProdName)
    {
        LOG_FUNC_ENTRY_PARAMS((szVerStr, szManu, szProdFam, szProdName))
        // See if DLL Handle exists
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if ( szVerStr )
            pHandle->m_SessionStruct.szVersion = szVerStr;
        if ( szManu )
            pHandle->m_SessionStruct.szManufact = szManu;
        if ( szProdFam )
            pHandle->m_SessionStruct.szFamily = szProdFam;
        if ( szProdName )
            pHandle->m_SessionStruct.szProduct = szProdName;
        if ( pHandle->m_bSessionAllocated && pHandle->m_pTwainSession )
        {
            pHandle->m_pTwainSession->FillTWIdentity(pHandle);
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAppInfo(LPTSTR szVerStr, LPTSTR szManu, LPTSTR szProdFam, LPTSTR szProdName)
    {
        if (szVerStr)
            szVerStr[0] = 0;

        if (szManu)
            szManu[0] = 0;

        if (szProdFam)
            szProdFam[0] = 0;

        if (szProdName)
            szProdName[0] = 0;

        LOG_FUNC_ENTRY_PARAMS((szVerStr, szManu, szProdFam, szProdName))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if ( szVerStr )
            std::copy(pHandle->m_SessionStruct.szVersion.begin(), pHandle->m_SessionStruct.szVersion.end(), szVerStr);
        if ( szManu )
            std::copy(pHandle->m_SessionStruct.szManufact.begin(), pHandle->m_SessionStruct.szManufact.end(), szManu);
        if ( szProdFam )
            std::copy(pHandle->m_SessionStruct.szFamily.begin(), pHandle->m_SessionStruct.szFamily.end(), szProdFam);
        if ( szProdName )
            std::copy(pHandle->m_SessionStruct.szProduct.begin(), pHandle->m_SessionStruct.szProduct.end(), szProdName);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szVerStr, szManu, szProdFam, szProdName))
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCountry(LONG nCountry)
    {
        LOG_FUNC_ENTRY_PARAMS((nCountry))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        pHandle->m_SessionStruct.nCountry = static_cast<TW_UINT16>(nCountry);
        if (pHandle->m_bSessionAllocated && pHandle->m_pTwainSession)
        {
            pHandle->m_pTwainSession->FillTWIdentity(pHandle);
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLanguage(LONG nLanguage)
    {
        LOG_FUNC_ENTRY_PARAMS((nLanguage))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        pHandle->m_SessionStruct.nLanguage = static_cast<TW_UINT16>(nLanguage);
        if (pHandle->m_bSessionAllocated && pHandle->m_pTwainSession)
        {
            pHandle->m_pTwainSession->FillTWIdentity(pHandle);
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    LONG DLLENTRY_DEF DTWAIN_GetCountry()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_SessionStruct.nCountry)
        CATCH_BLOCK(DTWAIN_FAILURE1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetLanguage()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_SessionStruct.nLanguage)
        CATCH_BLOCK(DTWAIN_FAILURE1)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetMajorMinorVersion(DWORD nMajor, DWORD nMinor)
    {
        LOG_FUNC_ENTRY_PARAMS((nMajor, nMinor))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        pHandle->m_SessionStruct.nMajorNum = static_cast<TW_UINT16>(nMajor);
        pHandle->m_SessionStruct.nMinorNum = static_cast<TW_UINT16>(nMinor);
        if (pHandle->m_bSessionAllocated && pHandle->m_pTwainSession)
        {
            pHandle->m_pTwainSession->FillTWIdentity(pHandle);
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetMajorMinorVersion(LPDWORD pMajor, LPDWORD pMinor)
    {
        LOG_FUNC_ENTRY_PARAMS((pMajor, pMinor))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if (pMajor)
            *pMajor = pHandle->m_SessionStruct.nMajorNum;
        if (pMinor)
            *pMinor = pHandle->m_SessionStruct.nMinorNum;
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pMajor, pMinor))
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }
}
