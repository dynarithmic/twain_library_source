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
#ifndef CTLENCODEUTILS_H
#define CTLENCODEUTILS_H

#include <string>
#include <string_view>
#include <array>

namespace dynarithmic
{
    class A85Encoder
    {
        public:
            A85Encoder() : count(0), width(72), pos(0), tuple(0) { }
            std::string EncodeA85(std::string_view strIn);

        private:
            int count = 0;
            unsigned long width = 0;
            unsigned long pos = 0;
            unsigned long tuple = 0;
            void processA85char(unsigned c);
            void cleanup85();
            void encode(unsigned long, int count);
            std::string strOut;
    };

    int FlateEncode(std::string_view inData, std::string& outData);
    int ASCII85Encode(std::string_view inData, std::string& outData);
    int ASCIIHexEncode(std::string_view inData, std::string& outData);
    int Base64Encode(const uint8_t* inData, std::string& outData, size_t len);
}
#endif