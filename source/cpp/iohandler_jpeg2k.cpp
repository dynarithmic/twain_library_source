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
#include "ctldib.h"
#include "ctliface.h"
#include "jpeg2kwriter.h"

using namespace dynarithmic;

// ============================================================
// Example HANDLE-based helper
// ============================================================

static bool WriteOneDibHandleToJpeg2000(const std::wstring& filename, const Jpeg2000SessionOptions& options, HANDLE hDib)
{
	LockedJpeg2000DibPage lockedPage(hDib);
	if (!lockedPage.IsValid())
		return false;

	Jpeg2000SessionWriter writer;
	if (!writer.Open(filename, options))
		return false;

	if (!writer.SetPageInfo(lockedPage.GetPage()))
		return false;

	if (!writer.WriteCurrentPage())
		return false;

	writer.Close();
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTL_Jpeg2KIOHandler::CTL_Jpeg2KIOHandler(CTL_TwainDib* pDib, const DTWAINImageInfoEx &ImageInfoEx)
: CTL_ImageIOHandler( pDib ), m_nFormat(0), m_ImageInfoEx(ImageInfoEx), m_pJpegHandler(nullptr)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTL_Jpeg2KIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    HANDLE hDib = {};
    if (!m_pDib || !(hDib = m_pDib->GetHandle()))
        return DTWAIN_ERR_DIB;

    if (!IsValidBitDepth(DTWAIN_JPEG2000, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

	Jpeg2000SessionOptions opts{};
	opts.useJP2Container = false;   // J2K codestream
	opts.compressionRate = 16.0f;   // match FreeImage default

    std::wstring fName = StringConversion::Convert_NativePtr_To_Wide(szFile);

	// Get the comment string (copyright information)
	char commentStr[256] = {};
	GetResourceStringA(IDS_DTWAIN_APPTITLE, commentStr, 255);
	opts.text.copyright = commentStr;

    if (!WriteOneDibHandleToJpeg2000(fName, opts, hDib))
		return DTWAIN_ERR_FILEWRITE;
    return DTWAIN_NO_ERROR;
}
