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
#include "ctllogfunctioncall.h"
#include "ctlstringutilsx.h"

using namespace dynarithmic;

void ParamOutputter::LogType(std::string_view outStr, const char* ptr)
{
    // ptr must be a pointer to a valid null terminated string, or nullptr.
    if (ptr)
        strm << outStr << "=\"" << TruncateStringWithMore(ptr, 256)
        << "\" (" << "0x" << std::hex << static_cast<const void*>(ptr) << ")" << std::dec;
    else
        strm << outStr << "=(null)";
}

void ParamOutputter::LogType(std::string_view outStr, const wchar_t* ptr)
{
    // ptr must be a pointer to a valid null terminated string, or nullptr.
    if (ptr)
        strm << outStr << "=\"" <<
        TruncateStringWithMore(StringConversion::Convert_WidePtr_To_Ansi(ptr), 256) <<
        "\" (" << "0x" << std::hex << static_cast<const void*>(ptr) << ")" << std::dec;
    else
        strm << outStr << "=(null)";
}

