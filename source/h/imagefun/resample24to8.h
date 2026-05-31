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
#ifndef RESAMPLE24TO8_H
#define RESAMPLE24TO8_H

#include <vector>
#include <algorithm>
#include <windows.h>
#include "dibutil.h"

#undef min
#undef max

inline HGLOBAL Create8bppGrayDib(uint32_t width, uint32_t height, bool bottomUp, LONG xPelsPerMeter,LONG yPelsPerMeter)
{
    const uint32_t paletteCount = 256;
    const uint32_t stride = dynarithmic::dib::calc_stride_bytes(width, 8);
    const uint32_t imageSize = stride * height;
    const size_t totalSize = sizeof(BITMAPINFOHEADER) + paletteCount * sizeof(RGBQUAD) +imageSize;

    HGLOBAL hDib = ::GlobalAlloc(GHND, totalSize);
    if (!hDib)
        return nullptr;

    void* p = ::GlobalLock(hDib);
    if (!p)
    {
        ::GlobalFree(hDib);
        return nullptr;
    }

    std::memset(p, 0, totalSize);

    auto* bih = static_cast<BITMAPINFOHEADER*>(p);
    bih->biSize = sizeof(BITMAPINFOHEADER);
    bih->biWidth = static_cast<LONG>(width);
    bih->biHeight = bottomUp ? static_cast<LONG>(height) : -static_cast<LONG>(height);
    bih->biPlanes = 1;
    bih->biBitCount = 8;
    bih->biCompression = BI_RGB;
    bih->biSizeImage = imageSize;
    bih->biXPelsPerMeter = xPelsPerMeter;
    bih->biYPelsPerMeter = yPelsPerMeter;
    bih->biClrUsed = paletteCount;
    bih->biClrImportant = paletteCount;

    auto* pal = reinterpret_cast<RGBQUAD*>(reinterpret_cast<uint8_t*>(bih) + bih->biSize);
    for (uint32_t i = 0; i < paletteCount; ++i)
    {
        pal[i].rgbRed = static_cast<BYTE>(i);
        pal[i].rgbGreen = static_cast<BYTE>(i);
        pal[i].rgbBlue = static_cast<BYTE>(i);
        pal[i].rgbReserved = 0;
    }

    ::GlobalUnlock(hDib);
    return hDib;
}

// 24-bpp BGR DIB -> 8-bpp grayscale DIB
// Returns a newly allocated DIB handle or nullptr on failure.
inline HGLOBAL Convert24bppDibTo8bppGray(HANDLE hSrcDib)
{
    dynarithmic::dib::LockedDib src(hSrcDib);
    if (!src.IsValid())
        return nullptr;
    if (src.BitsPerPixel() != 24)
        return nullptr;

    HGLOBAL hDst = Create8bppGrayDib(src.Width(), src.Height(), src.BottomUp(), src.Header()->biXPelsPerMeter, src.Header()->biYPelsPerMeter);
    if (!hDst)
        return nullptr;

    dynarithmic::dib::LockedDib dst(hDst);
    if (!dst.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    const uint32_t width = src.Width();
    const uint32_t height = src.Height();
    const uint32_t srcStride = src.StrideBytes();
    const uint32_t dstStride = dst.StrideBytes();

    const uint8_t* srcBits = src.Bits();
    uint8_t* dstBits = dst.Bits();
    if (!srcBits || !dstBits)
        return nullptr;

    for (uint32_t y = 0; y < height; ++y)
    {
        const uint32_t srcY = src.BottomUp() ? (height - 1 - y) : y;
        const uint32_t dstY = dst.BottomUp() ? (height - 1 - y) : y;

        const uint8_t* sRow = srcBits + static_cast<size_t>(srcY) * srcStride;
        uint8_t* dRow = dstBits + static_cast<size_t>(dstY) * dstStride;

        for (uint32_t x = 0; x < width; ++x)
        {
            const uint8_t b = sRow[x * 3 + 0];
            const uint8_t g = sRow[x * 3 + 1];
            const uint8_t r = sRow[x * 3 + 2];

            // Integer luminance approximation:
            // gray = 0.299R + 0.587G + 0.114B
            const uint8_t gray = static_cast<uint8_t>((77u * r + 150u * g + 29u * b + 128u) >> 8);

            dRow[x] = gray;
        }

        // Zero row padding for cleanliness
        if (dstStride > width)
            std::memset(dRow + width, 0, dstStride - width);
    }

    return hDst;
}

// Optional RAII-friendly wrapper
inline dynarithmic::dib::unique_dib Convert24bppDibTo8bppGrayUnique(HANDLE hSrcDib)
{
    return dynarithmic::dib::unique_dib(Convert24bppDibTo8bppGray(hSrcDib));
}
#endif
