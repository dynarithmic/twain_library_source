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
#ifndef CTLSOURCESELECT_H
#define CTLSOURCESELECT_H
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>
#include "dtwtype.h"
#include "ctlstringdefs.h"
#include "dtwain_standard_defs.h"

namespace dynarithmic
{
    struct SourceSelectionOptions;
    class CTL_TwainDLLHandle;

    struct CustomPlacement
    {
        LONG nOptions;
        int xpos;
        int ypos;
        HWND hWndParent;
        std::wstring sTitle;  // This will always be a UTF16 title
        std::vector<CTL_StringType> aIncludeNames;
        std::vector<CTL_StringType> aExcludeNames;
        std::unordered_map<CTL_StringType, CTL_StringType> mapNames;
        CustomPlacement() : nOptions(0), xpos(0), ypos(0), hWndParent(nullptr) {}
    };

    struct SelectStruct
    {
        CTL_StringType SourceName;
        CustomPlacement CS;
        CTL_TwainDLLHandle* pHandle;
        std::function<std::vector<TCHAR>(SelectStruct&)> getDefaultFunc;
        std::function<std::vector<CTL_StringType>(SelectStruct&)> getNameListFunc;
        int nItems;
    };

    DTWAIN_SOURCE SourceSelect(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& options);
}
#endif