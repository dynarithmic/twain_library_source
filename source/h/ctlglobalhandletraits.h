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
#ifndef CTLGLOBALHANDLETRAITS_H
#define CTLGLOBALHANDLETRAITS_H

#include <memory>
#include "dtwain_standard_defs.h"

namespace dynarithmic
{
    struct DTWAINGlobalHandle_CloseTraits
    {
        static void Destroy(HANDLE h);
        void operator()(HANDLE h) const { Destroy(h); }
    };

    struct DTWAINGlobalHandle_ClosePtrTraits
    {
        static void Destroy(HANDLE* h);
        void operator()(HANDLE* h) const { Destroy(h); }
    };

    struct DTWAINGlobalHandle_CloseFreeTraits
    {
        static void Destroy(HANDLE h);
        void operator()(HANDLE h) const { Destroy(h); }
    };

    template <typename T, typename UnLockFn, typename FreeFn>
    struct DTWAINGlobalHandle_GenericUnlockFreeTraits
    {
        static void Destroy(T* h)
        {
            UnLockFn::Unlock(h);
            FreeFn::Free(h);
        }
        void operator()(T* h) { Destroy(h); }
    };

    using DTWAINGlobalHandlePtr_RAII = std::unique_ptr<HANDLE, DTWAINGlobalHandle_ClosePtrTraits>;
    using DTWAINGlobalHandle_RAII = std::unique_ptr<void, DTWAINGlobalHandle_CloseTraits>;
    using DTWAINGlobalHandleUnlockFree_RAII = std::unique_ptr<void, DTWAINGlobalHandle_CloseFreeTraits>;

    struct HandleRAII
    {
        LPBYTE m_pByte;
        DTWAINGlobalHandle_RAII m_raii;
        HandleRAII(HANDLE h);
        LPBYTE getData() const { return m_pByte; }
        HandleRAII(HandleRAII&) = delete;
        HandleRAII& operator=(HandleRAII&) = delete;
    };
}
#endif
