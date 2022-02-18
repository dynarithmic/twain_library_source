/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#include "ctldib.h"
#include "ctliface.h"
#include "ctltwmgr.h"
#include "ctlfileutils.h"
using namespace dynarithmic;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////      Postscript handler    /////////////////////////////////////////////////////////////////////////////
CTL_PSIOHandler::CTL_PSIOHandler(CTL_TwainDib* pDib, int /*nFormat*/, const DTWAINImageInfoEx &ImageInfoEx,
                                 LONG PSType, bool IsMultiPage)
                                 :   CTL_ImageIOHandler( pDib ),
                                    m_nFormat(0),
                                    m_ImageInfoEx(ImageInfoEx),
                                    m_pJpegHandler(nullptr),
                                    m_PSType(PSType),
                                    m_bIsMultiPage(IsMultiPage)
{

    // Create a JPEG and TIFF handler locally

    //     m_pTiffHandler = new CTL_TiffIOHandler(pDib, CTL_TwainDib::TiffFormatGROUP4, ImageInfoEx);
}

int CTL_PSIOHandler::WriteBitmap(LPCTSTR szFile, bool bOpenFile, int /*fhFile*/, LONG64 MultiStage)
{
    const auto s = reinterpret_cast<DibMultiPageStruct*>(MultiStage);
    // Now add this to PDF page
    CPSImageHandler PSHandler(szFile, m_ImageInfoEx);
    CTL_StringType szTempPath;

    if ( !s || s->Stage == DIB_MULTI_FIRST )
    {
        // This is the first page, so allocate a TIFF handler here
        // Get the bit depth
        if ( m_pDib )
        {
            const int bitdepth = m_pDib->GetDepth();

            if (!IsValidBitDepth(DTWAIN_PS_ENCAPSULATED, bitdepth))
                return DTWAIN_ERR_INVALID_BITDEPTH;

            LONG FileType = 0;
            switch ( bitdepth )
            {
            case 1:
                FileType = DTWAIN_TIFFG4;
                break;

            case 8:
            case 24:
                FileType = DTWAIN_TIFFPACKBITS;
                break;
            }

            if ( m_bIsMultiPage )
            {
                switch(FileType)
                {
                case DTWAIN_TIFFG4:
                    FileType = DTWAIN_TIFFG4MULTI;
                    break;

                case DTWAIN_TIFFPACKBITS:
                    FileType = DTWAIN_TIFFPACKBITSMULTI;
                    break;
                }
            }

            // Create the TIFF handler
            m_pTiffHandler = std::make_shared<CTL_TiffIOHandler>(m_pDib, FileType, m_ImageInfoEx);
        }
    }

    if (!s || s->Stage != DIB_MULTI_LAST)
    {
        // Create a temporary TIFF file
        //...
        szTempPath = GetDTWAINTempFilePath();
        if ( szTempPath.empty() )
            return DTWAIN_ERR_FILEWRITE;

        if ( m_pDib )
        {
            // make a temporary TIFF file
            {
                szTempPath += StringWrapper::GetGUID() + _T("TIF");
                const std::string szTempPathA = StringConversion::Convert_Native_To_Ansi(szTempPath);
                CTL_TwainAppMgr::WriteLogInfoA("Temporary Image File is " + szTempPathA + "\n");

                // Create a TIFF file
                m_pTiffHandler->SetDib(m_pDib);
                const int bRet = m_pTiffHandler->WriteBitmap(szTempPath.c_str(), bOpenFile, 0, MultiStage);
                if ( bRet != 0 )
                {
                    CTL_TwainAppMgr::WriteLogInfoA("Error creating temporary Image File " + szTempPathA + "\n");
                    return bRet;
                }
                else
                    CTL_TwainAppMgr::WriteLogInfoA("Image file created successfully " + szTempPathA + "\n");
                PSHandler.SetImageType(1);
            }
        }
    }
    if ( MultiStage )
        PSHandler.SetMultiPageStatus(s);

    const int bRet = PSHandler.WriteGraphicFile(this, szTempPath.c_str(), nullptr, nullptr);
    if ( bRet != 0 )
    {
        delete_file( szTempPath.c_str() );
    }

    if ( s )
        PSHandler.GetMultiPageStatus(s);
    return bRet;
}
