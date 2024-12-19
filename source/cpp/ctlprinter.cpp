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
#include "errorcheck.h"
#include "arrayfactory.h"

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAvailablePrintersArray(DTWAIN_SOURCE Source, DTWAIN_ARRAY AvailPrinters)
{
    LOG_FUNC_ENTRY_PARAMS((Source, AvailPrinters))
    const DTWAIN_BOOL bRet = DTWAIN_SetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTER, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, AvailPrinters );
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

// Deprecated -- Use DTWAIN_SetAvailablePrintersArray
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAvailablePrinters(DTWAIN_SOURCE Source, LONG lpAvailPrinters)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpAvailPrinters))
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTER) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();

    DTWAIN_ARRAY Array = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 32);
    if ( !Array )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    // Destroys array when out of scope
    DTWAINArrayLowLevel_RAII a(pHandle, Array);
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);

    LONG j = 0;
    for ( LONG i = 0; i < 8; i++ )
    {
        if ( lpAvailPrinters & 1L << i )
        {
            vValues[j] = i;
            ++j;
        }
     }

    const DTWAIN_BOOL bRet = DTWAIN_SetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTER, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, Array );
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

// Deprecated -- Use DTWAIN_SetPrinterEx
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinter(DTWAIN_SOURCE Source, LONG nPrinter, DTWAIN_BOOL bSetCurrent)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nPrinter, bSetCurrent))
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTER) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    auto pSource = reinterpret_cast<CTL_ITwainSource*>(Source);
    auto pHandle = pSource->GetDTWAINHandle();
    DTWAIN_ARRAY Array = CreateArrayFromCap(pHandle, nullptr, DTWAIN_CV_CAPPRINTER, 1);
    if ( !Array )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    DTWAINArrayLowLevel_RAII a(pHandle, Array);

    LONG SetType = DTWAIN_CAPSET;
    if ( !bSetCurrent )
        SetType = DTWAIN_CAPRESET;
    bool bFound = false;
    for ( LONG i = 0; i < 8; i++ )
    {
        if ( nPrinter >> i == 1L )
        {
            nPrinter = i;
            bFound = true;
            break;
        }
    }
    bool bRet = false;
    if ( bFound )
    {
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);
        if ( !vValues.empty() )
            vValues[0] = nPrinter;
        bRet = DTWAIN_SetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTER, SetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, Array)?true:false;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterEx(DTWAIN_SOURCE Source, LONG nPrinter, DTWAIN_BOOL bSetCurrent)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nPrinter, bSetCurrent))
    if (!DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTER))
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    DTWAIN_ARRAY Array = CreateArrayFromCap(pHandle, nullptr, DTWAIN_CV_CAPPRINTER, 1);
    if (!Array)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    DTWAINArrayLowLevel_RAII a(pHandle, Array);

    LONG SetType = DTWAIN_CAPSET;
    if (!bSetCurrent)
        SetType = DTWAIN_CAPRESET;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);
    DTWAIN_BOOL bRet = 0;
    if (!vValues.empty())
    {
         vValues[0] = nPrinter;
         bRet = DTWAIN_SetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTER, SetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, Array);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPrinterEnabled(DTWAIN_SOURCE Source, LONG Printer)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Printer))
    LONG Current;
    if ( DTWAIN_GetPrinter(Source, &Current, TRUE ) )
    {
        if ( Current == Printer || Printer == DTWAIN_ANYSUPPORT)
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterStrings(DTWAIN_SOURCE Source, DTWAIN_ARRAY ArrayString, LPLONG pNumStrings)
{
    LOG_FUNC_ENTRY_PARAMS((Source, ArrayString, pNumStrings))
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTERSTRING) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    auto& factory = pHandle->m_ArrayFactory;
    // Check if array is of the correct type
    DTWAIN_Check_Error_Condition_0_Ex(pHandle,
                    [&]{ return !factory->is_valid(ArrayString, CTL_ArrayFactory::arrayTag::StringType);},
                        DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
    const size_t nStrings = factory->size(ArrayString);
    if ( nStrings == 0 )
    {
        if (pNumStrings)
            *pNumStrings = 0;
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }

    bool bRet;
    if ( nStrings == 1 )
    {
        // First try one value
        bRet = DTWAIN_SetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTERSTRING, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, ArrayString)?true:false;
        if ( !bRet )
        // Try enumerations
            bRet = DTWAIN_SetCapValuesEx(Source, DTWAIN_CV_CAPPRINTERSTRING, DTWAIN_CAPSETAVAILABLE,
                                      DTWAIN_CONTENUMERATION, ArrayString)?true:false;
        if ( bRet )
        {
            if ( pNumStrings )
                *pNumStrings = 1;
        }
    }
    else
    {
        // Try enumerations
        bRet = DTWAIN_SetCapValuesEx(Source, DTWAIN_CV_CAPPRINTERSTRING, DTWAIN_CAPSETAVAILABLE,
                                  DTWAIN_CONTENUMERATION, ArrayString)?true:false;
        if ( bRet )
        {
            if ( pNumStrings )
                *pNumStrings = static_cast<LONG>(nStrings);
        }
        else
        {
        //  try one value
            bRet = DTWAIN_SetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTERSTRING, DTWAIN_CAPSET,
                DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, ArrayString)?true:false;
            if ( bRet )
            {
                if ( pNumStrings )
                    *pNumStrings = 1;
            }
        }
    }
    if ( !bRet )
    {
        if ( pNumStrings )
            *pNumStrings = 0;
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pNumStrings))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_ARRAY GetPrinterMode(DTWAIN_SOURCE Source, LONG GetType)
{
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTERMODE) )
        return nullptr;
    DTWAIN_ARRAY Array = nullptr;
    const DTWAIN_BOOL bRet = DTWAIN_GetCapValuesEx2(Source, DTWAIN_CV_CAPPRINTERMODE, GetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &Array);
    if ( bRet )
        return Array;
    DTWAIN_ArrayDestroy(Array);
    return nullptr;
}
