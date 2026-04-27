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
#include "ctlfileutils.h"
#include "icowriter.h"
#include "iohandler_ico.h"

using namespace dynarithmic;

static bool WriteOneDibHandleToIco(const std::wstring& filename, const IcoSessionOptions& options, HANDLE hDib)
{
	LockedIcoDibPage lockedPage(hDib);
	if (!lockedPage.IsValid())
		return false;

	IcoSessionWriter writer;
	if (!writer.Open(filename, options))
		return false;

	if (!writer.SetPageInfo(lockedPage.GetPage()))
		return false;

	if (!writer.WriteCurrentPage())
		return false;

	writer.Close();
	return true;
}

int CTL_IcoIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct*)
{
    if (!m_pDib)
        return DTWAIN_ERR_DIB;

    HANDLE hDib = m_pDib->GetHandle();
    if (!hDib)
        return DTWAIN_ERR_DIB;

    dynarithmic::dib::LockedDib dibHandle(m_pDib->GetHandle());
    int height = dibHandle.Height();
    int width = dibHandle.Width();

    if (!m_ImageInfoEx.IsVistaIcon)
    {

        if (m_ImageInfoEx.IsIcoResized && (height > 255 || width > 255))
        {
            height = 255;
            width = 255;
            m_pDib->ResampleDib({ static_cast<double>(width), static_cast<double>(height) }, CTL_ITwainSource::RESIZE_FLAG);
            hDib = m_pDib->GetHandle();
        }
        else
            return DTWAIN_ERR_INVALIDICONFORMAT;
    }

    if (!parent_directory_exists(szFile).first)
        return DTWAIN_ERR_FILEOPEN;

    std::wstring fName = StringConversion::Convert_NativePtr_To_Wide(szFile);

    if (!m_ImageInfoEx.IsVistaIcon)
    {
        IcoSessionOptions opts{};
        opts.mode = IcoMode::Classic;
        if (!WriteOneDibHandleToIco(fName, opts, hDib))
            return DTWAIN_ERR_FILEWRITE;
    }
    else
    {
        IcoSessionOptions opts{};
        opts.mode = IcoMode::VistaPng;

        if (!WriteOneDibHandleToIco(fName, opts, hDib))
            return DTWAIN_ERR_FILEWRITE;
    }
    return DTWAIN_NO_ERROR;
}