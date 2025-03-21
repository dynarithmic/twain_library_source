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
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

LONG DLLENTRY_DEF DTWAIN_GetPaperSizeName(LONG paperNumber, LPTSTR outName, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((paperNumber, outName, nSize))
    VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto& pdfmediamap = CTL_StaticData::GetPDFMediaMap();
    auto iter = pdfmediamap.find(paperNumber);
    LONG nActualCharactersCopied = 0;
    if (iter != pdfmediamap.end())
    { 
        CTL_StringType pageName = StringConversion::Convert_Ansi_To_Native(iter->second.first);
        nActualCharactersCopied = StringWrapper::CopyInfoToCString(pageName, outName, nSize);
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((outName))
    LOG_FUNC_EXIT_NONAME_PARAMS(nActualCharactersCopied)
    CATCH_BLOCK(-1)
}
