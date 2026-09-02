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
#ifndef CTLFILESAVEDEFS_H
#define CTLFILESAVEDEFS_H

#include <vector>
#include <string>
#include "mapdefs.h"
#include "ctlstringdefs.h"

namespace dynarithmic
{
    struct FileSaveNode
    {
        int m_FileType;
        CTL_StringType m_sTotalFilter;
        CTL_StringType m_sExtension;
        FileSaveNode();
        FileSaveNode(int fType, CTL_StringType filter1, CTL_StringType filter2, CTL_StringType ext);
        CTL_StringType& GetTotalFilter() { return m_sTotalFilter; }
        CTL_StringType& GetExtension() { return m_sExtension; }
    };

    struct FileFormatNode
    {
        std::string m_formatName;
        std::vector<std::string> m_vExtensions;
        FileFormatNode(std::string name, std::vector<std::string> vExt) :
            m_formatName(std::move(name)), m_vExtensions(std::move(vExt)) {}
    };

    using CTL_FileSaveMap = BASIC_MAPTYPE_<int, FileSaveNode>;
    using CTL_AvailableFileFormatsMap = BASIC_MAPTYPE_<LONG, FileFormatNode>;
}
#endif
