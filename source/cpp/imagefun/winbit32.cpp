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
#include <math.h>
#include <sstream>
#include "winbit32.h"
#include "ctliface.h"
#include "ctltwmgr.h"
#include "ctlfileutils.h"
#include "FreeImagePlus.h"

#ifdef _MSC_VER
#pragma warning (disable:4244)
#pragma warning (disable:4018)
#pragma warning (disable:4310)
#pragma warning (disable:4309)
#pragma warning (disable:4244)
#endif

static int getNumOnBits(BYTE value);


//#define ISOURCE_INIT_STRING "{98F28E51-C24B-B1B4-9232-0080C8DA7A5E}"
#define GetAValue(rgb)      ((BYTE)((rgb)>>24))

using namespace dynarithmic;

char CDibInterface::masktable[]={static_cast<signed char>(0x80),0x40,0x20,0x10,0x08,0x04,0x02,0x01};
char CDibInterface::bittable[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,static_cast<signed char>(0x80)};
char CDibInterface::bayerPattern[8][8] = {
      { 0,32, 8,40, 2,34,10,42,  },
      { 48,16,56,24,50,18,58,26, },
      { 12,44, 4,36,14,46, 6,38, },
      { 60,28,52,20,62,30,54,22, },
      {  3,35,11,43, 1,33, 9,41, },
      { 51,19,59,27,49,17,57,25, },
      { 15,47, 7,39,13,45, 5,37, },
      { 63,31,55,23,61,29,53,21  },
    };

CDibInterface::CDibInterface() : bytesleft(0), nextbyte(0), bytebuffer{}, m_lasterror(0) {}

int  CDibInterface::WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE handle, void *pUserInfo)
{
    UINT32 wid, ht;
    UINT32 bpp;

    // Get a lock to the DIB
    BYTE *pImage = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(handle));
    DTWAINGlobalHandle_RAII dibHandle(handle);
    if ( !pImage )
        return IS_ERR_INVALIDBMP;

    // Get the BPP
    GetBitsPerPixel(pImage, &bpp);

    // Get Width
    GetWidth(pImage, &wid);

    // Get Height
    GetHeight(pImage, &ht);

    BYTE *pImage2 = pImage;

    // Open a file destination
    OpenOutputFile(path);

    // Check if any error occurred
    LONG err = GetLastError();
    if ( err == IS_ERR_OK)
    {
        const int nUsedColors = CalculateUsedPaletteEntries(bpp);
        const LPBITMAPINFOHEADER bi = (LPBITMAPINFOHEADER)pImage2;
        bi->biClrUsed = nUsedColors;
        StringStreamA strm;
        strm << nUsedColors;
        CTL_TwainAppMgr::WriteLogInfoA("Image has " + strm.str() + " colors\n");

        err = WriteImage(ptrHandler,
                         pImage2,
                         wid,
                         ht,
                         bpp,
                         nUsedColors,
                         GetPalettePtr(pImage2, nUsedColors),
                         pUserInfo);
        const bool goodClose = CloseOutputFile();
        if ( err != 0 || !goodClose)
        {
            if (!goodClose)
                err = DTWAIN_ERR_FILEWRITE;
            err = DTWAIN_ERR_FILEXFERSTART - static_cast<int>(err);
            delete_file(path);
        }
    }
    return err;
}

RGBQUAD* CDibInterface::GetPalettePtr(BYTE *pDibData, int bpp)
{
  if ( pDibData && bpp < 16)
  {
      BYTE *pPalette = (pDibData + sizeof(BITMAPINFOHEADER));
      return (RGBQUAD *)pPalette;
  }
  return nullptr;
}

unsigned char * CDibInterface::GetScanLine(BYTE *pDib, int scanline)
{
    if ( pDib )
    {
        return CalculateScanLine(GetDibBits(pDib), GetPitch(pDib), scanline);
    }
    return nullptr;
}

unsigned CDibInterface::GetLine(BYTE *pDib)
{
    if ( pDib )
    {
        UINT32 width;
        UINT32 bpp;
        GetWidth(pDib, &width);
        GetBitsPerPixel(pDib, &bpp);
        return ((width * bpp) + 7) / 8;
    }
    return 0;
}

unsigned CDibInterface::GetLine(BYTE *pDib, BYTE *pDest, int nWhichLine)
{
    const auto bi = reinterpret_cast<LPBITMAPINFOHEADER>(pDib);
    UINT32 ht;
    GetHeight(pDib, &ht);
    const int nLineWidth = LPBlinewidth(bi);

    memcpy(pDest, GetDibBits(pDib) + nLineWidth * (ht - nWhichLine -1), nLineWidth);
    return nLineWidth;
}

unsigned CDibInterface::GetPitch(BYTE *pDib)
{
    if ( pDib )
        return (GetLine(pDib) + 3) & ~3;
    return 0;
}

unsigned CDibInterface::GetPitch(fipImage& pDib)
{
    return (pDib.getScanWidth() + 3) & ~3;
}

BYTE * CDibInterface::GetDibBits(BYTE *pDib)
{
    if ( pDib )
    {
        const LPBITMAPINFOHEADER pBi = reinterpret_cast<LPBITMAPINFOHEADER>(pDib);
        const int nColors = pBi->biClrUsed;
        BYTE *p = (BYTE *)pBi;
        return p + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * nColors;
    }
    return nullptr;
}

int CDibInterface::GetDibPalette(fipImage& lpbi,LPSTR palette)
{
    const BITMAPINFO* header = lpbi.getInfo();
    const unsigned int j = (std::min)(1 << header->bmiHeader.biBitCount, 256);

    for (unsigned int i = 0; i < j; i++)
    {
        palette[i*RGB_SIZE + RGB_RED] = header->bmiColors[i].rgbRed;
        palette[i*RGB_SIZE + RGB_GREEN] = header->bmiColors[i].rgbGreen;
        palette[i*RGB_SIZE + RGB_BLUE] = header->bmiColors[i].rgbBlue;
    }

    return(j);
}

// Function to ensure that DIB data is on DWORD boundaries
HANDLE CDibInterface::CopyDib(HANDLE hDib)
{
    const HANDLE dibCopy = NormalizeDib(hDib, true);
    if ( dibCopy != hDib)
        return dibCopy;
    return nullptr;
}

// Function to ensure that DIB data is on DWORD boundaries
HANDLE CDibInterface::NormalizeDib(HANDLE hDib, bool bReturnCopy /* = false */ )
{
    HANDLE hNewDib = nullptr;
    if (!hDib)
        return nullptr;
    BYTE *pImage = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hDib));
    {
        DTWAINGlobalHandle_RAII dibHandle(hDib);

        UINT32 width, height, bpp;
        GetWidth(pImage, &width);
        GetHeight(pImage, &height);
        GetBitsPerPixel( pImage, &bpp );

        // Create another DIB based on this DIB's data
        hNewDib = CreateDIB(width, height, bpp, reinterpret_cast<LPSTR>(GetPalettePtr(pImage, bpp)));

        if (!hNewDib)
            return hDib;

        BYTE *pNewImage = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hNewDib));
        DTWAINGlobalHandle_RAII dibHandle2(hNewDib);

        // Copy bits from old to new

        // Compute the stride for the old bitmap
        const LONG OldStride = CalculateLine(width, bpp);

        // This is always DWORD aligned
        const LONG NewStride = CalculateEffWidth(width,bpp);

        // Point to the DIB data
        pImage = GetDibBits(pImage);
        pNewImage = GetDibBits(pNewImage);

        // Loop through the data, copying from old to new
        if (pImage && pNewImage)
        {
            for ( UINT32 i = 0; i < height; ++i)
            {
                memcpy(pNewImage, pImage, OldStride);
                pImage += OldStride;
                pNewImage += NewStride;
            }
        }
    }
    if ( !bReturnCopy )
        ImageMemoryHandler::GlobalFree(hDib);
    return hNewDib;
}

HANDLE CDibInterface::CreateDIB(int width, int height, int bpp, LPSTR palette/*=NULL*/)
{
    height = abs(height);

    int dib_size = sizeof(BITMAPINFOHEADER);
    dib_size += sizeof(RGBQUAD) * CalculateUsedPaletteEntries(bpp);
    dib_size += CalculateEffWidth(width, bpp) * height;

    const HANDLE hDib = ImageMemoryHandler::GlobalAlloc (GHND | GMEM_ZEROINIT, dib_size);

    if (hDib != nullptr)
    {
        const auto bih = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hDib));
        DTWAINGlobalHandle_RAII dibHandle(hDib);

        // write out the BITMAPINFOHEADER
        bih->biSize             = sizeof(BITMAPINFOHEADER);
        bih->biWidth            = width;
        bih->biHeight           = height;
        bih->biPlanes           = 1;
        bih->biCompression      = 0;
        bih->biBitCount         = static_cast<WORD>(bpp);
        bih->biClrUsed          = CalculateUsedPaletteEntries(bpp);
        bih->biClrImportant     = bih->biClrUsed;

        if(palette != nullptr)
        {
            const auto pRgb = reinterpret_cast<RGBQUAD*>(reinterpret_cast<LPSTR>(bih) + static_cast<unsigned>(bih->biSize));

            for(DWORD i=0;i<bih->biClrUsed;++i)
            {
                pRgb[i].rgbRed=static_cast<char>(palette[i * RGB_SIZE + RGB_RED]);
                pRgb[i].rgbGreen=static_cast<char>(palette[i * RGB_SIZE + RGB_GREEN]);
                pRgb[i].rgbBlue=static_cast<char>(palette[i * RGB_SIZE + RGB_BLUE]);
                pRgb[i].rgbReserved=0;
            }
        }
        return hDib;
    }
    return nullptr;
}

HANDLE CDibInterface::NegateDIB(HANDLE hDib)
{
    fipImage fw;
    if ( !fipImageUtility::copyFromHandle(fw, hDib) )
        return nullptr;
    fipWinImage_RAII raii(&fw);
    if ( fw.invert() )
        return nullptr;
    return fipImageUtility::copyToHandle(fw);
}

HANDLE CDibInterface::ResampleDIB(HANDLE hDib, long newx, long newy)
{
    fipImage fw;
    if (!fipImageUtility::copyFromHandle(fw, hDib))
        return nullptr;
    fipWinImage_RAII raii(&fw);
    fw.rescale(newx, newy, FILTER_BSPLINE);
    return fipImageUtility::copyToHandle(fw);
}

HANDLE CDibInterface::ResampleDIB(HANDLE hDib, double xscale, double yscale)
{
    BYTE *pImage = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hDib));
    DTWAINGlobalHandle_RAII dibHandler(hDib);
    UINT32 wid, ht;

    // Get Width
    GetWidth(pImage, &wid);

    // Get Height
    GetHeight(pImage, &ht);

    const long newx = static_cast<long>(xscale * static_cast<double>(wid));
    const long newy = static_cast<long>(yscale * static_cast<double>(ht));
    return ResampleDIB(hDib, newx, newy);
}

HANDLE CDibInterface::IncreaseBpp(HANDLE hDib, long newbpp)
{
    fipImage fw;
    if (!fipImageUtility::copyFromHandle(fw, hDib))
        return nullptr;
    fipWinImage_RAII raii(&fw);
    switch (newbpp)
    {
        case 4:
            fw.convertTo4Bits();
        break;
        case 8:
            fw.convertTo8Bits();
        break;
        case 16:
            fw.convertTo16Bits565();
        break;
        case 24:
            fw.convertTo24Bits();
        break;
        case 32:
            fw.convertTo32Bits();
        break;
        default:
            return nullptr;
    }
    return fipImageUtility::copyToHandle(fw);
}

HANDLE CDibInterface::DecreaseBpp(HANDLE hDib, long newbpp)
{
    return IncreaseBpp(hDib, newbpp);
}

HANDLE CDibInterface::CropDIB(HANDLE handle, const FloatRect& ActualRect, const FloatRect& RequestedRect,int sourceunit,
                              int destunit, int dpi, bool bConvertActual, int& retval)
{
    retval = IS_ERR_OK;

    fipImage fw;
    if (!fipImageUtility::copyFromHandle(fw, handle))
        return nullptr;
    fipWinImage_RAII raii(&fw);

    const UINT32 width = fw.getWidth();
    const UINT32 height = fw.getHeight();

    // Convert the actual rectangle first if necessary
    // This assumes that the actual rect is in pixels, but
    // the source unit does not match up correctly
    FloatRect TempActual = ActualRect;
    if ( bConvertActual )
        TempActual = Normalize(fw, ActualRect, ActualRect, DTWAIN_PIXELS, sourceunit, dpi);

    // Now return a normalized rectangle from the actual and requested rectangles
    const FloatRect NormalizedRect = Normalize(fw, TempActual, RequestedRect, sourceunit, destunit, dpi);

    const double left = NormalizedRect.left;
    const double top = NormalizedRect.top;
    const double right = NormalizedRect.right;
    const double bottom = NormalizedRect.bottom;

    // DIBs are stored upside down, so adjust coordinates here
    const int newbottom = height - static_cast<UINT32>(top);
    const int newtop = height - static_cast<UINT32>(bottom);

    long startx = (std::max)(0L, (std::min)(static_cast<long>(left), static_cast<long>(width)));
    long endx = (std::max)(0L, (std::min)(static_cast<long>(right), static_cast<long>(width)));

    long starty = (std::max)(0L,(std::min)(static_cast<long>(newtop), static_cast<long>(height)));
    long endy =   (std::max)(0L,(std::min)(static_cast<long>(newbottom), static_cast<long>(height)));

    if (startx==endx || starty==endy)
    {
        retval = IS_ERR_BADPARAM;
        return nullptr;
    }

    if (startx>endx)
    {
        const long tmp=startx;
        startx=endx;
        endx=tmp;
    }
    if (starty>endy)
    {
        const long tmp=starty;
        starty=endy;
        endy=tmp;
    }

    if ( fw.crop(startx, starty, endx, endy) )
        return fipImageUtility::copyToHandle(fw);
    return nullptr;
}

FloatRect CDibInterface::Normalize(fipImage& pImage, const FloatRect& ActualRect, const FloatRect& RequestedRect,
                                   int sourceunit, int destunit, int dpi)
{
    const std::map<LONG, double> Measurement = {{DTWAIN_INCHES, 1.0},
                                                  {DTWAIN_TWIPS, 1440.0},
                                                  {DTWAIN_POINTS, 72.0},
                                                  {DTWAIN_PICAS, 6.0},
                                                  {DTWAIN_CENTIMETERS, 2.54}};

    const UINT32 width = pImage.getWidth();

    // Set up a return rect
    FloatRect fRect = RequestedRect;

    // Check dimensions
    if (fabs(ActualRect.right - ActualRect.left) < 1.0 )
        return fRect;

    const UINT32 pitch = GetPitch(pImage);
    if ( pitch == 0 )
        return fRect;

    const auto iterSourceUnit = Measurement.find(sourceunit);
    const auto iterDestUnit = Measurement.find(destunit);

    // Convert Actual rect to pixels
    double PixelsPerInch = dpi;
    switch(sourceunit)
    {
        case DTWAIN_PIXELS:
            break;

        case DTWAIN_INCHES:
        case DTWAIN_TWIPS:
        case DTWAIN_CENTIMETERS:
        case DTWAIN_POINTS:
        case DTWAIN_PICAS:
        {
            const double NumInches = (ActualRect.right - ActualRect.left) / iterSourceUnit->second;
            PixelsPerInch = static_cast<double>(width) / NumInches;
        }
        break;
    }

    switch(destunit)
    {
        case DTWAIN_PIXELS:
            break;

        case DTWAIN_INCHES:
        case DTWAIN_TWIPS:
        case DTWAIN_CENTIMETERS:
        case DTWAIN_POINTS:
        case DTWAIN_PICAS:
        {
            if ( sourceunit == DTWAIN_PIXELS )
            {
                fRect.left   = RequestedRect.left / PixelsPerInch;
                fRect.right  = RequestedRect.right  /  PixelsPerInch;
                fRect.top    = RequestedRect.top   /  PixelsPerInch;
                fRect.bottom = RequestedRect.bottom  / PixelsPerInch;
            }
            else
            {
                fRect.left   = (RequestedRect.left / iterDestUnit->second) * PixelsPerInch;
                fRect.right  = (RequestedRect.right  / iterSourceUnit->second) * PixelsPerInch;
                fRect.top    = (RequestedRect.top   / iterSourceUnit->second)  * PixelsPerInch;
                fRect.bottom = (RequestedRect.bottom  / iterSourceUnit->second) * PixelsPerInch;
            }
        }
        break;

    }
    return fRect;
}

int getNumOnBits(BYTE value)
{
    return (value & 0x01) +
           ((value & 0x02) >> 1) +
           ((value & 0x04) >> 2) +
           ((value & 0x08) >> 3) +
           ((value & 0x10) >> 4) +
           ((value & 0x20) >> 5) +
           ((value & 0x40) >> 6) +
           ((value & 0x80) >> 7);
}


// Test for blank page here
bool CDibInterface::IsBlankDIB(HANDLE hDib, double threshold)
{
    if ( !hDib )
        return false;

    CTL_TwainDib dibHandler(hDib);
    dibHandler.SetAutoDelete(false);
    const auto bpp = dibHandler.GetBitsPerPixel();
    switch (bpp)
    {
        case 1:
        {
            // Just count up the white pixels and black pixels
            auto bitsOffsetVal = dibHandler.GetBitsOffset();
            if (!bitsOffsetVal)
                return false;
            DWORD bitsOffset = *bitsOffsetVal;
            BYTE* pDib = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hDib));
            DTWAINGlobalHandle_RAII hDibHandler(hDib);
            const DWORD totalSize = ImageMemoryHandler::GlobalSize(hDib);
            uint64_t numBlack = 0;
            uint64_t numWhite = 0;
            while (bitsOffset < totalSize)
            {
                const auto countWhite = std::bitset<8>(*(pDib + bitsOffset)).count();
                numWhite += countWhite;
                numBlack += 8 - countWhite;
                ++bitsOffset;
            }
            const double pctwhite = static_cast<double>(numWhite) / static_cast<double>(numWhite + numBlack);
            return pctwhite >= threshold;
        }
        case 8:
        {
            // 8 BPP must use a palette
            auto bitsOffsetVal = dibHandler.GetBitsOffset();
            if (!bitsOffsetVal)
                return false;
            DWORD bitsOffset = *bitsOffsetVal;
            const auto numColors = dibHandler.GetNumColors();

            // Build a histogram of the colors from palette that have been detected
            std::vector<unsigned int> histogram(numColors);
            BYTE* pDib = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hDib));
            DTWAINGlobalHandle_RAII hDibHandler(hDib);
            const DWORD totalSize = ImageMemoryHandler::GlobalSize(hDib);
            DWORD componentTotal = 0;

            // tally up the count for each color found
            while (bitsOffset < totalSize)
            {
                if ( *(pDib + bitsOffset) < numColors )
                    ++histogram[*(pDib + bitsOffset)];
                ++bitsOffset;
                ++componentTotal;
            }

            // Search for first color that goes over the blank threshold
            const auto iter = std::find_if(histogram.begin(), histogram.end(), [&](unsigned int n)
                                                              { return static_cast<double>(n) / static_cast<double>(componentTotal) >= threshold;});
            return iter != histogram.end();
        }
        default:
            // higher bit counts get sent out to the helper function
            return IsBlankDIBHelper(hDib, threshold);
    }
    return false;
}

bool CDibInterface::IsBlankDIBHelper(HANDLE hDib, double threshold)
{
    fipImage fw;
    if (!fipImageUtility::copyFromHandle(fw, hDib))
        return NULL;
    fipWinImage_RAII raii(&fw);

    if (hDib)
    {
        // do a simple test here
        const UINT32 width = fw.getWidth();
        const UINT32 height = fw.getHeight();

        // loop for all pixels
        RGBQUAD rgb;
        UINT32 numwhite = 0;
        UINT32 totalcomponents = 0;
        for (UINT32 row = 0; row < height; ++row)
        {
            for (UINT32 col = 0; col < width; ++col)
            {
                fw.getPixelColor(col, row, &rgb);
                numwhite += getNumOnBits(rgb.rgbRed) + getNumOnBits(rgb.rgbGreen) + getNumOnBits(rgb.rgbBlue);
                totalcomponents += 24;
            }
        }

        const double pctwhite = static_cast<double>(numwhite) / static_cast<double>(totalcomponents);
        return pctwhite >= threshold;
    }
    return false;
}

LPSTR CDibInterface::GetMonoPalette(LPSTR palette)
{
    static char pal[]="\000\000\000\377\377\377";
    if(palette != nullptr)
        memcpy(palette,pal,6);
    return static_cast<LPSTR>(pal);
}

////////////////////////////////////////////////////////////////////////

bool CDibInterface::GetWidth(BYTE *pDIB, UINT32 *puWidth)
{
    if (pDIB== nullptr)
    {
        *puWidth = 0;
        return false;
    }

    if (((BITMAPINFOHEADER *)pDIB)->biSize!=sizeof(BITMAPINFOHEADER))
    {
        *puWidth = 0;
        return false;
    }

    *puWidth = ((BITMAPINFOHEADER *)pDIB)->biWidth;
    return true;
}

////////////////////////////////////////////////////////////////////////

bool CDibInterface::GetHeight(BYTE *pDIB, UINT32 *piHeight)
{
    if (pDIB== nullptr)
    {
        *piHeight = 0;
        return false;
    }

    if (((BITMAPINFOHEADER *)pDIB)->biSize!=sizeof(BITMAPINFOHEADER))
    {
        *piHeight = 0;
        return false;
    }

    *piHeight = ((BITMAPINFOHEADER *)pDIB)->biHeight;
    return true;
}

////////////////////////////////////////////////////////////////////////

bool CDibInterface::GetBitsPerPixel(BYTE *pDIB, UINT32 *puBitCount)
{
    if (pDIB== nullptr)
    {
        *puBitCount = 0;
        return false;
    }

    if (((BITMAPINFOHEADER *)pDIB)->biSize!=sizeof(BITMAPINFOHEADER))
    {
        *puBitCount = 0;
        return false;
    }

    *puBitCount = ((BITMAPINFOHEADER *)pDIB)->biBitCount;
    return true;
}

bool CDibInterface::OpenOutputFile(LPCTSTR pFileName)
{
    if (!pFileName)
    {
        SetError(DTWAIN_ERR_FILEOPEN);
        return false;
    }

    if (pFileName[0] == 0)
    {
        SetError(DTWAIN_ERR_FILEOPEN);
        return false;
    }

    const auto result = m_fStream.OpenOutputFile(pFileName);
    if (!result)
    {
        SetError(DTWAIN_ERR_FILEOPEN);
        return false;
    }
    SetError(0);
    return true;
}

bool CDibInterface::CloseOutputFile()
{
    const auto retVal = m_fStream.CloseOutputFile();
    if ( retVal )
    {
        SetError(0);
        return true;
    }
    SetError(DTWAIN_ERR_FILEWRITE);
    return false;
}

bool CDibInterface::IsGrayScale(BYTE *pImage, int bpp)
{
#ifdef _WIN32
    const LPBITMAPINFOHEADER pHeader = (LPBITMAPINFOHEADER)pImage;
    RGBQUAD* ppal=GetPalettePtr(pImage, bpp);
    if(!ppal || pHeader->biClrUsed == 0 )
        return false;

    for(DWORD i=0; i<pHeader->biClrUsed;i++)
    {
        if (ppal[i].rgbBlue!=i || ppal[i].rgbGreen!=i || ppal[i].rgbRed!=i)
            return false;
    }
    return true;
#else
    return false;
#endif
}

bool CDibInterface::IsGrayScale(HANDLE hDib, int bpp)
{
#ifdef _WIN32
    bool bRetval = false;
    BYTE *pImageTemp = static_cast<BYTE*>(GlobalLock(hDib));
    DTWAINGlobalHandle_RAII dibHandle(hDib);
    if ( pImageTemp )
        bRetval = IsGrayScale(pImageTemp, bpp);
    return bRetval;
#else
    fipImage fw;
    if (fipImageUtility::copyFromHandle(fw, hDib))
        return false;
    fipWinImage_RAII raii(&fw);
    return fw.isGrayscale();
#endif
}

int CDibInterface::putbufferedbyte(WORD byte, std::ofstream& fh, bool bRealEOF, int *pStatus/*=NULL*/)
{
    if ( pStatus )
        *pStatus = 0;
    if(byte==static_cast<WORD>(EOF) && bRealEOF)
    {
        fh.write(reinterpret_cast<char*>(bytebuffer), bytesleft);
        bytesleft=0;
        return(byte);
    }
    else
    {
        if(bytesleft >= BYTEBUFFERSIZE)
        {
            fh.write(reinterpret_cast<char*>(bytebuffer), bytesleft);
            bytesleft=0;
        }
        bytebuffer[bytesleft++]=static_cast<char>(byte);
        return(byte);
    }
}

int CDibInterface::putbyte(WORD byte, std::ofstream& fh)
{
    fh.write(reinterpret_cast<char*>(&byte), 1);
    return(byte);
}

double CDibInterface::GetScaleFactorPerInch(LONG Unit)
{
    switch( Unit )
    {
        case DTWAIN_TWIPS:
            return 1440.0;
        case DTWAIN_POINTS:
            return 72.0;
        case DTWAIN_PICAS:
            return 6.0;
        case DTWAIN_CENTIMETERS:
            return 2.54;
    }
    return 1.0;
}

HBITMAP CDibInterface::DIBToBitmap(HANDLE hDIB, HPALETTE hPal)
{
    HPALETTE hOldPal = nullptr;

    if (!hDIB)
        return nullptr;

    BYTE* lpDIBHdr = static_cast<BYTE*>(GlobalLock(hDIB));

    DTWAINGlobalHandle_RAII dibHandle(hDIB);
    const BYTE* lpDIBBits = CDibInterface::GetDibBits(lpDIBHdr);
    HDC hDC = GetDC(nullptr);

    if (!hDC)
        return nullptr;

    DTWAINDeviceContextRelease_RAII dcHandle({nullptr, hDC });

    if (hPal)
        hOldPal = SelectPalette(hDC, hPal, FALSE);

    RealizePalette(hDC);

    const HBITMAP hBitmap = CreateDIBitmap(hDC,
                                           reinterpret_cast<LPBITMAPINFOHEADER>(lpDIBHdr),
                                           CBM_INIT,
                                           lpDIBBits,
                                           reinterpret_cast<LPBITMAPINFO>(lpDIBHdr),
                                           DIB_RGB_COLORS);

    if (!hBitmap)
        return nullptr;

    if (hOldPal)
        SelectPalette(hDC, hOldPal, FALSE);

    return hBitmap;
}
