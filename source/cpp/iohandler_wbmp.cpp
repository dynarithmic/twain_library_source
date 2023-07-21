/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#include "../cximage/ximage.h"

using namespace dynarithmic;

int CTL_WBMPIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    if ( !m_pDib )
        return DTWAIN_ERR_DIB;

    HANDLE hDib = m_pDib->GetHandle();
    if ( !hDib )
        return DTWAIN_ERR_DIB;

    if (!IsValidBitDepth(DTWAIN_WBMP, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

    int height = m_pDib->GetHeight();
    int width = m_pDib->GetWidth();

    if ( height > 255 || width > 255)
    {
        if (m_ImageInfoEx.IsWBMPResized)
        {
            height = 255;
            width = 255;
            if (width == 0)
                return DTWAIN_ERR_INVALIDWBMP;
            m_pDib->ResampleDib({static_cast<double>(width), static_cast<double>(height)}, CTL_ITwainSource::RESIZE_FLAG);
            hDib = m_pDib->GetHandle();
        }
        else
            return DTWAIN_ERR_INVALIDWBMP; 
    }

    if (!parent_directory_exists(szFile).first)
        return DTWAIN_ERR_FILEOPEN;

    return SaveToFile(hDib, szFile, FIF_WBMP, 0, DTWAIN_INCHES, { 0, 0 });
}
