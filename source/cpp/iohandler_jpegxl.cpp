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

#include <windows.h>
#include "iohandler_jpegxl.h"
#include "ctldib32ex.h"
#include "ctlstaticdata.h"
#include "ctlstringconversion.h"
#include "ctltwainlogging.h"

using namespace dynarithmic;

void CTL_JPegXLIOHandler::InitializeJXLSupport()
{
    auto jxlPath = stringconversion::Convert_Native_To_Wide(CTL_StaticData::GetJPEGXLPath());
    auto jxlLoadVal = CTL_StaticData::GetJPEGXLLoader().Load(jxlPath); 
    if ( !jxlLoadVal.first )
    {
        jxlError.windowsError = jxlLoadVal.second;
        m_bJxlAvailable = false;
        LogWin32Error(jxlError.windowsError);
        return;
    }
    m_bJxlAvailable = jxlLoadVal.first;
}

int CTL_JPegXLIOHandler::WriteBitmap(LPCTSTR szFile, bool, int, DibMultiPageStruct*) 
{
    if (!m_bJxlAvailable)
        return DTWAIN_ERR_JXL_NOTAVAILABLE;

    if (!m_pDib || !szFile) 
        return DTWAIN_ERR_FILEWRITE;

    LockedDibPage locked(m_pDib->GetHandle());
    if (!locked.IsValid()) 
        return DTWAIN_ERR_FILEWRITE;

    const auto page = JxlSessionWriter::MakePreparedJxlPage(locked.GetView());
    if (!page) 
        return DTWAIN_ERR_FILEWRITE;
    const std::wstring filename = stringconversion::Convert_NativePtr_To_Wide(szFile);
    JxlSessionWriter writer;
    if (m_ImageInfoEx.nJpegXLQuality < 100)
    {
        options_.lossless = false;
        options_.qualitySetting = m_ImageInfoEx.nJpegXLQuality;
    }

    options_.copyright = GetCopyrightString();

    if (!writer.Open(filename, options_) || !writer.SetPageInfo(*page) || !writer.WriteCurrentPage()) 
        return DTWAIN_ERR_FILEWRITE;
    writer.Close();
    return DTWAIN_NO_ERROR;
}
