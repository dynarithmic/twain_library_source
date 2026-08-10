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
#ifndef CTLTWAINMEMORYIMPL_H
#define CTLTWAINMEMORYIMPL_H
#include "twain.h"

namespace dynarithmic
{
    class CTL_TwainDynMemoryHandler
    {
            TW_HANDLE m_handle;
            TW_MEMREF m_memPtr;
            TW_UINT32 m_memSize;

        public:
            TW_HANDLE getHandle() const { return m_handle; }
            TW_MEMREF getMemoryPtr() const { return m_memPtr; }
            TW_UINT32 getMemorySize() const { return m_memSize; }
            void setHandle(TW_HANDLE h) { m_handle = h; }
            void setMemoryPtr(TW_MEMREF p) { m_memPtr = p; }
            void setMemorySize(TW_UINT32 s) { m_memSize = s; }
            CTL_TwainDynMemoryHandler(TW_HANDLE h=nullptr, TW_MEMREF p=nullptr, TW_UINT32 memSize = 0)
                    : m_handle(h), m_memPtr(p), m_memSize(memSize) {}
    };

    // mimics 2.0 memory function pointers
    class CTL_TwainMemoryFunctions
    {
        public:
            virtual ~CTL_TwainMemoryFunctions() = default;
            virtual TW_HANDLE AllocateMemory(TW_UINT32 size) = 0;
            virtual void      FreeMemory(TW_HANDLE h) = 0;
            virtual TW_MEMREF LockMemory(TW_HANDLE h) = 0;
            virtual void      UnlockMemory(TW_HANDLE h) = 0;

            TW_MEMREF AllocateMemoryPtr(TW_UINT32 size, TW_HANDLE* pHandle = nullptr)
            {
                const TW_HANDLE h = AllocateMemory(size);
                if ( h )
                {
                    if ( pHandle )
                       *pHandle = h;
                    return LockMemory(h);
                }
                if ( pHandle )
                  *pHandle = nullptr;
                return nullptr;
            }

            TW_MEMREF ReallocateMemory(CTL_TwainDynMemoryHandler& memHandler, TW_UINT32 newSize)
            {
                // Allocate new memory
                const TW_HANDLE newHandle = AllocateMemory(newSize);
                if (!newHandle)
                    return nullptr;

                // copy old memory to new memory
                const TW_MEMREF oldMem = memHandler.getMemoryPtr();
                const TW_MEMREF newMem = LockMemory(newHandle);
                memcpy(newMem, oldMem, (std::min)(newSize, memHandler.getMemorySize()));
                UnlockMemory(newMem);

                // delete the old memory
                UnlockMemory(memHandler.getHandle());
                FreeMemory(memHandler.getHandle());

                // copy memHandler by constructing a new memory handler
                memHandler = CTL_TwainDynMemoryHandler(newHandle, newMem, newSize);
                return newHandle;
            }
    };


    class CTL_LegacyTwainMemoryFunctions : public CTL_TwainMemoryFunctions
    {
        public:
        #ifdef WIN32
            TW_HANDLE AllocateMemory(TW_UINT32 size) override { return GlobalAlloc(GHND, size); }
            void      FreeMemory(TW_HANDLE h) override { if (h) GlobalFree( h ); }
            TW_MEMREF LockMemory(TW_HANDLE h) override
            { if (h) return GlobalLock(h); return nullptr; }
            void      UnlockMemory(TW_HANDLE h) override { if (h) GlobalUnlock(h); }
        #else
            TW_HANDLE AllocateMemory(TW_UINT32) { return nullptr; }
            void      FreeMemory(TW_HANDLE) { }
            TW_MEMREF LockMemory(TW_HANDLE) { return nullptr; }
            void      UnlockMemory(TW_HANDLE) { }
        #endif
    };


    class CTL_Twain2MemoryFunctions : public CTL_TwainMemoryFunctions
    {
        public:
            TW_ENTRYPOINT m_EntryPoint;
            TW_HANDLE AllocateMemory(TW_UINT32 size) override { return m_EntryPoint.DSM_MemAllocate(size); }
            void      FreeMemory(TW_HANDLE h) override { if (h) m_EntryPoint.DSM_MemFree(h); }
            TW_MEMREF LockMemory(TW_HANDLE h) override
            { if (h) return m_EntryPoint.DSM_MemLock(h); return nullptr; }
            void      UnlockMemory(TW_HANDLE h) override { if (h) m_EntryPoint.DSM_MemUnlock(h); }
    };
}
#endif