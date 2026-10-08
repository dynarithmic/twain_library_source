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
#include "ctldtwainhandle.h"
#include "ctlconstexprutils.h"
#include "ctlsourceacquire.h"
#include "errorcheck.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

//////////////////// Source information functions /////////////////////////
extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetSaveFileType(DTWAIN_SOURCE Source, LONG FileType)
    {
        LOG_FUNC_ENTRY_PARAMS((Source, FileType))
        auto [pHandle, pSource] = VerifyHandles(Source);

        // Must be a supported type and not multipage being passed in
        if ( IsSupportedFileType(Source, FileType, FALSE) && 
             !IsFileTypeMultiPage(static_cast<CTL_TwainFileFormatEnum>(FileType)))
        {
            // Get the acquire file status and check the current type
            auto& acquireStatus = pSource->GetAcquireFileStatusRef();
            bool isMultiPage = IsFileTypeMultiPage(acquireStatus.GetAcquireFileFormat());

            // Cannot reset multipage file type.  Only works if file acquire type being 
            // acquired is single-page type.
            DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return isMultiPage; }, 
                                                  DTWAIN_ERR_FILE_FORMAT, FALSE, FUNC_MACRO);

            // Reset the file type
            acquireStatus.SetAcquireFileFormat(static_cast<CTL_TwainFileFormatEnum>(FileType));
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        }
        DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_FILE_FORMAT, FALSE, FUNC_MACRO);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
        CATCH_BLOCK_LOG_PARAMS(false)
    }

    LONG DLLENTRY_DEF DTWAIN_GetSaveFileType(DTWAIN_SOURCE Source)
    {
        LOG_FUNC_ENTRY_PARAMS((Source))
        auto [pHandle, pSource] = VerifyHandles(Source);
        auto& acquireStatus = pSource->GetAcquireFileStatusRef();
        LOG_FUNC_EXIT_NONAME_PARAMS((acquireStatus.GetAcquireFileFormat()))
        CATCH_BLOCK_LOG_PARAMS(-1)
    }
}
