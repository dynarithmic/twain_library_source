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
#include "cppfunc.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

static bool ImageFileFormatCapHandler(DTWAIN_SOURCE Source, CTL_TwainDLLHandle* pHandle, LONG lFileType)
{
    // Set up for the handling of setting ICAP_IMAGEFILEFORMAT
    DTWAIN_ARRAY tempArray = CreateArrayFromCap(pHandle, nullptr, ICAP_IMAGEFILEFORMAT, 1);
    DTWAINArrayPtr_RAII raii4(pHandle, &tempArray);
    auto& tempBuffer = pHandle->m_ArrayFactory->underlying_container_t<LONG>(tempArray);
    tempBuffer[0] = lFileType;

    // Set the capability
    return DTWAIN_SetCapValuesEx2(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, tempArray);
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumCompressionTypesEx2(DTWAIN_SOURCE Source, LONG lFileType)
{
    struct ResetImageFormatRAII
    {
        DTWAIN_SOURCE Source;
        CTL_TwainDLLHandle* pHandle;
        LONG origSetting;
        bool bDoReset;
        ResetImageFormatRAII(DTWAIN_SOURCE theSource, CTL_TwainDLLHandle* theHandle, LONG orig) :
            Source(theSource), pHandle(theHandle), origSetting(orig), bDoReset(true) {}
        ~ResetImageFormatRAII()
        {
            if (bDoReset)
                ImageFileFormatCapHandler(Source, pHandle, origSetting);
        }
    };

    LOG_FUNC_ENTRY_PARAMS((Source, lFileType))
    if (!Source)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    auto [pHandle, pSource] = VerifyHandles(Source);

    // Check if the file type is supported
    BOOL bFileGood = DTWAIN_IsFileXferSupported(Source, lFileType);
    if (!bFileGood)
    {
        // Not supported, so return error
        DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }

    // 1) Get the current file format
    DTWAIN_ARRAY aCurrentFileFormat;
    DTWAIN_GetCapValuesEx2(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aCurrentFileFormat);
    DTWAINArrayPtr_RAII raii2(pHandle, &aCurrentFileFormat);
    auto& vCurrentFormat = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCurrentFileFormat);

    // 2) Return error if current format cannot be retrieved
    if (vCurrentFormat.empty())
    {
        // Not supported, so return error
        DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }

    // 3) If the current format is not equal to lFileFormat, need to change temporarily to lFileFormat
    if (vCurrentFormat.front() != lFileType)
    {
        auto bChangedOk = ImageFileFormatCapHandler(Source, pHandle, lFileType);
        if (!bChangedOk)
        {
            // return error
            DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        }
    }

    // This resets the ICAP_IMAGEFILEFORMAT cap back to the original value on return
    ResetImageFormatRAII resetter(Source, pHandle, vCurrentFormat.front());

    // 4) Get the compressions for this type
    auto tempCompression = DTWAIN_EnumCompressionTypesEx(Source);

    // 5) Don't need to reset the compression type back to the original if current
    //    is the same as the original
    if (vCurrentFormat.front() == lFileType)
        resetter.bDoReset = false;
    auto& vFormats = pHandle->m_ArrayFactory->underlying_container_t<LONG>(tempCompression);
    LOG_FUNC_EXIT_NONAME_PARAMS(tempCompression)
    CATCH_BLOCK(DTWAIN_ARRAY{})
}

