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
#include "iohandler_wmf.h"
#include "metafilewriter.h"
#include "ctldib32ex.h"

using namespace dynarithmic;

static bool WriteOneDibHandleToMetafile(const std::wstring& filename, HANDLE hDib, const MetafileSessionOptions& options)
{
    LockedDibPage locked(hDib);
    if (!locked.IsValid())
        return false;

    MetafileSessionWriter writer;
    if (!writer.Open(filename, options))
        return false;

	auto pageInfo = MetafileSessionWriter::MakePreparedMetafileDibPage(locked.GetView());
	if (!pageInfo.has_value())
		return false;

    if (!writer.WritePage(pageInfo.value()))
        return false;

    return writer.Close();
}

int CTL_WmfIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    MetafileSessionOptions opts;
    if (m_nFormat == CTL_TwainDib::WmfFormat)
        opts.type = MetafileType::Wmf;
    else
        opts.type = MetafileType::Emf;

    opts.description = StringConversion::Convert_Ansi_To_Wide(GetCopyrightString());
    std::wstring sFileName = StringConversion::Convert_NativePtr_To_Wide(szFile);
    auto retval = WriteOneDibHandleToMetafile(sFileName, m_pDib->GetHandle(), opts);
    if (!retval)
        return DTWAIN_ERR_FILEWRITE;
    return DTWAIN_NO_ERROR;
}
