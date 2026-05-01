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
#include "iohandler_ps.h"
#include "ctliface.h"
#include "ctldib32ex.h"

using namespace dynarithmic;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////      Postscript handler    /////////////////////////////////////////////////////////////////////////////
CTL_PSIOHandler::CTL_PSIOHandler(CTL_TwainDib* pDib, int /*nFormat*/, const DTWAINImageInfoEx &ImageInfoEx)
                                 :   CTL_ImageIOHandler( pDib ),
                                    m_nFormat(0),
                                    m_ImageInfoEx(ImageInfoEx)
{
}

int CTL_PSIOHandler::WriteBitmap(LPCTSTR szFile, bool bOpenFile, int /*fhFile*/, DibMultiPageStruct* pMultiPageStruct)
{
    // Now add this to PDF page
    bool isFirstPage = (!pMultiPageStruct || pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_FIRST);
    bool isLastPage = (!pMultiPageStruct || pMultiPageStruct->Stage == 0 || pMultiPageStruct->Stage == DIB_MULTI_LAST);
    bool isNextPage = (pMultiPageStruct && pMultiPageStruct->Stage == DIB_MULTI_NEXT);

    if ( isFirstPage )
    {
        const int bitdepth = m_pDib->GetDepth();

        if (!IsValidBitDepth(DTWAIN_PS_ENCAPSULATED, bitdepth))
            return DTWAIN_ERR_INVALID_BITDEPTH;

		LockedDibPage page(m_pDib->GetHandle());
		if (!page.IsValid())
			return DTWAIN_ERR_FILEWRITE;

		PsSessionOptions opts{};

		switch (m_ImageInfoEx.PostscriptType)
		{
		    case DTWAIN_POSTSCRIPT1:
		    case DTWAIN_POSTSCRIPT1MULTI:
				opts.level = PsLevel::Level1;
			    break;

		    case DTWAIN_POSTSCRIPT2:
		    case DTWAIN_POSTSCRIPT2MULTI:
				opts.level = PsLevel::Level2;
                opts.invert1bpp = true;
				break;

			case DTWAIN_POSTSCRIPT3:
			case DTWAIN_POSTSCRIPT3MULTI:
				opts.level = PsLevel::Level3;
				opts.invert1bpp = true;
				break;
		}

		std::wstring fName = StringConversion::Convert_NativePtr_To_Wide(szFile);

		opts.creator = GetCopyrightString();

		if (!m_psSessionWriter.Open(fName, opts))
			return DTWAIN_ERR_FILEWRITE;

		auto pageInfo = PsSessionWriter::MakePreparedPsDibPage(page.GetView());
		if (!pageInfo.has_value())
			return DTWAIN_ERR_FILEWRITE; 

		auto retVal = m_psSessionWriter.WritePage(pageInfo.value());
        return retVal ? DTWAIN_NO_ERROR : DTWAIN_ERR_FILEWRITE;
    }
    else
    if ( isNextPage )
    {
		LockedDibPage page(m_pDib->GetHandle());
		if (!page.IsValid())
			return DTWAIN_ERR_FILEWRITE;

		auto pageInfo = PsSessionWriter::MakePreparedPsDibPage(page.GetView());
		if (!pageInfo.has_value())
			return DTWAIN_ERR_FILEWRITE;

		auto retVal = m_psSessionWriter.WritePage(pageInfo.value());
		return retVal ? DTWAIN_NO_ERROR : DTWAIN_ERR_FILEWRITE;
    }
    else
    if ( isLastPage )
    {
		bool ok = m_psSessionWriter.Close();
		return ok?DTWAIN_NO_ERROR:DTWAIN_ERR_FILEWRITE;
    }
    return DTWAIN_NO_ERROR;
}
