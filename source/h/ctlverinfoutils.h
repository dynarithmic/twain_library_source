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
#ifndef CTLVERINFOUTILS_H
#define CTLVERINFOUTILS_H

#include <array>
#include "dtwain_version.h"
#include "dtwaindefs.h"
#include "dtwain_standard_defs.h"

namespace dynarithmic
{
    constexpr std::array<int, 4> GetDTWAINDLLVersionInfo() noexcept
    {
        constexpr std::array<int, 4> aDLLVersion = { DTWAIN_MAJOR_VERSION,DTWAIN_MINOR_VERSION, DTWAIN_PATCHLEVEL_VERSION, DTWAIN_BUILDNUMBER_VERSION };
        return aDLLVersion;
    }

    constexpr LONG GetDTWAINVersionType() noexcept
    {
        LONG lVersionType = 0;
        #ifdef UNICODE
            lVersionType |= DTWAIN_UNICODE_VERSION;
        #endif

        #ifdef DTWAIN_DEBUG
            lVersionType |= DTWAIN_DEVELOP_VERSION;
        #endif

        #if defined (WIN64) || defined(_WIN64)
            lVersionType |= DTWAIN_64BIT_VERSION;
            #else
            #if defined (WIN32) || defined(_WIN32)
                lVersionType |= DTWAIN_32BIT_VERSION;
            #endif
        #endif

        #ifdef DTWAIN_DEVELOP_DLL
            lVersionType |= DTWAIN_DEVELOP_VERSION;
        #endif

        #if DTWAIN_BUILD_LOGCALLSTACK == 1
            lVersionType |= DTWAIN_CALLSTACK_LOGGING;
        #endif

        #if DTWAIN_BUILD_LOGCALLSTACK == 1 && DTWAIN_BUILD_LOGPOINTERS == 1
            lVersionType |= DTWAIN_CALLSTACK_LOGGING_PLUS;
        #endif
            return lVersionType;
    }
}
#endif
