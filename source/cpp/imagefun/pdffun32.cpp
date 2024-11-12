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
#ifdef _MSC_VER
#pragma warning (disable:4786)
#endif
#include "pdffun32.h"
#undef min
#undef max
#include <string>
#include <cmath>
#include <pdfconst.h>
#include <utility>
#include <memory>
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "ctlfileutils.h"
#include "dtwain_float_utils.h"
#include "logwriterutils.h"

using namespace dynarithmic;

CPDFImageHandler::CPDFImageHandler(CTL_StringType sFileName, DTWAINImageInfoEx ImageInfoEx) :
        CDibInterface(), m_ImageInfoEx(std::move(ImageInfoEx)),
        m_sFileName(std::move(sFileName)),
        m_sAuthor("(None)"),
        m_sProducer("(None)"),
        m_sTitle("(None)"),
        m_sSubject("(None)"),
        m_sKeywords("(None)"),
        m_sCreator("(None)"),
        m_nImageType(0),
        m_nError{},
        m_dpi(72)
{
    LoadPDFLibrary();
}

bool CPDFImageHandler::LoadPDFLibrary()
{
    m_nError = 0;
    return true;
}



std::string CPDFImageHandler::GetFileExtension() const
{
    return "PDF";
}

HANDLE CPDFImageHandler::GetFileInformation(LPCSTR /*path*/)
{
    return nullptr;
}

bool CPDFImageHandler::OpenOutputFile(LPCTSTR /*pFileName*/)
{
    return true;
}

int CPDFImageHandler::WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo)
{
    int retval = 0;
    std::shared_ptr<PDFINFO> pPDFInfo;

    if ( m_MultiPageStruct.Stage == DIB_MULTI_FIRST || m_MultiPageStruct.Stage == 0 )
    {
        pPDFInfo = std::make_shared<PDFINFO>();
        pPDFInfo->m_Interface = std::make_unique<PDFInterface>();
        pPDFInfo->ImageInfoEx = m_ImageInfoEx;
        m_MultiPageStruct.pUserData = pPDFInfo;

        // Open the file, return if there is an error
        auto pDocument = pPDFInfo->m_Interface->DTWLIB_PDFGetNewDocument();

        if ( !pDocument || !pPDFInfo->m_Interface->DTWLIB_PDFOpenNewFile(pDocument, m_sFileName.c_str()) )
        {
            pPDFInfo->IsFileOpened = false;
            pPDFInfo->IsPDFStarted = false;
            if ( pDocument )
                pPDFInfo->m_Interface->DTWLIB_PDFReleaseDocument(pDocument);
            pPDFInfo.reset();
            return DTWAIN_ERR_FILEWRITE;
        }

        pPDFInfo->IsFileOpened = true;
        pPDFInfo->nCurrentPage  = 1;
        m_MultiPageStruct.pUserData = pPDFInfo;
        pPDFInfo->sFileName     = m_sFileName;
        pPDFInfo->m_Interface->DTWLIB_PDFSetCompression(pDocument, false);
        pPDFInfo->m_Interface->DTWLIB_PDFSetNoCompression(pDocument, false);

        // Set the ASCII Hex compression
        pPDFInfo->m_Interface->DTWLIB_PDFSetASCIICompression(pDocument, pPDFInfo->ImageInfoEx.PDFUseASCIICompression);

        // turn on other compression flags in the PDF object
        if ( pPDFInfo->ImageInfoEx.PDFUseCompression )
            pPDFInfo->m_Interface->DTWLIB_PDFSetCompression(pDocument, true);   // Use Flate compression
        else
            pPDFInfo->m_Interface->DTWLIB_PDFSetNoCompression(pDocument, true); // Use no compression

        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pDocument, PDF_AUTHOR, m_sAuthor.c_str());
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pDocument, PDF_PRODUCER, m_sProducer.c_str());
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pDocument, PDF_TITLE, m_sTitle.c_str());
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pDocument, PDF_KEYWORDS, m_sKeywords.c_str());
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pDocument, PDF_SUBJECT, m_sSubject.c_str());
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pDocument, PDF_CREATOR, m_sCreator.c_str());

        if ( !pPDFInfo->m_Interface->DTWLIB_PDFStartCreation(pDocument) )
        {
            pPDFInfo->IsPDFStarted = false;
            pPDFInfo->m_Interface->DTWLIB_PDFReleaseDocument (pDocument);
            pPDFInfo.reset();
            return DTWAIN_ERR_FILEWRITE;
        }

        auto& imageinfo = pPDFInfo->ImageInfoEx;
        pPDFInfo->m_Interface->DTWLIB_PDFSetPolarity(pDocument, imageinfo.nPDFPolarity);
        // Test the encryption here
        if ( pPDFInfo->ImageInfoEx.bIsPDFEncrypted)
        {
            pPDFInfo->m_Interface->DTWLIB_PDFSetEncryption(pDocument,
                                                            imageinfo.PDFOwnerPassword.c_str(),
                                                            imageinfo.PDFUserPassword.c_str(),
                                                            imageinfo.PDFPermissions,
                                                            imageinfo.bUseStrongEncryption?TRUE:false,
                                                            imageinfo.bIsAESEncrypted?TRUE:FALSE);
        }

        pPDFInfo->IsPDFStarted = true;
        pPDFInfo->pPDFdoc = pDocument;
    }
    else
    if ( m_MultiPageStruct.Stage == DIB_MULTI_LAST )
    {
        pPDFInfo = std::dynamic_pointer_cast<PDFINFO>(m_MultiPageStruct.pUserData);
        if ( !pPDFInfo )
            return DTWAIN_ERR_FILEWRITE;
        try
        {
            if ( pPDFInfo->IsPDFStarted )
                pPDFInfo->m_Interface->DTWLIB_PDFEndCreation(pPDFInfo->pPDFdoc);
            if ( pPDFInfo->pPDFdoc )
            {
                pPDFInfo->m_Interface->DTWLIB_PDFReleaseDocument(pPDFInfo->pPDFdoc);
            }

            retval = 0;
            if ( !pPDFInfo->IsPDFStarted ||
                 !pPDFInfo->IsFileOpened )
                 retval = DTWAIN_ERR_FILEWRITE;
            pPDFInfo.reset();
            return retval;
        }
        catch(...)
        {
            if ( pPDFInfo->IsPDFStarted )
                pPDFInfo->m_Interface->DTWLIB_PDFEndCreation(pPDFInfo->pPDFdoc);
            if ( pPDFInfo->pPDFdoc )
            {
                pPDFInfo->m_Interface->DTWLIB_PDFReleaseDocument(pPDFInfo->pPDFdoc);
                pPDFInfo.reset();
            }
            return DTWAIN_ERR_FILEWRITE;
        }
    }
    else
    if ( m_MultiPageStruct.Stage == DIB_MULTI_NEXT)
    {
        pPDFInfo = std::dynamic_pointer_cast<PDFINFO>(m_MultiPageStruct.pUserData);
        if ( !pPDFInfo )
            return DTWAIN_ERR_FILEWRITE;
        pPDFInfo->nCurrentPage++;
        pPDFInfo->ImageInfoEx = *static_cast<DTWAINImageInfoEx*>(pUserInfo);
        if ( !pPDFInfo->IsFileOpened || !pPDFInfo->IsPDFStarted )
        {
            pPDFInfo.reset();
            return DTWAIN_ERR_FILEWRITE;
        }
    }
    else
    if ( !pPDFInfo )
        return DTWAIN_ERR_FILEWRITE;

    // Initialize the page dimensions depending on the image information
    retval = InitializePDFPage(pPDFInfo.get(), bitmap);
    // Set the thumbnail if used
    if ( pPDFInfo->ImageInfoEx.PDFUseThumbnail )
    {
        pPDFInfo->m_Interface->DTWLIB_PDFSetThumbnailFile(pPDFInfo->pPDFdoc, m_sThumbnailFile.c_str());
    }

    pPDFInfo->m_Interface->DTWLIB_PDFSetImageType(pPDFInfo->pPDFdoc, m_nImageType);
    if ( m_nImageType == 0 )
    {
        pPDFInfo->m_Interface->DTWLIB_PDFSetDPI(pPDFInfo->pPDFdoc, pPDFInfo->ImageInfoEx.ResolutionX);
    }

    // Set any other text to write (searchable text is included in this)
    if (m_MultiPageStruct.Stage != DIB_MULTI_LAST)
    {
        const auto pSource = pPDFInfo->ImageInfoEx.theSource;
        const auto pHandle = pSource->GetDTWAINHandle();
        const auto iter = pHandle->m_mapPDFTextElement.find(pSource);
        if (iter != pHandle->m_mapPDFTextElement.end())
        {
            auto it = iter->second.begin();
            const auto it2 = iter->second.end();
            while (it != it2 )
            {
                pPDFInfo->m_Interface->DTWLIB_PDFAddPageText(pPDFInfo->pPDFdoc, it->get());
                ++it;
            }
        }
    }

    if (!pPDFInfo->m_Interface->DTWLIB_PDFWritePage(pPDFInfo->pPDFdoc, path))
    {
        delete_file(path);
        pPDFInfo->m_Interface->DTWLIB_PDFReleaseDocument(pPDFInfo->pPDFdoc);
        pPDFInfo.reset();
        return DTWAIN_ERR_FILEWRITE;
    }

    // Add the file to the array for later deletion
    // delete the temporary file
    delete_file(path);

    if ( m_MultiPageStruct.Stage == 0)
    {
        pPDFInfo->m_Interface->DTWLIB_PDFEndCreation(pPDFInfo->pPDFdoc);
        pPDFInfo->m_Interface->DTWLIB_PDFReleaseDocument(pPDFInfo->pPDFdoc);
        RemoveAllImageFiles(pPDFInfo.get());
        pPDFInfo.reset();
    }

    return retval;
}


int CPDFImageHandler::InitializePDFPage(const PDFINFO* pPDFInfo, HANDLE bitmap)
{
    // Initialize the page.

    // Get the orientation
    int rotation = pPDFInfo->ImageInfoEx.PDFOrientation;
    std::string sDimensions;

    if ( rotation != DTWAIN_PDF_PORTRAIT &&
         rotation != DTWAIN_PDF_LANDSCAPE )
         rotation = DTWAIN_PDF_PORTRAIT;

    // Set the rotation
    pPDFInfo->m_Interface->DTWLIB_PDFSetLongField(pPDFInfo->pPDFdoc, PDF_ORIENTATION, rotation);

    // Check if normal paper size is specified
    if ( !(pPDFInfo->ImageInfoEx.PDFPageSize == DTWAIN_PDF_CUSTOMSIZE) &&
         !(pPDFInfo->ImageInfoEx.PDFPageSize == DTWAIN_PDF_VARIABLEPAGESIZE) &&
         !(pPDFInfo->ImageInfoEx.PDFPageSize == DTWAIN_PDF_PIXELSPERMETERSIZE))
    {
        // One of the default page sized (A4, USLETTER, etc.)
        pPDFInfo->m_Interface->DTWLIB_PDFSetLongField(pPDFInfo->pPDFdoc, PDF_MEDIABOX, pPDFInfo->ImageInfoEx.PDFPageSize);
    }
    else
    if ( pPDFInfo->ImageInfoEx.PDFPageSize == DTWAIN_PDF_CUSTOMSIZE )
    {
        // Dimensions specified by the user
        std::ostringstream sBuf;
        sBuf << "[0 0 " << static_cast<int>(pPDFInfo->ImageInfoEx.PDFCustomSize[0]) << " " << static_cast<int>(pPDFInfo->ImageInfoEx.PDFCustomSize[1]) << "]";
        sDimensions = sBuf.str();
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_MEDIABOX, sDimensions.c_str());
    }
    else
    if ( pPDFInfo->ImageInfoEx.PDFPageSize == DTWAIN_PDF_VARIABLEPAGESIZE )
    {
        pPDFInfo->m_Interface->DTWLIB_PDFSetLongField(pPDFInfo->pPDFdoc, PDF_MEDIABOX, -1);
    }
    else
    {
        // Determine the size of the page, given the DIB dimensions and bytes per meter
        auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(bitmap));

        const auto xInches = static_cast<double>(pbi->biXPelsPerMeter) / 39.37;
        const auto yInches = static_cast<double>(pbi->biYPelsPerMeter) / 39.37;
        const auto xWidth = static_cast<double>(pbi->biWidth);
        const auto yHeight = static_cast<double>(pbi->biHeight);

        if ( float_equal(xInches,0.0) || float_equal(yInches, 0.0) )
        {
            ImageMemoryHandler::GlobalUnlock(bitmap);
            return DTWAIN_ERR_BAD_DIB_PAGE; // this page cannot be created due to improper pels per meter
        }

        const double widthInPoints = xWidth / xInches * 72.0;
        const double heightInPoints = yHeight / yInches * 72.0;

        // Dimensions specified by the user
        std::ostringstream sBuf;
        sBuf << "[0 0 " << widthInPoints << " " << heightInPoints << "]";
        sDimensions = sBuf.str();
        pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_MEDIABOX, sDimensions.c_str());

        if ( CTL_StaticData::GetLogFilterFlags() )
        {
            std::string sOut = "PDF Computed media box: ";
            sOut += sDimensions;
            LogWriterUtils::WriteLogInfoIndentedA(sOut);
        }
    }

    // This will set the scaling
    // Best fit overrides all scale types
    if ( pPDFInfo->ImageInfoEx.PDFScaleType == DTWAIN_PDF_FITPAGE )
    {
        pPDFInfo->m_Interface->DTWLIB_PDFSetLongField(pPDFInfo->pPDFdoc, PDF_SCALETYPE, DTWAIN_PDF_FITPAGE);
    }
    else
    if ( pPDFInfo->ImageInfoEx.PDFScaleType == DTWAIN_PDF_NOSCALING )
    {
        pPDFInfo->m_Interface->DTWLIB_PDFSetLongField(pPDFInfo->pPDFdoc, PDF_SCALETYPE, DTWAIN_PDF_NOSCALING);
    }
    else
    if ( pPDFInfo->ImageInfoEx.PDFScaleType == DTWAIN_PDF_CUSTOMSCALE )
    {
        pPDFInfo->m_Interface->DTWLIB_PDFSetLongField(pPDFInfo->pPDFdoc, PDF_SCALETYPE, DTWAIN_PDF_CUSTOMSCALE);
        pPDFInfo->m_Interface->DTWLIB_PDFSetScaling(pPDFInfo->pPDFdoc,pPDFInfo->ImageInfoEx.PDFCustomScale[0],
                                      pPDFInfo->ImageInfoEx.PDFCustomScale[1]);

    }

    pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_AUTHOR,   std::string("(" + StringConversion::Convert_Native_To_Ansi(pPDFInfo->ImageInfoEx.PDFAuthor) + ")").c_str());
    pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_PRODUCER, std::string("(" + StringConversion::Convert_Native_To_Ansi(pPDFInfo->ImageInfoEx.PDFProducer) + ")").c_str());
    pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_KEYWORDS, std::string("(" + StringConversion::Convert_Native_To_Ansi(pPDFInfo->ImageInfoEx.PDFKeywords) + ")").c_str());
    pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_TITLE,    std::string("(" + StringConversion::Convert_Native_To_Ansi(pPDFInfo->ImageInfoEx.PDFTitle) + ")").c_str());
    pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_SUBJECT,  std::string("(" + StringConversion::Convert_Native_To_Ansi(pPDFInfo->ImageInfoEx.PDFSubject) + ")").c_str());
    pPDFInfo->m_Interface->DTWLIB_PDFSetNameField(pPDFInfo->pPDFdoc, PDF_CREATOR,  std::string("(" + StringConversion::Convert_Native_To_Ansi(pPDFInfo->ImageInfoEx.PDFCreator) + ")").c_str());
    return 0;
}

void CPDFImageHandler::RemoveAllImageFiles(PDFINFO *pPDFInfo)
{
    auto it = pPDFInfo->TempFileArray.begin();
    while (it != pPDFInfo->TempFileArray.end())
    {
        delete_file((*it).c_str());
        ++it;
    }
}

void CPDFImageHandler::SetMultiPageStatus(DibMultiPageStruct *pStruct)
{

    if ( pStruct )
        m_MultiPageStruct = *pStruct;
}


void CPDFImageHandler::GetMultiPageStatus(DibMultiPageStruct *pStruct)
{
    *pStruct = m_MultiPageStruct;
}

int CPDFImageHandler::WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE * /*pImage2*/, UINT32 /*wid*/, UINT32 /*ht*/,
                                 UINT32 /*bpp*/, UINT32 /*nColors*/, RGBQUAD * /*pPal*/, void * /*pUserInfo*/)
{
    return 0;
}

void CPDFImageHandler::SetSearchableText(const std::string& sText)
{
    m_sSearchableText = sText;
}

void CPDFImageHandler::AddPDFTextElement(PDFTextElementPtr element) const
{
    m_ImageInfoEx.theSource->SetPDFValue(StringConversion::Convert_Ansi_To_Native("PDFTEXTELEMENTKEY"), element);
}
