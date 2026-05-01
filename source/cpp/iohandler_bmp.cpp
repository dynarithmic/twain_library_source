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
#include "iohandler_bmp.h"
#include "bmprlewriter.h"
#include "ctliface.h"
#include "ctldib32ex.h"

using namespace dynarithmic;

static std::pair<bool, int> SaveBMPRLE(LPCTSTR szFile, HANDLE hDib)
{
    std::wstring filename = StringConversion::Convert_NativePtr_To_Wide(szFile);
    LockedDibPage lockedPage(hDib);
    if (!lockedPage.IsValid())
        return { false, DTWAIN_ERR_DIB };

    BmpRle8Writer writer;
    if (!writer.Open(filename))
        return { false, DTWAIN_ERR_FILEOPEN };

	auto pageInfo = BmpRle8Writer::MakePreparedBmpRle8Page(lockedPage.GetView());
	if (!pageInfo.has_value())
        return { false, DTWAIN_ERR_DIB };

    if (!writer.SetPageInfo(pageInfo.value()))
        return { false, DTWAIN_ERR_FILEWRITE };

    if (!writer.WriteCurrentPage())
        return { false, DTWAIN_ERR_FILEWRITE };

    writer.Close();
    return { true, DTWAIN_NO_ERROR };
}

int CTL_BmpIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct*)
{
    HANDLE hDib = m_pDib->GetHandle();

    if (m_ImageInfoEx.IsRLE)
    {
        // Save as a BMP-RLE file
        auto bOk = SaveBMPRLE(szFile, hDib);
        return bOk.second;
    }

    // "Regular" BMP file
    HANDLE hHandleToWrite = CTL_TwainDib::CreateBMPBitmapFromDIB(hDib);
    if (hHandleToWrite)
    {
        auto pBytes = static_cast<char *>(ImageMemoryHandler::GlobalLock(hHandleToWrite));
        DTWAINGlobalHandleUnlockFree_RAII raii(hHandleToWrite);
        std::ofstream ofs(StringConversion::Convert_NativePtr_To_Ansi(szFile), std::ios::binary);
        if (!ofs)
            return DTWAIN_ERR_FILEWRITE;
        auto nBytes = ImageMemoryHandler::GlobalSize(hHandleToWrite);
        ofs.write(pBytes, nBytes);
        return DTWAIN_NO_ERROR;
    }
    return DTWAIN_ERR_DIB;
}

