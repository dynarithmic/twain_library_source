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

// TWAINDSM 2.0 memory related functions
template <typename VarType>
struct TwainAllocation
{
    CTL_TwainDLLHandle* m_pHandle = nullptr;
    TwainAllocation(CTL_TwainDLLHandle* pHandle) : m_pHandle(pHandle) {}
    HANDLE operator()(VarType memSize)
    {
        HANDLE h = {};
        if (m_pHandle->m_TwainMemoryFunc)
            h = m_pHandle->m_TwainMemoryFunc->AllocateMemory(memSize);
        return h;
    }
};

// General Windows memory allocation functions
template <typename VarType>
struct SystemAllocation
{
    HANDLE operator()(VarType memSize)
    {
        HANDLE h = ImageMemoryHandler::GlobalAlloc(GHND, memSize);
        #ifdef WIN32
        if (!h)
            dynarithmic::LogWin32Error(ImageMemoryHandler::GetLastError());
        #endif
        return h;
    }
};

template <typename AllocationFunc, typename VarType>
static HANDLE GeneralAllocator(ULONG64 memSize, AllocationFunc fn)
{
    // Call the memory allocation function
    return fn(static_cast<VarType>(memSize)); 
}

HANDLE  DLLENTRY_DEF DTWAIN_AllocateMemory(DWORD memSize)
{
    LOG_FUNC_ENTRY_PARAMS((memSize))
    auto h = GeneralAllocator<SystemAllocation<SIZE_T>, SIZE_T>(memSize, SystemAllocation<SIZE_T>{});
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK(HANDLE())
}

// DTWAIN 2.0 memory related functions for 64-bit allocations
HANDLE  DLLENTRY_DEF DTWAIN_AllocateMemory64(ULONG64 memSize)
{
    LOG_FUNC_ENTRY_PARAMS((memSize))
    #ifndef _WIN64
        #ifdef WIN32
            // Make sure that the amount of memory is not > 4GB
            SIZE_T maxMemory = static_cast<SIZE_T>(std::min<ULONG64>(memSize, std::numeric_limits<SIZE_T>::max()));
            auto h = GeneralAllocator<SystemAllocation<SIZE_T>, SIZE_T>(maxMemory, SystemAllocation<SIZE_T>{});
        #endif
    #else
        // 64-bit, so we don't care what memSize is.
        auto h = GeneralAllocator<SystemAllocation<SIZE_T>, SIZE_T>(memSize, SystemAllocation<SIZE_T>{});
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK(HANDLE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FreeMemory(HANDLE h)
{
    LOG_FUNC_ENTRY_PARAMS((h))
    const DTWAIN_BOOL bRet = ImageMemoryHandler::GlobalFree(h) ? TRUE : FALSE;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_MEMORY_PTR DLLENTRY_DEF DTWAIN_LockMemory(HANDLE h)
{
    LOG_FUNC_ENTRY_PARAMS((h))
    const DTWAIN_MEMORY_PTR ptr = ImageMemoryHandler::GlobalLock(h);
    LOG_FUNC_EXIT_NONAME_PARAMS(ptr)
    CATCH_BLOCK(DTWAIN_MEMORY_PTR())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_UnlockMemory(HANDLE h)
{
    LOG_FUNC_ENTRY_PARAMS((h))
    const DTWAIN_BOOL bRet = ImageMemoryHandler::GlobalUnlock(h);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

HANDLE  DLLENTRY_DEF DTWAIN_AllocateMemoryEx(DWORD memSize)
{
    LOG_FUNC_ENTRY_PARAMS((memSize))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    TwainAllocation<DWORD> allocation(pHandle);
    auto h = GeneralAllocator<TwainAllocation<DWORD>, DWORD>(memSize, allocation);
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK(HANDLE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FreeMemoryEx(HANDLE h)
{
    LOG_FUNC_ENTRY_PARAMS((h))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_BOOL bRet = FALSE;
    if (pHandle->m_TwainMemoryFunc)
    {
        pHandle->m_TwainMemoryFunc->FreeMemory(h);
        bRet = TRUE;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_MEMORY_PTR DLLENTRY_DEF DTWAIN_LockMemoryEx(HANDLE h)
{
    LOG_FUNC_ENTRY_PARAMS((h))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_MEMORY_PTR ptr = nullptr;
    if (pHandle->m_TwainMemoryFunc)
        ptr = pHandle->m_TwainMemoryFunc->LockMemory(h);
    LOG_FUNC_EXIT_NONAME_PARAMS(ptr)
    CATCH_BLOCK(DTWAIN_MEMORY_PTR())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_UnlockMemoryEx(HANDLE h)
{
    LOG_FUNC_ENTRY_PARAMS((h))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_BOOL bRet = FALSE;
    if (pHandle->m_TwainMemoryFunc)
    {
        pHandle->m_TwainMemoryFunc->UnlockMemory(h);
        bRet = TRUE;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}
