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
#include "pngwriter.h"
#include "iohandler_png.h"
#include "ctldib32ex.h"

using namespace dynarithmic;

int CTL_PngIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    struct PngWriterRAII
    {
        PngSessionWriter* m_pWriter = nullptr;
        PngWriterRAII(PngSessionWriter* pWriter) : m_pWriter(pWriter) {}
        ~PngWriterRAII()
        {
            try
            {
                if (m_pWriter)
                    m_pWriter->Close();
            }
            catch (...) {}
        }
    };

    HANDLE hDib = m_pDib->GetHandle();

	LockedDibPage lockedPage(hDib);
	if (!lockedPage.IsValid())
		return DTWAIN_ERR_FILEWRITE;
    std::wstring sFileName = StringConversion::Convert_NativePtr_To_Wide(szFile);

    PngSessionOptions sessionOptions;

    sessionOptions.text.copyright = GetCopyrightString();

	PngSessionWriter writer;

    if (!writer.Open(sFileName, sessionOptions))
        return DTWAIN_ERR_FILEWRITE;

	PngWriterRAII raii(&writer);

	auto pageInfo = PngSessionWriter::MakePreparedPngDibPage(lockedPage.GetView());
	if (!pageInfo.has_value())
		return DTWAIN_ERR_FILEWRITE;

	if (!writer.SetPageInfo(pageInfo.value()))
		return DTWAIN_ERR_FILEWRITE; 

    auto retVal = writer.WriteCurrentPage();
    return retVal.second;
}

