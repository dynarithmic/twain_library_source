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
#include <sstream>
#include "ctldib.h"
#include "ctliface.h"
#include "ctltwmgr.h"
#include "ctlfileutils.h"
#include "tiff.h"
using namespace dynarithmic;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTL_TiffIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* pMultiPageStruct)
{
    HANDLE hDib = nullptr;

    // Check if this is the first page
    CTL_TwainAppMgr::WriteLogInfoA("Writing TIFF or Postscript file\n");

    // Get the current TIFF type from the Source
    if ( m_ImageInfoEx.theSource &&
        !m_ImageInfoEx.IsPDF &&
        !m_ImageInfoEx.IsPostscript &&
        !m_ImageInfoEx.IsPostscriptMultipage &&
        !m_ImageInfoEx.IsOCRTempImage)
        m_nFormat = m_ImageInfoEx.theSource->GetAcquireFileType();

    if ( !pMultiPageStruct || pMultiPageStruct->Stage == DIB_MULTI_FIRST )
    {
        // Check for the Postscript option
        if ( m_ImageInfoEx.IsPostscript )
        {
            CTL_StringType szTempPath;
            // This is a postscript save, so
            // create a temp file
            szTempPath = GetDTWAINTempFilePath(m_ImageInfoEx.theSource->GetDTWAINHandle());
            if ( szTempPath.empty() )
                return DTWAIN_ERR_FILEWRITE;
            szTempPath += StringWrapper::GetGUID() +  _T("TIF");

            CTL_TwainAppMgr::WriteLogInfoA("Temporary Image File is " + StringConversion::Convert_Native_To_Ansi(szTempPath) + "\n");

            // OK, now remember that the file we are writing is a TIF file, and this is
            // the file that is created first
            sActualFileName = std::move(szTempPath);
            sPostscriptName = szFile;
        }
        else
        {
            // Just a normal TIFF file, no Postscript
            sActualFileName = szFile;

            // Attempt to delete the file
            if ( !delete_file(sActualFileName.c_str()) )
                CTL_TwainAppMgr::WriteLogInfoA("Could not delete existing file " + StringConversion::Convert_Native_To_Ansi(sActualFileName) + "\n");
        }
    }

    bool bNotLastFile = false;
    if ( !pMultiPageStruct )
        bNotLastFile = true;
    else
    if ( pMultiPageStruct->Stage != DIB_MULTI_LAST )
            bNotLastFile = true;
    if ( bNotLastFile )
    {
        CTL_TwainAppMgr::WriteLogInfoA("Retrieving DIB:\n");
        if ( !m_pDib )
        {
            CTL_TwainAppMgr::WriteLogInfoA("Dib not found!\n");
            return DTWAIN_ERR_DIB;
        }
        hDib = m_pDib->GetHandle();
        if ( !hDib )
        {
            CTL_TwainAppMgr::WriteLogInfoA("Dib handle not found!\n");
            return DTWAIN_ERR_DIB;
        }
    }
    //    int nRes = m_pDib->GetResolution();
    int nLibTiff;
    switch( m_nFormat )
    {
        case CTL_TwainDib::TiffFormatLZW:
        case CTL_TwainDib::TiffFormatLZWMULTI:
        case CTL_TwainDib::BigTiffFormatLZW:
        case CTL_TwainDib::BigTiffFormatLZWMULTI:
                nLibTiff = COMPRESSION_LZW;  
            break;

        case CTL_TwainDib::TiffFormatNONE:
        case CTL_TwainDib::TiffFormatNONEMULTI:
        case CTL_TwainDib::BigTiffFormatNONE:
        case CTL_TwainDib::BigTiffFormatNONEMULTI:
                nLibTiff = COMPRESSION_NONE;
            break;

        case CTL_TwainDib::TiffFormatGROUP3:
        case CTL_TwainDib::TiffFormatGROUP3MULTI:
        case CTL_TwainDib::BigTiffFormatGROUP3:
        case CTL_TwainDib::BigTiffFormatGROUP3MULTI:
            nLibTiff = COMPRESSION_CCITTFAX3;
            break;

        case CTL_TwainDib::TiffFormatGROUP4:
        case CTL_TwainDib::TiffFormatGROUP4MULTI:
        case CTL_TwainDib::BigTiffFormatGROUP4:
        case CTL_TwainDib::BigTiffFormatGROUP4MULTI:
            nLibTiff = COMPRESSION_CCITTFAX4;
            break;

        case CTL_TwainDib::TiffFormatPACKBITS:
        case CTL_TwainDib::TiffFormatPACKBITSMULTI:
        case CTL_TwainDib::BigTiffFormatPACKBITS:
        case CTL_TwainDib::BigTiffFormatPACKBITSMULTI:
            nLibTiff = COMPRESSION_PACKBITS;
            break;

        case CTL_TwainDib::TiffFormatDEFLATE:
        case CTL_TwainDib::TiffFormatDEFLATEMULTI:
        case CTL_TwainDib::BigTiffFormatDEFLATE:
        case CTL_TwainDib::BigTiffFormatDEFLATEMULTI:
            nLibTiff = COMPRESSION_ADOBE_DEFLATE;
            break;

        case CTL_TwainDib::TiffFormatJPEG:
        case CTL_TwainDib::TiffFormatJPEGMULTI:
        case CTL_TwainDib::BigTiffFormatJPEG:
        case CTL_TwainDib::BigTiffFormatJPEGMULTI:
            nLibTiff = COMPRESSION_JPEG;
            break;

        case CTL_TwainDib::TiffFormatPIXARLOG:
        case CTL_TwainDib::TiffFormatPIXARLOGMULTI:
            nLibTiff = COMPRESSION_PIXARLOG;
            break;

        default:
             return DTWAIN_ERR_INVALID_BITDEPTH;
    }

    if (bNotLastFile && !IsValidBitDepth(m_nFormat, m_pDib->GetBitsPerPixel()) )
        return DTWAIN_ERR_INVALID_BITDEPTH;

    CTIFFImageHandler TIFFHandler(nLibTiff, m_ImageInfoEx);

    if ( pMultiPageStruct )
    {
        TIFFHandler.SetMultiPageStatus(pMultiPageStruct);
    }
    int retval;
    if ( bNotLastFile )
    {
        SetNumPagesWritten(GetNumPagesWritten()+1);
        CTL_TwainAppMgr::WriteLogInfoA("Writing TIFF / PS page\n");
        retval = TIFFHandler.WriteGraphicFile(this, sActualFileName.c_str(), hDib);
        if ( retval != 0 )
            SetPagesOK(false);
        else
            SetOnePageWritten(true);
        CTL_TwainAppMgr::WriteLogInfoA("Writing TIFF / PS page\n");
        StringStreamA strm;
        strm << "Return from writing intermediate image = " << retval << "\n";
        CTL_TwainAppMgr::WriteLogInfoA(strm.str());
    }
    else
    {
        // Close the multi-page TIFF file
        CTL_TwainAppMgr::WriteLogInfoA("Closing TIFF / PS file\n");
        retval = TIFFHandler.WriteImage(this,nullptr,0,0,0,0, nullptr);
        if ( !AllPagesOK() )
        {
            if ( !IsOnePageWritten() )
            {
                delete_file( sActualFileName.c_str() );
                retval = DTWAIN_ERR_FILEXFERSTART - DTWAIN_ERR_FILEWRITE;
            }
        }
        StringStreamA strm;
        strm << "Return from writing last image = " << retval << "\n";
        CTL_TwainAppMgr::WriteLogInfoA(strm.str());
    }
    if ( (!pMultiPageStruct || pMultiPageStruct->Stage == DIB_MULTI_LAST) && retval == 0 )
    {
        // Convert the TIFF file to Postscript if necessary
        if ( m_ImageInfoEx.IsPostscript )
        {
            // This will have to call the routine to convert
            LONG Level;
            switch(m_ImageInfoEx.PostscriptType)
            {
            case DTWAIN_POSTSCRIPT1:
            case DTWAIN_POSTSCRIPT1MULTI:
                Level = 1;
                break;

            case DTWAIN_POSTSCRIPT2:
            case DTWAIN_POSTSCRIPT2MULTI:
                Level = 2;
                break;
            default:
                Level = 3;
                break;
            }
            CTL_StringType sTitle;
            sTitle = m_ImageInfoEx.PSTitle;
            if ( sTitle.empty() )
                sTitle = _T("DTWAIN Postscript");
            retval = CTIFFImageHandler::Tiff2PS(sActualFileName.c_str(), sPostscriptName.c_str(),Level,
                                                sTitle.c_str(), m_ImageInfoEx.PSType==DTWAIN_PS_ENCAPSULATED);

            if ( retval == -1 )
                retval = DTWAIN_ERR_FILEWRITE;
            delete_file( sActualFileName.c_str());
        }
    }
    if ( pMultiPageStruct )
        TIFFHandler.GetMultiPageStatus(pMultiPageStruct);

    return retval;
}
