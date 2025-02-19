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
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "errorcheck.h"

using namespace dynarithmic;

/////////////////////////////  TIFF Settings ///////////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTIFFInvert(DTWAIN_SOURCE Source, LONG Setting)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Setting))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetPhotometric( !Setting );
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTIFFCompressType(DTWAIN_SOURCE Source, LONG Setting)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Setting))
    auto [pHandle, pSource] = VerifyHandles(Source);
    // Must be in state 4 or higher
    auto& acquireFileStatus = pSource->GetAcquireFileStatusRef();

    const SourceState theState = pSource->GetState();
    const bool bIsTiff = dynarithmic::IsFileTypeTIFF(static_cast<CTL_TwainFileFormatEnum>(Setting));
    LONG curAcquireType = acquireFileStatus.GetAcquireFileFormat();
    const bool bIsCurTiff = dynarithmic::IsFileTypeTIFF(static_cast<CTL_TwainFileFormatEnum>(curAcquireType));

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return theState < SOURCE_STATE_UIENABLED;}, DTWAIN_ERR_INVALID_STATE, false,
                                        FUNC_MACRO);

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !bIsTiff;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !bIsCurTiff;}, DTWAIN_ERR_FILE_FORMAT, false, FUNC_MACRO);

    const bool bIsTiffMulti = dynarithmic::IsFileTypeMultiPage(static_cast<CTL_TwainFileFormatEnum>(curAcquireType));
    if (bIsTiffMulti)
        acquireFileStatus.SetAcquireFileFormat(dynarithmic::GetMultiPageType(static_cast<CTL_TwainFileFormatEnum>(Setting)));
    else
        acquireFileStatus.SetAcquireFileFormat(static_cast<CTL_TwainFileFormatEnum>(Setting));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}
