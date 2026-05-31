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

#include "imagefilewriterbase.h"

using namespace dynarithmic;
LockedDibPage::LockedDibPage(HANDLE hDib) : dib_(hDib)
{
    if (!dib_.IsValid())
        return;

    view_.width = dib_.Width();
    view_.height = dib_.Height();
    view_.bitsPerPixel = dib_.BitsPerPixel();
    view_.strideBytes = dib_.StrideBytes();
    view_.bottomUp = dib_.BottomUp();
    view_.bits = dib_.Bits();
    view_.palette = dib_.Palette();
    view_.paletteEntries = dib_.PaletteEntries();
    view_.bih = dib_.Header();
    view_.xDPI = static_cast<uint16_t>(dib_.XDpi());
    view_.yDPI = static_cast<uint16_t>(dib_.YDpi());
    valid_ = true;
}

