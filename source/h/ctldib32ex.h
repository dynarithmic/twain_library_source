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

#ifndef CTLDIB32EX_H
#define CTLDIB32EX_H

//// Special bitmap routines
#include <windows.h>
#include "dtwaindefs.h"
#include "blankpage.h"
#include "fltrect.h"

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
namespace dynarithmic
{
    struct DTWAINImageInfoEx;
    class CTL_ImageIOHandler;

    typedef std::shared_ptr<CTL_ImageIOHandler> CTL_ImageIOHandlerPtr;
    ///////////////////////////////////////////////////////////////////////////////
    class CTL_HBitmap
    {
        public:
            CTL_HBitmap(HBITMAP h= nullptr) {m_hBitmap = h;}
            operator HBITMAP() const { return m_hBitmap; }
            HBITMAP GetHBitmap() const {return m_hBitmap;}

        private:
            HBITMAP m_hBitmap;
    };


    class CTL_TwainDibInfo
    {
        public:
            CTL_TwainDibInfo();

            void    SetDib(HANDLE hDib);
            void    SetPalette( HPALETTE hPal );
            void    DeleteAllDibInfo();

            HANDLE   GetDib() const;
            HPALETTE GetPalette() const;

            bool operator == (const CTL_TwainDibInfo& rInfo) const;
            void DeleteDibPalette();
            void DeleteDib();

        private:
            HANDLE      m_hDib;
            HPALETTE    m_hPal;
    };

    class CTL_TwainDib
    {
        public:
            friend class CTL_TwainDibArray;
            friend class CTL_ImageIOHandler;

            enum { DibFormat=DTWAIN_BMP,
                   BmpFormat = DTWAIN_BMP,
                   JpegFormat = DTWAIN_JPEG,
                   PDFFormat = DTWAIN_PDF,
                   PDFFormatMULTI = DTWAIN_PDFMULTI,
                   PcxFormat = DTWAIN_PCX,
                   DcxFormat = DTWAIN_DCX,
                   TgaFormat = DTWAIN_TGA,
                   TgaFormatRLE = DTWAIN_TGA_RLE,
                   BmpFormatRLE = DTWAIN_BMP_RLE, 
                   TiffFormatLZW = DTWAIN_TIFFLZW,
                   TiffFormatNONE = DTWAIN_TIFFNONE,
                   TiffFormatGROUP3 = DTWAIN_TIFFG3,
                   TiffFormatGROUP4 = DTWAIN_TIFFG4,
                   TiffFormatPACKBITS = DTWAIN_TIFFPACKBITS,
                   TiffFormatDEFLATE = DTWAIN_TIFFDEFLATE,
                   TiffFormatJPEG = DTWAIN_TIFFJPEG,
                   TiffFormatJBIG = DTWAIN_TIFFJBIG,
                   TiffFormatPIXARLOG = DTWAIN_TIFFPIXARLOG,
                   TiffFormatNONEMULTI = DTWAIN_TIFFNONEMULTI,
                   TiffFormatGROUP3MULTI = DTWAIN_TIFFG3MULTI,
                   TiffFormatGROUP4MULTI  = DTWAIN_TIFFG4MULTI,
                   TiffFormatPACKBITSMULTI = DTWAIN_TIFFPACKBITSMULTI,
                   TiffFormatDEFLATEMULTI = DTWAIN_TIFFDEFLATEMULTI,
                   TiffFormatJPEGMULTI = DTWAIN_TIFFJPEGMULTI,
                   TiffFormatLZWMULTI = DTWAIN_TIFFLZWMULTI,
                   TiffFormatJBIGMULTI = DTWAIN_TIFFJBIGMULTI,
                   TiffFormatPIXARLOGMULTI = DTWAIN_TIFFPIXARLOGMULTI,
                   BigTiffFormatLZW = DTWAIN_BIGTIFFLZW,
                   BigTiffFormatLZWMULTI = DTWAIN_BIGTIFFLZWMULTI,
                   BigTiffFormatNONE = DTWAIN_BIGTIFFNONE,
                   BigTiffFormatNONEMULTI = DTWAIN_BIGTIFFNONEMULTI, 
                   BigTiffFormatPACKBITS = DTWAIN_BIGTIFFPACKBITS, 
                   BigTiffFormatPACKBITSMULTI = DTWAIN_BIGTIFFPACKBITSMULTI,
                   BigTiffFormatDEFLATE = DTWAIN_BIGTIFFDEFLATE,
                   BigTiffFormatDEFLATEMULTI = DTWAIN_BIGTIFFDEFLATEMULTI,
                   BigTiffFormatGROUP3 = DTWAIN_BIGTIFFG3,
                   BigTiffFormatGROUP3MULTI = DTWAIN_BIGTIFFG3MULTI,
                   BigTiffFormatGROUP4 = DTWAIN_BIGTIFFG4,
                   BigTiffFormatGROUP4MULTI = DTWAIN_BIGTIFFG4MULTI,
                   BigTiffFormatJPEG = DTWAIN_BIGTIFFJPEG,
                   BigTiffFormatJPEGMULTI = DTWAIN_BIGTIFFJPEGMULTI,
                   WmfFormat = DTWAIN_WMF,
                   EmfFormat = DTWAIN_EMF,
                   GifFormat = DTWAIN_GIF,
                   PngFormat = DTWAIN_PNG,
                   PsdFormat = DTWAIN_PSD,
                   Jpeg2000Format = DTWAIN_JPEG2000,
                   PSFormatLevel1 = DTWAIN_POSTSCRIPT1,
                   PSFormatLevel2 = DTWAIN_POSTSCRIPT2,
                   PSFormatLevel3 = DTWAIN_POSTSCRIPT3,
                   PSFormatLevel1Multi = DTWAIN_POSTSCRIPT1MULTI,
                   PSFormatLevel2Multi = DTWAIN_POSTSCRIPT2MULTI,
                   PSFormatLevel3Multi = DTWAIN_POSTSCRIPT3MULTI,
                   TextFormat = DTWAIN_TEXT,
                   TextFormatMulti = DTWAIN_TEXTMULTI,
                   IcoFormat = DTWAIN_ICO,
                   IcoVistaFormat = DTWAIN_ICO_VISTA,
                   IcoFormatResized = DTWAIN_ICO_RESIZED,
                   WBMPFormat = DTWAIN_WBMP,
                   WBMPFormatResized = DTWAIN_WBMP_RESIZED,
                   WEBPFormat = DTWAIN_WEBP,
                   PBMFormat = DTWAIN_PBM,
                   JpegXRFormat = DTWAIN_JPEGXR,
                   SVGFormat = DTWAIN_SVG,
                   SVGZFormat = DTWAIN_SVGZ,
                   RawFormat=9999};

            // Setting/Getting
            void        SetHandle(HANDLE hDib, bool bSetPalette=true);
            HANDLE      GetHandle() const;

            CTL_TwainDib& operator =(const CTL_TwainDib& rDib);
            operator    HANDLE() const { return m_TwainDibInfo.GetDib(); }
            // Destroying

            // Dib related functions
            int         GetDepth() const;
            int         GetWidth() const ;
            int         GetHeight() const;
            int         GetNumColors() const;
            int         GetResolution() const;
            int         GetBitsPerPixel() const;
            std::optional<DWORD> GetBitsOffset() const;
            bool        IsGrayScale() const;
            BlankDIBInfo IsBlankDIB(double threshold) const;
            void        Delete();
            bool        FlipBitMap(bool bRGB=false);

            // Auto deletion flag
            CTL_TwainDib&  SetAutoDelete(bool bSet=true) {m_bAutoDelete = bSet; return *this; }
            CTL_TwainDib&  SetAutoDeletePalette(bool bSet = true) { m_bAutoDeletePalette = bSet; return *this; }
            bool        IsAutoDelete() const { return m_bAutoDelete; }
            bool        IsAutoDeletePalette() const { return m_bAutoDeletePalette; }

            // Write an image file
            int         WriteDibBitmap(DTWAINImageInfoEx& ImageInfo, LPCTSTR szFile, 
                                       int nFormat=BmpFormat, bool bOpenFile=true, int fh=0);

            // Write a multi-page DIB file
            CTL_ImageIOHandlerPtr WriteFirstPageDibMulti(DTWAINImageInfoEx& ImageInfo, LPCTSTR szFile, int nFormat,
                                                        bool bOpenFile, int fhFile, int &nStatus);
            int WriteNextPageDibMulti(CTL_ImageIOHandlerPtr& pImgHandler, int nFormat, int &nStatus, const DTWAINImageInfoEx& ImageInfo);
            static int WriteLastPageDibMulti(CTL_ImageIOHandlerPtr& pImgHandler, int &nStatus, bool bSaveFile=true);

            // Crop a DIB
            int CropDib(const FloatRect& ActualRect, const FloatRect& RequestedRect,
                        LONG SourceUnit, LONG DestUnit, int dpi,
                        int flags);

            int ResampleDib(double xscale, double yscale);
            int ResampleDib(long newx, long newy);

            // Increase bpp
            bool IncreaseBpp(unsigned long bpp);

            // Decrease bpp
            bool DecreaseBpp(unsigned long bpp);

            // Normalize a DIB
            int NormalizeDib();

            // Negate DIB
            int NegateDib();

            static void ResolvePostscriptOptions(const DTWAINImageInfoEx& Info, int &nFormat );

            CTL_TwainDib();
            CTL_TwainDib(HANDLE hDib, HWND hWnd= nullptr);
            CTL_TwainDib(const CTL_TwainDib& rDib);
            void swap(CTL_TwainDib& left, CTL_TwainDib& rt) noexcept;

            static constexpr int PixelToBytes(int n) { return (n+7)/8; }
            WORD               PaletteSize (void* pv);
            HPALETTE           GetPalette() const {return m_TwainDibInfo.GetPalette(); }
            static constexpr int WidthInBytes(int i)  { return (i+31)/32*4; }
            static             HANDLE CreateBMPBitmapFromDIB(HANDLE hDib);

            // Destruction
            ~CTL_TwainDib();

        protected:
            void        Init();
            void        SetEqual(const CTL_TwainDib& rDib);
            bool        IncreaseBppImpl(unsigned long bpp, bool bIncrease);


        private:
            CTL_TwainDibInfo m_TwainDibInfo;
            bool        m_bAutoDelete;
            bool        m_bAutoDeletePalette;
            bool        m_bIsValid;
    };

    typedef std::shared_ptr<CTL_TwainDib> CTL_TwainDibPtr;

    class CTL_TwainDibArray
    {
        public:
            CTL_TwainDibArray(bool bAutoDelete = true);
            ~CTL_TwainDibArray();

            // Dib page creation
            CTL_TwainDibPtr CreateDib();
            CTL_TwainDibPtr CreateDib(HANDLE hDib, HWND hWnd= nullptr);
            CTL_TwainDibPtr CreateDib(const CTL_TwainDib& rDib);

            // Dib page deletion
            bool          RemoveDib( CTL_TwainDibPtr pDib);
            bool          RemoveDib( size_t nWhere );
            bool          RemoveDib( HANDLE hDib );

            // Dib memory deletion
            bool          DeleteDibMemory(CTL_TwainDibPtr Dib);
            bool          DeleteDibMemory(size_t nWhere );
            bool          DeleteDibMemory(HANDLE hDib );

            // Remove All Dibs from array
            void RemoveAllDibs();

            // Provide conversion for retrieval
            const CTL_TwainDibPtr& GetAt(size_t nPos) const
            { return m_TwainDibArray.at(nPos); }
            CTL_TwainDibPtr&       GetAt(size_t nPos);
            CTL_TwainDibPtr&       operator[](size_t nPos);
            const CTL_TwainDibPtr&  operator[](size_t nPos) const;
            CTL_TwainDibPtr&       Back() noexcept { return m_TwainDibArray.back(); }
            CTL_TwainDibPtr&       Front() noexcept { return m_TwainDibArray[0]; }
            CTL_TwainDibPtr&       BackAt() { return GetAt(GetSize() - 1); }
            CTL_TwainDibPtr&       FrontAt() { return GetAt(0); }

            // Deletion of globally locked dib memory.  If TRUE, dib memory is deallocated when Dib object is deleted
            bool    IsAutoDelete() const;
            size_t  GetSize() const { return m_TwainDibArray.size(); }


        private:
            CTL_TwainDibPtr InitializeDibInfo(CTL_TwainDibPtr Dib);
            bool    m_bAutoDelete;
            std:: vector<CTL_TwainDibPtr> m_TwainDibArray;
    };
}
#endif
