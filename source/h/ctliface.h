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
#ifndef CTLIFACE_H
#define CTLIFACE_H

#ifdef _MSC_VER
#pragma warning( disable : 4786)
#pragma warning (disable : 4127)
#endif

#ifdef _WIN32
    #include "winlibraryloader_impl.inl"
#else
    #include "linuxlibraryloader_impl.inl"
#endif
#include <memory>
#include "dtwtype.h"
#include "winconst.h"
namespace dynarithmic
{
    template <typename T>
    struct dtwain_library_loader : library_loader_impl
    {
        static T get_func_ptr(void* handle, const char* name)
        {
            return static_cast<T>(get(handle, name));
        }
    };

    struct SysInitializeOptions
    {
        bool showErrorBox = false;
        bool createErrorLog = false;
        bool createMinimalSetup = false;
    };

    class CTL_TwainDLLHandle;
    struct SysInitializeOptions;
    DTWAIN_HANDLE SysInitializeImpl(const SysInitializeOptions& initOptions);
    bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId);
    LONG DTWAIN_CloseAllSources();
    HMODULE GetDLLInstance();

    void LoadOCRInterfaces(CTL_TwainDLLHandle *pHandle);
    void UnloadOCRInterfaces(CTL_TwainDLLHandle *pHandle);

    DTWAIN_HANDLE GetDTWAINHandle_Internal();
    void SysDestroyNoCheck();
    void  DTWAIN_InternalThrowException();
}

#endif
