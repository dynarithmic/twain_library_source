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

#ifndef CTLDIB32_H
#define CTLDIB32_H

//// Special bitmap routines
#include "imagefun/imgfunc.h"
#include <vector>
#include <utility>
#include <boost/optional.hpp>
#include <boost/container/flat_map.hpp>
#include "dibmulti.h"
#include "dibinfox.h"
#include "fltrect.h"
#include "FreeImage.h"
#include "FreeImagePlus.h"
#include <unordered_map>
#include "dtwain_filetypes.h"
#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
namespace dynarithmic
{
    class CTL_TwainDibArray;
    class CTL_TwainDib;

    //////////////////// Io Handler
    #define BYTEBUFFERSIZE      2048

    class CTL_ImageIOHandler
    {
        public:
            CTL_ImageIOHandler();
            CTL_ImageIOHandler( CTL_TwainDib *pDib );
            virtual ~CTL_ImageIOHandler() = default;
            void    SetDib( CTL_TwainDib *pDib ) { m_pDib = pDib; }
            virtual int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pMultiDibStruct = nullptr) = 0;
            std::shared_ptr<DibMultiPageData> GetMultiDibData() const { return pMultiDibData; }
            void SetMultiDibData(std::shared_ptr<DibMultiPageData> pData) { pMultiDibData = std::move(pData);  }
            void SetMultiDibInfo(const DibMultiPageStruct &s);
            DibMultiPageStruct GetMultiDibInfo() const;
            bool AllPagesOK() const { return m_bAllWritten; }
            unsigned int GetNumPagesWritten() const { return m_nPage; }
            void SetPagesOK(bool bSet) { m_bAllWritten = bSet; }
            void SetNumPagesWritten(unsigned nPages) { m_nPage = nPages; }
            void SetOnePageWritten(bool bSet) { m_bOnePageWritten = bSet; }
            bool IsOnePageWritten() const { return m_bOnePageWritten; }
            virtual void SetImageInfo(const DTWAINImageInfoEx& /*ImageInfo*/) { }
            void SetBaseImageInfo(const DTWAINImageInfoEx& ImageInfo) { m_ImageInfo = ImageInfo; }
            const DTWAINImageInfoEx& GetBaseImageInfo() const { return m_ImageInfo; }
            CTL_TwainDib* GetDib() const { return m_pDib; }
            static bool IsValidBitDepth(LONG FileType, LONG bitDepth);
            int SaveToFile(HANDLE hDib, LPCTSTR szFile, FREE_IMAGE_FORMAT fmt, int flags, UINT unitOfMeasure,
                           std::pair<LONG, LONG> res, const std::tuple<double, double, double, double>& multipler_pr = { 1,1, 0.5, 0.5 }) const;
            static auto& GetSupportedBPPMap() { return s_supportedBitDepths; }

        protected:
            CTL_TwainDib *m_pDib;
            DTWAINImageInfoEx m_ImageInfo;
            unsigned int bytesleft,nextbyte;
            char bytebuffer[BYTEBUFFERSIZE];
            void resetbuffer();
            char bittable[8];
            char masktable[8];
            std::shared_ptr<DibMultiPageData> pMultiDibData;
            DibMultiPageStruct m_DibMultiPageStruct;
            unsigned m_nPage;
            bool m_bAllWritten;
            bool m_bOnePageWritten;
            static boost::container::flat_map<LONG, std::vector<int>> s_supportedBitDepths;
    };

    typedef std::shared_ptr<CTL_ImageIOHandler> CTL_ImageIOHandlerPtr;

    #ifndef DTWAIN_NOIMAGE_SUPPORT
    ////////////////////// Supported file handlers ///////////////////////////
    class CTL_BmpIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_BmpIOHandler() : CTL_ImageIOHandler() {}
            CTL_BmpIOHandler( CTL_TwainDib *pDib, DTWAINImageInfoEx& imageInfo ) : CTL_ImageIOHandler(pDib ), m_ImageInfoEx(imageInfo) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

        private:
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_JpegIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_JpegIOHandler(int Quality=75, BOOL bJpegProgressive=FALSE) :
                    CTL_ImageIOHandler(),
                    m_nJpegQuality(Quality),
                    m_bJpegProgressive(bJpegProgressive) {}

            CTL_JpegIOHandler( CTL_TwainDib *pDib, DTWAINImageInfoEx& ImageInfoEx)
                            : CTL_ImageIOHandler(pDib ),
                            m_nJpegQuality(75),
                            m_bJpegProgressive(false),
                            m_ImageInfoEx(ImageInfoEx) {}

            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

        private:
            int     m_nJpegQuality;
            BOOL    m_bJpegProgressive;
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_TiffIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_TiffIOHandler(int nFormat, DTWAINImageInfoEx& ImageInfoEx) : CTL_ImageIOHandler(), m_nFormat(nFormat),
                                m_ImageInfoEx(ImageInfoEx) {}
            CTL_TiffIOHandler( CTL_TwainDib *pDib, int nFormat, DTWAINImageInfoEx &ImageInfoEx ): CTL_ImageIOHandler(pDib),
                            m_nFormat(nFormat), m_ImageInfoEx(ImageInfoEx) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct) override;
            void SetTiffFormat(int nFormat) { m_nFormat = nFormat; }
            int  GetTiffFormat() const { return m_nFormat; }
            CTL_StringType GetFileName() const { return sActualFileName; }
            CTL_StringType GetPostscriptName() const { return sPostscriptName; }

        private:
            int m_nFormat;
            DTWAINImageInfoEx m_ImageInfoEx;
            CTL_StringType sActualFileName;
            CTL_StringType sPostscriptName;
    };

    class CTL_PngIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_PngIOHandler() : CTL_ImageIOHandler() {};
            CTL_PngIOHandler( CTL_TwainDib *pDib, DTWAINImageInfoEx& ImageInfoEx) : CTL_ImageIOHandler( pDib ), m_ImageInfoEx(ImageInfoEx) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
        private:
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_PcxIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_PcxIOHandler() : CTL_ImageIOHandler(), m_nFormat{} {};
            CTL_PcxIOHandler( CTL_TwainDib *pDib, int nFormat, DTWAINImageInfoEx& ImageInfoEx ) : CTL_ImageIOHandler(pDib),
            m_nFormat(nFormat), m_ImageInfoEx(ImageInfoEx) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

        private:
            int m_nFormat;
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_TgaIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_TgaIOHandler()  : CTL_ImageIOHandler() {}
            CTL_TgaIOHandler( CTL_TwainDib *pDib, DTWAINImageInfoEx& ImageInfoEx) : CTL_ImageIOHandler( pDib ), m_ImageInfoEx(ImageInfoEx) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

        private:
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_WmfIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_WmfIOHandler() : CTL_ImageIOHandler(), m_nFormat{} {}
            CTL_WmfIOHandler( CTL_TwainDib *pDib, int nFormat ) :
                                CTL_ImageIOHandler( pDib ), m_nFormat(nFormat) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

        private:
            int m_nFormat;
    };

    class CTL_PsdIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_PsdIOHandler()  : CTL_ImageIOHandler() {};
            CTL_PsdIOHandler( CTL_TwainDib *pDib ) : CTL_ImageIOHandler( pDib ) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
    };

    class CTL_PDFIOHandler final : public CTL_ImageIOHandler
    {
        public:
            CTL_PDFIOHandler(CTL_TwainDib* pDib, int nFormat, const DTWAINImageInfoEx &ImageInfoEx);
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
            ~CTL_PDFIOHandler() override = default;

            void SetImageInfo(const DTWAINImageInfoEx& ImageInfo) override
            {
                m_ImageInfoEx = ImageInfo;
            }

        private:
            int GetOCRText(LPCTSTR szFileName, int pageType, std::string& sText);
            static bool CheckValidConvertType(int fileType, int pageType);
            int m_nFormat;
            DTWAINImageInfoEx m_ImageInfoEx;
            CTL_JpegIOHandler m_JpegHandler;
            CTL_TiffIOHandler m_TiffHandler;
    };


    class CTL_PSIOHandler final : public CTL_ImageIOHandler
    {
        public:
            CTL_PSIOHandler(CTL_TwainDib* pDib, int nFormat, const DTWAINImageInfoEx &ImageInfoEx,
                            LONG PSType, bool IsMultiPage);
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
            ~CTL_PSIOHandler() override = default;

        private:
            int m_nFormat;
            DTWAINImageInfoEx m_ImageInfoEx;
            CTL_JpegIOHandler* m_pJpegHandler;
            std::shared_ptr<CTL_TiffIOHandler> m_pTiffHandler;

            LONG m_PSType;
            bool m_bIsMultiPage;
    };

    class CTL_Jpeg2KIOHandler final : public CTL_ImageIOHandler
    {
        public:
            CTL_Jpeg2KIOHandler(CTL_TwainDib* pDib, const DTWAINImageInfoEx &ImageInfoEx);
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
            ~CTL_Jpeg2KIOHandler() override = default;

        private:
            int m_nFormat;
            DTWAINImageInfoEx m_ImageInfoEx;
            CTL_JpegIOHandler* m_pJpegHandler;
    };

    class CTL_GifIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_GifIOHandler(CTL_TwainDib* pDib)  :
            CTL_ImageIOHandler(pDib) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
    };

    class CTL_IcoIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_IcoIOHandler(CTL_TwainDib* pDib, DTWAINImageInfoEx& ImageInfoEx)  : 
                            CTL_ImageIOHandler(pDib), m_ImageInfoEx(ImageInfoEx) {}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

        private:
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_PBMIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_PBMIOHandler() : CTL_ImageIOHandler() {}
            CTL_PBMIOHandler(CTL_TwainDib *pDib) : CTL_ImageIOHandler(pDib){}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
        private:
            DTWAINImageInfoEx m_ImageInfoEx;
    };

    class CTL_WBMPIOHandler : public CTL_ImageIOHandler
    {
    public:
        CTL_WBMPIOHandler(CTL_TwainDib* pDib, DTWAINImageInfoEx& ImageInfoEx)  : CTL_ImageIOHandler(pDib),
            m_ImageInfoEx(ImageInfoEx) {}
        int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;

    private:
        DTWAINImageInfoEx m_ImageInfoEx;
    };

    class OCREngine;

    class CTL_TextIOHandler : public CTL_ImageIOHandler
    {
    public:
        CTL_TextIOHandler(CTL_TwainDib* pDib, int nInputFormat, DTWAINImageInfoEx ImageInfoEx,
                          OCREngine* pEngine);
        int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
        ~CTL_TextIOHandler() override = default;

        void SetImageInfo(const DTWAINImageInfoEx& ImageInfo) override
        {
            m_ImageInfoEx = ImageInfo;
        }

    private:
        int m_nInputFormat;
        DTWAINImageInfoEx m_ImageInfoEx;
        OCREngine *m_pOCREngine;
    };

    class CTL_WebpIOHandler : public CTL_ImageIOHandler
    {
        public:
            CTL_WebpIOHandler() : CTL_ImageIOHandler() {}
            CTL_WebpIOHandler(CTL_TwainDib *pDib) : CTL_ImageIOHandler(pDib){}
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
    };

    #endif  // NOIMAGE_SUPPORT

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
            boost::optional<DWORD> GetBitsOffset() const;
            bool        IsGrayScale() const;
            bool        IsBlankDIB(double threshold) const;
            void        Delete();
            bool        FlipBitMap(bool bRGB=false);

            // Auto deletion flag
            CTL_TwainDib&  SetAutoDelete(bool bSet=true) {m_bAutoDelete = bSet; return *this; }
            CTL_TwainDib&  SetAutoDeletePalette(bool bSet = true) { m_bAutoDeletePalette = bSet; return *this; }
            bool        IsAutoDelete() const { return m_bAutoDelete; }
            bool        IsAutoDeletePalette() const { return m_bAutoDeletePalette; }

            // Read an image file
            static HANDLE      ReadDibBitmap(LPCSTR lpszFileName);
            // Write an image file
            int         WriteDibBitmap(DTWAINImageInfoEx& ImageInfo, LPCTSTR szFile, 
                                       int nFormat=BmpFormat, bool bOpenFile=true, int fh=0);

            // Write a multi-page DIB file
            CTL_ImageIOHandlerPtr WriteFirstPageDibMulti(DTWAINImageInfoEx& ImageInfo, LPCTSTR szFile, int nFormat,
                                                        bool bOpenFile, int fhFile, int &nStatus);
            int WriteNextPageDibMulti(CTL_ImageIOHandlerPtr& pImgHandler, int &nStatus,
                                      const DTWAINImageInfoEx& ImageInfo);
            static int WriteLastPageDibMulti(CTL_ImageIOHandlerPtr& pImgHandler, int &nStatus, bool bSaveFile=true);


            // Special JPEG stuff
            void SetJpegValues(int nQuality, bool bProgressive);

            // Crop a DIB
            int CropDib(const FloatRect& ActualRect, const FloatRect& RequestedRect,
                        LONG SourceUnit, LONG DestUnit, int dpi,
                        int flags);

            int ResampleDib(const FloatRect& ResampleRect, int flags);
            int ResampleDib(double xscale, double yscale);

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
            CTL_TwainDib(LPCSTR lpszFileName, HWND hWnd= nullptr);
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
            static int         DibNumColors(void* pv);
            void        Init();
            void        SetEqual(const CTL_TwainDib& rDib);

        private:
            CTL_TwainDibInfo m_TwainDibInfo;
            bool        m_bAutoDelete;
            bool        m_bAutoDeletePalette;
            bool        m_bIsValid;
            bool        m_bJpegProgressive;
            int         m_nJpegQuality;
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
            CTL_TwainDibPtr CreateDib(LPCSTR lpszFileName, HWND hWnd= nullptr);
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
            CTL_TwainDibPtr        GetAt(size_t nPos);
            CTL_TwainDibPtr        operator[](size_t nPos);

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
