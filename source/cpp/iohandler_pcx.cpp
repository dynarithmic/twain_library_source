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
#include "ctltwainmanager.h"
#include "pcxwriter.h"
#include "iohandler_pcx.h"

using namespace dynarithmic;
int CTL_PcxIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* pMultiPageStruct)
{
    HANDLE hDib = nullptr;

    if ( !pMultiPageStruct || pMultiPageStruct->Stage != DIB_MULTI_LAST )
    {
        if ( !m_pDib )
            return DTWAIN_ERR_DIB;

        hDib = m_pDib->GetHandle();
        if ( !hDib )
            return DTWAIN_ERR_DIB;
    }

    if (pMultiPageStruct && pMultiPageStruct->Stage != DIB_MULTI_LAST && !IsValidBitDepth(DTWAIN_PCX, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

    bool bIsFirstPage = (!pMultiPageStruct || pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_FIRST);
    bool bIsLastPage = (!pMultiPageStruct || pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_LAST);
    bool isDCX = (m_nFormat == DTWAIN_DCX);

    std::wstring filename = StringConversion::Convert_NativePtr_To_Wide(szFile);

    if ( bIsFirstPage )
    {
		LockedDibPage locked(hDib);
		if (!locked.IsValid())
			return DTWAIN_ERR_FILEWRITE;

		PcxSessionOptions opts{};
		opts.writeDcx = isDCX;

		auto pageInfo = PcxSessionWriter::MakePreparedPcxDibPage(locked.GetView());
		if (!pageInfo.has_value())
			return false;

		if (!output.OnFirstPage(filename, opts, pageInfo.value()))
			return DTWAIN_ERR_FILEWRITE;
        return DTWAIN_NO_ERROR;
	}
    else
    if ( !bIsLastPage && isDCX)
    {
		LockedDibPage locked(hDib);
		if (!locked.IsValid())
			return DTWAIN_ERR_FILEWRITE;

		auto pageInfo = PcxSessionWriter::MakePreparedPcxDibPage(locked.GetView());
		if (!pageInfo.has_value())
			return false;

		if (!output.OnNextPage(pageInfo.value()))
			return DTWAIN_ERR_FILEWRITE;
        return DTWAIN_NO_ERROR;
    }
    else
    if ( bIsLastPage || !isDCX )
    {
		if (!output.OnLastPage())
			return DTWAIN_ERR_FILEWRITE;
        return DTWAIN_NO_ERROR;
    }
    return DTWAIN_NO_ERROR;
}
