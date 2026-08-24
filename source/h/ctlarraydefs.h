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
#ifndef CTLARRAYDEFS_H
#define CTLARRAYDEFS_H

#include <vector>
#include "twain.h"
#include "dtwain_standard_defs.h"

namespace dynarithmic
{
    using CTL_IntArray = std::vector<int>;
    using CTL_TwainCapArray = std::vector<TW_UINT16>;
    using CTL_RealArray = std::vector<double>;
    using CTL_HDIBArray = std::vector<HANDLE>;
}
#endif
