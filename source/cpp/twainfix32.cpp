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

#include "twainfix32.h"

namespace dynarithmic
{
    float Fix32ToFloat(TW_FIX32 Fix32)
    {
        return static_cast<float>(Fix32.Whole) + static_cast<float>(Fix32.Frac) / static_cast<float>(65536.0);
    }

    TW_FIX32 FloatToFix32(float fnum)
    {
        TW_FIX32 fix32_value {};
        const bool sign = fnum < 0 ? true : false;
        auto value = static_cast<TW_INT32>(fnum * 65536.0 + (sign ? -0.5 : 0.5));
        fix32_value.Whole = static_cast<TW_INT16>(value >> 16);
        fix32_value.Frac = static_cast<TW_UINT16>(value & 0x0000ffffL);
        return fix32_value;
    }
}
