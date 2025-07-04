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
#include "cppfunc.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
using namespace dynarithmic;


// Unused
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainTimeout( LONG milliseconds )
{
    LOG_FUNC_ENTRY_PARAMS((milliseconds))
    #if 0
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CTL_StaticData::SetTimeoutValue(milliseconds);
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

// Unused
LONG DLLENTRY_DEF DTWAIN_GetTwainTimeout()
{
    LOG_FUNC_ENTRY_PARAMS(())
    #if 0
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
        LOG_FUNC_EXIT_NONAME_PARAMS(-1)
    LOG_FUNC_EXIT_NONAME_PARAMS((LONG)CTL_StaticData::GetTimeoutValue())
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(0)
    CATCH_BLOCK(-1)
}

