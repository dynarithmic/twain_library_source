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

using namespace dynarithmic;
int CTL_BmpIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct*)
{
    if ( !m_pDib )
        return DTWAIN_ERR_DIB;

    const HANDLE hDib = m_pDib->GetHandle();
    if ( !hDib )
        return DTWAIN_ERR_DIB;
    if ( !IsValidBitDepth(DTWAIN_BMP, m_pDib->GetBitsPerPixel()))
            return DTWAIN_ERR_INVALID_BITDEPTH;

    HANDLE hHandleToWrite = CTL_TwainDib::CreateBMPBitmapFromDIB(hDib);
    if (hHandleToWrite)
    {
        auto pBytes = (char*)ImageMemoryHandler::GlobalLock(hHandleToWrite);
        DTWAINGlobalHandleUnlockFree_RAII raii(hHandleToWrite);
        std::ofstream ofs(StringConversion::Convert_NativePtr_To_Ansi(szFile), std::ios::binary);
        if (!ofs)
            return DTWAIN_ERR_FILEWRITE;
        auto nBytes = ImageMemoryHandler::GlobalSize(hHandleToWrite);
        ofs.write(pBytes, nBytes);
        ofs.close();
        if (!ofs)
            return DTWAIN_ERR_FILEWRITE;
        return DTWAIN_NO_ERROR;
    }
    return DTWAIN_ERR_DIB;
}

