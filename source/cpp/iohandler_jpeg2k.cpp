/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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

using namespace dynarithmic;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTL_Jpeg2KIOHandler::CTL_Jpeg2KIOHandler(CTL_TwainDib* pDib, const DTWAINImageInfoEx &ImageInfoEx)
: CTL_ImageIOHandler( pDib ), m_nFormat(0), m_ImageInfoEx(ImageInfoEx), m_pJpegHandler(nullptr)
{ }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CTL_Jpeg2KIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    if ( !m_pDib )
        return DTWAIN_ERR_DIB;

    const HANDLE hDib = m_pDib->GetHandle();
    if ( !hDib )
        return DTWAIN_ERR_DIB;

    if (!IsValidBitDepth(DTWAIN_JPEG2000, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

    return SaveToFile(hDib, szFile, FIF_J2K, 0, m_ImageInfoEx.UnitOfMeasure, { m_ImageInfoEx.ResolutionX, m_ImageInfoEx.ResolutionY });
}
