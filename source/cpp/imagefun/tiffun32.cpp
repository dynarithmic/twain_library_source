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
#if 0
    auto tiffPtr = std::dynamic_pointer_cast<TiffMultiPageData>(m_MultiPageStruct.pUserData);
    if ( tiffPtr )
    {
        auto fp = tiffPtr->fp;
        if (fp)
        {
            // Set the TIFF type to close the TIFF file out with (regular or BigTiff)
            int32_t flags = 0;
            if (m_ImageInfoEx.IsBigTiff)
                flags = 1L << 31;
            m_nError = FreeImage_CloseMultiBitmap(fp, flags) ? false : true;
        }
    }
#endif
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
    static constexpr std::array<std::pair<unsigned long, int>, 7> compressionFlags = 
                                                    { {{COMPRESSION_PACKBITS, TIFF_PACKBITS},
                                                     {COMPRESSION_ADOBE_DEFLATE, TIFF_ADOBE_DEFLATE},
                                                     {COMPRESSION_NONE, TIFF_NONE},
                                                     {COMPRESSION_CCITTFAX3, TIFF_CCITTFAX3},
                                                     {COMPRESSION_CCITTFAX4, TIFF_CCITTFAX4},
                                                     {COMPRESSION_LZW, TIFF_LZW},
                                                     {COMPRESSION_JPEG, TIFF_JPEG}
                                                    } };

    // Check if this is first page of multi-page TIFF or
    // if only a single page TIFF
	auto ptrTiffHandler = static_cast<CTL_TiffIOHandler*>(ptrHandler);
	auto& outputHandler = ptrTiffHandler->GetOutputHandler();

	if (m_MultiPageStruct.Stage == DIB_MULTI_LAST)
	{
		outputHandler.OnLastPage();
		m_bWriteOk = TRUE;
		return DTWAIN_NO_ERROR;
	}

    TiffSessionOptions tiffOptions;

    // Set the big tiff option if this is a big TIFF file
    if (dynarithmic::IsFileTypeBigTiff(static_cast<CTL_TwainFileFormatEnum>(ptrTiffHandler->GetTiffFormat())))
        tiffOptions.containerFormat = TiffContainerFormat::BigTiff;

    // Get the DIB
    LockedDibPage lockedPage(bitmap);

    // Get the page settings
    TiffPageSettings tiffPageSettings;

	auto retVal = ProcessCompressionType();
	if (!retVal.first )
		return retVal.second;
	tiffPageSettings.compression = TranslateCompression(retVal.second);
    if (tiffPageSettings.compression == TiffCompression::Group3 ||
        tiffPageSettings.compression == TiffCompression::Group4)
    {
        tiffPageSettings.invertBilevelPolarity = true;
        if (tiffPageSettings.compression == TiffCompression::Group3)
            tiffPageSettings.group3Options = GROUP3OPT_2DENCODING | GROUP3OPT_FILLBITS;
    }

    if (m_MultiPageStruct.Stage == 0 || m_MultiPageStruct.Stage == DIB_MULTI_FIRST)
    {
        const std::wstring fnameW = StringConversion::Convert_NativePtr_To_Wide(path);

        // These on handlers should return a pair {true/false, error_return_code}
        outputHandler.OnFirstPage(fnameW, tiffOptions, lockedPage.GetPage(), tiffPageSettings);
        if (m_MultiPageStruct.Stage == 0)
			// These on handlers should return a pair {true/false, error_return_code}
            outputHandler.OnLastPage();
#if 0
        outputHandler
        auto ptrTiffData = std::make_shared<TiffMultiPageData>();
        ptrTiffData->fp = nullptr;
        const std::string fname = StringConversion::Convert_NativePtr_To_Ansi(path);
        {
            const std::ofstream ofs(fname);
            if (!ofs)
                return DTWAIN_ERR_FILEOPEN;
        }
        if (m_MultiPageStruct.Stage != 0)
        {
            ptrTiffData->fp = FreeImage_OpenMultiBitmap(FIF_TIFF, fname.c_str(), true, false, false, 0);
            FreeImage_SetPageNumberEx(ptrTiffData->fp, 0);
            if ( !ptrTiffData->fp )
                return DTWAIN_ERR_FILEOPEN;
        }
        m_MultiPageStruct.pUserData = ptrTiffData;
#endif
    }
    else
    {
		// These on handlers should return a pair {true/false, error_return_code}
		bool bOk = outputHandler.OnNextPage(lockedPage.GetPage(), tiffPageSettings);
        if (bOk)
            return DTWAIN_NO_ERROR;
        return DTWAIN_ERR_FILEWRITE;
    }
#if 0
    auto ptrTiffData = std::dynamic_pointer_cast<TiffMultiPageData>(m_MultiPageStruct.pUserData);
    fipImage im;
    fipImageUtility::copyFromHandle(im, bitmap);
    fipWinImage_RAII raii(&im);
    unsigned long compression;
    const int retVal = ProcessCompressionType(im, compression);
    if (retVal != 0)
        return retVal;

    double factor = GetScaleFactorPerInch(m_ImageInfoEx.UnitOfMeasure);
    switch (m_ImageInfoEx.UnitOfMeasure)
    {
        case DTWAIN_CENTIMETERS:
            factor = 1.0;
    }
    im.setHorizontalResolution(m_ImageInfoEx.ResolutionX * factor / 2.54);
    im.setVerticalResolution(m_ImageInfoEx.ResolutionY * factor / 2.54);

    fipTag ft;
    char commentStr[256] = {};
    GetResourceStringA(IDS_DTWAIN_APPTITLE, commentStr, 255);
    ft.setKeyValue("Comment", commentStr);
    im.setMetadata(FIMD_COMMENTS, "Comment", ft);

    const auto iter = dynarithmic::generic_array_finder_if(compressionFlags, [&](const auto& pr) { return pr.first == compression; });
    // Set the TIFF type to write (regular or BigTiff)
    int32_t flagsWord = 0;
    if (m_ImageInfoEx.IsBigTiff)
        flagsWord = 1L << 31;

    if (m_MultiPageStruct.Stage == 0)
    {

        // Set the flags for the TIFF plugin to use
        const int32_t flagsValue = flagsWord | compressionFlags[iter.second].second;
        const auto retVal2 = im.saveEx(FIF_TIFF, StringConversion::Convert_Native_To_Ansi(path).c_str(), m_MultiPageStruct.Page, flagsValue);

        if (retVal2 == 1)
        {
            ++m_MultiPageStruct.Page;
            return DTWAIN_NO_ERROR;
        }
        return DTWAIN_ERR_FILEWRITE;
    }

    // this is a multipage write
    if (!ptrTiffData)
        return DTWAIN_ERR_FILEWRITE; // Issue with writing the file.  File to write may not be valid.
    FreeImage_AppendPageEx(ptrTiffData->fp, im, flagsWord | compressionFlags[iter.second].second);
    FreeImage_SetPageNumberEx(ptrTiffData->fp, FreeImage_GetPageNumber(ptrTiffData->fp) + 1);
    ++m_MultiPageStruct.Page;
#endif
    return 0;
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

#if 0
	long samplesperpixel = 0;
	long bitspersample = 0;
	long photometric = 0;
	compression = 0;
	bool bOk = false;
	switch (bpp)
	{
	    case 1:
		    samplesperpixel = 1;
		    bitspersample = 1;
		    photometric = PHOTOMETRIC_MINISWHITE;
		    if (m_nFormat == COMPRESSION_NONE ||
			    m_nFormat == COMPRESSION_LZW ||
			    m_nFormat == COMPRESSION_PACKBITS ||
			    m_nFormat == COMPRESSION_CCITTRLE ||
			    m_nFormat == COMPRESSION_CCITTFAX3 ||
			    m_nFormat == COMPRESSION_CCITTFAX4 ||
			    m_nFormat == COMPRESSION_JBIG ||
			    m_nFormat == COMPRESSION_PIXARLOG ||
			    m_nFormat == COMPRESSION_ADOBE_DEFLATE)
		    {
			    compression = m_nFormat;
			    bOk = true;
		    }
		    break;
	    case 4:
	    case 8:
	    case 14:
	    case 16:
		    samplesperpixel = 1;
		    bitspersample = bpp;
		    photometric = PHOTOMETRIC_PALETTE;
		    if (m_nFormat == COMPRESSION_NONE ||
			    m_nFormat == COMPRESSION_LZW ||
			    m_nFormat == COMPRESSION_PACKBITS ||
			    m_nFormat == COMPRESSION_JPEG ||
			    m_nFormat == COMPRESSION_ADOBE_DEFLATE ||
			    m_nFormat == COMPRESSION_JBIG ||
			    m_nFormat == COMPRESSION_PIXARLOG)
		    {
			    compression = m_nFormat;
			    bOk = true;
		    }

		    if (m_nFormat == COMPRESSION_JPEG) {}
		    break;
	    case 24:
	    case 32:
		    samplesperpixel = 3;
		    bitspersample = 8;
		    if (m_nFormat == COMPRESSION_NONE ||
			    m_nFormat == COMPRESSION_LZW ||
			    m_nFormat == COMPRESSION_PACKBITS ||
			    m_nFormat == COMPRESSION_JPEG ||
			    m_nFormat == COMPRESSION_JBIG ||
			    m_nFormat == COMPRESSION_PIXARLOG ||
			    m_nFormat == COMPRESSION_ADOBE_DEFLATE)
		    {
			    compression = m_nFormat;
			    bOk = true;
		    }
		    break;
  	}
    if (!bOk)
	    return DTWAIN_ERR_BADBITSPERPIXEL;
    return 0;
}
#endif

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