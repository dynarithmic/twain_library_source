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
#ifndef WINBIT32_H
#define WINBIT32_H

#include <string>
#include <fstream>
#include <windows.h>
#include "fltrect.h"
#include "dibmulti.h"
#include "ctlstringdefs.h"
#include "dtwain_raii.h"
#include "dtwain_filesystem.h"
#include "blankpage.h"

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif

#ifdef NO_NATIVE_UINT32
  typedef unsigned long UINT32;
  typedef unsigned long ULONG32;
#endif

#define GREY1(r,g,b) (BYTE)(((WORD)r*77 + (WORD)g*150 + (WORD)b*29) >> 8)   // .299R + .587G + .114B
#define GREY2(r,g,b) (BYTE)(((WORD)r*169 + (WORD)g*256 + (WORD)b*87) >> 9)  // .33R + 0.5G + .17B
#define RGB565(b, g, r) (((b) >> 3) | (((g) >> 2) << 5) | (((r) >> 3) << 11))
#define RGB555(b, g, r) (((b) >> 3) | (((g) >> 3) << 5) | (((r) >> 3) << 10))
#define WIDTHBYTES(i)   ((((i)+31)/32)*4)
#define PIXELS2BYTES(n) (((n)+7)/8)


#define GetChunkyPixel(pxx,nxx) (!((nxx) & 1)) ? (((pxx)[(nxx)>>1] >> 4) & 0x0f) : ((pxx)[(nxx)>>1] & 0x0f)
#define LPBlinewidth(lpbi) (WIDTHBYTES((WORD)lpbi->biWidth*lpbi->biBitCount))
#define LPBwidth(lpbi)  (lpbi->biWidth)
#define LPBdepth(lpbi)  (lpbi->biHeight)
#define LPBbits(lpbi)   (lpbi->biBitCount)
#define LPBcolours(lpbi) (lpbi->biClrUsed)
#define LPBimage(lpbi)  ((HPSTR)lpbi+lpbi->biSize+(long)(lpbi->biClrUsed*sizeof(RGBQUAD)))

#ifdef WIN32
#define PLATFORM                "32-bit Windows"
#define FixedGlobalAlloc(n)     (char *)malloc(n)
#define FixedGlobalFree(p)      free(p)
#define FixedGlobalRealloc(p,n) realloc(p,n)
#define hmemcpy_(d,s,n)          memcpy(d,s,n)
typedef char* HPSTR;
#endif

#define RGB_RED         0
#define RGB_GREEN       1
#define RGB_BLUE        2
#define RGB_SIZE        3
#define WRGB_RED        2
#define WRGB_GREEN      1
#define WRGB_BLUE       0
#define DITHERBRIGHTNESS    20
#define DITHERCONTRAST      20
#define BYTEBUFFERSIZE  2048

namespace dynarithmic
{

#ifndef _WIN32
    #define GMEM_MOVEABLE 0x0002
    #define GMEM_DDESHARE 0x2000
    struct MemoryNode
    {
        std::vector<char> ptr;
    };

    struct ImageMemoryHandler
    {
        static LPVOID GlobalLock(HANDLE hDib) { return hDib; }
        static BOOL GlobalUnlock(HGLOBAL) { return 1; }
        static DWORD GetLastError() { return 1; }

        static HGLOBAL GlobalAlloc(UINT n, SIZE_T numBytes)
        {
            MemoryNode *pNode = new MemoryNode;
            pNode->ptr.resize(numBytes);
            return pNode;
        }

        static HGLOBAL GlobalFree(HGLOBAL h)
        {
            MemoryNode *pNode = reinterpret_cast<MemoryNode*>(h);
            delete pNode;
            return 0;
        }

        static HGLOBAL GlobalReAlloc(HGLOBAL hMem, SIZE_T dwBytes, UINT uFlags)
        {
            MemoryNode *pNode = reinterpret_cast<MemoryNode*>(hMem);
            pNode->ptr.resize(dwBytes);
            return hMem;
        }

        static HGLOBAL GlobalReAllocPr(HGLOBAL hMem, SIZE_T dwBytes, UINT uFlags) { return GlobalReAlloc(hMem, dwBytes, uFlags);}
        static BOOL DeleteObject(HGDIOBJ h) { return 1; }
        static LPVOID  GlobalAllocPr(UINT n, SIZE_T numBytes) {return GlobalLock(GlobalAlloc(n, numBytes)); }
        static LPVOID  GlobalReAllocPr(UINT n, SIZE_T numBytes) {return GlobalLock(GlobalAlloc(n, numBytes)); }
        static BOOL    GlobalFreePr(LPVOID h) {GlobalFree(h); return 1;}
        static HGLOBAL GlobalHandle(LPCVOID h) { return (HGLOBAL)h; }
        static SIZE_T  GlobalSize(HGLOBAL h)
        {
            if ( !h )
                return 0;
            MemoryNode *pNode = reinterpret_cast<MemoryNode*>(h);
            return pNode->ptr.size();
        }
    };

    typedef struct _SMALL_RECT {
        SHORT Left;
        SHORT Top;
        SHORT Right;
        SHORT Bottom;
    } SMALL_RECT, *PSMALL_RECT;

    #define GetRValue(rgb)      (LOBYTE(rgb))
    #define GetGValue(rgb)      (LOBYTE(((WORD)(rgb)) >> 8))
    #define GetBValue(rgb)      (LOBYTE((rgb)>>16))

#else
    #include <windowsx.h>
    struct ImageMemoryHandler
    {
        static LPVOID GlobalLock(HANDLE hDib) { return ::GlobalLock(hDib); }
        static BOOL GlobalUnlock(HGLOBAL h) { return ::GlobalUnlock(h); }
        static DWORD GetLastError() { return ::GetLastError(); }
        static HGLOBAL GlobalFree(HGLOBAL h) { return ::GlobalFree(h); }
        static BOOL DeleteObject(HGDIOBJ h) { return ::DeleteObject(h); }
        static HGLOBAL GlobalAlloc(UINT n, SIZE_T numBytes) { return ::GlobalAlloc(n, numBytes); }
        static HGLOBAL GlobalReAlloc(HGLOBAL hMem, SIZE_T dwBytes, UINT uFlags) { return ::GlobalReAlloc(hMem, dwBytes, uFlags); }
        static HGLOBAL GlobalReAllocPr(HGLOBAL hMem, SIZE_T dwBytes, UINT uFlags) { return GlobalReAllocPtr(hMem, dwBytes, uFlags); }
        static LPVOID  GlobalAllocPr(UINT n, SIZE_T numBytes) { return GlobalAllocPtr(n, numBytes); }
        static BOOL    GlobalFreePr(LPVOID h) { GlobalUnlockPtr(h); GlobalFreePtr(h); return TRUE;}
        static HGLOBAL GlobalHandle(LPCVOID h) { return ::GlobalHandle(h); }
        static SIZE_T  GlobalSize(HGLOBAL h) { return ::GlobalSize(h); }
    };
#endif
    class CTL_ImageIOHandler;
    enum {
        FIC_MINISWHITE = 0,             // min value is white
        FIC_MINISBLACK = 1,             // min value is black
        FIC_RGB        = 2,             // RGB color model
        FIC_PALETTE    = 3,             // color map indexed
        FIC_RGBALPHA   = 4,             // RGB color model with alpha channel
    };

    #define RGB_RED         0
    #define RGB_GREEN       1
    #define RGB_BLUE        2
    #define RGB_SIZE        3

    class CDibInterface
    {
        public:
            CDibInterface();
            virtual ~CDibInterface() = default;

            // Virtual interface
            virtual int     WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE * /*pImage2*/, UINT32 /*wid*/, UINT32 /*ht*/, UINT32 /*bpp*/, UINT32 /*nColors*/, RGBQUAD * /*pPal*/,
                                       void * /*pUserInfo*/ = nullptr) { return TRUE; }

            virtual void SetMultiPageStatus(DibMultiPageStruct * /*pStruct*/) { }
            virtual void GetMultiPageStatus(DibMultiPageStruct * /*pStruct*/) { }
            virtual int WriteGraphicFile(CTL_ImageIOHandler* /*pThis*/, LPCTSTR /*path*/, HANDLE /*bitmap*/, void * /*pUserInfo*/ = nullptr) = 0;
            static HANDLE CreateDIB(int width, int height, int bpp, LPSTR palette= nullptr);

            LONG    GetLastError() const { return m_lasterror; }
            static bool    IsGrayScale(HANDLE hDib, int bpp);
            static bool    IsBlankDIB(HANDLE hDib, double threshold=0.99);
            static BlankDIBInfo IsBlankDIBEx(HANDLE hDib, double threshold = 99.0);

            // Crop functions
            static HANDLE ResampleDIB(HANDLE hDib, long newx, long newy);
            static HANDLE ResampleDIB(HANDLE hDib, double scalex, double scaley);
            static HANDLE IncreaseBpp(HANDLE hDib, long newbpp);
            static HANDLE DecreaseBpp(HANDLE hDib, long newbpp);
            static HANDLE RotateDIB(HANDLE hDib, float angle);
            static HANDLE IncreaseDecreaseBpp(HANDLE hDib, long newbpp, bool bIncrease);
            static HANDLE CropDIB(HANDLE handle, const FloatRect& ActualRect,const FloatRect& RequestedRect,int sourceunit,
                                  int destunit, int dpi, bool bConvertActual, int& retval);
            static HANDLE NegateDIB(HANDLE hDib);

            // Normalization of irregular DIBs
            static HANDLE NormalizeDib(HANDLE hDib, bool bReturnCopy = false);

            // Copy function
            static HANDLE CopyDib(HANDLE hDib);

            // Convert Dib to HBITMAP
            static HBITMAP DIBToBitmap(HANDLE hDib, HPALETTE hPal = nullptr);

            virtual bool OpenOutputFile(LPCTSTR pFileName);
            virtual bool CloseOutputFile();

        protected:
            void SetError(LONG nError) { m_lasterror = nError; }
            virtual void DestroyAllObjects() { }

            DibMultiPageStruct m_MultiPageStruct;

        private:
            LONG m_lasterror;
            CTL_StringType m_sFileName;
    };
}
#endif
    
