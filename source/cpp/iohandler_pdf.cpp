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
#include <unordered_map>
#include <array>
#include "ctldib.h"
#include "arrayfactory.h"
#include "ctliface.h"
#include "ctltwmgr.h"
#include "ctlfileutils.h"

using namespace dynarithmic;

struct PDFDimensions
{
    std::array<double,4> mediabox{};
    PDFDimensions() : mediabox{ 0,0,0,0 } { }

    PDFDimensions(double d1, double d2, double d3, double d4) : mediabox {}
    {
        SetDimensions(d1, d2, d3, d4);
    }

    void SetDimensions(double d1, double d2, double d3, double d4)
    {
        mediabox[0] = d1; mediabox[1] = d2; mediabox[2] = d3; mediabox[3] = d4;
    }

    void GetDimensions(double& d1, double &d2, double& d3, double& d4) const
    {
        d1 = mediabox[0]; d2 = mediabox[1]; d3 = mediabox[2]; d4 = mediabox[3];
    }
};

typedef std::pair<double, double> PDFPosition;

typedef std::unordered_map<LONG, PDFDimensions> PDFDimensionsMap;
typedef std::vector<PDFTextElement> PDFStringToTextElement;

struct AllPDFDimensions
{
    PDFDimensionsMap m_mediaMap;
    AllPDFDimensions()
    {
        m_mediaMap[ DTWAIN_FS_USLETTER    ] =  PDFDimensions(0,0, 612.00, 792.00);
        m_mediaMap[ DTWAIN_FS_USLEGAL     ] =  PDFDimensions(0,0, 612.00, 1008.00);
        m_mediaMap[ DTWAIN_FS_USEXECUTIVE ] =  PDFDimensions(0,0, 521.86, 756.00);
        m_mediaMap[ DTWAIN_FS_USLEDGER    ] =  PDFDimensions(0,0, 792.00, 1224.00);
        m_mediaMap[ DTWAIN_FS_USSTATEMENT ] =  PDFDimensions(0,0, 396.00, 792.00);
        m_mediaMap[ DTWAIN_FS_BUSINESSCARD] =  PDFDimensions(0,0, 144.00, 290.00);
        m_mediaMap[ DTWAIN_FS_4A0         ] =  PDFDimensions(0,0, 4767.87, 6740.79);
        m_mediaMap[ DTWAIN_FS_2A0         ] =  PDFDimensions(0,0, 3370.39, 4767.87);
        m_mediaMap[ DTWAIN_FS_A0          ] =  PDFDimensions(0,0, 2383.94, 3370.39);
        m_mediaMap[ DTWAIN_FS_A1          ] =  PDFDimensions(0,0, 1683.78, 2383.94);
        m_mediaMap[ DTWAIN_FS_A2          ] =  PDFDimensions(0,0, 1190.55, 1683.78);
        m_mediaMap[ DTWAIN_FS_A3          ] =  PDFDimensions(0,0, 841.89, 1190.55);
        m_mediaMap[ DTWAIN_FS_A4          ] =  PDFDimensions(0,0, 595.28, 841.89);
        m_mediaMap[ DTWAIN_FS_A5          ] =  PDFDimensions(0,0, 419.53, 595.28);
        m_mediaMap[ DTWAIN_FS_A6          ] =  PDFDimensions(0,0, 297.64, 419.53);
        m_mediaMap[ DTWAIN_FS_A7          ] =  PDFDimensions(0,0, 209.76, 297.64);
        m_mediaMap[ DTWAIN_FS_A8          ] =  PDFDimensions(0,0, 147.40, 209.76);
        m_mediaMap[ DTWAIN_FS_A9          ] =  PDFDimensions(0,0, 104.88, 147.40);
        m_mediaMap[ DTWAIN_FS_A10         ] =  PDFDimensions(0,0, 73.70, 104.88);
        m_mediaMap[ DTWAIN_FS_ISOB0       ] =  PDFDimensions(0,0, 2834.65, 4008.19);
        m_mediaMap[ DTWAIN_FS_ISOB1       ] =  PDFDimensions(0,0, 2004.09, 2834.65);
        m_mediaMap[ DTWAIN_FS_ISOB2       ] =  PDFDimensions(0,0, 1417.32, 2004.09);
        m_mediaMap[ DTWAIN_FS_ISOB3       ] =  PDFDimensions(0,0, 1000.63, 1417.32);
        m_mediaMap[ DTWAIN_FS_ISOB4       ] =  PDFDimensions(0,0, 708.66, 1000.63);
        m_mediaMap[ DTWAIN_FS_ISOB5       ] =  PDFDimensions(0,0, 498.90, 708.66);
        m_mediaMap[ DTWAIN_FS_ISOB6       ] =  PDFDimensions(0,0, 354.33, 498.90);
        m_mediaMap[ DTWAIN_FS_ISOB7       ] =  PDFDimensions(0,0, 249.45, 354.33);
        m_mediaMap[ DTWAIN_FS_ISOB8       ] =  PDFDimensions(0,0, 175.75, 249.45);
        m_mediaMap[ DTWAIN_FS_ISOB9       ] =  PDFDimensions(0,0, 124.72, 175.75);
        m_mediaMap[ DTWAIN_FS_ISOB10      ] =  PDFDimensions(0,0, 87.87, 124.72);
        m_mediaMap[ DTWAIN_FS_C0          ] =  PDFDimensions(0,0, 2599.37, 3676.54);
        m_mediaMap[ DTWAIN_FS_C1          ] =  PDFDimensions(0,0, 1836.85, 2599.37);
        m_mediaMap[ DTWAIN_FS_C2          ] =  PDFDimensions(0,0, 1298.27, 1836.85);
        m_mediaMap[ DTWAIN_FS_C3          ] =  PDFDimensions(0,0, 918.43, 1298.27);
        m_mediaMap[ DTWAIN_FS_C4          ] =  PDFDimensions(0,0, 649.13, 918.43);
        m_mediaMap[ DTWAIN_FS_C5          ] =  PDFDimensions(0,0, 459.21, 649.13);
        m_mediaMap[ DTWAIN_FS_C6          ] =  PDFDimensions(0,0, 323.15, 459.21);
        m_mediaMap[ DTWAIN_FS_C7          ] =  PDFDimensions(0,0, 229.61, 323.15);
        m_mediaMap[ DTWAIN_FS_C8          ] =  PDFDimensions(0,0, 161.57, 229.61);
        m_mediaMap[ DTWAIN_FS_C9          ] =  PDFDimensions(0,0, 113.39, 161.57);
        m_mediaMap[ DTWAIN_FS_C10         ] =  PDFDimensions(0,0, 79.37, 113.39);
        m_mediaMap[ DTWAIN_FS_JISB0       ] =  PDFDimensions(0,0, 2923.2, 4125.6);
        m_mediaMap[ DTWAIN_FS_JISB1       ] =  PDFDimensions(0,0, 2066.4, 2923.2);
        m_mediaMap[ DTWAIN_FS_JISB2       ] =  PDFDimensions(0,0, 1461.6, 2066.4);
        m_mediaMap[ DTWAIN_FS_JISB3       ] =  PDFDimensions(0,0, 1029.6, 1461.6);
        m_mediaMap[ DTWAIN_FS_JISB4       ] =  PDFDimensions(0,0, 727.2,  1029.6);
        m_mediaMap[ DTWAIN_FS_JISB5       ] =  PDFDimensions(0,0, 518.4,  727.2);
        m_mediaMap[ DTWAIN_FS_JISB6       ] =  PDFDimensions(0,0, 360,  518.4);
        m_mediaMap[ DTWAIN_FS_JISB7       ] =  PDFDimensions(0,0, 259.2, 360);
        m_mediaMap[ DTWAIN_FS_JISB8       ] =  PDFDimensions(0,0, 180, 259.2);
        m_mediaMap[ DTWAIN_FS_JISB9       ] =  PDFDimensions(0,0, 129.6, 180);
        m_mediaMap[ DTWAIN_FS_JISB10      ] =  PDFDimensions(0,0, 93.6, 129.6);
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTL_PDFIOHandler::CTL_PDFIOHandler(CTL_TwainDib* pDib, int /*nFormat*/, const DTWAINImageInfoEx &ImageInfoEx)
: CTL_ImageIOHandler( pDib ), m_nFormat(0),
        m_ImageInfoEx(ImageInfoEx),
        m_JpegHandler(pDib, m_ImageInfoEx), m_TiffHandler(pDib, CTL_TwainDib::TiffFormatNONE, m_ImageInfoEx)
{
    // Create a JPEG and TIFF handler locally
    m_ImageInfoEx.IsPDF = true;
}

static char CleanupOCRText1(char ch)
{
    if ( !isprint(ch))
        return ' ';
    return ch;
}

struct OCRTextInfo
{
    std::vector<LONG> xPos;
    std::vector<LONG> yPos;
    std::vector<LONG> xDim;
    std::vector<LONG> yDim;
    std::string OCRChar;
    PDFDimensions m_RealDimensions;
    PDFDimensions m_ScaledDimensions;
    std::pair<LONG, LONG> m_ImageDimensions;
    std::pair<double, double> m_PDFScalingFactorForPage;

    void GetCharacterPositionAndHeightInPDF(LONG nChar, PDFPosition& pdfPosition, double& fontHeight, double& fontWidth)
    {
        double xP = m_RealDimensions.mediabox[2] * xPos[nChar] / m_ImageDimensions.first;  // get PDF x-position of character
        double yP = m_RealDimensions.mediabox[3] * yPos[nChar] / m_ImageDimensions.second; // get PDF y-position of character
        yP = m_RealDimensions.mediabox[3] - yP;
        fontHeight = m_RealDimensions.mediabox[3] * yDim[nChar] / m_ImageDimensions.second * 1.5; // get font height of character
        fontWidth = m_RealDimensions.mediabox[2] * xDim[nChar] / m_ImageDimensions.first * 1.5;
        yP -= fontHeight/2.0;  // adjust y-position per PDF coordinate system
        pdfPosition = std::make_pair(xP, yP);  // make the pair and get out
    }

    double FontWidthFromPixelSize(LONG PixelSize) const
    { return m_RealDimensions.mediabox[2] * PixelSize / m_ImageDimensions.first * 1.5; }

    double FontHeightFromPixelSize(LONG PixelSize) const
    { return m_RealDimensions.mediabox[3] * PixelSize / m_ImageDimensions.second * 1.5; }

    PDFPosition PDFPositionFromPixelPosition(LONG x, LONG y, double fontHeight) const
    {
        double xP = m_RealDimensions.mediabox[2] * x / m_ImageDimensions.first;  // get PDF x-position of character
        double yP = m_RealDimensions.mediabox[3] * y / m_ImageDimensions.second; // get PDF y-position of character
        yP = m_RealDimensions.mediabox[3] - yP;
        yP -= fontHeight / 2.0;
        return std::make_pair(xP, yP);
    }
};

static bool GetOCRCharacterInformation( OCREngine* pEngine, OCRTextInfo& tInfo, HANDLE hBitmap, const DTWAINImageInfoEx& imageInfoEx );
static PDFStringToTextElement CreatePDFTextElementMap(OCRTextInfo& tInfo);

static void InitializeDisplayStatus(PDFTextElementPtr& pElement)
{
    pElement->hasBeenDisplayed = false;
}

struct PDFTextElementEraser
{
    PDFTextElementEraser(LONG Flags) : m_Flags(Flags) {}
    bool operator()(const PDFTextElementPtr& pElement) const
    {
        return pElement->displayFlags & m_Flags?true:false;
    }

    LONG m_Flags;
};

int CTL_PDFIOHandler::WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fhFile, DibMultiPageStruct* pMultiPageStruct)
{
    // Now add this to PDF page
    CPDFImageHandler PDFHandler(szFile, m_ImageInfoEx);
    CTL_StringType szTempFile;
    const auto pHandle = m_ImageInfoEx.theSource->GetDTWAINHandle();

    if (!pMultiPageStruct || pMultiPageStruct->Stage == DIB_MULTI_FIRST)
    {
        auto* pSource = m_ImageInfoEx.theSource;
        auto it = pHandle->m_mapPDFTextElement.find(pSource);
        if (it != pHandle->m_mapPDFTextElement.end())
        {
        // any PDF text is initialized to not written
            std::for_each(it->second.begin(), it->second.end(),InitializeDisplayStatus);
        }
    }

    if (!pMultiPageStruct || pMultiPageStruct->Stage != DIB_MULTI_LAST)
    {
        if ( !m_ImageInfoEx.IsImageFileCreated )
        {
            // Create a temporary JPEG file
            //...
            szTempFile = GetDTWAINTempFilePath(m_ImageInfoEx.theSource->GetDTWAINHandle());
            if ( szTempFile.empty() )
            {
                return DTWAIN_ERR_FILEWRITE;
            }
            if ( m_pDib )
            {
                int bRet;
                // Make a JPEG from this info
                if ( m_pDib->GetDepth() > 1 )
                {
                    szTempFile += StringWrapper::GetGUID() + _T(".JPG");
                    auto szTempFileA = StringConversion::Convert_Native_To_Ansi(szTempFile);
                    CTL_TwainAppMgr::WriteLogInfoA("Temporary Image File is " + szTempFileA + "\n");

                    // Create a JPEG
                    m_JpegHandler.SetDib(m_pDib);
                    bRet = m_JpegHandler.WriteBitmap(szTempFile.c_str(), bOpenFile, fhFile);
                    if ( bRet != 0 )
                    {
                        CTL_TwainAppMgr::WriteLogInfoA("Error creating temporary Image File " + szTempFileA + "\n");
                        delete_file(szTempFile.c_str());
                        return bRet;
                    }
                    else
                        CTL_TwainAppMgr::WriteLogInfoA("Image file created successfully " + szTempFileA + "\n");
                    PDFHandler.SetImageType(0);
                }
                else
                    // make a CCITTFaxDecode
                {
                    DibMultiPageStruct dps = {};
                    szTempFile += StringWrapper::GetGUID() + _T(".TIF");
                    auto szTempFileA = StringConversion::Convert_Native_To_Ansi(szTempFile);

                    CTL_TwainAppMgr::WriteLogInfoA("Temporary Image File is " + szTempFileA + "\n");

                    // Create a TIFF file
                    m_TiffHandler.SetDib(m_pDib);
                    dps.Stage = DIB_MULTI_FIRST;
                    bRet = m_TiffHandler.WriteBitmap(szTempFile.c_str(), bOpenFile, 0, &dps);

                    if ( bRet != 0 )
                    {
                        CTL_TwainAppMgr::WriteLogInfoA("Error creating temporary Image File " + szTempFileA + "\n");
                        return bRet;
                    }
                    else
                    {
                        dps.Stage = DIB_MULTI_LAST;
                        bRet = m_TiffHandler.WriteBitmap(szTempFile.c_str(), bOpenFile, 0, &dps);
                        CTL_TwainAppMgr::WriteLogInfoA("Image file created successfully " + szTempFileA + "\n");
                    }
                    PDFHandler.SetImageType(1);
                }
            }
        }
        else
        {
            // call splitpath
            CTL_StringArrayType pathValues;
            StringWrapper::SplitPath(m_ImageInfoEx.szImageFileName, pathValues);
            szTempFile = m_ImageInfoEx.szImageFileName;
            if ( StringWrapper::CompareNoCase(pathValues[StringWrapper::EXTENSION_POS], _T("TIF")))
                PDFHandler.SetImageType(1);
            else
                PDFHandler.SetImageType(0);
        }
    }
    if ( pMultiPageStruct )
        PDFHandler.SetMultiPageStatus(pMultiPageStruct);

    // If OCR text is desired, then get the text for the page now
    int bRet;
    unsigned int nCount = 0;
    m_ImageInfoEx.IsSearchableTextOnPage = false;

    if ( m_ImageInfoEx.IsOCRUsedForPDF && (!pMultiPageStruct || pMultiPageStruct->Stage != DIB_MULTI_LAST))
    {
        OCRTextInfo ocrTextInfo;
        CTL_TwainAppMgr::SendTwainMsgToWindow(m_ImageInfoEx.theSession, nullptr,
                                              DTWAIN_TN_PDFOCRREADY,
                                             reinterpret_cast<LPARAM>(m_ImageInfoEx.theSource));

        bRet = GetOCRText(szTempFile.c_str(), PDFHandler.GetImageType(), ocrTextInfo.OCRChar);
        if ( bRet != 0 )
        {
            // Error occurred
            CTL_TwainAppMgr::SendTwainMsgToWindow(m_ImageInfoEx.theSession, nullptr,
                DTWAIN_TN_PDFOCRERROR, reinterpret_cast<LPARAM>(m_ImageInfoEx.theSource));
            bRet = DTWAIN_ERR_OCR_RECOGNITIONERROR;
            delete_file(szTempFile.c_str());
        }
        else
        {
            // Save this text to the PDF file as searchable, invisible text
            // Test
            CTL_TwainAppMgr::SendTwainMsgToWindow(m_ImageInfoEx.theSession, nullptr,
                DTWAIN_TN_PDFOCRDONE, reinterpret_cast<LPARAM>(m_ImageInfoEx.theSource));

            // Get the handle to the text buffer
            // Clean up the text if necessary... To do
            //...
            OCREngine *pEngine = pHandle->m_pOCRDefaultEngine.get();

            // Get the OCR information
            GetOCRCharacterInformation( pEngine, ocrTextInfo, m_pDib->GetHandle(), m_ImageInfoEx );

            if ( pEngine->GetBaseOption(OCROPTION_STORECLEANTEXT1))
                std::transform(ocrTextInfo.OCRChar.begin(), ocrTextInfo.OCRChar.end(), ocrTextInfo.OCRChar.begin(),
                               CleanupOCRText1);

            PDFStringToTextElement pElMap = CreatePDFTextElementMap(ocrTextInfo);

            auto itStart = pElMap.begin();
            auto itEnd = pElMap.end();

            while ( itStart != itEnd )
            {
                DTWAIN_PDFTEXTELEMENT TextElement = DTWAIN_CreatePDFTextElement(m_ImageInfoEx.theSource); // add to Source array of elements
                auto pElement = static_cast<PDFTextElement *>(TextElement);
                *pElement = *itStart;
                if ( itStart == pElMap.begin())
                {
                    // Iterator to last item added to the Text element list
                    m_ImageInfoEx.PDFSearchableTextRange.first =
                        std::prev(pHandle->m_mapPDFTextElement[m_ImageInfoEx.theSource].end());
                }
                ++itStart;
                ++nCount;
            }
            if ( nCount > 0 )
            {
                m_ImageInfoEx.PDFSearchableTextRange.second =
                    std::prev(pHandle->m_mapPDFTextElement[m_ImageInfoEx.theSource].end());
                m_ImageInfoEx.IsSearchableTextOnPage = true;
            }
        }
    }

    CTL_TwainAppMgr::WriteLogInfoA("Writing 1 page of PDF file...\n");
    bRet = PDFHandler.WriteGraphicFile(this, szTempFile.c_str(), m_pDib?m_pDib->GetHandle(): nullptr, &m_ImageInfoEx);
    CTL_TwainAppMgr::WriteLogInfoA("Finished writing 1 page of PDF file...\n");

    // Destroy the local text elements
    if (nCount > 0 )
    {
        pHandle->m_mapPDFTextElement[m_ImageInfoEx.theSource].erase(m_ImageInfoEx.PDFSearchableTextRange.first,
                                                                    m_ImageInfoEx.PDFSearchableTextRange.second);
    }

    // erase temporary (current page) text elements
    PDFTextElementEraser eraser(DTWAIN_PDFTEXT_CURRENTPAGE);
    auto& mapElement = pHandle->m_mapPDFTextElement[m_ImageInfoEx.theSource];
    mapElement.erase(std::remove_if(mapElement.begin(), mapElement.end(), eraser), mapElement.end());

    if ( bRet != 0 )
    {
        delete_file( szTempFile.c_str() );
    }

    if ( pMultiPageStruct )
        PDFHandler.GetMultiPageStatus(pMultiPageStruct);
    return bRet;
}

int CTL_PDFIOHandler::GetOCRText(LPCTSTR filename, int pageType, std::string& sText)
{
    sText.clear();

    CTL_StringType sFileToUse = filename;

    // Get the temp file path
    const auto pHandle = m_ImageInfoEx.theSource->GetDTWAINHandle();
    auto szTempPath = GetDTWAINTempFilePath(pHandle);

    OCREngine *pEngine = pHandle->m_pOCRDefaultEngine.get();
    if (!pEngine)
        return DTWAIN_ERR_OCR_INVALIDENGINE;

    static constexpr OCRPDFInfo::enumPDFColorType allColorTypes[] = {OCRPDFInfo::PDFINFO_COLOR, OCRPDFInfo::PDFINFO_BW};
    static constexpr LONG defaultPageTypes[] = {DTWAIN_JPEG, DTWAIN_TIFFNONE };
    static constexpr LONG defaultBitDepths[] = { 8, 1 };
    static constexpr LONG defaultPixelTypes[] = {DTWAIN_PT_RGB, DTWAIN_PT_BW};

    // Get the page type
    int index = 0;
    if ( pageType == 1 ) // this is a BW image
        index = 1;

    // Get the OCR file type
    LONG fileType = pEngine->m_OCRPDFInfo.FileType[allColorTypes[index]];
    if ( !fileType )
        return DTWAIN_ERR_OCR_INVALIDFILETYPE;

    // Now see if this type matches the native type
    // for b/w PDF files, DTWAIN_TIFFNONE / PixelType == DTWAIN_PT_BW / BitDepth == 1
    bool bMustConvert = true;
    const LONG OCRBitDepth = pEngine->m_OCRPDFInfo.BitDepth[allColorTypes[index]];
    const LONG pixelType = pEngine->m_OCRPDFInfo.PixelType[allColorTypes[index]];

    // Check the BPP for the OCR's file type
    if ( fileType == defaultPageTypes[index] )
    {
        if ( pixelType == defaultPixelTypes[index] )
        {
            if ( OCRBitDepth == defaultBitDepths[index] )
                bMustConvert = false;
        }
    }

    if ( bMustConvert )
    {
        CTL_ImageIOHandlerPtr pHandler;

        if ( !CheckValidConvertType(fileType, pageType) )
            return DTWAIN_ERR_OCR_INVALIDFILETYPE;

        // Now get the proper IO handler
        switch( fileType )
        {
            case DTWAIN_BMP:
                pHandler = std::make_shared<CTL_BmpIOHandler>(m_pDib, m_ImageInfoEx);
            break;

            case DTWAIN_TIFFG4:
            case DTWAIN_TIFFG3:
            case DTWAIN_TIFFNONE:
            case DTWAIN_TIFFLZW:
                pHandler = std::make_shared<CTL_TiffIOHandler>(m_pDib, fileType, m_ImageInfoEx);
                m_TiffHandler.SetDib(m_pDib);
            break;
        }

        // Now create a temp name
        szTempPath += StringWrapper::GetGUID() + _T("TMP");
        // If we need to convert the BPP to the one supported by the
        // OCR engine, do it now.
        HANDLE hNewDib= nullptr;

        if ( OCRBitDepth > defaultBitDepths[index] )
            // Create a new dib with an increased bpp for the OCR engine to work with
            hNewDib = CDibInterface::IncreaseBpp(m_pDib->GetHandle(), OCRBitDepth);
        else
        if ( OCRBitDepth < defaultBitDepths[index])
            hNewDib = CDibInterface::DecreaseBpp(m_pDib->GetHandle(), OCRBitDepth);
        else
            hNewDib = m_pDib->GetHandle();

        int bRetWrite = 0;

        // If not the original DIB, temporarily replace existing DIB
        // with the new one
        if ( hNewDib != m_pDib->GetHandle() )
        {
            CTL_TwainDib* oldDib = m_pDib;
            CTL_TwainDib theDib(hNewDib);
            pHandler->SetDib(&theDib);

            // Write the bitmap
            bRetWrite = pHandler->WriteBitmap(szTempPath.c_str(), 0, 0);

            // remove the temp dib
            theDib.Delete();

            // replace with the original DIB
            pHandler->SetDib(oldDib);
        }
        else
            bRetWrite = pHandler->WriteBitmap(szTempPath.c_str(), 0, 0);

        if ( bRetWrite != 0 )
        {
            CTL_TwainAppMgr::WriteLogInfo(_T("Error creating temporary OCR Image File ") + szTempPath + _T("\n"));
            return bRetWrite;
        }
        sFileToUse = std::move(szTempPath);
    }
    // Just OCR the text here
    DTWAIN_ARRAY aValues = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 1);
    if ( aValues )
    {
        DTWAINArrayLowLevel_RAII a(pHandle, aValues);
        DTWAIN_ArraySetAtLong(aValues, 0, fileType );
        const LONG bRet1 = DTWAIN_SetOCRCapValues(static_cast<DTWAIN_OCRENGINE>(pEngine), DTWAIN_OCRCV_IMAGEFILEFORMAT, DTWAIN_CAPSET, aValues);
        if ( bRet1 )
        {
            const LONG bRet = DTWAIN_ExecuteOCR(static_cast<DTWAIN_OCRENGINE>(pEngine), sFileToUse.c_str(), 0, 0);
            if ( bRet )
            {
                // OCRed the text.  Now retrieve it and return it
                LONG dataSize;
                DTWAIN_GetOCRText(static_cast<DTWAIN_OCRENGINE>(pEngine), 0, nullptr, 0, &dataSize, DTWAINOCR_COPYDATA);
                if ( dataSize <= 0 )
                {
                    sText = {};
                    return 0;
                }
                std::vector<TCHAR> charBuffer(dataSize);
                DTWAIN_GetOCRText(static_cast<DTWAIN_OCRENGINE>(pEngine), 0, &charBuffer[0], dataSize, &dataSize, DTWAINOCR_COPYDATA);

                // Now send a notification to the application
                const auto pSession = m_ImageInfoEx.theSource->GetTwainSession();
                const LONG bSave = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_QUERYOCRTEXT, reinterpret_cast<LPARAM>(m_ImageInfoEx.theSource));

                // Delete the temp file if we created one
                if ( bMustConvert )
                    delete_file(sFileToUse.c_str());

                if ( bSave )
                {
                    sText = StringConversion::Convert_Native_To_Ansi(charBuffer.data(), charBuffer.size());
                    return 0;
                }
                else
                    return 0;
            }
            else
                return DTWAIN_ERR_OCR_RECOGNITIONERROR;
        }
    }
    return DTWAIN_ERR_OCR_INVALIDFILETYPE;
}

bool CTL_PDFIOHandler::CheckValidConvertType(int fileType, int pageType)
{
    switch ( pageType )
    {
        case 0:
        case 1:
            return fileType == DTWAIN_BMP ||
                   fileType == DTWAIN_TIFFG3 ||
                   fileType == DTWAIN_TIFFG4 ||
                   fileType == DTWAIN_TIFFLZW;
    }
    return false;
}


bool GetOCRCharacterInformation( OCREngine* pEngine, OCRTextInfo& tInfo, HANDLE hBitmap, const DTWAINImageInfoEx& imageInfoEx )
{
    const DTWAIN_OCRTEXTINFOHANDLE tInfoHandle = DTWAIN_GetOCRTextInfoHandle(static_cast<DTWAIN_OCRENGINE>(pEngine), 0);
    if ( !tInfoHandle )
        return false;

    // Get the text info length
    LONG bufSize;
    DTWAIN_GetOCRText( static_cast<DTWAIN_OCRENGINE>(pEngine), 0, nullptr, 0, &bufSize, DTWAINOCR_COPYDATA ); // first get the buffer size

    // size the components in the struct
    tInfo.xDim.resize(bufSize);
    tInfo.yDim.resize(bufSize);
    tInfo.xPos.resize(bufSize);
    tInfo.yPos.resize(bufSize);

    // Get the data
    DTWAIN_GetOCRTextInfoLongEx(tInfoHandle, DTWAIN_OCRINFO_CHARXPOS,  &tInfo.xPos[0], bufSize);
    DTWAIN_GetOCRTextInfoLongEx(tInfoHandle, DTWAIN_OCRINFO_CHARYPOS, &tInfo.yPos[0], bufSize);
    DTWAIN_GetOCRTextInfoLongEx(tInfoHandle, DTWAIN_OCRINFO_CHARXWIDTH, &tInfo.xDim[0], bufSize);
    DTWAIN_GetOCRTextInfoLongEx(tInfoHandle, DTWAIN_OCRINFO_CHARYWIDTH, &tInfo.yDim[0], bufSize);

    // Get the original bitmap info translated into PDF units
    const LPBITMAPINFOHEADER pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hBitmap));
    DTWAINGlobalHandle_RAII dibHandle(hBitmap);

    const double xInches = static_cast<double>(pbi->biXPelsPerMeter) / 39.37;
    const double yInches = static_cast<double>(pbi->biYPelsPerMeter) / 39.37;
    const double xWidth = static_cast<double>(pbi->biWidth);
    const double yHeight = static_cast<double>(pbi->biHeight);
    const double widthInPoints = xWidth / xInches * 72.0;
    const double heightInPoints = yHeight / yInches * 72.0;

    tInfo.m_RealDimensions.SetDimensions(0,0, widthInPoints, heightInPoints );

    // test scaling of normal PDF page
    AllPDFDimensions pdfdims;
    const PDFDimensionsMap::const_iterator it = pdfdims.m_mediaMap.find(imageInfoEx.PDFPageSize);
    if ( it != pdfdims.m_mediaMap.end() )
    {
        double d1, d2, d3, d4;
        it->second.GetDimensions(d1, d2, d3, d4);
        tInfo.m_ScaledDimensions.SetDimensions(d1, d2, d3, d4);
    }
    else
    {
        switch (imageInfoEx.PDFPageSize )
        {
            case DTWAIN_PDF_CUSTOMSIZE:
                tInfo.m_ScaledDimensions.SetDimensions(0, 0, imageInfoEx.PDFCustomSize[0], imageInfoEx.PDFCustomSize[1]);
            break;

            case DTWAIN_PDF_PIXELSPERMETERSIZE:
                tInfo.m_ScaledDimensions.SetDimensions(0, 0, widthInPoints, heightInPoints);
            break;

            case DTWAIN_PDF_VARIABLEPAGESIZE:
                tInfo.m_ScaledDimensions.SetDimensions(0, 0, xWidth, yHeight);
            break;
        }
    }
    tInfo.m_PDFScalingFactorForPage.first = tInfo.m_ScaledDimensions.mediabox[2] / tInfo.m_RealDimensions.mediabox[2];
    tInfo.m_PDFScalingFactorForPage.second = tInfo.m_ScaledDimensions.mediabox[3] / tInfo.m_RealDimensions.mediabox[3];

    // end test
    tInfo.m_ImageDimensions = std::make_pair(static_cast<LONG>(xWidth), static_cast<LONG>(yHeight));
    return true;
}

PDFStringToTextElement CreatePDFTextElementMap(OCRTextInfo& tInfo)
{
    // Identify strings in OCRTextInfo
    // 1) For each string:
    //  a) determine max xDim and yDim (could be average if max does not work).  Determine scaling from these numbers.
    //  b) The starting position in PDF is always the first character in string
    //  c) All other text info fields are "static"
    std::vector<unsigned> PositionVec;
    StringArray strArray;
    StringWrapperA::TokenizeEx(tInfo.OCRChar, " ", strArray, false, &PositionVec);
    PDFStringToTextElement pMap;
    pMap.reserve(strArray.size());
    PDFTextElement element;
    for (size_t i = 0; i < strArray.size(); ++i)
    {
        element.m_text = strArray[i];
        // get the max x-dimension for this word
        LONG maxWidth = *std::max_element(tInfo.xDim.begin() + PositionVec[i],
                                          tInfo.xDim.begin() + PositionVec[i] + strArray[i].size());
        LONG maxHeight = *std::max_element(tInfo.yDim.begin() + PositionVec[i],
                                           tInfo.yDim.begin() + PositionVec[i] + strArray[i].size());
        const double fheight = tInfo.FontHeightFromPixelSize(maxHeight);
        element.scalingX = tInfo.FontWidthFromPixelSize(maxWidth) * tInfo.m_PDFScalingFactorForPage.first;
        element.scalingY = fheight * tInfo.m_PDFScalingFactorForPage.second;
        element.colorRGB = 0;
        element.fontSize = 1;
        element.renderMode = DTWAIN_PDFRENDER_INVISIBLE; // change this to invisible later!
        PDFPosition pPos = tInfo.PDFPositionFromPixelPosition(tInfo.xPos[PositionVec[i]],
                                                              tInfo.yPos[PositionVec[i]],
                                                              fheight);
        pPos.first = pPos.first * tInfo.m_ScaledDimensions.mediabox[2] / tInfo.m_RealDimensions.mediabox[2];
        pPos.second = pPos.second * tInfo.m_ScaledDimensions.mediabox[3] / tInfo.m_RealDimensions.mediabox[3];
        element.xpos = pPos.first;
        element.ypos = pPos.second;
        element.displayFlags = DTWAIN_PDFTEXT_CURRENTPAGE | DTWAIN_PDFTEXT_NOWORDSPACING;
        element.m_font.m_fontName = "Courier";
        pMap.push_back(element);
    }
    return pMap;
}
