/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#ifndef CTLFILEUTILS_H
#define CTLFILEUTILS_H

#include "ctliface.h"

namespace dynarithmic
{
    bool delete_file(LPCTSTR filename);
    std::pair<bool, std::string> parent_directory_exists(LPCTSTR filename);
    bool file_exists(LPCTSTR filename);
    std::pair<bool, std::string> create_directory(LPCTSTR dirName);
    CTL_StringType temp_directory_path(bool bWithSeparator=true);
    CTL_StringType get_parent_directory(LPCTSTR filename);
    bool directory_writeable(LPCTSTR filename);
}
#endif
