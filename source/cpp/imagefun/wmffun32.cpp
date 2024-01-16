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
#include "wmffun32.h"
#include "ctliface.h"
using namespace dynarithmic;
std::string CWMFImageHandler::GetFileExtension() const
{
    return "WMF";
}

HANDLE CWMFImageHandler::GetFileInformation(LPCSTR /*path*/)
{
    return nullptr;
}

int CWMFImageHandler::WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo/*=NULL*/)
{
    return CDibInterface::WriteGraphicFile(ptrHandler, path, bitmap, pUserInfo);
}


int CWMFImageHandler::WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht,
                                 UINT32 bpp, UINT32 nColors, RGBQUAD *pPal, void *pUserInfo)
{
    #ifndef _WIN32
    return DTWAIN_ERR_INVALIDBMP;
    #else
    switch(m_nWMFType)
    {
        case 0:
            return WriteWMF(pImage2, wid, ht, bpp, nColors, pPal, pUserInfo);
    }
    return WriteEMF(pImage2, wid, ht, bpp, nColors, pPal, pUserInfo);
    #endif
}

WORD CWMFImageHandler::CalculateAPMCheckSum( APMHEADER apmfh )
{
    LPWORD  lpWord;

    // Start with the first word
    WORD wResult = *(lpWord = (LPWORD)&apmfh);
    // XOR in each of the other 9 words
    for(WORD i = 1;i<=9;i++)
    {
        wResult ^= lpWord[i];
    }
    return wResult;
}

int CWMFImageHandler::WriteEMF(BYTE *pDib, UINT32 width, UINT32 height,
                               UINT32 /*bpp*/, UINT32 /*nColors*/, RGBQUAD * /*pPal*/, void * /*pUserInfo*/)
{
    #ifndef _WIN32
    return DTWAIN_ERR_INVALIDBMP;
    #else
    int error = 0;
    std::vector<BYTE> bitBuf;
    HDC dc = GetDC(nullptr);
    if (dc!= nullptr)
    {
        auto pr = std::make_pair(static_cast<HWND>(nullptr), dc);
        DTWAINDeviceContextRelease_RAII dcRAII(&pr);
        const int iDPIX = GetDeviceCaps(dc, LOGPIXELSX);
        const int iDPIY = GetDeviceCaps(dc, LOGPIXELSY);

        const int iWidthMM = GetDeviceCaps(dc, HORZSIZE);
        const int iHeightMM = GetDeviceCaps(dc, VERTSIZE);
        const int iWidthPels = GetDeviceCaps(dc, HORZRES);
        const int iHeightPels = GetDeviceCaps(dc, VERTRES);

        RECT rect;
        rect.left=0;rect.right=width;
        rect.top=0;rect.bottom=height;

        rect.left = rect.left * iWidthMM * 100 / iWidthPels;
        rect.top = rect.top * iHeightMM * 100/ iHeightPels;
        rect.right = rect.right * iWidthMM * 100/ iWidthPels;
        rect.bottom = rect.bottom * iHeightMM * 100/ iHeightPels;

        const HDC hMetaDC = ::CreateEnhMetaFile(dc, nullptr, &rect, _T(""));
        if (hMetaDC!= nullptr)
        {
            SetMapMode(hMetaDC, MM_ISOTROPIC);
            SetWindowExtEx(hMetaDC, iDPIX, iDPIY, nullptr);
            SetViewportExtEx(hMetaDC, iDPIX, iDPIY, nullptr);

            // Get source bitmap info
            const BITMAPINFO *bmInfo = (LPBITMAPINFO)pDib ;

            // assumes a generic 24bit DIB
            const LPVOID lpDIBBits = static_cast<LPVOID>(pDib + bmInfo->bmiHeader.biSize);

            // lines returns the number of lines actually displayed
            const UINT32 lines = StretchDIBits(hMetaDC,
                                               0,0,
                                               width, height,
                                               0,0,
                                               width, height,
                                               lpDIBBits,
                                               bmInfo,
                                               DIB_RGB_COLORS,
                                               SRCCOPY);

            const HENHMETAFILE hMeta = CloseEnhMetaFile(hMetaDC);
             if (lines==height)
             {
                // How big will the metafile bits be?
                const UINT32 dwSize = GetEnhMetaFileBits( hMeta, 0, nullptr);

                if (dwSize!=0)
                {
                    // Allocate that much memory
                    bitBuf.resize(dwSize);
                    BYTE *pBits = &bitBuf[0];

                    // grab the metafile guts
                    GetEnhMetaFileBits( hMeta, dwSize, pBits);

                    // Write the metafile bits
                    GetOutputFileHandle().write(reinterpret_cast<char*>(pBits), dwSize);
                }
                else
                    error = DTWAIN_ERR_GETWINMETAFILEBITS;
             }
             else
                 error = DTWAIN_ERR_DC;
            DeleteEnhMetaFile(hMeta);
        }
        else
            error = DTWAIN_ERR_DC;
    }
    else
       error = DTWAIN_ERR_DC;

    return error;
    #endif
}


int CWMFImageHandler::WriteWMF(BYTE *pDib, UINT32 width, UINT32 height,
                               UINT32 /*bpp*/, UINT32 /*nColors*/, RGBQUAD * /*pPal*/, void * /*pUserInfo*/)
{
    #ifndef _WIN32
    return DTWAIN_ERR_INVALIDBMP;
    #else
    int error = 0;
    HDC dc = GetDC(nullptr);
    if (dc!= nullptr)
    {
        auto pr = std::make_pair(static_cast<HWND>(nullptr), dc);
        DTWAINDeviceContextRelease_RAII raii(&pr);
        const int iDPIX = GetDeviceCaps(dc, LOGPIXELSX);
        const int iDPIY = GetDeviceCaps(dc, LOGPIXELSY);

        const int iWidthMM = GetDeviceCaps(dc, HORZSIZE);
        const int iHeightMM = GetDeviceCaps(dc, VERTSIZE);
        const int iWidthPels = GetDeviceCaps(dc, HORZRES);
        const int iHeightPels = GetDeviceCaps(dc, VERTRES);

        RECT rect;
        rect.left=0;rect.right = width;
        rect.top=0;rect.bottom = height;

        rect.left = rect.left * iWidthMM * 100 / iWidthPels;
        rect.top = rect.top * iHeightMM * 100 / iHeightPels;
        rect.right = rect.right * iWidthMM * 100 / iWidthPels;
        rect.bottom = rect.bottom * iHeightMM * 100 / iHeightPels;

        const HDC hMetaDC = ::CreateEnhMetaFile(dc, nullptr,  &rect, _T(""));
        if (hMetaDC!= nullptr)
        {

            SetMapMode(hMetaDC, MM_ANISOTROPIC);
            SetWindowExtEx(hMetaDC, iDPIX, iDPIY, nullptr);
            SetViewportExtEx(hMetaDC, iDPIX, iDPIY, nullptr);

            // Get source bitmap info
            const BITMAPINFO *bmInfo = (LPBITMAPINFO)pDib ;

            // assumes a generic 24bit DIB
            const LPVOID lpDIBBits = static_cast<LPVOID>(pDib + bmInfo->bmiHeader.biSize);


            UINT32 lines = SetDIBitsToDevice(hMetaDC,
                                             0, 0,
                                             width, height,
                                             0, 0,
                                             0, height,
                                             lpDIBBits,
                                             bmInfo,
                                             DIB_PAL_COLORS);

            const HENHMETAFILE hMeta = CloseEnhMetaFile(hMetaDC);
            if (lines==height)
            {
                // How big will the metafile bits be?
                const UINT32 dwSize = GetWinMetaFileBits( hMeta, 0, nullptr, MM_ANISOTROPIC, dc  );

                if (dwSize!=0)
                {
                    std::vector<BYTE> bitBuf;
                    // Write the Aldus Placeable Header
                    APMHEADER       APMHeader = {};
                    ENHMETAHEADER   emh = {};

                    emh.nSize = sizeof(ENHMETAHEADER);
                    // Fill in the enhanced metafile header
                    GetEnhMetaFileHeader( hMeta, sizeof( ENHMETAHEADER ), &emh );

                    // Fill in the Aldus Placeable Header
                    APMHeader.dwKey = 0x9ac6cdd7l;
                    APMHeader.hmf = 0;
                    APMHeader.bbox.Top = static_cast<SHORT>(1000 * emh.rclFrame.top / 2540);
                    APMHeader.bbox.Left = static_cast<SHORT>(1000 * emh.rclFrame.left / 2540);
                    APMHeader.bbox.Right = static_cast<SHORT>(1000 * emh.rclFrame.right / 2540);
                    APMHeader.bbox.Bottom = static_cast<SHORT>(1000 * emh.rclFrame.bottom / 2540);
                    APMHeader.wInch = 1000;
                    APMHeader.dwReserved = 0;
                    APMHeader.wCheckSum = CalculateAPMCheckSum( APMHeader );

                    GetOutputFileHandle().write(reinterpret_cast<char *>(&APMHeader), sizeof(APMHEADER));

                    // Allocate that much memory
                    bitBuf.resize(dwSize);
                    BYTE *pBits = &bitBuf[0];
                    if (0) //bitBuf == NULL)
                    {
                        error = DTWAIN_ERR_MEM;
                    }
                    else
                    {

                        // Let windows convert the enhanced metafile to a 16 windows metafile
                        GetWinMetaFileBits( hMeta, dwSize, pBits, MM_ANISOTROPIC, dc );

                        // Write the metafile bits
                        GetOutputFileHandle().write(reinterpret_cast<char *>(pBits), dwSize);
                    }
                }
                else
                    error = DTWAIN_ERR_GETWINMETAFILEBITS;
                DeleteEnhMetaFile(hMeta);
            }
            else
                error = DTWAIN_ERR_DC;
            }
        else
            error = DTWAIN_ERR_DC;
        }
    else
        error = DTWAIN_ERR_DC;
    return error;
    #endif
}
