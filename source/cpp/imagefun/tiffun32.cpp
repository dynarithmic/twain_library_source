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
 #ifdef _MSC_VER
#pragma warning (disable : 4786)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include "tiffun32.h"
#include "pdflib32.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "tiff.h"

using namespace dynarithmic;

CTL_StringType CTIFFImageHandler::s_AppInfo;

std::string CTIFFImageHandler::GetFileExtension() const
{
    return "TIF";
}

HANDLE CTIFFImageHandler::GetFileInformation(LPCSTR /*path*/)
{
    return nullptr;
}

void CTIFFImageHandler::SetMultiPageStatus(DibMultiPageStruct *pStruct)
{

    if ( pStruct )
        m_MultiPageStruct = *pStruct;
}


void CTIFFImageHandler::GetMultiPageStatus(DibMultiPageStruct *pStruct)
{
    *pStruct = m_MultiPageStruct;
}

bool CTIFFImageHandler::OpenOutputFile(LPCTSTR pFileName)
{
    StringTraits::Copy(m_FileName, pFileName);
    SetError(0);
    return true;
}

bool CTIFFImageHandler::CloseOutputFile()
{
    return true;
}

void CTIFFImageHandler::DestroyAllObjects()
{
}

static constexpr TiffCompression TranslateCompression(int nCompression)
{
    switch (nCompression)
    {
	    case COMPRESSION_NONE: return TiffCompression::None;
        case COMPRESSION_CCITTFAX3: return TiffCompression::Group3;
    	case COMPRESSION_CCITTFAX4: return TiffCompression::Group4;
    	case COMPRESSION_LZW: return TiffCompression::Lzw;
    	case COMPRESSION_ADOBE_DEFLATE: return TiffCompression::Flate;
    	case COMPRESSION_PACKBITS: return TiffCompression::PackBits;
        case COMPRESSION_JPEG:     return TiffCompression::Jpeg;
    }
    return TiffCompression::None;
}

int CTIFFImageHandler::WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo/*=NULL*/)
{
    // Check if this is first page of multi-page TIFF or
    // if only a single page TIFF
	auto ptrTiffHandler = static_cast<CTL_TiffIOHandler*>(ptrHandler);
	auto& outputHandler = ptrTiffHandler->GetOutputHandler();

    // Last page, which only signals that TIFF file is to be closed
	if (m_MultiPageStruct.Stage == DIB_MULTI_LAST)
	{
		outputHandler.OnLastPage();
		m_bWriteOk = TRUE;
		return DTWAIN_NO_ERROR;
	}

    // If we get here, then a TIFF page will be produced and appended
    TiffSessionOptions tiffOptions;

    // Set the big tiff option if this is a big TIFF file
    if (dynarithmic::IsFileTypeBigTiff(static_cast<CTL_TwainFileFormatEnum>(ptrTiffHandler->GetTiffFormat())))
        tiffOptions.containerFormat = TiffContainerFormat::BigTiff;

    // Get the DIB
    LockedDibPage lockedPage(bitmap);

	// Get a reference to the DIB
	auto& theDibPage = lockedPage.GetPageRef();

    // Get the page settings
    TiffPageSettings tiffPageSettings;

    // Check if the compression is supported (this should always work)
	auto retVal = ProcessCompressionType();
	if (!retVal.first )
		return retVal.second;

    // Convert the compression to one the TiffWriter knows about
	tiffPageSettings.compression = TranslateCompression(retVal.second);

    // Now go through the special Group 3/4 options
    if (tiffPageSettings.compression == TiffCompression::Group3 ||
        tiffPageSettings.compression == TiffCompression::Group4)
    {
        tiffPageSettings.invertBilevelPolarity = true;
        if (tiffPageSettings.compression == TiffCompression::Group3)
            tiffPageSettings.group3Options = GROUP3OPT_2DENCODING | GROUP3OPT_FILLBITS;
    }

    // Set the JPEG quality
    tiffPageSettings.jpegQuality = m_ImageInfoEx.nJpegQuality;

    // Set the photometric setting
    if (theDibPage.bitsPerPixel == 1)
        theDibPage.bwpolarity = m_ImageInfoEx.PhotoMetric;

    if (m_MultiPageStruct.Stage == 0 || m_MultiPageStruct.Stage == DIB_MULTI_FIRST)
    {
        // If first page or if single page TIFF, write first page
        const std::wstring fnameW = StringConversion::Convert_NativePtr_To_Wide(path);

        // Get the comment string (copyright information)
    	char commentStr[256] = {};
		GetResourceStringA(IDS_DTWAIN_APPTITLE, commentStr, 255);
        tiffOptions.software = commentStr;

        // These on handlers should return a pair {true/false, error_return_code}
        auto writeRetValue = outputHandler.OnFirstPage(fnameW, tiffOptions, theDibPage, tiffPageSettings);
        if (!writeRetValue.first)
            return writeRetValue.second;

        if (m_MultiPageStruct.Stage == 0)
        {
            // These on handlers should return a pair {true/false, error_return_code}
            writeRetValue = outputHandler.OnLastPage();
            return writeRetValue.second;
        }
    }
    else
    {
        // Write subsequent page
		// These on handlers should return a pair {true/false, error_return_code}
		auto writeRetValue = outputHandler.OnNextPage(theDibPage, tiffPageSettings);
        return writeRetValue.second;
    }
    return DTWAIN_NO_ERROR;
}

std::pair<int, int> CTIFFImageHandler::ProcessCompressionType() const
{
    return ProcessCompressionTypeInternal();
}

std::pair<int, int> CTIFFImageHandler::ProcessCompressionTypeInternal() const
{
	switch (m_nFormat)
	{
	    case CTL_TwainDib::TiffFormatLZW:
	    case CTL_TwainDib::TiffFormatLZWMULTI:
	    case CTL_TwainDib::BigTiffFormatLZW:
	    case CTL_TwainDib::BigTiffFormatLZWMULTI:
            return { true, COMPRESSION_LZW };

	    case CTL_TwainDib::TiffFormatNONE:
	    case CTL_TwainDib::TiffFormatNONEMULTI:
	    case CTL_TwainDib::BigTiffFormatNONE:
	    case CTL_TwainDib::BigTiffFormatNONEMULTI:
            return { true, COMPRESSION_NONE };

	    case CTL_TwainDib::TiffFormatGROUP3:
	    case CTL_TwainDib::TiffFormatGROUP3MULTI:
	    case CTL_TwainDib::BigTiffFormatGROUP3:
	    case CTL_TwainDib::BigTiffFormatGROUP3MULTI:
    		return { true, COMPRESSION_CCITTFAX3 };

	    case CTL_TwainDib::TiffFormatGROUP4:
	    case CTL_TwainDib::TiffFormatGROUP4MULTI:
	    case CTL_TwainDib::BigTiffFormatGROUP4:
	    case CTL_TwainDib::BigTiffFormatGROUP4MULTI:
            return { true, COMPRESSION_CCITTFAX4 };

	    case CTL_TwainDib::TiffFormatPACKBITS:
	    case CTL_TwainDib::TiffFormatPACKBITSMULTI:
	    case CTL_TwainDib::BigTiffFormatPACKBITS:
	    case CTL_TwainDib::BigTiffFormatPACKBITSMULTI:
            return { true, COMPRESSION_PACKBITS };

	    case CTL_TwainDib::TiffFormatDEFLATE:
	    case CTL_TwainDib::TiffFormatDEFLATEMULTI:
	    case CTL_TwainDib::BigTiffFormatDEFLATE:
	    case CTL_TwainDib::BigTiffFormatDEFLATEMULTI:
            return { true, COMPRESSION_ADOBE_DEFLATE };

	    case CTL_TwainDib::TiffFormatJPEG:
	    case CTL_TwainDib::TiffFormatJPEGMULTI:
	    case CTL_TwainDib::BigTiffFormatJPEG:
	    case CTL_TwainDib::BigTiffFormatJPEGMULTI:
            return { true, COMPRESSION_JPEG };

	    case CTL_TwainDib::TiffFormatPIXARLOG:
	    case CTL_TwainDib::TiffFormatPIXARLOGMULTI:
            return { true, COMPRESSION_PIXARLOG };
	}
    return { false, DTWAIN_ERR_INVALID_BITDEPTH };
}

int CTIFFImageHandler::WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht,
                                  UINT32 bpp, UINT32 /*nColors*/, RGBQUAD * /*pPal*/, void * /*pUserInfo*/)
{
    DestroyAllObjects();
    return m_nError;
}

int CTIFFImageHandler::Tiff2PS(LPCTSTR szFileIn, LPCTSTR szFileOut, LONG PSType,
                               LPCTSTR szTitle, bool PSEncapsulated)
{
    return PDFInterface().DTWLIB_PSWriteFile(szFileIn, szFileOut, PSType, szTitle, PSEncapsulated);
}