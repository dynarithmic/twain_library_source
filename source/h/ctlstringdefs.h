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
#ifndef CTLSTRINGDEFS_H
#define CTLSTRINGDEFS_H

#include <string>
#include <vector>
#include <sstream>

namespace dynarithmic
{
    using StringArray = std::vector<std::string>;
    using StringArrayW = std::vector<std::wstring>;
    using StringStreamA = std::stringstream;
    using StringStreamW = std::wstringstream;
    using StringStreamOutA = std::ostringstream;
    using StringStreamOutW = std::wostringstream;
    using StringStreamInA = std::istringstream;
    using StringStreamInW = std::wistringstream;
    using OutputFileStreamW = std::wofstream;
    using OutputBaseStreamW = std::wostream;
    using OutputFileStreamA = std::ofstream;
    using OutputBaseStreamA = std::ostream;
    using InputFileStreamA = std::ifstream;
    using InputFileStreamW = std::wifstream;
    using InputBaseStreamW = std::wistream;
    using InputBaseStreamA = std::istream;
    using CTL_StringTypeA = std::string;
    using CTL_StringTypeW = std::wstring;
 
#ifdef UNICODE 
    using CTL_StringType = std::wstring;
    using CTL_StringViewType = std::wstring_view;
    using CTL_StringArrayType = StringArrayW;
    using CTL_StringStreamType = std::wostringstream;
    using CTL_StringStreamInType = std::wistringstream;
    using CTL_InputFileStreamType = std::wifstream;
    using CTL_InputBaseStreamType = std::wistream; 
    using CTL_OutputFileStreamType = std::wofstream;
    using CTL_OutputBaseStreamType = std::wostream; 
#else 
    using CTL_StringType = std::string;
    using CTL_StringViewType = std::string_view;
    using CTL_StringArrayType = StringArray;
    using CTL_StringStreamType = std::ostringstream;
    using CTL_StringStreamInType = std::istringstream;
    using CTL_InputFileStreamType = std::ifstream;
    using CTL_InputBaseStreamType = std::istream; 
    using CTL_OutputFileStreamType = std::ofstream;
    using CTL_OutputBaseStreamType = std::ostream; 
#endif
}
#endif