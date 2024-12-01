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
#include "ctltwainmanager.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctldib.h"
using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FlipBitmap( HANDLE hDib )
{
    LOG_FUNC_ENTRY_PARAMS((hDib))
    // Flip bitmap
    CTL_TwainDib ThisDib;
    ThisDib.SetHandle(hDib);
    ThisDib.FlipBitMap(TRUE);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertDIBToBitmap(HANDLE hDib, HANDLE hPalette)
{
    LOG_FUNC_ENTRY_PARAMS((hDib, hPalette))
    const auto retVal = CDibInterface::DIBToBitmap(hDib, static_cast<HPALETTE>(hPalette));
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(HANDLE())
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertDIBToFullBitmap(HANDLE hDib, DTWAIN_BOOL isBMP)
{
    LOG_FUNC_ENTRY_PARAMS((hDib, isBMP))
    HANDLE returnHandle = {};
    if (isBMP)
    {
        returnHandle = CTL_TwainDib::CreateBMPBitmapFromDIB(hDib);
        LOG_FUNC_EXIT_NONAME_PARAMS(returnHandle)
    }
    else
    {
        HandleRAII raii(hDib);
        const LPBYTE pDibData = raii.getData();
        returnHandle = GlobalAlloc(GMEM_FIXED, GlobalSize(hDib));
        const HandleRAII raii2(returnHandle);
        const LPBYTE bFullImage = raii2.getData();
        std::copy_n(pDibData, GlobalSize(hDib), &bFullImage[0]);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(returnHandle)
    CATCH_BLOCK(HANDLE())
}
