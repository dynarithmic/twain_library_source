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
#include "iohandler_tiff.h"
#include "tiffun32.h"

using namespace dynarithmic;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTL_TiffIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* pMultiPageStruct)
{
    // Get the current TIFF type from the Source
    if (m_ImageInfoEx.theSource &&
        !m_ImageInfoEx.IsPDF &&
        !m_ImageInfoEx.IsOCRTempImage)
        m_nFormat = m_ImageInfoEx.theSource->GetAcquireFileStatusRef().GetAcquireFileFormat();

    bool isFirstPage = (!pMultiPageStruct || pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_FIRST);
    bool isLastPage =  (!pMultiPageStruct || pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_LAST);
    bool isWriteablePage = isFirstPage || !isLastPage;

	if (isWriteablePage && m_pDib && !IsValidBitDepth(m_nFormat, m_pDib->GetBitsPerPixel()))
		return DTWAIN_ERR_INVALID_BITDEPTH;

    sActualFileName = szFile;
    if (isFirstPage)
    {
        // Attempt to delete the file
        if (!delete_file(sActualFileName.c_str()))
            LogWriterUtils::WriteLogInfoIndentedA("Could not delete existing file " + StringConversion::Convert_Native_To_Ansi(sActualFileName));
    }

    if (isFirstPage)
		LogWriterUtils::WriteLogInfoIndentedA("Writing TIFF file");

    CTIFFImageHandler imgHandler(m_nFormat, m_ImageInfoEx);
    imgHandler.SetMultiPageStatus(pMultiPageStruct);
	if (!isLastPage)
	{
		SetNumPagesWritten(GetNumPagesWritten() + 1);
		LogWriterUtils::WriteLogInfoIndentedA("Writing TIFF page");
	}
    auto retVal = imgHandler.WriteGraphicFile(this, sActualFileName.c_str(), m_pDib->GetHandle());
    return retVal;
}
