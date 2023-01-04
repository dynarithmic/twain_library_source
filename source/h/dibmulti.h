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
#ifndef DIBMULTI_H
#define DIBMULTI_H

#define DIB_MULTI_FIRST     1
#define DIB_MULTI_NEXT      2
#define DIB_MULTI_LAST      3

#include <string>
#include "ctlobstr.h"
namespace dynarithmic
{
    struct DibMultiPageData
    {
        virtual ~DibMultiPageData() = default;
    };

    struct DibMultiPageStruct
    {
        int Stage = 0;
        int Page = 0;
        std::shared_ptr<DibMultiPageData> pUserData;
        CTL_StringType strName;
        DibMultiPageStruct() = default;
    };
}
#endif
