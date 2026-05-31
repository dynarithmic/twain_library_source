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
#include <array>
#include <algorithm>

#include "cppfunc.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
using namespace dynarithmic;

// This file processes both patch codes and bar code caps, since currently the capabilities
// described by the TWAIN specification as of TWAIN 2.5 have identical names (except for the 
// BAR and PATCH differences in the name.
static bool IsCapInList(CTL_ITwainSource* pSource, const std::array<LONG, 7>& capList)
{
    const auto iter = std::find_if(std::begin(capList), std::end(capList), [&](LONG val)
        { return pSource->IsCapInSupportedList(static_cast<TW_UINT16>(val)) ? true : false; });
    const bool bRet = iter != std::end(capList);
    return bRet;
}

/////////////// Patch code support //////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPatchcodeCapsSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    constexpr std::array<LONG, 7> nCapToTest = {
        ICAP_PATCHCODESEARCHMODE,
        ICAP_PATCHCODEDETECTIONENABLED,
        ICAP_SUPPORTEDPATCHCODETYPES   ,
        ICAP_PATCHCODEMAXSEARCHPRIORITIES,
        ICAP_PATCHCODESEARCHPRIORITIES,
        ICAP_PATCHCODEMAXRETRIES,
        ICAP_PATCHCODETIMEOUT
    };
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto bRet = IsCapInList(pSource, nCapToTest);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

/////////////// barcode support //////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsBarcodeCapsSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
        constexpr std::array<LONG, 7> nCapToTest = {
            ICAP_BARCODESEARCHMODE,
            ICAP_BARCODEDETECTIONENABLED,
            ICAP_SUPPORTEDBARCODETYPES   ,
            ICAP_BARCODEMAXSEARCHPRIORITIES,
            ICAP_BARCODESEARCHPRIORITIES,
            ICAP_BARCODEMAXRETRIES,
            ICAP_BARCODETIMEOUT
    };

    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    auto bRet = IsCapInList(pSource, nCapToTest);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

/////////////////////////
