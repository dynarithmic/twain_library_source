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
#include "errorcheck.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

// Sets the ICAP_XFERMECH and ICAP_COMPRESSION caps with the Mode and lFileType values
static bool ImageFileFormatCapHandler(DTWAIN_SOURCE Source, CTL_TwainDLLHandle* pHandle, LONG lFileType, LONG Mode)
{
    std::array<std::pair<LONG, LONG>, 2> capsToSet = { { {ICAP_XFERMECH, Mode}, {ICAP_IMAGEFILEFORMAT, lFileType} } };
    for (auto& val : capsToSet)
    {
        DTWAIN_ARRAY tempArray1 = CreateArrayFromCap(pHandle, nullptr, val.first, 1);
        DTWAINArrayPtr_RAII raii1(pHandle, &tempArray1);
        auto& tempBuffer1 = pHandle->m_ArrayFactory->underlying_container_t<LONG>(tempArray1);
        tempBuffer1[0] = val.second;

        // Set the capability
        bool bOk = DTWAIN_SetCapValuesEx2(Source, val.first, DTWAIN_CAPSET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, tempArray1);
        if (!bOk)
            return false;

        // Don't set the file format
        if (val.first == ICAP_XFERMECH && Mode == TWSX_MEMORY)
            return bOk;
    }
    return true;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetFileCompressionType(DTWAIN_SOURCE Source, LPLONG lpCompression)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpCompression))
    auto [pHandle, pSource] = VerifyHandles(Source);
    if (lpCompression)
        *lpCompression = pSource->GetCompressionType();
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpCompression))
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(FALSE)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileCompressionType(DTWAIN_SOURCE Source, LONG lCompression, DTWAIN_BOOL bIsCustom)
{
    static constexpr std::array<LONG, 15> aCompressionFormats =
    { {
        TWCP_NONE,
        TWCP_PACKBITS,
        TWCP_GROUP31D,
        TWCP_GROUP31DEOL,
        TWCP_GROUP32D,
        TWCP_GROUP4,
        TWCP_JPEG,
        TWCP_LZW,
        TWCP_JBIG,
        TWCP_PNG,
        TWCP_RLE4,
        TWCP_RLE8,
        TWCP_BITFIELDS,
        TWCP_ZIP,
        TWCP_JPEG2000
    } };

    LOG_FUNC_ENTRY_PARAMS((Source, lCompression))
    auto [pHandle, pSource] = VerifyHandles(Source);
    if (!bIsCustom)
    {
        auto iter = std::find(aCompressionFormats.begin(),
            aCompressionFormats.end(), lCompression);
        if ( iter == aCompressionFormats.end() )
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, [] {return true; }, DTWAIN_ERR_INVALID_PARAM, NULL, FUNC_MACRO);
    }
    pSource->SetCompressionType(lCompression);
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(FALSE)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumCompressionTypesEx2(DTWAIN_SOURCE Source, LONG lFileType, DTWAIN_BOOL bUseBufferedMode)
{
    struct ResetImageFormatRAII
    {
        DTWAIN_SOURCE Source;
        CTL_TwainDLLHandle* pHandle;
        std::array<LONG, 2> origSetting = {};
        bool bDoReset;
        ResetImageFormatRAII(DTWAIN_SOURCE theSource, CTL_TwainDLLHandle* theHandle, 
                             const std::array<LONG, 2>& origXferMech) :
            Source(theSource), pHandle(theHandle), origSetting(origXferMech), bDoReset(true) {}
        ~ResetImageFormatRAII()
        {
            if (bDoReset)
                ImageFileFormatCapHandler(Source, pHandle, origSetting[0], origSetting[1]);
        }
    };

    LOG_FUNC_ENTRY_PARAMS((Source, lFileType))
    if (!Source)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto lMode = bUseBufferedMode ? TWSX_MEMORY : TWSX_FILE;

    // Check if already retrieved.
    auto& compressionMap = pSource->GetCompressionMap();
    auto iter1 = compressionMap.find(lMode);
    if ( iter1 != compressionMap.end())
    {
        // Already did the compression transfer type.  See if the file type has been done
        std::vector<LONG>* vValues = nullptr;
        auto& mapValues = iter1->second;
        if ( lMode == TWSX_MEMORY )
            vValues = &mapValues[0];
        else
        {
            // See if the file type was resolved already
            auto iter2 = mapValues.find(lFileType);
            if (iter2 != mapValues.end())
                vValues = &iter2->second;
        }
        if (vValues)
        {
            // Already resolved, so just create an array, copy, and return
            DTWAIN_ARRAY aValues = DTWAIN_ArrayCreate(DTWAIN_ARRAYLONG, vValues->size());
            if (aValues)
            {
                auto& vCurrentValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aValues);
                std::copy(vValues->begin(), vValues->end(), vCurrentValues.begin());
                return aValues;
            }
            else
            {
                // No memory, so return error
                DTWAIN_SetLastError(DTWAIN_ERR_MEM);
                LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
            }
        }
    }

    // Check if the file type is supported
    if (lMode == TWSX_FILE)
    {
        BOOL bFileGood = DTWAIN_IsFileXferSupported(Source, lFileType);
        if (!bFileGood)
        {
            // Set the compression map file type to empty vector
            compressionMap[TWSX_FILE][lFileType] = {};

            // Not supported, so return error
            DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        }
    }

    // 1) Get the current file format
    DTWAIN_ARRAY aCurrentFileFormat;
    std::vector<LONG>* ptrVectCurrentFormat = nullptr;
    bool bGotCurrentFileFormat = DTWAIN_GetCapValuesEx2(Source, ICAP_IMAGEFILEFORMAT, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aCurrentFileFormat);
    if (bGotCurrentFileFormat && lMode == TWSX_FILE)
    {
        DTWAINArrayPtr_RAII raii2(pHandle, &aCurrentFileFormat);
        auto& vCurrentFormat = pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCurrentFileFormat);
        if (vCurrentFormat.empty())
        {
            // return error
            DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        }
        ptrVectCurrentFormat = &pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCurrentFileFormat);
    }

    // 3) Get the current xfermech format
    DTWAIN_ARRAY aCurrentXferMech;
    DTWAIN_GetCapValuesEx2(Source, ICAP_XFERMECH, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &aCurrentXferMech);
    DTWAINArrayPtr_RAII raii3(pHandle, &aCurrentXferMech);
    auto& vCurrentXferMech= pHandle->m_ArrayFactory->underlying_container_t<LONG>(aCurrentXferMech);

    auto bChangedOk = ImageFileFormatCapHandler(Source, pHandle, lFileType, lMode);
    if (!bChangedOk)
    {
        // return error
        DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }

    // This resets the ICAP_IMAGEFILEFORMAT and ICAP_XFERMECH caps back to the original value on return
    ResetImageFormatRAII resetter(Source, pHandle, { ptrVectCurrentFormat?ptrVectCurrentFormat->front():-1, vCurrentXferMech.front() });

    // 5) Get the compressions for this type
    auto tempCompression = DTWAIN_EnumCompressionTypesEx(Source);

    // 6) Set the compression map
    auto& vAllTypes = pHandle->m_ArrayFactory->underlying_container_t<LONG>(tempCompression);
    if (lMode == TWSX_MEMORY)
        lFileType = 0;
    compressionMap[lMode][lFileType] = vAllTypes;

    LOG_FUNC_EXIT_NONAME_PARAMS(tempCompression)
    CATCH_BLOCK(DTWAIN_ARRAY{})
}

