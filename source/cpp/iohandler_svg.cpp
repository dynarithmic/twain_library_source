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
#include <string>
#include "ctliface.h"
#include "iohandler_svg.h"
#include "ctldib32ex.h"
#include "svgwriter.h"

using namespace dynarithmic;

static bool WriteOneDibHandleToSvg(const std::wstring& filename, const SvgSessionOptions& options, HANDLE hDib)
{
    LockedDibPage lockedPage(hDib);
    if (!lockedPage.IsValid())
        return false;

    SvgSessionWriter writer;
    if (!writer.Open(filename, options))
        return false;

    auto pageInfo = SvgSessionWriter::MakePreparedSvgPage(lockedPage.GetView());
    if (!pageInfo.has_value())
        return false;

    if (!writer.SetPageInfo(pageInfo.value()))
        return false;

    if (!writer.WritePage())
        return false;

    writer.Close();
    return true;
}


int CTL_SVGIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct*)
{
    HANDLE hDib = hDib = m_pDib->GetHandle();

    SvgSessionOptions opts{};
    if (m_isSVGZ)
        opts.type = SvgOutputType::Svgz;
    else
        opts.type = SvgOutputType::Svg;

    // Get the comment string (copyright information)
    opts.comment = CTL_StaticData::GetAppTitleHTML(); 

    std::wstring fName = StringConversion::Convert_NativePtr_To_Wide(szFile);

    if (!WriteOneDibHandleToSvg(fName, opts, hDib))
        return DTWAIN_ERR_FILEWRITE;
    return DTWAIN_NO_ERROR;
}
