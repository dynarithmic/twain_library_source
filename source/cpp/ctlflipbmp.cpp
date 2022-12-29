/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#include "ctltwmgr.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctldib.h"
#include "errorcheck.h"
using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FlipBitmap( HANDLE hDib )
{
    LOG_FUNC_ENTRY_PARAMS((hDib))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // Flip bitmap
    CTL_TwainDib ThisDib;

    ThisDib.SetHandle(hDib);
    ThisDib.FlipBitMap(TRUE);

    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertDIBToBitmap(HANDLE hDib, HANDLE hPalette)
{
    LOG_FUNC_ENTRY_PARAMS((hDib, hPalette))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);

    const auto retVal = CDibInterface::DIBToBitmap(hDib, static_cast<HPALETTE>(hPalette));

    LOG_FUNC_EXIT_PARAMS(retVal)
    CATCH_BLOCK(HANDLE())
}

struct HandleRAII 
{
    LPBYTE m_pByte;
    DTWAINGlobalHandle_RAII m_raii;
    HandleRAII(HANDLE h) : m_raii(h), m_pByte(static_cast<LPBYTE>(GlobalLock(h))) {}
    LPBYTE getData() const { return m_pByte; }
};

HANDLE DLLENTRY_DEF DTWAIN_ConvertDIBToFullBitmap(HANDLE hDib, DTWAIN_BOOL isBMP)
{
    LOG_FUNC_ENTRY_PARAMS((hDib, isBMP))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);

    // if hDIB is NULL, do nothing
    if (!hDib)
        LOG_FUNC_EXIT_PARAMS(HANDLE(0))

    HandleRAII raii(hDib);
    const LPBYTE pDibData = raii.getData();

    HANDLE returnHandle = nullptr;

    // attach file header if this is a DIB
    if (isBMP)
    {
        BITMAPFILEHEADER fileheader;
        memset(&fileheader, 0, sizeof(BITMAPFILEHEADER));
        fileheader.bfType = 'MB';
        const auto lpbi = reinterpret_cast<LPBITMAPINFOHEADER>(pDibData);
        const unsigned int bpp = lpbi->biBitCount;
        fileheader.bfSize = GlobalSize(hDib) + sizeof(BITMAPFILEHEADER);
        fileheader.bfReserved1 = 0;
        fileheader.bfReserved2 = 0;
        fileheader.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER)) +
            lpbi->biSize + CDibInterface::CalculateUsedPaletteEntries(bpp) * sizeof(RGBQUAD);

        // we need to attach the bitmap header info onto the data
        const unsigned int totalSize = GlobalSize(hDib) + sizeof(BITMAPFILEHEADER);
        // Allocate for returned handle
        returnHandle = static_cast<HANDLE>(GlobalAlloc(GMEM_FIXED, totalSize));
        const HandleRAII raii2(returnHandle);
        if (const LPBYTE bFullImage = raii2.getData())
        {
            char *pFileHeader = reinterpret_cast<char *>(&fileheader);
            std::copy_n(pFileHeader, sizeof(BITMAPFILEHEADER), &bFullImage[0]);
            std::copy_n(pDibData, GlobalSize(hDib), &bFullImage[sizeof(BITMAPFILEHEADER)]);
        }
    }
    else
    {
        returnHandle = GlobalAlloc(GMEM_FIXED, GlobalSize(hDib));
        const HandleRAII raii2(returnHandle);
        const LPBYTE bFullImage = raii2.getData();
        std::copy_n(pDibData, GlobalSize(hDib), &bFullImage[0]);
    }
    LOG_FUNC_EXIT_PARAMS(returnHandle)
    CATCH_BLOCK(HANDLE())
}
