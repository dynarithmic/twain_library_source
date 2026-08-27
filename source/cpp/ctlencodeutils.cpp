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

#include <string>
#include <string_view>
#include "ctlencodeutils.h"

#include "ctlstringutilsx.h"
#include "zlib.h"

#ifdef _MSC_VER
    #pragma message ("Using ZLib version " ZLIB_VERSION)
#endif

using namespace dynarithmic;

////////////////////////////////////////////////////////
// A85 Encode function
std::string A85Encoder::EncodeA85(std::string_view strIn)
{
    strOut.clear();
    strOut.reserve(strIn.length());
    for (const char ch : strIn)
        processA85char(ch);
    cleanup85();
    return strOut;
}

void A85Encoder::processA85char(unsigned c)
{
    c = c & 0x00FF;
    switch (count++)
    {
        case 0: tuple |= c << 24; break;
        case 1: tuple |= c << 16; break;
        case 2: tuple |= c <<  8; break;
        case 3:
            tuple |= c;
            if (tuple == 0)
            {
                strOut += 'z';
                if (pos++ >= width)
                {
                    pos = 0;
                    strOut += '\n';
                }
            }
            else
                encode(tuple, count);
            tuple = 0;
            count = 0;
        break;
    }
}

void A85Encoder::encode(unsigned long tupleParam, int countParam)
{
    char buf[5], *s = buf;
    int i = 5;
    do {
        *s++ = static_cast<char>(tupleParam % 85);
        tupleParam /= 85;
    } while (--i > 0);
    i = countParam;
    do
    {
        char ch = *--s;
        ch += '!';
        strOut += ch;
        if (pos++ >= width)
        {
            pos = 0;
            strOut += '\n';
        }
    } while (i-- > 0);
}

void A85Encoder::cleanup85(void)
{
    if (count > 0)
        encode(tuple, count);
    if (pos + 2 > width)
        strOut += '\n';
    strOut += "~>";
}

int dynarithmic::ASCII85Encode(std::string_view inData, std::string& outData)
{
    A85Encoder encoder;
    outData = encoder.EncodeA85(inData);
    return 1;
}
//////////////////////////////////////////////////////
// HexEncode utility
int dynarithmic::ASCIIHexEncode(std::string_view inData, std::string& outData)
{
    outData = HexStringFromUChars<std::string>(reinterpret_cast<const std::make_unsigned_t<std::string::value_type>*>(inData.data()), 
                                                inData.size(), true);
    outData += '>';
    return 1;
}
/////////////////////////////////////////////
// Base64 Encode
static constexpr char kBase64Table[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int dynarithmic::Base64Encode(const uint8_t* inData, std::string& out, size_t len)
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

    return static_cast<int>(out.size());
}
/////////////////////////////////////////////////
// Flate Encode
int dynarithmic::FlateEncode(std::string_view inData, std::string& outData)
{
    unsigned long compressedLen = static_cast<long>(static_cast<double>(inData.size()) * 1.2 + 12);
    outData.resize(compressedLen);
    const int result = compress2(reinterpret_cast<unsigned char*>(outData.data()), &compressedLen, reinterpret_cast<const unsigned char*>(inData.data()), inData.size(), 9);
    outData.resize(compressedLen);
    return result;
}
