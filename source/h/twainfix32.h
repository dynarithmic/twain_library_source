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
#ifndef TWAINFIX32_H
#define TWAINFIX32_H
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <twain.h>

namespace dynarithmic
{
    /* Fixed point structure type. */
    struct TW_FIX32Ex : TW_FIX32
    {
        TW_FIX32Ex(TW_FIX32 fix32)
        {
            Whole = fix32.Whole;
            Frac = fix32.Frac;
        }

        TW_FIX32Ex()
        {
            Whole = 0;
            Frac = 0;
        }

        bool operator==(const TW_FIX32Ex& rhs) const
        {
            return Whole == rhs.Whole && Frac == rhs.Frac;
        }
        bool operator!=(const TW_FIX32Ex& rhs) const
        {
            return !(*this == rhs);
        }
    };

    float Fix32ToFloat(TW_FIX32 Fix32);
    TW_FIX32  FloatToFix32(float fnum);
    TW_FIX32Ex FloatToFix32Ex(float fnum);
}
#endif
