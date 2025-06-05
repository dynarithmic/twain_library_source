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
#include <cstdio>
#include <functional>
#include <algorithm>
#include "cppfunc.h"
#include "errorcheck.h"
using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireStripSizes( DTWAIN_SOURCE Source, LPDWORD lpMin, LPDWORD lpMax, LPDWORD lpPreferred )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpMin, lpMax, lpPreferred))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    TW_SETUPMEMXFER Xfer;
    const bool bRet = CTL_TwainAppMgr::GetMemXferValues(pSource, &Xfer)?true:false;

    if( bRet )
    {
        if( lpMin )
            *lpMin = Xfer.MinBufSize;

        if( lpMax )
            *lpMax = Xfer.MaxBufSize;

        if( lpPreferred )
            *lpPreferred = Xfer.Preferred;
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpMin, lpMax, lpPreferred))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireStripBuffer(DTWAIN_SOURCE Source, HANDLE hMem)
{
    LOG_FUNC_ENTRY_PARAMS((Source,hMem))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    if ( !hMem )
    {
        pSource->SetUserStripBuffer(nullptr);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    else
    {
        const SIZE_T dSize = ImageMemoryHandler::GlobalSize( hMem );
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !dSize; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
        pSource->SetUserStripBuffer(hMem);
        pSource->SetUserStripBufSize(dSize);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireStripSize(DTWAIN_SOURCE Source, DWORD StripSize)
{
    LOG_FUNC_ENTRY_PARAMS((Source, StripSize))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    DWORD MinSize, MaxSize;
    if ( StripSize == 0 )
    {
        pSource->SetUserStripBufSize(StripSize);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }

    if ( DTWAIN_GetAcquireStripSizes(Source, &MinSize, &MaxSize, nullptr) )
    {
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return StripSize < MinSize || StripSize > MaxSize;},
                                                    DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    else
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{ return true;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    pSource->SetUserStripBufSize(StripSize);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

HANDLE DLLENTRY_DEF DTWAIN_GetAcquireStripBuffer(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const HANDLE h = pSource->GetUserStripBuffer();
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK_LOG_PARAMS(HANDLE())
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireStripData(DTWAIN_SOURCE Source, LPLONG lpCompression, LPDWORD lpBytesPerRow,
                                                    LPDWORD lpColumns, LPDWORD lpRows, LPDWORD XOffset,
                                                    LPDWORD YOffset, LPDWORD lpBytesWritten)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpCompression, lpBytesPerRow,lpColumns, lpRows, XOffset,YOffset, lpBytesWritten))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const TW_IMAGEMEMXFER* pBuffer = pSource->GetBufferStripData();

    if ( lpCompression )
        *lpCompression = pBuffer->Compression;
    if ( lpBytesPerRow)
        *lpBytesPerRow = pBuffer->BytesPerRow;
    if ( lpColumns )
        *lpColumns = pBuffer->Columns;
    if ( lpRows )
        *lpRows = pBuffer->Rows;
    if ( XOffset )
        *XOffset = pBuffer->XOffset;
    if ( YOffset )
        *YOffset = pBuffer->YOffset;
    if ( lpBytesWritten)
        *lpBytesWritten = pBuffer->BytesWritten;
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpCompression, lpBytesPerRow, lpColumns, lpRows, XOffset, YOffset, lpBytesWritten))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}