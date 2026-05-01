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
#include "tgawriter.h"
#include "iohandler_tga.h"
#include "ctldib32ex.h"

using namespace dynarithmic;

// ============================================================
// Example HANDLE-based helper
// ============================================================

static bool WriteOneDibHandleToTga(const std::wstring& filename, const TgaSessionOptions& options, HANDLE hDib)
{
	LockedDibPage lockedPage(hDib);
	if (!lockedPage.IsValid())
		return false;

	TgaSessionWriter writer;
	if (!writer.Open(filename, options))
		return false;

	auto pageInfo = TgaSessionWriter::MakePreparedTgaDibPage(lockedPage.GetView());
	if (!pageInfo.has_value())
		return false;

	if (!writer.SetPageInfo(pageInfo.value()))
		return false;

	if (!writer.WriteCurrentPage())
		return false;

	writer.Close();
	return true;
}

int CTL_TgaIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    HANDLE hDib = {};
    if (!m_pDib || !(hDib = m_pDib->GetHandle()))
        return DTWAIN_ERR_DIB;

    if (!IsValidBitDepth(DTWAIN_TGA, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

	std::wstring fName = StringConversion::Convert_NativePtr_To_Wide(szFile);

	TgaSessionOptions opts{};
    opts.useRle = m_ImageInfoEx.IsRLE;

	opts.comment = GetCopyrightString();

	if (!WriteOneDibHandleToTga(fName, opts, hDib))
		return DTWAIN_ERR_FILEWRITE;
    return DTWAIN_NO_ERROR;
}
