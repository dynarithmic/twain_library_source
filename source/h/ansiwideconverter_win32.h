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
#ifndef ANSIWIDECONVERTER_WIN32_H
#define ANSIWIDECONVERTER_WIN32_H
#include <windows.h>
#include <string>
#include <limits>

namespace dynarithmic
{
    namespace utfconverter
    {
        inline std::pair<std::wstring, bool> UTF8ToUTF16(std::string_view utf8)
        {
            if (utf8.empty())
                return { {}, true };

            int size = MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),static_cast<int>(utf8.size()),nullptr,0);

            if (size == 0)
                return { {}, false };

            std::wstring result(size, 0);

            MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,utf8.data(),static_cast<int>(utf8.size()),result.data(),size);

            return { result, true };
        }

        inline std::pair<std::string, bool> UTF16ToUTF8(std::wstring_view utf16string)
        {
            if (utf16string.empty())
            {
                return { {}, true };
            }

            int size = WideCharToMultiByte(CP_UTF8,0,utf16string.data(),static_cast<int>(utf16string.size()),nullptr,0,nullptr,nullptr);

            if (size == 0)
                return { {}, false };

            std::string result(size, 0);
            int chars_converted = WideCharToMultiByte(CP_UTF8,0,utf16string.data(),static_cast<int>(utf16string.size()),&result[0],size,nullptr,nullptr);
            result.resize(chars_converted);
            return { result, true };
        }
    }
}
#endif // ANSIWIDECONVERTER_WIN32_H
