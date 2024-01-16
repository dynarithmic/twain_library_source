/*
This file is part of the Dynarithmic TWAIN Library (DTWAIN).
Copyright (c) 2002-2024 Dynarithmic Software.

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
#include "zlib.h"
#include "flateencode.h"

int FlateEncode(const std::string&inData, std::string& outData)
{
    unsigned long compressedLen = static_cast<long>(static_cast<double>(inData.size()) * 1.2 + 12);
    outData.resize(compressedLen);
    const int result = compress2(reinterpret_cast<unsigned char*>(&outData[0]), &compressedLen,reinterpret_cast<const unsigned char*>(inData.data()), static_cast<uLong>(inData.size()), 9);
    outData.resize(compressedLen);
    return result;
}
