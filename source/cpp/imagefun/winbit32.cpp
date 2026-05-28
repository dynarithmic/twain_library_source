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
#include <cmath>
#include "winbit32.h"
#include "ctliface.h"
#include "ctlconstexprfind.h"
#include "resample24to8.h"
#include "dibutil.h"

#ifdef _MSC_VER
#pragma warning (disable:4244)
#pragma warning (disable:4018)
#pragma warning (disable:4310)
#pragma warning (disable:4309)
#pragma warning (disable:4244)
#endif

#define GetAValue(rgb)      ((BYTE)((rgb)>>24))

using namespace dynarithmic;


// Lower level routines
static FloatRect Normalize(const dynarithmic::dib::LockedDib& hDib, const FloatRect& ActualRect, const FloatRect& RequestedRect,
						   int sourceunit, int destunit, int dpi)
{
	static constexpr std::array<std::pair<LONG, double>, 5> Measurement = { {{DTWAIN_INCHES, 1.0},
																	  {DTWAIN_TWIPS, 1440.0},
																	  {DTWAIN_POINTS, 72.0},
																	  {DTWAIN_PICAS, 6.0},
																	  {DTWAIN_CENTIMETERS, 2.54}} };
	const UINT32 width = hDib.Width();

	// Set up a return rect
	FloatRect fRect = RequestedRect;

	// Check dimensions
	if (fabs(ActualRect.right - ActualRect.left) < 1.0)
		return fRect;

    const UINT32 pitch = hDib.Pitch(); 
	if (pitch == 0)
		return fRect;

	const auto iterSourceUnit = generic_array_finder_if(Measurement, [&](auto& pr) { return pr.first == sourceunit; });
	const auto iterDestUnit = generic_array_finder_if(Measurement, [&](auto& pr) { return pr.first == destunit; });

	// If not found return the original rect
	if (!iterSourceUnit.first || !iterDestUnit.first)
		return fRect;

	auto actualSourceUnit = Measurement[iterSourceUnit.second].second;
	auto actualDestUnit = Measurement[iterDestUnit.second].second;

	// Convert Actual rect to pixels
	double PixelsPerInch = dpi;
	switch (sourceunit)
	{
	    case DTWAIN_PIXELS:
		    break;

	    case DTWAIN_INCHES:
	    case DTWAIN_TWIPS:
	    case DTWAIN_CENTIMETERS:
	    case DTWAIN_POINTS:
	    case DTWAIN_PICAS:
	    {
		    const double NumInches = (ActualRect.right - ActualRect.left) / actualSourceUnit;
		    PixelsPerInch = static_cast<double>(width) / NumInches;
	    }
	    break;
	}

	switch (destunit)
	{
	    case DTWAIN_PIXELS:
		    break;

	    case DTWAIN_INCHES:
	    case DTWAIN_TWIPS:
	    case DTWAIN_CENTIMETERS:
	    case DTWAIN_POINTS:
	    case DTWAIN_PICAS:
	    {
		    if (sourceunit == DTWAIN_PIXELS)
		    {
			    fRect.left = RequestedRect.left / PixelsPerInch;
			    fRect.right = RequestedRect.right / PixelsPerInch;
			    fRect.top = RequestedRect.top / PixelsPerInch;
			    fRect.bottom = RequestedRect.bottom / PixelsPerInch;
		    }
		    else
		    {
			    fRect.left = RequestedRect.left / actualDestUnit * PixelsPerInch;
			    fRect.right = RequestedRect.right / actualDestUnit * PixelsPerInch;
			    fRect.top = RequestedRect.top / actualDestUnit * PixelsPerInch;
			    fRect.bottom = RequestedRect.bottom / actualDestUnit * PixelsPerInch;
		    }
	    }
	    break;
	}
	return fRect;

}

CDibInterface::CDibInterface() : m_lasterror(0) {}

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
    {
        dynarithmic::dib::LockedDib dibHandle(hDib);

        BYTE* pImage = const_cast<BYTE*>(dibHandle.HeaderAsBytePtr());
        UINT32 width, height, bpp;
        width = dibHandle.Width();
        height = dibHandle.Height();
        bpp = dibHandle.BitsPerPixel();
        auto palettePtr = dibHandle.PalettePtrMutable();
        // Create another DIB based on this DIB's data
        hNewDib = CreateDIB(width, height, bpp, reinterpret_cast<LPSTR>(palettePtr));
        if (!hNewDib)
            return hDib;

        dynarithmic::dib::LockedDib dibHandle2(hNewDib);

        // Compute the stride for the old bitmap
        const LONG OldStride = dibHandle.StrideBytes();

        // This is always DWORD aligned
        const LONG NewStride = dibHandle.EffectiveWidth();

        // Point to the DIB data
        BYTE* pNewImage = const_cast<BYTE*>(dibHandle2.HeaderAsBytePtr());

        // Loop through the data, copying from old to new
        if (pImage && pNewImage)
        {
            for (UINT32 i = 0; i < height; ++i)
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
    dib_size += sizeof(RGBQUAD) * dynarithmic::dib::effective_palette_entries(bpp);
    dib_size += dynarithmic::dib::effective_width(width, bpp) * height;

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
        bih->biClrUsed          = dynarithmic::dib::effective_palette_entries(bpp);
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
    return dynarithmic::dib::NegateDib(hDib);
}

HANDLE CDibInterface::ResampleDIB(HANDLE hDib, long newx, long newy)
{
    return dynarithmic::dib::ResizeDib(hDib, newx, newy);
}

HANDLE CDibInterface::ResampleDIB(HANDLE hDib, double xscale, double yscale)
{
    dynarithmic::dib::LockedDib dibHandle(hDib);
    UINT32 wid, ht;

    // Get Width
    wid = dibHandle.Width();

    // Get Height
    ht = dibHandle.Height();

    const long newx = static_cast<long>(xscale * static_cast<double>(wid));
    const long newy = static_cast<long>(yscale * static_cast<double>(ht));
    return ResampleDIB(hDib, newx, newy);
}

HANDLE CDibInterface::IncreaseDecreaseBpp(HANDLE hDib, long newbpp, bool bIncrease)
{
    dynarithmic::dib::LockedDib dibHandle(hDib);
    uint32_t bpp = dibHandle.BitsPerPixel();

    // RGB to Gray (24 -> 8) special routine
    if (bpp == 24 && newbpp == 8)
    {
        HANDLE newDib = Convert24bppDibTo8bppGray(hDib);
        return newDib;
    }

    if (bIncrease)
        return dynarithmic::dib::IncreaseDibBpp(hDib, newbpp);
    return dynarithmic::dib::DecreaseDibBpp(hDib, newbpp);
}

HANDLE CDibInterface::RotateDIB(HANDLE hDib, float angle)
{
    if (!hDib)
        return nullptr;
    return dynarithmic::dib::Rotate(hDib, angle);
}

HANDLE CDibInterface::IncreaseBpp(HANDLE hDib, long newbpp)
{
    return IncreaseDecreaseBpp(hDib, newbpp, true);
}

HANDLE CDibInterface::DecreaseBpp(HANDLE hDib, long newbpp)
{
    return IncreaseDecreaseBpp(hDib, newbpp, false);
}

HANDLE CDibInterface::CropDIB(HANDLE handle, const FloatRect& ActualRect, const FloatRect& RequestedRect, int sourceunit,
                              int destunit, int dpi, bool bConvertActual, int& retval)
{
	retval = IS_ERR_OK;

    dynarithmic::dib::LockedDib dibHandle(handle);

    const UINT32 width = dibHandle.Width();
	const UINT32 height = dibHandle.Height();

	// Convert the actual rectangle first if necessary
	// This assumes that the actual rect is in pixels, but
	// the source unit does not match up correctly
	FloatRect TempActual = ActualRect;
	if (bConvertActual)
		TempActual = Normalize(dibHandle, ActualRect, ActualRect, DTWAIN_PIXELS, sourceunit, dpi);

	// Now return a normalized rectangle from the actual and requested rectangles
	const FloatRect NormalizedRect = Normalize(dibHandle, TempActual, RequestedRect, sourceunit, destunit, dpi);

	const double left = NormalizedRect.left;
	const double top = NormalizedRect.top;
	const double right = NormalizedRect.right;
	const double bottom = NormalizedRect.bottom;

	// DIBs are stored upside down, so adjust coordinates here
	const int newbottom = height - static_cast<UINT32>(top);
	const int newtop = height - static_cast<UINT32>(bottom);

	long startx = (std::max)(0L, (std::min<long>)(left, width));
	long endx = (std::max)(0L, (std::min<long>)(right, width));

	long starty = (std::max)(0L, (std::min<long>)(newtop, height));
	long endy = (std::max)(0L, (std::min<long>)(newbottom, height));

	if (startx == endx || starty == endy)
	{
		retval = IS_ERR_BADPARAM;
		return nullptr;
	}

	if (startx > endx)
	{
		const long tmp = startx;
		startx = endx;
		endx = tmp;
	}
	if (starty > endy)
	{
		const long tmp = starty;
		starty = endy;
		endy = tmp;
	}

    return dynarithmic::dib::CropDib(handle, startx, starty, endx, endy);
}

// Test for blank page here
bool CDibInterface::IsBlankDIB(HANDLE hDib, double threshold)
{
    return IsBlankDIBEx(hDib, threshold / 100.0).m_bIsBlank;
}

////////////////////////////////////////////////////////////////////////
bool CDibInterface::OpenOutputFile(LPCTSTR pFileName)
{
    return true;
}

bool CDibInterface::CloseOutputFile()
{
    return true;
}

bool CDibInterface::IsGrayScale(HANDLE hDib, int bpp)
{
    dynarithmic::dib::LockedDib dibHandle(hDib);
    return dynarithmic::dib::is_grayscale_palette(dibHandle.Palette(), dibHandle.PaletteEntries());
}

HBITMAP CDibInterface::DIBToBitmap(HANDLE hDIB, HPALETTE hPal)
{
    HPALETTE hOldPal = nullptr;

    if (!hDIB)
        return nullptr;
    dynarithmic::dib::LockedDib dibHandle(hDIB);
    const BYTE* lpDIBBits = dibHandle.Bits();
    HDC hDC = GetDC(nullptr);

    if (!hDC)
        return nullptr;
    auto pr = std::make_pair(static_cast<HWND>(nullptr), hDC);
    DTWAINDeviceContextRelease_RAII dcHandle(&pr);

    if (hPal)
        hOldPal = SelectPalette(hDC, hPal, FALSE);

    RealizePalette(hDC);

    const HBITMAP hBitmap = CreateDIBitmap(hDC,
                                           dibHandle.Header(),
                                           CBM_INIT,
                                           lpDIBBits,
                                           reinterpret_cast<LPBITMAPINFO>(dibHandle.HeaderMutable()), 
                                           DIB_RGB_COLORS);

    if (!hBitmap)
        return nullptr;

    if (hOldPal)
        SelectPalette(hDC, hOldPal, FALSE);

    return hBitmap;
}
