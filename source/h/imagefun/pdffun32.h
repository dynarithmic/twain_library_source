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

#ifndef PDFFUN32_H
#define PDFFUN32_H

#include <string>
#include <utility>
#include <vector>

#ifndef WINBIT32_H
#include "winbit32.h"
#endif
#include "ctlstringdefs.h"
#include "dibinfox.h"
#include "pdffont_basic.h"

namespace dynarithmic
{
    class PdfDocument;
    using PdfDocumentPtr = std::shared_ptr<PdfDocument>;
    class PDFInterface
    {
        public:
            static PdfDocumentPtr DTWLIB_PDFGetNewDocument();
            static bool DTWLIB_PDFOpenNewFile(const PdfDocumentPtr& pDoc, LPCTSTR szFile);
            static void DTWLIB_PDFSetCompression(const PdfDocumentPtr& pDoc, bool bCompress);
            static void DTWLIB_PDFSetNameField(const PdfDocumentPtr& pDoc, int32_t nWhich, const char* szName);
            static bool DTWLIB_PDFStartCreation(const PdfDocumentPtr& pDoc, int majorv, int minorv);
            static bool DTWLIB_PDFEndCreation(const PdfDocumentPtr& pDoc);
            static void DTWLIB_PDFSetImageType(const PdfDocumentPtr& pDoc, int32_t iType);
            static void DTWLIB_PDFSetLongField(const PdfDocumentPtr& pDoc, int32_t nWhich, int32_t nValue);
            static bool DTWLIB_PDFWritePage(const PdfDocumentPtr& pDoc, LPCTSTR szPath);
            static void DTWLIB_PDFSetScaling(const PdfDocumentPtr& pDoc, double xscale, double yscale);
            static void DTWLIB_PDFReleaseDocument(PdfDocumentPtr pDoc);
            static void DTWLIB_PDFSetThumbnailFile(const PdfDocumentPtr& pDoc, LPCTSTR szPath);
            static void DTWLIB_PDFSetDPI(const PdfDocumentPtr& pDoc, int32_t dpi);
            static void DTWLIB_PDFSetEncryption(const PdfDocumentPtr& pDoc, LPCTSTR szOwnerPass,
                                                LPCTSTR szUserPass, int32_t Permissions,
                                                bool bUseStrongEncrypt,
                                                bool bUseAESEncryption,
                                                uint32_t nKeyLength);

            static void DTWLIB_PDFSetASCIICompression(const PdfDocumentPtr& pDoc, bool bCompression);
            static void DTWLIB_PDFSetNoCompression(const PdfDocumentPtr& pDoc, bool bCompression);
            static void DTWLIB_PDFSetSearchableText(const PdfDocumentPtr& pDoc, const char* text);
            static void DTWLIB_PDFAddPageText(const PdfDocumentPtr& pDoc, PDFTextElement* pElement);
            static void DTWLIB_PDFSetPolarity(const PdfDocumentPtr& pDoc, int32_t Polarity);
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
            int WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo= nullptr) override;
            int WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht, UINT32 bpp, UINT32 cpal, RGBQUAD *pPal, void *pUserInfo= nullptr) override;

            void SetMultiPageStatus(DibMultiPageStruct *pStruct) override;
            void GetMultiPageStatus(DibMultiPageStruct *pStruct) override;

            void SetAuthor(std::string_view s) { m_sAuthor = s; }
            void SetProducer(std::string_view s) { m_sProducer = s; }
            void SetTitle(std::string_view s) { m_sTitle = s; }
            void SetSubject(std::string_view s) { m_sSubject = s; }
            void SetKeywords(std::string_view s) { m_sKeywords = s; }
            void SetCreator(std::string_view s) {m_sCreator = s; }
            void SetImageType(int nWhich) { m_nImageType = nWhich; }
            int  GetImageType() const { return m_nImageType; }
            void SetThumbnailFile(CTL_StringType s) { m_sThumbnailFile = std::move(s); }
            void SetDPI(LONG dpi) { m_dpi = dpi; }
            LONG GetDPI() const { return m_dpi; }
            void SetSearchableText(std::string_view s);
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
