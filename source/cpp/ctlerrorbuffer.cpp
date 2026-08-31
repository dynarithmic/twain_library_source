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
#include "errorcheck.h"
#include "ctldtwainhandle.h"
using namespace dynarithmic;

extern "C"
{
    LONG DLLENTRY_DEF DTWAIN_GetLastError()
    {
        LOG_FUNC_ENTRY_PARAMS(())

        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
        if ( !pHandle )
        {
            LONG err = DTWAIN_ERR_BAD_HANDLE;
            if (!CTL_StaticData::ResourcesLoaded())
                err = CTL_StaticData::GetResourceLoadError();
            LOG_FUNC_EXIT_NONAME_PARAMS(err)
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_lLastError)
        CATCH_BLOCK(DTWAIN_ERR_BAD_HANDLE)
    }

    LONG DLLENTRY_DEF DTWAIN_SetLastError(LONG nError)
    {
        LOG_FUNC_ENTRY_PARAMS((nError))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
        if (!pHandle)
        {
            LONG err = DTWAIN_ERR_BAD_HANDLE;
            if (!CTL_StaticData::ResourcesLoaded())
                err = DTWAIN_ERR_RESOURCES_NOT_FOUND;
            LOG_FUNC_EXIT_NONAME_PARAMS(err)
        }
        pHandle->m_lLastError = nError;
        LOG_FUNC_EXIT_NONAME_PARAMS(DTWAIN_NO_ERROR)
        CATCH_BLOCK(-1)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetErrorBuffer(LPDTWAIN_ARRAY ArrayBuffer)
    {
        LOG_FUNC_ENTRY_PARAMS((ArrayBuffer))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);

        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] { return !ArrayBuffer; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

        const size_t nEntries = (std::min)(static_cast<size_t>(pHandle->m_nErrorBufferThreshold), pHandle->m_vErrorBuffer.size());
        auto retVal = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, static_cast<LONG>(nEntries));
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] {return !retVal.second; }, retVal.first, false, FUNC_MACRO);

        auto theArray =  retVal.second;
        DTWAINArrayLowLevelPtr_RAII raii(pHandle, &theArray);

        auto& vIn = pHandle->m_ArrayFactory->underlying_container_t<LONG>(theArray);
        std::copy_n(pHandle->m_vErrorBuffer.begin(), nEntries, vIn.begin());
        MoveArray(pHandle, ArrayBuffer, &theArray);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }


    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ClearErrorBuffer(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        std::deque<int> tempdeque;
        tempdeque.swap(pHandle->m_vErrorBuffer);

        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetErrorBufferThreshold(DWORD nErrors)
    {
        LOG_FUNC_ENTRY_PARAMS((nErrors))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);

        // Minimum of 50 errors
        const DWORD nEntries = (std::max<DWORD>)(nErrors, 50);

        // clear buffer
        pHandle->m_nErrorBufferThreshold = nEntries;

        std::deque<int> tempdeque;
        tempdeque.swap(pHandle->m_vErrorBuffer);

        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DWORD DLLENTRY_DEF DTWAIN_GetErrorBufferThreshold(VOID_PROTOTYPE)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        const DWORD nValues = pHandle->m_nErrorBufferThreshold;
        LOG_FUNC_EXIT_NONAME_PARAMS(nValues)
        CATCH_BLOCK(-1)
    }
}
