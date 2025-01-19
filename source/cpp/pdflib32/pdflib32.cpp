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
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include <pdflib32.h>
#include <dtwainpdf.h>
#include <pdfconst.h>

namespace dynarithmic
{
    PdfDocumentPtr PDFInterface::DTWLIB_PDFGetNewDocument()
    {
        return std::make_shared<PdfDocument>();
    }

    void PDFInterface::DTWLIB_PDFReleaseDocument(PdfDocumentPtr pDoc)
    {
    }

    bool PDFInterface::DTWLIB_PDFOpenNewFile(PdfDocumentPtr pDoc, LPCTSTR szFile)
    {
        return pDoc->OpenNewPDFFile(szFile);
    }

    void PDFInterface::DTWLIB_PDFSetCompression(PdfDocumentPtr pDoc, bool bCompress)
    {
        pDoc->SetCompression(bCompress);
    }

    void PDFInterface::DTWLIB_PDFSetNameField(PdfDocumentPtr pDoc, int32_t nWhich, const char *szName)
    {
    switch (nWhich)
    {
        case PDF_AUTHOR:
            pDoc->SetAuthor(szName);
        break;
        case PDF_PRODUCER:
            pDoc->SetProducer(szName);
        break;
        case PDF_TITLE:
            pDoc->SetTitle(szName);
        break;
        case PDF_KEYWORDS:
            pDoc->SetKeywords(szName);
        break;
        case PDF_SUBJECT:
            pDoc->SetSubject(szName);
        break;
        case PDF_MEDIABOX:
            pDoc->SetMediaBox(szName);
        break;
        case PDF_CREATOR:
            pDoc->SetCreator(szName);
        break;
        default: ;
    }
}

    bool PDFInterface::DTWLIB_PDFStartCreation(PdfDocumentPtr pDoc, int majorv, int minorv)
    {
        return pDoc->StartPDFCreation(majorv, minorv);
    }

    bool PDFInterface::DTWLIB_PDFEndCreation(PdfDocumentPtr pDoc)
    {
        return pDoc->EndPDFCreation();
    }

    void PDFInterface::DTWLIB_PDFSetImageType(PdfDocumentPtr pDoc, int32_t iType)
    {
        pDoc->SetImageType(iType);
    }

    bool PDFInterface::DTWLIB_PDFWritePage(PdfDocumentPtr pDoc, LPCTSTR szPath)
    {
        return pDoc->WritePage(szPath);
    }

    void PDFInterface::DTWLIB_PDFSetLongField(PdfDocumentPtr pDoc, int32_t nWhich, int32_t nValue)
    {
    switch (nWhich)
    {
        case PDF_ORIENTATION:
            pDoc->SetOrientation(nValue);
        break;

        case PDF_MEDIABOX:
            pDoc->SetMediaBox(nValue);
        break;

        case PDF_SCALETYPE:
            pDoc->SetScaleType(nValue);
        break;
        default: ;
    }
}

    void PDFInterface::DTWLIB_PDFSetScaling(PdfDocumentPtr pDoc, double xscale, double yscale)
    {
        pDoc->SetScaling(xscale, yscale);
    }

    void PDFInterface::DTWLIB_PDFSetThumbnailFile(PdfDocumentPtr pDoc, LPCTSTR szPath)
    {
        pDoc->SetThumbnailFile(szPath);
    }

    void PDFInterface::DTWLIB_PDFSetDPI(PdfDocumentPtr pDoc, int32_t dpi)
    {
        pDoc->SetDPI(dpi);
    }

    void PDFInterface::DTWLIB_PDFSetEncryption(PdfDocumentPtr pDoc, LPCTSTR szOwnerPass,
                                LPCTSTR szUserPass, int32_t Permissions,
                                          bool bUseStrongEncrypt,
                                          bool bUseAESEncryption)
{
        pDoc->SetEncryption(szOwnerPass ? szOwnerPass : _T(""),
                                                   szUserPass?szUserPass:_T(""),
                                                   Permissions,
                                                   bUseStrongEncrypt,
                                                   bUseAESEncryption);
}

    void PDFInterface::DTWLIB_PDFSetASCIICompression(PdfDocumentPtr pDoc, bool bSetCompression)
    {
        pDoc->SetASCIICompression(bSetCompression);
    }

    void PDFInterface::DTWLIB_PDFSetNoCompression(PdfDocumentPtr pDoc, bool bSetCompression)
    {
        pDoc->SetNoCompression(bSetCompression);
    }

    void PDFInterface::DTWLIB_PDFSetSearchableText(PdfDocumentPtr pDoc, const char *text)
    {
        pDoc->SetSearchableText(text);
    }

    void PDFInterface::DTWLIB_PDFAddPageText(PdfDocumentPtr pDoc, PDFTextElement* pElement)
{
/*    PDFTextElement element;

    element.m_text = szText;
    element.xpos = xPos;
    element.ypos = yPos;
    element.m_font.m_fontName = fontName;
    element.fontSize = fontSize;
    element.colorRGB = colorRGB;
    element.renderMode = renderMode;
    element.strokeWidth = strokeWidth;
    element.scaling = scaling;
    element.wordSpacing = wordSpacing;
    element.charSpacing = charSpacing;
    element.displayFlags = flags;
    element.scalingX = scalingX;
    element.scalingY = scalingY;
*/
        pDoc->AddTextElement(pElement);
}

    void PDFInterface::DTWLIB_PDFSetPolarity(PdfDocumentPtr pDoc, int32_t Polarity)
{
        pDoc->SetPolarity(Polarity);
    }
}