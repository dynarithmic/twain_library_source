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
using namespace dynarithmic;

DTWAIN_DIBUPDATE_PROC DLLENTRY_DEF DTWAIN_SetUpdateDibProc(DTWAIN_DIBUPDATE_PROC DibProc)
{
    LOG_FUNC_ENTRY_PARAMS((DibProc))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const DTWAIN_DIBUPDATE_PROC oldproc = pHandle->m_pDibUpdateProc;
    pHandle->m_pDibUpdateProc = DibProc;
    LOG_FUNC_EXIT_NONAME_PARAMS(oldproc)
    CATCH_BLOCK(DTWAIN_DIBUPDATE_PROC())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_DeleteDIB(HANDLE Dib)
{
    LOG_FUNC_ENTRY_PARAMS((Dib))
    BOOL bOk = ImageMemoryHandler::GlobalUnlock(Dib);
    if ( !bOk )
        LOG_FUNC_EXIT_NONAME_PARAMS(0)
    ImageMemoryHandler::GlobalFree(Dib);
    LOG_FUNC_EXIT_NONAME_PARAMS(bOk)
    CATCH_BLOCK(FALSE)
}
