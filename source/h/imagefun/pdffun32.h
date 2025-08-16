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

#ifndef PDFFUN32_H
#define PDFFUN32_H

#include <string>
#include <utility>
#include <vector>

#ifndef WINBIT32_H
#include "winbit32.h"
#endif
#include "ctlobstr.h"
#include "dibinfox.h"
#include "pdffont_basic.h"

namespace dynarithmic
{
    class PdfDocument;
    using PdfDocumentPtr = std::shared_ptr<PdfDocument>;
    class PDFInterface
    {
    public:
        PdfDocumentPtr DTWLIB_PDFGetNewDocument();
        bool DTWLIB_PDFOpenNewFile(PdfDocumentPtr pDoc, LPCTSTR szFile);
        void DTWLIB_PDFSetCompression(PdfDocumentPtr pDoc, bool bCompress);
        void DTWLIB_PDFSetNameField(PdfDocumentPtr pDoc, int32_t nWhich, const char* szName);
        bool DTWLIB_PDFStartCreation(PdfDocumentPtr pDoc, int majorv, int minorv);
        bool DTWLIB_PDFEndCreation(PdfDocumentPtr pDoc);
        void DTWLIB_PDFSetImageType(PdfDocumentPtr pDoc, int32_t iType);
        void DTWLIB_PDFSetLongField(PdfDocumentPtr pDoc, int32_t nWhich, int32_t nValue);
        bool DTWLIB_PDFWritePage(PdfDocumentPtr pDoc, LPCTSTR szPath);
        void DTWLIB_PDFSetScaling(PdfDocumentPtr pDoc, double xscale, double yscale);
        void DTWLIB_PDFReleaseDocument(PdfDocumentPtr pDoc);
        void DTWLIB_PDFSetThumbnailFile(PdfDocumentPtr pDoc, LPCTSTR szPath);
        void DTWLIB_PDFSetDPI(PdfDocumentPtr pDoc, int32_t dpi);
        void DTWLIB_PDFSetEncryption(PdfDocumentPtr pDoc, LPCTSTR szOwnerPass,
                                     LPCTSTR szUserPass, int32_t Permissions,
                                     bool bUseStrongEncrypt,
                                     bool bUseAESEncryption,
                                     uint32_t nKeyLength);

        LONG DTWLIB_PSWriteFile(LPCTSTR szFileIn,
            LPCTSTR szFileOut,
            LONG PSType,
            LPCTSTR szTitle,
            bool bUseEncapsulated);

        void DTWLIB_PDFSetASCIICompression(PdfDocumentPtr pDoc, bool bCompression);
        void DTWLIB_PDFSetNoCompression(PdfDocumentPtr pDoc, bool bCompression);
        void DTWLIB_PDFSetSearchableText(PdfDocumentPtr pDoc, const char* text);
        void DTWLIB_PDFAddPageText(PdfDocumentPtr pDoc, PDFTextElement* pElement); /*LPCSTR szTest, LONG xPos, LONG yPos,
                                   LPCSTR fontName, double fontSize, LONG colorRGB,
                                   LONG renderMode, double scaling, double charSpacing,
                                   double wordSpacing,
                                   LONG riseValue, LONG flags, double scalingX, double scalingY);*/
        void DTWLIB_PDFSetPolarity(PdfDocumentPtr pDoc, int32_t Polarity);
    };

    struct PDFINFO : DibMultiPageData
    {
        PDFINFO() : pPDFdoc(nullptr), nCurrentPage(0), IsFileOpened(false), IsPDFStarted(false) {}
        PdfDocumentPtr pPDFdoc;
        std::unique_ptr<PDFInterface> m_Interface;
        int nCurrentPage;
        bool IsFileOpened;
        bool IsPDFStarted;

        // PDF Information
        CTL_StringType sFileName;
        CTL_StringType sAuthor;
        CTL_StringType sProducer;
        CTL_StringType sTitle;
        CTL_StringType sSubject;
        CTL_StringType sKeywords;
        CTL_StringType sCreator;

        CTL_StringArrayType TempFileArray;
        DTWAINImageInfoEx ImageInfoEx;
    };
    #ifndef DTWAIN_LIMITED_VERSION
    class CPDFImageHandler : public CDibInterface
    {
        private:
            DTWAINImageInfoEx m_ImageInfoEx;

        public:
            CPDFImageHandler(CTL_StringType sFileName, DTWAINImageInfoEx ImageInfoEx);

            LONG GetErrorCode() const { return m_nError; }

            // Virtual interface
            std::string GetFileExtension() const override;
            HANDLE  GetFileInformation(LPCSTR path) override;
            int WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo= nullptr) override;
            int WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht, UINT32 bpp, UINT32 cpal, RGBQUAD *pPal, void *pUserInfo= nullptr) override;

            void SetMultiPageStatus(DibMultiPageStruct *pStruct) override;
            void GetMultiPageStatus(DibMultiPageStruct *pStruct) override;

            void SetAuthor(const std::string& s) { m_sAuthor = s; }
            void SetProducer(const std::string& s) { m_sProducer = s; }
            void SetTitle(const std::string& s) { m_sTitle = s; }
            void SetSubject(const std::string& s) { m_sSubject = s; }
            void SetKeywords(const std::string& s) { m_sKeywords = s; }
            void SetCreator(const std::string& s) {m_sCreator = s; }
            void SetImageType(int nWhich) { m_nImageType = nWhich; }
            int  GetImageType() const { return m_nImageType; }
            void SetThumbnailFile(CTL_StringType s) { m_sThumbnailFile = std::move(s); }
            void SetDPI(LONG dpi) { m_dpi = dpi; }
            LONG GetDPI() const { return m_dpi; }
            void SetSearchableText(const std::string& s);
            void AddPDFTextElement(PDFTextElementPtr element) const;

        protected:
            bool OpenOutputFile(LPCTSTR pFileName) override;
            static int  InitializePDFPage(const PDFINFO* pPDFInfo, HANDLE bitmap);
            static void RemoveAllImageFiles(PDFINFO *pPDFInfo);
            bool LoadPDFLibrary();

        private:

            CTL_StringType m_sFileName;
            std::string m_sAuthor;
            std::string m_sProducer;
            std::string m_sTitle;
            std::string m_sSubject;
            std::string m_sKeywords;
            CTL_StringType m_sThumbnailFile;
            std::string m_sCreator;
            std::string m_sSearchableText;

            int m_nImageType;
            LONG m_nError;
            LONG m_dpi;
    };

    class CPSImageHandler : public CPDFImageHandler
    {
        public:
              CPSImageHandler(CTL_StringType sFileName, const DTWAINImageInfoEx &ImageInfoEx) :
              CPDFImageHandler(std::move(sFileName), ImageInfoEx) { }

    };
}
#endif
#endif
