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
#include <base64encode.h>

static const char kBase64Table[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64Encode(const uint8_t* inData, std::string& out, size_t len)
{
    out.clear();
    out.reserve(((len + 2) / 3) * 4);

    for (size_t i = 0; i < len; i += 3)
    {
        uint32_t n = inData[i] << 16;
        if (i + 1 < len) 
            n |= inData[i + 1] << 8;
        if (i + 2 < len) 
            n |= inData[i + 2];

        out.push_back(kBase64Table[(n >> 18) & 63]);
        out.push_back(kBase64Table[(n >> 12) & 63]);

        if (i + 1 < len)
            out.push_back(kBase64Table[(n >> 6) & 63]);
        else
            out.push_back('=');

        if (i + 2 < len)
            out.push_back(kBase64Table[n & 63]);
        else
            out.push_back('=');
    }

    return out.size();
}
