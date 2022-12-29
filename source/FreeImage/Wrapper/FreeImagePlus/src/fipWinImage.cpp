// ==========================================================
// fipWinImage class implementation
//
// Design and implementation by
// - Herv� Drolon (drolon@infonie.fr)
//
// This file is part of FreeImage 3
//
// COVERED CODE IS PROVIDED UNDER THIS LICENSE ON AN "AS IS" BASIS, WITHOUT WARRANTY
// OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, WITHOUT LIMITATION, WARRANTIES
// THAT THE COVERED CODE IS FREE OF DEFECTS, MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE
// OR NON-INFRINGING. THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE COVERED
// CODE IS WITH YOU. SHOULD ANY COVERED CODE PROVE DEFECTIVE IN ANY RESPECT, YOU (NOT
// THE INITIAL DEVELOPER OR ANY OTHER CONTRIBUTOR) ASSUME THE COST OF ANY NECESSARY
// SERVICING, REPAIR OR CORRECTION. THIS DISCLAIMER OF WARRANTY CONSTITUTES AN ESSENTIAL
// PART OF THIS LICENSE. NO USE OF ANY COVERED CODE IS AUTHORIZED HEREUNDER EXCEPT UNDER
// THIS DISCLAIMER.
//
// Use at your own risk!
// ==========================================================

#include "FreeImagePlus.h"
#include <cstring>
#ifndef _WIN32
#define BI_RGB       0L
#define BI_BITFIELDS 3L
#endif

// marker used for clipboard copy / paste

static inline void
SET_FREEIMAGE_MARKER(BITMAPINFOHEADER *bmih, FIBITMAP *dib) {
    // Windows constants goes from 0L to 5L
    // Add 0xFF to avoid conflicts
    bmih->biCompression = 0xFF + FreeImage_GetImageType(dib);
}

static inline FREE_IMAGE_TYPE
GET_FREEIMAGE_MARKER(BITMAPINFOHEADER *bmih) {
    return static_cast<FREE_IMAGE_TYPE>(bmih->biCompression - 0xFF);
}

BOOL fipImageUtility::copyFromHandle(fipImage& im, HANDLE hMem, bool isHandle)
{
    if (!hMem)
    {
        return FALSE;
    }

    BYTE *lpVoid = nullptr;
    // Get a pointer to the bitmap
#ifdef _WIN32
    struct raii_
    {
        HANDLE h;
        raii_(HANDLE h_ = nullptr) : h(h_) {}
        ~raii_() { if (h) GlobalUnlock(h); }
    };

    if (isHandle)
        lpVoid = static_cast<BYTE*>(GlobalLock(hMem));
    else
        lpVoid = static_cast<BYTE*>(hMem);
    raii_ r(isHandle ? hMem : nullptr);
#else
    lpVoid = (BYTE*)hMem;
#endif

    const RGBQUAD *pPalette = nullptr;
    DWORD bitfields[3] = { 0, 0, 0 };

    // Get a pointer to the bitmap header
    BITMAPINFOHEADER* pHead = (BITMAPINFOHEADER*)lpVoid;

    // Get a pointer to the palette
    if (pHead->biBitCount < 16) {
        pPalette = (RGBQUAD *)((BYTE *)pHead + sizeof(BITMAPINFOHEADER));
    }

    // Get a pointer to the pixels
    BYTE* bits = (BYTE*)pHead + sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * pHead->biClrUsed;

    if (pHead->biCompression == BI_BITFIELDS) {
        // Take into account the color masks that specify the red, green, and blue components (16- and 32-bit)
        const unsigned mask_size = 3 * sizeof(DWORD);
        memcpy(&bitfields[0], bits, mask_size);
        bits += mask_size;
    }

    if (lpVoid) {

        // Allocate a new FIBITMAP

        FREE_IMAGE_TYPE image_type = FIT_BITMAP;
        // Use a hack to decide if the clipboard contains non standard bitmaps ...
        switch (GET_FREEIMAGE_MARKER(pHead)) {
        case FIT_UINT16:
        case FIT_INT16:
        case FIT_UINT32:
        case FIT_INT32:
        case FIT_FLOAT:
        case FIT_DOUBLE:
        case FIT_COMPLEX:
        case FIT_RGB16:
        case FIT_RGBA16:
        case FIT_RGBF:
        case FIT_RGBAF:
            image_type = GET_FREEIMAGE_MARKER(pHead);
            break;
        }
        if (!im.setSize(image_type, static_cast<WORD>(pHead->biWidth), static_cast<WORD>(pHead->biHeight), pHead->biBitCount, bitfields[2], bitfields[1], bitfields[0])) {
            return FALSE;
        }
        // Copy the palette
        if ( pPalette)
            memcpy(FreeImage_GetPalette(im), pPalette, pHead->biClrUsed * sizeof(RGBQUAD));
        // Copy the bitmap
        memcpy(FreeImage_GetBits(im), bits, FreeImage_GetPitch(im) * FreeImage_GetHeight(im));
        return TRUE;
    }
    return FALSE;
};

HANDLE fipImageUtility::copyToHandle(const fipImage& im)
{
    HANDLE hMem = nullptr;
    if (im)
    {
        // Get equivalent DIB size
        long dib_size = sizeof(BITMAPINFOHEADER);
        dib_size += FreeImage_GetColorsUsed(im) * sizeof(RGBQUAD);
        dib_size += FreeImage_GetPitch(im) * FreeImage_GetHeight(im);

        // Allocate a DIB
#ifdef _WIN32
        hMem = GlobalAlloc(GHND, dib_size);
        BYTE *dib = static_cast<BYTE*>(GlobalLock(hMem));
#else
        BYTE *dib = new BYTE[dib_size];
#endif

        memset(dib, 0, dib_size);
        BYTE *p_dib = static_cast<BYTE*>(dib);

        // Copy the BITMAPINFOHEADER
        const BITMAPINFOHEADER *bih = FreeImage_GetInfoHeader(im);
        memcpy(p_dib, bih, sizeof(BITMAPINFOHEADER));
        if (FreeImage_GetImageType(im) != FIT_BITMAP) {
            // this hack is used to store the bitmap type in the biCompression member of the BITMAPINFOHEADER
            SET_FREEIMAGE_MARKER((BITMAPINFOHEADER*)p_dib, im);
        }
        p_dib += sizeof(BITMAPINFOHEADER);

        // Copy the palette
        const RGBQUAD *pal = FreeImage_GetPalette(im);
        memcpy(p_dib, pal, FreeImage_GetColorsUsed(im) * sizeof(RGBQUAD));
        p_dib += FreeImage_GetColorsUsed(im) * sizeof(RGBQUAD);

        // Copy the bitmap
        const BYTE *bits = FreeImage_GetBits(im);
        memcpy(p_dib, bits, FreeImage_GetPitch(im) * FreeImage_GetHeight(im));
#ifdef _WIN32
        GlobalUnlock(hMem);
#endif
    }
    return hMem;
}

#ifdef _WIN32

///////////////////////////////////////////////////////////////////
// Construction / Destruction

fipWinImage::fipWinImage(FREE_IMAGE_TYPE image_type, unsigned width, unsigned height, unsigned bpp) : fipImage(image_type, width, height, bpp) {
    _display_dib = nullptr;
    _bDeleteMe = FALSE;
    // default tone mapping operator
    _tmo = FITMO_DRAGO03;
    _tmo_param_1 = 0;
    _tmo_param_2 = 0;
    _tmo_param_3 = 1;
    _tmo_param_4 = 0;
}

fipWinImage::~fipWinImage() {
    if(_bDeleteMe) {
        fipImage::clear();
    }
}

void fipWinImage::clear() {
    // delete _display_dib
    if(_bDeleteMe) {
        fipImage::clear();
    }
    _display_dib = nullptr;
    _bDeleteMe = FALSE;
    // delete base class data
    fipImage::clear();
}

BOOL fipWinImage::isValid() const {
    return fipImage::isValid();
}

///////////////////////////////////////////////////////////////////
// Copying

fipWinImage& fipWinImage::operator=(const fipImage& Image) {
    // delete _display_dib
    if(_bDeleteMe) {
        FreeImage_Unload(_display_dib);
    }
    _display_dib = nullptr;
    _bDeleteMe = FALSE;
    // clone the base class
    fipImage::operator=(Image);

    return *this;
}

fipWinImage& fipWinImage::operator=(const fipWinImage& Image) {
    if(this != &Image) {
        // delete _display_dib
        if(_bDeleteMe) {
            FreeImage_Unload(_display_dib);
        }
        _display_dib = nullptr;
        _bDeleteMe = FALSE;
        // copy tmo data
        _tmo = Image._tmo;
        _tmo_param_1 = Image._tmo_param_1;
        _tmo_param_2 = Image._tmo_param_2;
        _tmo_param_3 = Image._tmo_param_3;
        _tmo_param_4 = Image._tmo_param_4;

        // clone the base class
        fipImage::operator=(Image);
    }
    return *this;
}

HANDLE fipWinImage::copyToHandle() const
{
    return fipImageUtility::copyToHandle(*this);
}

BOOL fipWinImage::copyFromHandle(HANDLE hMem, bool isHandle)
{
    return fipImageUtility::copyFromHandle(*this, hMem, isHandle);
}

BOOL fipWinImage::copyFromBitmap(HBITMAP hbmp) {
    if(hbmp) {
        BITMAP bm;
        // Get informations about the bitmap
        GetObject(hbmp, sizeof(BITMAP), (LPSTR) &bm);
        // Create the image
        setSize(FIT_BITMAP, static_cast<WORD>(bm.bmWidth), static_cast<WORD>(bm.bmHeight), static_cast<WORD>(bm.bmBitsPixel));

        // The GetDIBits function clears the biClrUsed and biClrImportant BITMAPINFO members (dont't know why)
        // So we save these infos below. This is needed for palettized images only.
        const int nColors = FreeImage_GetColorsUsed(_dib.get());

        // Create a device context for the bitmap
        const HDC dc = GetDC(nullptr);
        // Copy the pixels
        int Success = GetDIBits(dc, // handle to DC
                                hbmp, // handle to bitmap
                                0, // first scan line to set
                                FreeImage_GetHeight(_dib.get()), // number of scan lines to copy
                                FreeImage_GetBits(_dib.get()), // array for bitmap bits
                                FreeImage_GetInfo(_dib.get()), // bitmap data buffer
                                DIB_RGB_COLORS // RGB
        );
        if(Success == 0) {
            FreeImage_OutputMessageProc(FIF_UNKNOWN, "Error : GetDIBits failed");
            ReleaseDC(nullptr, dc);
            return FALSE;
        }
        ReleaseDC(nullptr, dc);

        // restore BITMAPINFO members

        FreeImage_GetInfoHeader(_dib.get())->biClrUsed = nColors;
        FreeImage_GetInfoHeader(_dib.get())->biClrImportant = nColors;

        return TRUE;
    }

    return FALSE;
}

BOOL fipWinImage::copyToClipboard(HWND hWndNewOwner) const {
    const HANDLE hDIB = copyToHandle();

    if(OpenClipboard(hWndNewOwner)) {
        if(EmptyClipboard()) {
            if(SetClipboardData(CF_DIB, hDIB) == nullptr) {
//              MessageBox(hWndNewOwner, "Unable to set Clipboard data", "FreeImage", MB_ICONERROR);
                CloseClipboard();
                return FALSE;
            }
        }
    }
    CloseClipboard();

    return TRUE;
}

BOOL fipWinImage::pasteFromClipboard() {
    if(!IsClipboardFormatAvailable(CF_DIB)) {
        return FALSE;
    }

    if(OpenClipboard(nullptr)) {
        BOOL bResult = FALSE;
        const HANDLE hDIB = GetClipboardData(CF_DIB);
        if(hDIB) {
            bResult = copyFromHandle(hDIB);
        }
        CloseClipboard();
        return bResult;
    }
    CloseClipboard();

    return FALSE;
}

///////////////////////////////////////////////////////////////////
// Screen capture

BOOL fipWinImage::captureWindow(HWND hWndApplicationWindow, HWND hWndSelectedWindow) {
    RECT r;

    // Get window size
    GetWindowRect(hWndSelectedWindow, &r);

    // Check if the window is out of the screen or maximixed
    int xshift = 0;
    int yshift = 0;
    int xScreen = GetSystemMetrics(SM_CXSCREEN);
    int yScreen = GetSystemMetrics(SM_CYSCREEN);
    if(r.right > xScreen) {
        r.right = xScreen;
    }
    if(r.bottom > yScreen) {
        r.bottom = yScreen;
    }
    if(r.left < 0) {
        xshift = -r.left;
        r.left = 0;
    }
    if(r.top < 0) {
        yshift = -r.top;
        r.top = 0;
    }

    const int width  = r.right  - r.left;
    const int height = r.bottom - r.top;

    if(width <= 0 || height <= 0) {
        return FALSE;
    }

    // Hide the application window.
    ShowWindow(hWndApplicationWindow, SW_HIDE);
    // Bring the window at the top most level
    SetWindowPos(hWndSelectedWindow, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    // Give enough time to refresh the window
    Sleep(500);

    // Prepare the DCs
    const HDC dstDC = GetDC(nullptr);
    const HDC srcDC = GetWindowDC(hWndSelectedWindow); // full window (GetDC(hWndSelectedWindow) = clientarea)
    const HDC memDC = CreateCompatibleDC(dstDC);

    // Copy the screen to the bitmap
    const HBITMAP bm = CreateCompatibleBitmap(dstDC, width, height);
    const HBITMAP oldbm = static_cast<HBITMAP>(SelectObject(memDC, bm));
    BitBlt(memDC, 0, 0, width, height, srcDC, xshift, yshift, SRCCOPY);

    // Redraw the application window.
    ShowWindow(hWndApplicationWindow, SW_SHOW);

    // Restore the position
    SetWindowPos(hWndSelectedWindow, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
    SetWindowPos(hWndApplicationWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

    // Convert the HBITMAP to a FIBITMAP
    copyFromBitmap(bm);

    // Free objects
    DeleteObject(SelectObject(memDC, oldbm));
    DeleteDC(memDC);

    // Convert 32-bit images to 24-bit
    if(getBitsPerPixel() == 32) {
        convertTo24Bits();
    }

    return TRUE;
}


///////////////////////////////////////////////////////////////////
// Painting operations

void fipWinImage::drawEx(HDC hDC, RECT& rcDest, BOOL useFileBkg, RGBQUAD *appBkColor, FIBITMAP *bg) const {
    // Convert to a standard bitmap if needed
    if(_bHasChanged) {
        if(_bDeleteMe) {
            FreeImage_Unload(_display_dib);
            _display_dib = nullptr;
            _bDeleteMe = FALSE;
        }

        const FREE_IMAGE_TYPE image_type = getImageType();
        if(image_type == FIT_BITMAP) {
            const BOOL bHasBackground = FreeImage_HasBackgroundColor(_dib.get());
            const BOOL bIsTransparent = FreeImage_IsTransparent(_dib.get());

            if(!bIsTransparent && (!bHasBackground || !useFileBkg))
            {
                // Copy pointer
                _display_dib.replace(_dib.get());// .reset(std::move(_dib);
            }
            else
            {
                // Create the transparent / alpha blended image
                _display_dib.replace(FreeImage_Composite(_dib.get(), useFileBkg, appBkColor, bg));
                if(_display_dib) {
                    // Remember to delete _display_dib
                    _bDeleteMe = TRUE;
                } else {
                    // Something failed: copy pointers
                    _display_dib.replace(_dib.get());
                }
            }
        } else {
            // Convert to a standard dib for display

            if(image_type == FIT_COMPLEX) {
                // Convert to type FIT_DOUBLE
                FIBITMAP *dib_double = FreeImage_GetComplexChannel(_dib.get(), FICC_MAG);
                // Convert to a standard bitmap (linear scaling)
                _display_dib.replace(FreeImage_ConvertToStandardType(dib_double, TRUE));
                // Free image of type FIT_DOUBLE
                FreeImage_Unload(dib_double);
            } else if(image_type == FIT_RGBF || image_type == FIT_RGBAF || image_type == FIT_RGB16) {
                // Apply a tone mapping algorithm and convert to 24-bit
                switch(_tmo) {
                    case FITMO_REINHARD05:
                        _display_dib.replace(FreeImage_TmoReinhard05Ex(_dib.get(), _tmo_param_1, _tmo_param_2, _tmo_param_3, _tmo_param_4));
                        break;
                    default:
                        _display_dib.replace(FreeImage_ToneMapping(_dib.get(), _tmo, _tmo_param_1, _tmo_param_2));
                        break;
                }
            } else if(image_type == FIT_RGBA16) {
                // Convert to 32-bit
                FIBITMAP *dib32 = FreeImage_ConvertTo32Bits(_dib.get());
                if(dib32)
                {
                    // Create the transparent / alpha blended image
                    _display_dib.replace(FreeImage_Composite(dib32, useFileBkg, appBkColor, bg));
                    FreeImage_Unload(dib32);
                }
            } else {
                // Other cases: convert to a standard bitmap (linear scaling)
                _display_dib.replace(FreeImage_ConvertToStandardType(_dib.get(), TRUE));
            }
            // Remember to delete _display_dib
            _bDeleteMe = TRUE;
        }

        _bHasChanged = FALSE;
    }

    // Draw the dib
    SetStretchBltMode(hDC, COLORONCOLOR);
    StretchDIBits(hDC, rcDest.left, rcDest.top,
        rcDest.right-rcDest.left, rcDest.bottom-rcDest.top,
        0, 0, FreeImage_GetWidth(_display_dib), FreeImage_GetHeight(_display_dib),
        FreeImage_GetBits(_display_dib), FreeImage_GetInfo(_display_dib), DIB_RGB_COLORS, SRCCOPY);

}

void fipWinImage::setToneMappingOperator(FREE_IMAGE_TMO tmo, double first_param, double second_param, double third_param, double fourth_param) {
    // avoid costly operations if possible ...
    if(_tmo != tmo || _tmo_param_1 != first_param || _tmo_param_2 != second_param || _tmo_param_3 != third_param || _tmo_param_4 != fourth_param) {
        _tmo = tmo;
        _tmo_param_1 = first_param;
        _tmo_param_2 = second_param;
        _tmo_param_3 = third_param;
        _tmo_param_4 = fourth_param;

        const FREE_IMAGE_TYPE image_type = getImageType();
        switch(image_type) {
            case FIT_RGBF:
            case FIT_RGBAF:
            case FIT_RGB16:
            case FIT_RGBA16:
                _bHasChanged = TRUE;
                break;
            default:
                break;
        }
    }
}

void fipWinImage::getToneMappingOperator(FREE_IMAGE_TMO *tmo, double *first_param, double *second_param, double *third_param, double *fourth_param) const {
    *tmo = _tmo;
    *first_param = _tmo_param_1;
    *second_param = _tmo_param_2;
    *third_param = _tmo_param_3;
    *fourth_param = _tmo_param_4;
}


#endif // _WIN32
