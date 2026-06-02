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
#include "wbmpwriter.h"
#include "iohandler_wbmp.h"
#include "ctldib32ex.h"

using namespace dynarithmic;

static bool WriteOneDibHandleToWbmp(const std::wstring& filename, const WbmpSessionOptions& options, HANDLE hDib)
{
    LockedDibPage lockedPage(hDib);
    if (!lockedPage.IsValid())
        return false;

    WbmpSessionWriter writer;
    if (!writer.Open(filename, options))
        return false;

    auto pageInfo = WbmpSessionWriter::MakePreparedWbmpDibPage(lockedPage.GetView());
    if (!pageInfo.has_value())
        return false;

    if (!writer.SetPageInfo(pageInfo.value()))
        return false;

    if (!writer.WriteCurrentPage())
        return false;

    writer.Close();
    return true;
}

int CTL_WBMPIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    HANDLE hDib = hDib = m_pDib->GetHandle();
    dynarithmic::dib::LockedDib dibHandle(m_pDib->GetHandle());
    int height = dibHandle.Height();
    int width = dibHandle.Width();

    if (m_ImageInfoEx.IsWBMPResized && (height > 255 || width > 255))
    {
        height = 255;
        width = 255;
        m_pDib->ResampleDib(static_cast<long>(width), static_cast<long>(height));
        hDib = m_pDib->GetHandle();
    }
    else
        return DTWAIN_ERR_INVALIDWBMP; 

    if (!parent_directory_exists(szFile).first)
        return DTWAIN_ERR_FILEOPEN;

    WbmpSessionOptions opts{};
    opts.reverseBitOrder = false; // set true only if your 1-bpp DIB rows are LSB-first

    std::wstring fName = StringConversion::Convert_NativePtr_To_Wide(szFile);

    if (!WriteOneDibHandleToWbmp(fName, opts, hDib))
        return DTWAIN_ERR_FILEWRITE;
    return DTWAIN_NO_ERROR;
}
