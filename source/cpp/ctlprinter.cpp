/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

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
#include "ctltwmgr.h"
#include "errorcheck.h"
#include "arrayfactory.h"

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAvailablePrintersArray(DTWAIN_SOURCE Source, DTWAIN_ARRAY AvailPrinters)
{
    LOG_FUNC_ENTRY_PARAMS((Source, AvailPrinters))
    const DTWAIN_BOOL bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPPRINTER, DTWAIN_CAPSET, AvailPrinters );
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

// Deprecated -- Use DTWAIN_SetAvailablePrintersArray
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAvailablePrinters(DTWAIN_SOURCE Source, LONG lpAvailPrinters)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpAvailPrinters))
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTER) )
        LOG_FUNC_EXIT_PARAMS(false)

    DTWAIN_ARRAY Array = CreateArrayFromFactory(DTWAIN_ARRAYLONG, 32);
    if ( !Array )
        LOG_FUNC_EXIT_PARAMS(false)

    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

    // Destroys array when out of scope
    DTWAINArrayLL_RAII a(Array);
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

    const DTWAIN_BOOL bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPPRINTER, DTWAIN_CAPSET, Array );
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

// Deprecated -- Use DTWAIN_SetPrinterEx
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinter(DTWAIN_SOURCE Source, LONG nPrinter, DTWAIN_BOOL bSetCurrent)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nPrinter, bSetCurrent))
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTER) )
        LOG_FUNC_EXIT_PARAMS(false)
    DTWAIN_ARRAY Array = DTWAIN_ArrayCreateFromCap(nullptr, DTWAIN_CV_CAPPRINTER, 1);
    if ( !Array )
        LOG_FUNC_EXIT_PARAMS(false)

    DTWAINArrayLL_RAII a(Array);

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
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);
        if ( !vValues.empty() )
            vValues[0] = nPrinter;
        bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPPRINTER, SetType, Array)?true:false;
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterEx(DTWAIN_SOURCE Source, LONG nPrinter, DTWAIN_BOOL bSetCurrent)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nPrinter, bSetCurrent))
    if (!DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTER))
        LOG_FUNC_EXIT_PARAMS(false)
    DTWAIN_ARRAY Array = DTWAIN_ArrayCreateFromCap(nullptr, DTWAIN_CV_CAPPRINTER, 1);
    if (!Array)
        LOG_FUNC_EXIT_PARAMS(false)

    DTWAINArrayLL_RAII a(Array);

    LONG SetType = DTWAIN_CAPSET;
    if (!bSetCurrent)
        SetType = DTWAIN_CAPRESET;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);
    DTWAIN_BOOL bRet = 0;
    if (!vValues.empty())
    {
         vValues[0] = nPrinter;
         bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPPRINTER, SetType, Array);
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPrinterEnabled(DTWAIN_SOURCE Source, LONG Printer)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Printer))
    LONG Current;
    if ( DTWAIN_GetPrinter(Source, &Current, TRUE ) )
    {
        if ( Current == Printer || Printer == DTWAIN_ANYSUPPORT)
            LOG_FUNC_EXIT_PARAMS(true)
    }
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetPrinterStrings(DTWAIN_SOURCE Source, DTWAIN_ARRAY ArrayString, LPLONG pNumStrings)
{
    LOG_FUNC_ENTRY_PARAMS((Source, ArrayString, pNumStrings))
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTERSTRING) )
        LOG_FUNC_EXIT_PARAMS(false)

    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *p = VerifySourceHandle( pHandle, Source );
    auto& factory = pHandle->m_ArrayFactory;
    if ( p )
    {
        // Check if array is of the correct type
        DTWAIN_Check_Error_Condition_0_Ex(pHandle,
                        [&]{ return !factory->is_valid(ArrayString, CTL_ArrayFactory::arrayTag::StringType);},
                         DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
    }
    else
        LOG_FUNC_EXIT_PARAMS(false)

    const size_t nStrings = factory->size(ArrayString);
    if ( nStrings == 0 )
    {
        if (pNumStrings)
            *pNumStrings = 0;
        LOG_FUNC_EXIT_PARAMS(true)
    }

    bool bRet;
    if ( nStrings == 1 )
    {
        // First try one value
        bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPPRINTERSTRING, DTWAIN_CAPSET, ArrayString)?true:false;
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
            bRet = DTWAIN_SetCapValues(Source, DTWAIN_CV_CAPPRINTERSTRING, DTWAIN_CAPSET,
                                    ArrayString)?true:false;
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
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_ARRAY GetPrinterMode(DTWAIN_SOURCE Source, LONG GetType)
{
    if ( !DTWAIN_IsCapSupported(Source, DTWAIN_CV_CAPPRINTERMODE) )
        return nullptr;
    DTWAIN_ARRAY Array = nullptr;
    const DTWAIN_BOOL bRet = DTWAIN_GetCapValues(Source, DTWAIN_CV_CAPPRINTERMODE, GetType, &Array);
    if ( bRet )
        return Array;
    DTWAIN_ArrayDestroy(Array);
    return nullptr;
}
