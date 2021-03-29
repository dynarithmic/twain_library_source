/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2021 Dynarithmic Software.

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
#include "ctltwmgr.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctldib.h"
#include "errorcheck.h"
using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FlipBitmap( HANDLE hDib )
{
    LOG_FUNC_ENTRY_PARAMS((hDib))
    CTL_TwainDLLHandle *pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // Flip bitmap
    CTL_TwainDib ThisDib;

    ThisDib.SetHandle(hDib);
    ThisDib.FlipBitMap(TRUE);

    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}
