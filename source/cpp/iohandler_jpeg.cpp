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
#include "iohandler_jpeg.h"
#include "jpegwriter.h"
#include "ctldib32ex.h"

using namespace dynarithmic;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static bool WriteOneDibHandleToJpeg(const std::wstring& filename,const JpegSessionOptions& options, HANDLE hDib)
{
    LockedDibPage lockedPage(hDib);
    if (!lockedPage.IsValid())
        return false;

    JpegSessionWriter writer;
    if (!writer.Open(filename, options))
        return false;

    auto preparedPage = JpegSessionWriter::MakePreparedJpegPage(lockedPage.GetView());
    if (!preparedPage.has_value())
        return false;

    if (!writer.SetPageInfo(preparedPage.value()))
        return false;

    if (!writer.WriteCurrentPage())
        return false;

    writer.Close();
    return true;
}

int CTL_JpegIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct*)
{
    HANDLE hDib = m_pDib->GetHandle();
    JpegSessionOptions opts{};
    opts.quality = m_ImageInfoEx.IsPDF ? m_ImageInfoEx.nPDFJpegQuality : m_ImageInfoEx.nJpegQuality;
    opts.progressive = m_ImageInfoEx.bProgressiveJpeg;
    opts.text.copyright = GetCopyrightString();
    std::wstring sFileName = StringConversion::Convert_NativePtr_To_Wide(szFile);

    if (!WriteOneDibHandleToJpeg(sFileName, opts, hDib))
        return DTWAIN_ERR_FILEWRITE;

    return DTWAIN_NO_ERROR;
}
