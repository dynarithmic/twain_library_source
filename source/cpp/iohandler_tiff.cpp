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
#include <sstream>
#include "ctldib.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "ctlfileutils.h"
#include "tiff.h"
#include "logwriterutils.h"
using namespace dynarithmic;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTL_TiffIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* pMultiPageStruct)
{
    // Get the current TIFF type from the Source
    if (m_ImageInfoEx.theSource &&
        !m_ImageInfoEx.IsPDF &&
        !m_ImageInfoEx.IsPostscript &&
        !m_ImageInfoEx.IsPostscriptMultipage &&
        !m_ImageInfoEx.IsOCRTempImage)
        m_nFormat = m_ImageInfoEx.theSource->GetAcquireFileStatusRef().GetAcquireFileFormat();

    bool isFirstPage = (pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_FIRST);
    bool isLastPage =  (pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_LAST);
    bool isWriteablePage = isFirstPage || !isLastPage;

	if (isWriteablePage && m_pDib && !IsValidBitDepth(m_nFormat, m_pDib->GetBitsPerPixel()))
		return DTWAIN_ERR_INVALID_BITDEPTH;

    // Check for the Postscript option
    if (m_ImageInfoEx.IsPostscript && m_ImageInfoEx.theSource)
    {
        if (isFirstPage)
        {
            CTL_StringType szTempPath;
            // This is a postscript save, so
            // create a temp file
            szTempPath = GetDTWAINTempFilePath(m_ImageInfoEx.theSource->GetDTWAINHandle());
            if (szTempPath.empty())
                return DTWAIN_ERR_FILEWRITE;

            szTempPath += StringWrapper::GetGUID() + _T("TIF");

            LogWriterUtils::WriteLogInfoIndentedA(GetResourceStringFromMap(IDS_LOGMSG_TEMPIMAGEFILETEXT) + " " + StringConversion::Convert_Native_To_Ansi(szTempPath));

            // OK, now remember that the file we are writing is a TIFF file, and this is
            // the file that is created first
            sActualFileName = std::move(szTempPath);
            sPostscriptName = szFile;
        }
    }
    else
    {
        sActualFileName = szFile;
        if (isFirstPage)
        {
            // Attempt to delete the file
            if (!delete_file(sActualFileName.c_str()))
                LogWriterUtils::WriteLogInfoIndentedA("Could not delete existing file " + StringConversion::Convert_Native_To_Ansi(sActualFileName));
        }
    }

    if (isFirstPage)
		LogWriterUtils::WriteLogInfoIndentedA("Writing TIFF or Postscript file");

    CTIFFImageHandler imgHandler(m_nFormat, m_ImageInfoEx);
    imgHandler.SetMultiPageStatus(pMultiPageStruct);
	if (!isLastPage)
	{
		SetNumPagesWritten(GetNumPagesWritten() + 1);
		LogWriterUtils::WriteLogInfoIndentedA("Writing TIFF / PS page");
	}
    auto retVal = imgHandler.WriteGraphicFile(this, sActualFileName.c_str(), m_pDib->GetHandle());

    if (retVal == DTWAIN_NO_ERROR)
    {
        // Convert the TIFF file to Postscript if necessary
        if (m_ImageInfoEx.IsPostscript && isLastPage)
        {
            // This will have to call the routine to convert
            LONG Level;
            switch (m_ImageInfoEx.PostscriptType)
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
            if (sTitle.empty())
                sTitle = _T("DTWAIN Postscript");
            retVal = CTIFFImageHandler::Tiff2PS(sActualFileName.c_str(), sPostscriptName.c_str(), Level,
                                                sTitle.c_str(), m_ImageInfoEx.PSType == DTWAIN_PS_ENCAPSULATED);

            if (retVal == -1)
                retVal = DTWAIN_ERR_FILEWRITE;
            delete_file(sActualFileName.c_str());
            sActualFileName.clear();
        }
    }
    return retVal;
}

CTL_TiffIOHandler::~CTL_TiffIOHandler()
{
    // Ensure that temporary Postscript file has been removed
    if (m_ImageInfoEx.IsPostscript)
    {
        if (!sPostscriptName.empty() && !sActualFileName.empty())
            delete_file(sActualFileName.c_str());
    }
}