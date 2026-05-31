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

#include <algorithm>
#include <cmath>
#include "dibutil.h"

/* These DIB utilities replace the previous CxImage functions */

using namespace dynarithmic::dib;

uint8_t dynarithmic::dib::GetIndexedPixel(const uint8_t* row, uint32_t x, uint16_t bpp)
{
    switch (bpp)
    {
        case 1:
            return static_cast<uint8_t>((row[x / 8] >> (7 - (x & 7))) & 1);

        case 4:
            return static_cast<uint8_t>(
                (x & 1) ? (row[x / 2] & 0x0F) : ((row[x / 2] >> 4) & 0x0F));

        case 8:
            return row[x];

        default:
            return 0;
    }
}

static HGLOBAL CreateDibLikeEx(uint32_t width, int32_t height, uint16_t dstBpp, 
                               LONG xPelsPerMeter, LONG yPelsPerMeter, uint32_t paletteEntries)
{
    const uint32_t stride = calc_stride_bytes(width, dstBpp);
    const uint32_t imageSize = stride * height;

    const size_t totalSize = sizeof(BITMAPINFOHEADER) + paletteEntries * sizeof(RGBQUAD) + imageSize;

    HGLOBAL hDst = GlobalAlloc(GHND, totalSize);
    if (!hDst)
        return nullptr;

    auto* dst = static_cast<BITMAPINFOHEADER*>(GlobalLock(hDst));
    if (!dst)
    {
        GlobalFree(hDst);
        return nullptr;
    }

    std::memset(dst, 0, totalSize);

    dst->biSize = sizeof(BITMAPINFOHEADER);
    dst->biWidth = width;
    dst->biHeight =
        height > 0 ? height : -height;
    dst->biPlanes = 1;
    dst->biBitCount = dstBpp;
    dst->biCompression = BI_RGB;
    dst->biSizeImage = imageSize;
    dst->biXPelsPerMeter = xPelsPerMeter;
    dst->biYPelsPerMeter = yPelsPerMeter;
    dst->biClrUsed = paletteEntries;
    dst->biClrImportant = paletteEntries;

    GlobalUnlock(hDst);
    return hDst;
}

HGLOBAL dynarithmic::dib::CreateDibLike(const BITMAPINFOHEADER& src, uint16_t dstBpp, uint32_t paletteEntries)
{
    return CreateDibLikeEx(src.biWidth, src.biHeight, dstBpp, src.biXPelsPerMeter, src.biYPelsPerMeter, paletteEntries);
}

static HGLOBAL CreateCroppedDibLike(const BITMAPINFOHEADER& src, uint32_t newWidth, uint32_t newHeight, uint16_t bpp, uint32_t paletteEntries)
{
    return CreateDibLikeEx(newWidth, newHeight, bpp, src.biXPelsPerMeter, src.biYPelsPerMeter, paletteEntries);
}

uint8_t dynarithmic::dib::NearestPaletteIndex(const RGBQUAD* pal, uint32_t count, const RGBQUAD& c)
{
    uint32_t best = 0;
    uint32_t bestDist = 0xFFFFFFFFu;

    for (uint32_t i = 0; i < count; ++i)
    {
        const int dr = int(c.rgbRed) - int(pal[i].rgbRed);
        const int dg = int(c.rgbGreen) - int(pal[i].rgbGreen);
        const int db = int(c.rgbBlue) - int(pal[i].rgbBlue);

        const uint32_t dist = static_cast<uint32_t>(dr * dr + dg * dg + db * db);

        if (dist < bestDist)
        {
            bestDist = dist;
            best = i;

            if (dist == 0)
                break;
        }
    }

    return static_cast<uint8_t>(best);
}

void dynarithmic::dib::FillGrayPalette(RGBQUAD* pal, uint32_t entries)
{
    if (!pal || entries == 0)
        return;

    for (uint32_t i = 0; i < entries; ++i)
    {
        const uint8_t v = static_cast<uint8_t>(
            entries == 1 ? 0 : (i * 255u) / (entries - 1u));

        pal[i].rgbRed = v;
        pal[i].rgbGreen = v;
        pal[i].rgbBlue = v;
        pal[i].rgbReserved = 0;
    }
}

void dynarithmic::dib::SetIndexedPixel(uint8_t* row, uint32_t x, uint16_t dstBpp, uint8_t idx)
{
    switch (dstBpp)
    {
        case 4:
        {
            uint8_t& b = row[x / 2];
            if ((x & 1) == 0)
                b = static_cast<uint8_t>((b & 0x0F) | ((idx & 0x0F) << 4));
            else
                b = static_cast<uint8_t>((b & 0xF0) | (idx & 0x0F));
            break;
        }

        case 8:
            row[x] = idx;
            break;
        }
}

RGBQUAD dynarithmic::dib::GetPixelAsRgb(const uint8_t* row, uint32_t x, uint16_t srcBpp, const RGBQUAD* srcPal, uint32_t srcPalEntries)
{
    RGBQUAD q{};

    switch (srcBpp)
    {
        case 1:
        case 4:
        case 8:
        {
            const uint8_t idx = GetIndexedPixel(row, x, srcBpp);
            if (srcPal && idx < srcPalEntries)
                return srcPal[idx];

            q.rgbRed = q.rgbGreen = q.rgbBlue = idx;
            return q;
        }

        case 16:
        {
            // Assumption: grayscale 16-bpp BI_RGB.
            const auto* p16 = reinterpret_cast<const uint16_t*>(row);
            const uint16_t v = p16[x];
            const uint8_t g = static_cast<uint8_t>(v >> 8);
            q.rgbRed = q.rgbGreen = q.rgbBlue = g;
            return q;
        }

        case 24:
        {
            const uint8_t* p = row + x * 3;
            q.rgbBlue = p[0];
            q.rgbGreen = p[1];
            q.rgbRed = p[2];
            return q;
        }

        case 32:
        {
            const uint8_t* p = row + x * 4;
            q.rgbBlue = p[0];
            q.rgbGreen = p[1];
            q.rgbRed = p[2];
            q.rgbReserved = p[3];
            return q;
        }

        default:
            return q;
    }
}

HANDLE dynarithmic::dib::IncreaseDibBpp(HANDLE hDib, uint16_t dstBpp)
{
    using namespace dynarithmic::dib;

    LockedDib srcDib(hDib);
    if (!srcDib.IsValid())
        return nullptr;

    const BITMAPINFOHEADER* src = srcDib.Header();
    if (!src)
        return nullptr;

    const uint16_t srcBpp = srcDib.BitsPerPixel();

    if (!IsSupportedBpp(srcBpp) || !IsSupportedBpp(dstBpp))
        return nullptr;

    if (dstBpp <= srcBpp)
        return nullptr;

    if (src->biCompression != BI_RGB)
        return nullptr;

    const uint32_t width = srcDib.Width();
    const uint32_t height = srcDib.Height();

    const uint32_t srcStride = srcDib.StrideBytes();
    const uint8_t* srcBits = srcDib.Bits();

    const RGBQUAD* srcPal = srcBpp <= 8 ? srcDib.Palette() : nullptr;

    const uint32_t srcPalEntries = srcBpp <= 8 ? srcDib.PaletteEntries() : 0;

    uint32_t dstPalEntries = 0;
    if (dstBpp == 4)
        dstPalEntries = 16;
    else if (dstBpp == 8)
        dstPalEntries = 256;

    HGLOBAL hDst = CreateDibLike(*src, dstBpp, dstPalEntries);
    if (!hDst)
        return nullptr;

    LockedDib dstDib(hDst);
    if (!dstDib.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    const uint32_t dstStride = dstDib.StrideBytes();
    uint8_t* dstBits = dstDib.Bits();
    RGBQUAD* dstPal = dstBpp <= 8 ? dstDib.PalettePtrMutable() : nullptr;

    if (dstBpp == 8)
    {
        FillGrayPalette(dstPal, 256);
    }
    else if (dstBpp == 4)
    {
        FillGrayPalette(dstPal, 16);

        if (srcPal && srcPalEntries > 0)
        {
            const uint32_t copyCount = std::min(srcPalEntries, dstPalEntries);
            std::memcpy(dstPal, srcPal, copyCount * sizeof(RGBQUAD));
        }
    }

    for (uint32_t y = 0; y < height; ++y)
    {
        const uint8_t* srow = srcBits + static_cast<size_t>(y) * srcStride;

        uint8_t* drow = dstBits + static_cast<size_t>(y) * dstStride;

        std::memset(drow, 0, dstStride);

        for (uint32_t x = 0; x < width; ++x)
        {
            const RGBQUAD c = GetPixelAsRgb(srow, x, srcBpp, srcPal, srcPalEntries);
            switch (dstBpp)
            {
                case 4:
                {
                    uint8_t idx = 0;

                    if (srcBpp <= 8)
                        idx = GetIndexedPixel(srow, x, srcBpp);
                    else
                        idx = NearestPaletteIndex(dstPal, dstPalEntries, c);

                    SetIndexedPixel(drow, x, 4, idx);
                    break;
                }

                case 8:
                {
                    uint8_t idx = 0;

                    if (srcBpp == 1)
                    {
                        const uint8_t bit = GetIndexedPixel(srow, x, 1);

                        if (srcPal && srcPalEntries >= 2)
                        {
                            const RGBQUAD& q = srcPal[bit];

                            idx = static_cast<uint8_t>(
                                (77u * q.rgbRed +
                                    150u * q.rgbGreen +
                                    29u * q.rgbBlue +
                                    128u) >> 8);
                        }
                        else
                        {
                            idx = bit ? 255 : 0;
                        }
                    }
                    else if (srcBpp <= 8)
                    {
                        idx = GetIndexedPixel(srow, x, srcBpp);
                    }
                    else
                    {
                        idx = NearestPaletteIndex(dstPal, dstPalEntries, c);
                    }

                    drow[x] = idx;
                    break;
                }
                case 16:
                {
                    const uint16_t gray =
                        static_cast<uint16_t>(
                            ((77u * c.rgbRed +
                                150u * c.rgbGreen +
                                29u * c.rgbBlue +
                                128u) >> 8) * 257u);

                    reinterpret_cast<uint16_t*>(drow)[x] = gray;
                    break;
                }

                case 24:
                {
                    uint8_t* p = drow + x * 3;
                    p[0] = c.rgbBlue;
                    p[1] = c.rgbGreen;
                    p[2] = c.rgbRed;
                    break;
                }

                case 32:
                {
                    uint8_t* p = drow + x * 4;
                    p[0] = c.rgbBlue;
                    p[1] = c.rgbGreen;
                    p[2] = c.rgbRed;
                    p[3] = 0xFF;
                    break;
                }
            }
        }
    }

    return hDst;
}


static uint8_t GrayFromRGB(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint8_t>(
        (77u * r + 150u * g + 29u * b + 128u) >> 8);
}

static uint8_t GrayFromRGBQUAD(const RGBQUAD& q)
{
    return GrayFromRGB(q.rgbRed, q.rgbGreen, q.rgbBlue);
}

static uint8_t GetPixelGray(const uint8_t* row, uint32_t x, uint16_t srcBpp, const RGBQUAD* srcPal, uint32_t srcPalEntries)
{
    switch (srcBpp)
    {
        case 1:
        case 4:
        case 8:
        {
            const uint8_t idx = dynarithmic::dib::GetIndexedPixel(row, x, srcBpp);

            if (srcPal && idx < srcPalEntries)
                return GrayFromRGBQUAD(srcPal[idx]);

            if (srcBpp == 1)
                return idx ? 255 : 0;

            return idx;
        }

        case 16:
        {
            const auto* p16 = reinterpret_cast<const uint16_t*>(row);
            return static_cast<uint8_t>(p16[x] >> 8);
        }

        case 24:
        case 32:
        {
            const uint8_t* p = row + x * (srcBpp/8);
            return GrayFromRGB(p[2], p[1], p[0]); // BGR
        }
    }

    return 0;
}

HANDLE dynarithmic::dib::DecreaseDibBpp(HANDLE hDib, uint16_t dstBpp)
{
    LockedDib srcDib(hDib);
    if (!srcDib.IsValid())
        return nullptr;

    const BITMAPINFOHEADER* src = srcDib.Header();
    if (!src)
        return nullptr;

    const uint16_t srcBpp = srcDib.BitsPerPixel();

    if (!IsSupportedBpp(srcBpp))
        return nullptr;

    switch (dstBpp)
    {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
            break;

        default:
            return nullptr;
    }

    if (dstBpp >= srcBpp)
        return nullptr;

    // In this policy, 24-bpp reduction is only meaningful as 32 -> 24.
    if (dstBpp == 24 && srcBpp != 32)
        return nullptr;

    if (src->biCompression != BI_RGB)
        return nullptr;

    const uint32_t width = srcDib.Width();
    const uint32_t height = srcDib.Height();

    const uint8_t* srcBits = srcDib.Bits();
    const uint32_t srcStride = srcDib.StrideBytes();

    const RGBQUAD* srcPal = srcBpp <= 8 ? srcDib.Palette() : nullptr;
        const uint32_t srcPalEntries = srcBpp <= 8 ? srcDib.PaletteEntries() : 0;

    uint32_t dstPalEntries = 0;
    if (dstBpp == 1)
        dstPalEntries = 2;
    else if (dstBpp == 4)
        dstPalEntries = 16;
    else if (dstBpp == 8)
        dstPalEntries = 256;

    HGLOBAL hDst = CreateDibLike(*src, dstBpp, dstPalEntries);
    if (!hDst)
        return nullptr;

    LockedDib dstDib(hDst);
    if (!dstDib.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    uint8_t* dstBits = dstDib.Bits();
    const uint32_t dstStride = dstDib.StrideBytes();

    RGBQUAD* dstPal = dstBpp <= 8 ? dstDib.PalettePtrMutable() : nullptr;

    if (dstBpp <= 8)
        FillGrayPalette(dstPal, dstPalEntries);

    for (uint32_t y = 0; y < height; ++y)
    {
        const uint8_t* srow = srcBits + static_cast<size_t>(y) * srcStride;
        uint8_t* drow = dstBits + static_cast<size_t>(y) * dstStride;
        std::memset(drow, 0, dstStride);
        for (uint32_t x = 0; x < width; ++x)
        {
            switch (dstBpp)
            {
                case 1:
                {
                    const uint8_t gray = GetPixelGray(srow, x, srcBpp, srcPal, srcPalEntries);

                    // 0 = black, 1 = white for normal 1-bpp DIB palette.
                    const uint8_t bit = gray >= 128 ? 1 : 0;

                    uint8_t& byte = drow[x / 8];
                    if (bit)
                        byte |= static_cast<uint8_t>(0x80 >> (x & 7));

                    break;
                }

                case 4:
                {
                    const uint8_t gray = GetPixelGray(srow, x, srcBpp, srcPal, srcPalEntries);
                    const uint8_t idx = static_cast<uint8_t>((gray * 15u + 127u) / 255u);
                    SetIndexedPixel(drow, x, 4, idx);
                    break;
                }

                case 8:
                {
                    drow[x] = GetPixelGray(srow, x, srcBpp, srcPal, srcPalEntries);
                    break;
                }

                case 16:
                {
                    const uint8_t gray = GetPixelGray(srow, x, srcBpp, srcPal, srcPalEntries);

                    const uint16_t gray16 = static_cast<uint16_t>(gray * 257u);

                    reinterpret_cast<uint16_t*>(drow)[x] = gray16;
                    break;
                }

                case 24:
                {
                    // 32 -> 24 only. Drop alpha.
                    const uint8_t* pSrc = srow + x * 4;
                    uint8_t* pDst = drow + x * 3;

                    pDst[0] = pSrc[0]; // B
                    pDst[1] = pSrc[1]; // G
                    pDst[2] = pSrc[2]; // R
                    break;
                }
            }
        }
    }
    return hDst;
}

HGLOBAL dynarithmic::dib::CloneDibHandle(HANDLE hDib)
{
    if (!hDib)
        return nullptr;

    const SIZE_T size = ::GlobalSize(hDib);
    if (size == 0)
        return nullptr;

    void* src = ::GlobalLock(hDib);
    if (!src)
        return nullptr;

    HGLOBAL hNew = ::GlobalAlloc(GHND, size);
    if (!hNew)
    {
        ::GlobalUnlock(hDib);
        return nullptr;
    }

    void* dst = ::GlobalLock(hNew);
    if (!dst)
    {
        ::GlobalFree(hNew);
        ::GlobalUnlock(hDib);
        return nullptr;
    }

    std::memcpy(dst, src, size);

    ::GlobalUnlock(hNew);
    ::GlobalUnlock(hDib);

    return hNew;
}

// Caller owns returned HANDLE and must GlobalFree() it.
HANDLE dynarithmic::dib::NegateDib(HANDLE hDib)
{
    using namespace dynarithmic::dib;

    if (!hDib)
        return nullptr;

    HGLOBAL hNew = CloneDibHandle(hDib);
    if (!hNew)
        return nullptr;

    LockedDib dib(hNew);
    if (!dib.IsValid())
    {
        ::GlobalFree(hNew);
        return nullptr;
    }

    BITMAPINFOHEADER* bih = dib.HeaderMutable();
    if (!bih || bih->biCompression != BI_RGB)
    {
        ::GlobalFree(hNew);
        return nullptr;
    }

    const uint16_t bpp = dib.BitsPerPixel();
    const uint32_t width = dib.Width();
    const uint32_t height = dib.Height();
    const uint32_t stride = dib.StrideBytes();

    uint8_t* bits = dib.Bits();
    RGBQUAD* pal = dib.PalettePtrMutable();
    const uint32_t palEntries = dib.PaletteEntries();

    switch (bpp)
    {
        case 1:
        case 4:
        case 8:
        {
            if (is_grayscale_palette(pal, palEntries))
            {
                // Grayscale indexed image:
                // invert actual pixel indices/data.
                for (uint32_t y = 0; y < height; ++y)
                {
                    uint8_t* row = bits + static_cast<size_t>(y) * stride;
                    for (uint32_t i = 0; i < stride; ++i)
                        row[i] = static_cast<uint8_t>(~row[i]);
                }
            }
            else
            {
                // Paletted color image:
                // keep indices unchanged, invert palette colors.
                for (uint32_t i = 0; i < palEntries; ++i)
                {
                    pal[i].rgbRed = static_cast<BYTE>(255 - pal[i].rgbRed);
                    pal[i].rgbGreen = static_cast<BYTE>(255 - pal[i].rgbGreen);
                    pal[i].rgbBlue = static_cast<BYTE>(255 - pal[i].rgbBlue);
                }
            }
            break;
        }

        case 16:
        {
            // Treat 16-bpp BI_RGB as 16-bit grayscale.
            for (uint32_t y = 0; y < height; ++y)
            {
                auto* row = reinterpret_cast<uint16_t*>(
                    bits + static_cast<size_t>(y) * stride);

                for (uint32_t x = 0; x < width; ++x)
                    row[x] = static_cast<uint16_t>(0xFFFFu - row[x]);
            }
            break;
        }

        case 24:
        case 32:
        {
            for (uint32_t y = 0; y < height; ++y)
            {
                uint8_t* row = bits + static_cast<size_t>(y) * stride;

                for (uint32_t x = 0; x < width; ++x)
                {
                    uint8_t* p = row + x * (bpp/8);
                    p[0] = static_cast<uint8_t>(255 - p[0]); // B
                    p[1] = static_cast<uint8_t>(255 - p[1]); // G
                    p[2] = static_cast<uint8_t>(255 - p[2]); // R
                }
            }
            break;
        }

        default:
            ::GlobalFree(hNew);
            return nullptr;
    }
    return hNew;
}

/// /////////////////////////////////////////////////////////////////////////
static uint8_t GetPackedPixel(const uint8_t* row, uint32_t x, uint16_t bpp)
{
    switch (bpp)
    {
        case 1:
            return static_cast<uint8_t>((row[x / 8] >> (7 - (x & 7))) & 1);

        case 4:
            return static_cast<uint8_t>((x & 1)
                    ? (row[x / 2] & 0x0F)
                    : ((row[x / 2] >> 4) & 0x0F));

        case 8:
            return row[x];

        default:
            return 0;
    }
}

static void SetPackedPixel(uint8_t* row, uint32_t x, uint16_t bpp, uint8_t value)
{
    switch (bpp)
    {
        case 1:
        {
            const uint8_t mask = static_cast<uint8_t>(0x80 >> (x & 7));
            if (value)
                row[x / 8] |= mask;
            else
                row[x / 8] &= static_cast<uint8_t>(~mask);
            break;
        }

        case 4:
        {
            uint8_t& byte = row[x / 2];

            if ((x & 1) == 0)
                byte = static_cast<uint8_t>((byte & 0x0F) | ((value & 0x0F) << 4));
            else
                byte = static_cast<uint8_t>((byte & 0xF0) | (value & 0x0F));

            break;
        }

        case 8:
            row[x] = value;
            break;
    }
}

HANDLE dynarithmic::dib::CropDib(HANDLE hDib, int left, int top, int right, int bottom)
{
    LockedDib srcDib(hDib);
    if (!srcDib.IsValid())
        return nullptr;

    const BITMAPINFOHEADER* srcHeader = srcDib.Header();
    if (!srcHeader)
        return nullptr;

    if (srcHeader->biCompression != BI_RGB)
        return nullptr;

    const uint16_t bpp = srcDib.BitsPerPixel();

    switch (bpp)
    {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
        case 32:
            break;

        default:
            return nullptr;
    }

    const uint32_t srcWidth = srcDib.Width();
    const uint32_t srcHeight = srcDib.Height();

    if (left < 0) left = 0;
    if (top < 0) top = 0;
    if (right > static_cast<int>(srcWidth)) right = static_cast<int>(srcWidth);
    if (bottom > static_cast<int>(srcHeight)) bottom = static_cast<int>(srcHeight);

    if (right <= left || bottom <= top)
        return nullptr;

    const uint32_t cropWidth = static_cast<uint32_t>(right - left);
    const uint32_t cropHeight = static_cast<uint32_t>(bottom - top);
    const uint32_t paletteEntries = bpp <= 8 ? srcDib.PaletteEntries() : 0;

    HGLOBAL hDst = CreateCroppedDibLike(*srcHeader, cropWidth, cropHeight, bpp, paletteEntries);

    if (!hDst)
        return nullptr;

    LockedDib dstDib(hDst);
    if (!dstDib.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    if (bpp <= 8 && paletteEntries > 0)
    {
        std::memcpy(dstDib.PalettePtrMutable(), srcDib.Palette(), paletteEntries * sizeof(RGBQUAD));
    }

    const uint8_t* srcBits = srcDib.Bits();
    uint8_t* dstBits = dstDib.Bits();

    const uint32_t srcStride = srcDib.StrideBytes();
    const uint32_t dstStride = dstDib.StrideBytes();

    const bool bottomUp = srcDib.BottomUp();

    auto srcRowForLogicalY = [&](uint32_t logicalY) -> const uint8_t*
    {
        const uint32_t physicalY = bottomUp
            ? (srcHeight - 1 - logicalY)
            : logicalY;
        return srcBits + static_cast<size_t>(physicalY) * srcStride;
    };

    auto dstRowForLogicalY = [&](uint32_t logicalY) -> uint8_t*
    {
        const uint32_t physicalY =
            bottomUp
            ? (cropHeight - 1 - logicalY)
            : logicalY;

        return dstBits + static_cast<size_t>(physicalY) * dstStride;
    };

    for (uint32_t y = 0; y < cropHeight; ++y)
    {
        const uint32_t srcY = static_cast<uint32_t>(top) + y;
        const uint8_t* srcRow = srcRowForLogicalY(srcY);
        uint8_t* dstRow = dstRowForLogicalY(y);

        std::memset(dstRow, 0, dstStride);

        switch (bpp)
        {
            case 1:
            case 4:
            {
                for (uint32_t x = 0; x < cropWidth; ++x)
                {
                    const uint32_t srcX = static_cast<uint32_t>(left) + x;
                    const uint8_t v = GetPackedPixel(srcRow, srcX, bpp);
                    SetPackedPixel(dstRow, x, bpp, v);
                }
                break;
            }

            case 8:
            {
                std::memcpy(dstRow,
                    srcRow + left,
                    cropWidth);
                break;
            }

            case 16:
            {
                std::memcpy(dstRow,
                    srcRow + static_cast<size_t>(left) * 2,
                    static_cast<size_t>(cropWidth) * 2);
                break;
            }

            case 24:
            case 32:
            {
                std::memcpy(dstRow,
                    srcRow + static_cast<size_t>(left) * (bpp/8),
                    static_cast<size_t>(cropWidth) * (bpp/8) );
                break;
            }
        }
    }

    return hDst;
}
//////////////////////////////////////////////////////////////////
static double CubicWeight(double x)
{
    // Catmull-Rom cubic filter, common bicubic interpolation kernel.
    x = std::abs(x);

    if (x <= 1.0)
        return (1.5 * x * x * x) - (2.5 * x * x) + 1.0;

    if (x < 2.0)
        return (-0.5 * x * x * x) + (2.5 * x * x) - (4.0 * x) + 2.0;

    return 0.0;
}

static uint8_t ClampU8(double v)
{
    if (v < 0.0) return 0;
    if (v > 255.0) return 255;
    return static_cast<uint8_t>(v + 0.5);
}

static uint16_t ClampU16(double v)
{
    if (v < 0.0) return 0;
    if (v > 65535.0) return 65535;
    return static_cast<uint16_t>(v + 0.5);
}

static const uint8_t* LogicalSrcRow(const dynarithmic::dib::LockedDib& dib,
    uint32_t logicalY)
{
    const uint32_t height = dib.Height();
    const uint32_t physicalY =
        dib.BottomUp()
        ? (height - 1 - logicalY)
        : logicalY;

    return dib.Bits() + static_cast<size_t>(physicalY) * dib.StrideBytes();
}

static uint8_t* LogicalDstRow(dynarithmic::dib::LockedDib& dib,
    uint32_t logicalY)
{
    const uint32_t height = dib.Height();
    const uint32_t physicalY =
        dib.BottomUp()
        ? (height - 1 - logicalY)
        : logicalY;

    return dib.Bits() + static_cast<size_t>(physicalY) * dib.StrideBytes();
}

static uint8_t SampleBicubic8(const dynarithmic::dib::LockedDib& src,
    double srcX,
    double srcY)
{
    const int width = static_cast<int>(src.Width());
    const int height = static_cast<int>(src.Height());

    const int ix = static_cast<int>(std::floor(srcX));
    const int iy = static_cast<int>(std::floor(srcY));

    double sum = 0.0;
    double weightSum = 0.0;

    for (int m = -1; m <= 2; ++m)
    {
        const int yy = std::clamp(iy + m, 0, height - 1);
        const double wy = CubicWeight(srcY - static_cast<double>(iy + m));

        const uint8_t* row = LogicalSrcRow(src, static_cast<uint32_t>(yy));

        for (int n = -1; n <= 2; ++n)
        {
            const int xx = std::clamp(ix + n, 0, width - 1);
            const double wx = CubicWeight(srcX - static_cast<double>(ix + n));
            const double w = wx * wy;

            sum += static_cast<double>(row[xx]) * w;
            weightSum += w;
        }
    }

    if (weightSum != 0.0)
        sum /= weightSum;

    return ClampU8(sum);
}

static uint16_t SampleBicubic16(const dynarithmic::dib::LockedDib& src,
    double srcX,
    double srcY)
{
    const int width = static_cast<int>(src.Width());
    const int height = static_cast<int>(src.Height());

    const int ix = static_cast<int>(std::floor(srcX));
    const int iy = static_cast<int>(std::floor(srcY));

    double sum = 0.0;
    double weightSum = 0.0;

    for (int m = -1; m <= 2; ++m)
    {
        const int yy = std::clamp(iy + m, 0, height - 1);
        const double wy = CubicWeight(srcY - static_cast<double>(iy + m));

        const auto* row =
            reinterpret_cast<const uint16_t*>(LogicalSrcRow(src, static_cast<uint32_t>(yy)));

        for (int n = -1; n <= 2; ++n)
        {
            const int xx = std::clamp(ix + n, 0, width - 1);
            const double wx = CubicWeight(srcX - static_cast<double>(ix + n));
            const double w = wx * wy;

            sum += static_cast<double>(row[xx]) * w;
            weightSum += w;
        }
    }

    if (weightSum != 0.0)
        sum /= weightSum;

    return ClampU16(sum);
}

struct BgraD
{
    double b = 0;
    double g = 0;
    double r = 0;
    double a = 255;
};

static BgraD SampleBicubic24Or32(const dynarithmic::dib::LockedDib& src,
    double srcX,
    double srcY,
    uint16_t bpp)
{
    const int width = static_cast<int>(src.Width());
    const int height = static_cast<int>(src.Height());
    const int bytesPerPixel = bpp == 32 ? 4 : 3;

    const int ix = static_cast<int>(std::floor(srcX));
    const int iy = static_cast<int>(std::floor(srcY));

    BgraD out{};
    out.a = bpp == 32 ? 0.0 : 255.0;

    double weightSum = 0.0;

    for (int m = -1; m <= 2; ++m)
    {
        const int yy = std::clamp(iy + m, 0, height - 1);
        const double wy = CubicWeight(srcY - static_cast<double>(iy + m));

        const uint8_t* row = LogicalSrcRow(src, static_cast<uint32_t>(yy));

        for (int n = -1; n <= 2; ++n)
        {
            const int xx = std::clamp(ix + n, 0, width - 1);
            const double wx = CubicWeight(srcX - static_cast<double>(ix + n));
            const double w = wx * wy;

            const uint8_t* p = row + static_cast<size_t>(xx) * bytesPerPixel;

            out.b += p[0] * w;
            out.g += p[1] * w;
            out.r += p[2] * w;

            if (bpp == 32)
                out.a += p[3] * w;

            weightSum += w;
        }
    }

    if (weightSum != 0.0)
    {
        out.b /= weightSum;
        out.g /= weightSum;
        out.r /= weightSum;
        out.a /= weightSum;
    }

    return out;
}

HANDLE dynarithmic::dib::ResizeDib(HANDLE hDib, int32_t newx, int32_t newy)
{
    using namespace dynarithmic::dib;

    if (newx <= 0 || newy <= 0)
        return nullptr;

    LockedDib src(hDib);
    if (!src.IsValid())
        return nullptr;

    const BITMAPINFOHEADER* srcHeader = src.Header();
    if (!srcHeader || srcHeader->biCompression != BI_RGB)
        return nullptr;

    const uint16_t bpp = src.BitsPerPixel();

    switch (bpp)
    {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
        case 32:
            break;

        default:
            return nullptr;
    }

    const uint32_t srcWidth = src.Width();
    const uint32_t srcHeight = src.Height();

    const uint32_t dstWidth = static_cast<uint32_t>(newx);
    const uint32_t dstHeight = static_cast<uint32_t>(newy);

    const uint32_t paletteEntries = bpp <= 8 ? src.PaletteEntries() : 0;

    HGLOBAL hDst = CreateCroppedDibLike(*srcHeader, dstWidth, dstHeight, bpp, paletteEntries);

    if (!hDst)
        return nullptr;

    LockedDib dst(hDst);
    if (!dst.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    if (bpp <= 8 && paletteEntries > 0)
    {
        std::memcpy(dst.PalettePtrMutable(), src.Palette(), paletteEntries * sizeof(RGBQUAD));
    }

    const double scaleX = static_cast<double>(srcWidth) / static_cast<double>(dstWidth);
    const double scaleY = static_cast<double>(srcHeight) / static_cast<double>(dstHeight);

    for (uint32_t y = 0; y < dstHeight; ++y)
    {
        uint8_t* dstRow = LogicalDstRow(dst, y);
        std::memset(dstRow, 0, dst.StrideBytes());

        const double srcY = (static_cast<double>(y) + 0.5) * scaleY - 0.5;

        switch (bpp)
        {
            case 1:
            case 4:
            {
                // Packed/indexed: nearest neighbor.
                const uint32_t nearestY =
                    static_cast<uint32_t>(
                        std::clamp<int>(
                            static_cast<int>(std::floor(srcY + 0.5)),
                            0,
                            static_cast<int>(srcHeight - 1)));

                const uint8_t* srcRow = LogicalSrcRow(src, nearestY);

                for (uint32_t x = 0; x < dstWidth; ++x)
                {
                    const double srcX =
                        (static_cast<double>(x) + 0.5) * scaleX - 0.5;

                    const uint32_t nearestX =
                        static_cast<uint32_t>(
                            std::clamp<int>(
                                static_cast<int>(std::floor(srcX + 0.5)),
                                0,
                                static_cast<int>(srcWidth - 1)));

                    const uint8_t v = GetPackedPixel(srcRow, nearestX, bpp);
                    SetPackedPixel(dstRow, x, bpp, v);
                }
                break;
            }

            case 8:
            {
                for (uint32_t x = 0; x < dstWidth; ++x)
                {
                    const double srcX =
                        (static_cast<double>(x) + 0.5) * scaleX - 0.5;

                    dstRow[x] = SampleBicubic8(src, srcX, srcY);
                }
                break;
            }

            case 16:
            {
                auto* dst16 = reinterpret_cast<uint16_t*>(dstRow);

                for (uint32_t x = 0; x < dstWidth; ++x)
                {
                    const double srcX =
                        (static_cast<double>(x) + 0.5) * scaleX - 0.5;

                    dst16[x] = SampleBicubic16(src, srcX, srcY);
                }
                break;
            }

            case 24:
            {
                for (uint32_t x = 0; x < dstWidth; ++x)
                {
                    const double srcX =
                        (static_cast<double>(x) + 0.5) * scaleX - 0.5;

                    const BgraD s = SampleBicubic24Or32(src, srcX, srcY, 24);

                    uint8_t* p = dstRow + static_cast<size_t>(x) * 3;
                    p[0] = ClampU8(s.b);
                    p[1] = ClampU8(s.g);
                    p[2] = ClampU8(s.r);
                }
                break;
            }

            case 32:
            {
                for (uint32_t x = 0; x < dstWidth; ++x)
                {
                    const double srcX =
                        (static_cast<double>(x) + 0.5) * scaleX - 0.5;

                    const BgraD s = SampleBicubic24Or32(src, srcX, srcY, 32);

                    uint8_t* p = dstRow + static_cast<size_t>(x) * 4;
                    p[0] = ClampU8(s.b);
                    p[1] = ClampU8(s.g);
                    p[2] = ClampU8(s.r);
                    p[3] = ClampU8(s.a);
                }
                break;
            }
        }
    }

    return hDst;
}

static const uint8_t* LogicalRow(const dynarithmic::dib::LockedDib& d, uint32_t y)
{
    const uint32_t physicalY = d.BottomUp() ? (d.Height() - 1 - y) : y;
    return d.Bits() + static_cast<size_t>(physicalY) * d.StrideBytes();
}

static uint8_t* LogicalRow(dynarithmic::dib::LockedDib& d, uint32_t y)
{
    const uint32_t physicalY = d.BottomUp() ? (d.Height() - 1 - y) : y;
    return d.Bits() + static_cast<size_t>(physicalY) * d.StrideBytes();
}

static void CopyOnePixel(const dynarithmic::dib::LockedDib& src, uint32_t sx, uint32_t sy, dynarithmic::dib::LockedDib& dst, 
                         uint32_t dx, uint32_t dy)
{
    const uint16_t bpp = src.BitsPerPixel();

    const uint8_t* srow = LogicalRow(src, sy);
    uint8_t* drow = LogicalRow(dst, dy);

    switch (bpp)
    {
        case 1:
        case 4:
        case 8:
            SetPackedPixel(drow, dx, bpp, GetPackedPixel(srow, sx, bpp));
            break;

        case 16:
            reinterpret_cast<uint16_t*>(drow)[dx] = reinterpret_cast<const uint16_t*>(srow)[sx];
            break;

        case 24:
        case 32:
            std::memcpy(drow + dx * (bpp / 8), srow + sx * (bpp / 8),  bpp / 8);
            break;
    }
}

static void FillDibWhite(dynarithmic::dib::LockedDib& d)
{
    const uint16_t bpp = d.BitsPerPixel();

    for (uint32_t y = 0; y < d.Height(); ++y)
    {
        uint8_t* row = LogicalRow(d, y);

        switch (bpp)
        {
            case 1:
                std::memset(row, 0xFF, d.StrideBytes());
                break;

            case 4:
                std::memset(row, 0xFF, d.StrideBytes());
                break;

            case 8:
                std::memset(row, 0xFF, d.StrideBytes());
                break;

            case 16:
            {
                auto* p = reinterpret_cast<uint16_t*>(row);
                for (uint32_t x = 0; x < d.Width(); ++x)
                    p[x] = 0xFFFF;
                break;
            }

            case 24:
                for (uint32_t x = 0; x < d.Width(); ++x)
                {
                    row[x * 3 + 0] = 255;
                    row[x * 3 + 1] = 255;
                    row[x * 3 + 2] = 255;
                }
                break;

            case 32:
                for (uint32_t x = 0; x < d.Width(); ++x)
                {
                    row[x * 4 + 0] = 255;
                    row[x * 4 + 1] = 255;
                    row[x * 4 + 2] = 255;
                    row[x * 4 + 3] = 255;
                }
                break;
        }
    }
}


static HANDLE RotateRightAngle(const dynarithmic::dib::LockedDib& src, int angle)
{
    const uint32_t sw = src.Width();
    const uint32_t sh = src.Height();
    const uint16_t bpp = src.BitsPerPixel();

    const uint32_t dw = (angle == 180) ? sw : sh;
    const uint32_t dh = (angle == 180) ? sh : sw;

    const uint32_t palEntries = bpp <= 8 ? src.PaletteEntries() : 0;

    HANDLE hDst = CreateCroppedDibLike(*src.Header(), dw, dh, bpp, palEntries);
    if (!hDst)
        return nullptr;

    dynarithmic::dib::LockedDib dst(hDst);
    if (!dst.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    if (bpp <= 8 && palEntries)
        std::memcpy(dst.PalettePtrMutable(), src.Palette(), palEntries * sizeof(RGBQUAD));

    FillDibWhite(dst);

    for (uint32_t y = 0; y < sh; ++y)
    {
        for (uint32_t x = 0; x < sw; ++x)
        {
            uint32_t dx = 0;
            uint32_t dy = 0;

            if (angle == 90)
            {
                dx = sh - 1 - y;
                dy = x;
            }
            else if (angle == 180)
            {
                dx = sw - 1 - x;
                dy = sh - 1 - y;
            }
            else // 270
            {
                dx = y;
                dy = sw - 1 - x;
            }

            CopyOnePixel(src, x, y, dst, dx, dy);
        }
    }

    return hDst;
}

static void SampleNearestToDst(const dynarithmic::dib::LockedDib& src, double sx, double sy,
                            dynarithmic::dib::LockedDib& dst, uint32_t dx, uint32_t dy)
{
    const uint32_t ix = static_cast<uint32_t>(
        std::clamp<int>(static_cast<int>(std::floor(sx + 0.5)),
            0,
            static_cast<int>(src.Width() - 1)));

    const uint32_t iy = static_cast<uint32_t>(
        std::clamp<int>(static_cast<int>(std::floor(sy + 0.5)),
            0,
            static_cast<int>(src.Height() - 1)));

    CopyOnePixel(src, ix, iy, dst, dx, dy);
}


static void SampleBicubicToDst(const dynarithmic::dib::LockedDib& src, double sx, double sy,
                            dynarithmic::dib::LockedDib& dst, uint32_t dx, uint32_t dy)
{
    const uint16_t bpp = src.BitsPerPixel();
    const int w = static_cast<int>(src.Width());
    const int h = static_cast<int>(src.Height());

    const int ix = static_cast<int>(std::floor(sx));
    const int iy = static_cast<int>(std::floor(sy));

    uint8_t* drow = LogicalRow(dst, dy);

    if (bpp == 8)
    {
        double sum = 0.0, weight = 0.0;

        for (int yy = -1; yy <= 2; ++yy)
        {
            const int py = std::clamp(iy + yy, 0, h - 1);
            const double wy = CubicWeight(sy - (iy + yy));
            const uint8_t* row = LogicalRow(src, py);

            for (int xx = -1; xx <= 2; ++xx)
            {
                const int px = std::clamp(ix + xx, 0, w - 1);
                const double wx = CubicWeight(sx - (ix + xx));
                const double ww = wx * wy;

                sum += row[px] * ww;
                weight += ww;
            }
        }

        drow[dx] = ClampU8(weight ? sum / weight : sum);
        return;
    }

    if (bpp == 16)
    {
        double sum = 0.0, weight = 0.0;

        for (int yy = -1; yy <= 2; ++yy)
        {
            const int py = std::clamp(iy + yy, 0, h - 1);
            const double wy = CubicWeight(sy - (iy + yy));
            const auto* row = reinterpret_cast<const uint16_t*>(LogicalRow(src, py));

            for (int xx = -1; xx <= 2; ++xx)
            {
                const int px = std::clamp(ix + xx, 0, w - 1);
                const double wx = CubicWeight(sx - (ix + xx));
                const double ww = wx * wy;

                sum += row[px] * ww;
                weight += ww;
            }
        }

        reinterpret_cast<uint16_t*>(drow)[dx] =
            ClampU16(weight ? sum / weight : sum);
        return;
    }

    const int bytes = bpp == 32 ? 4 : 3;
    double acc[4] = { 0, 0, 0, bpp == 32 ? 0.0 : 255.0 };
    double weight = 0.0;

    for (int yy = -1; yy <= 2; ++yy)
    {
        const int py = std::clamp(iy + yy, 0, h - 1);
        const double wy = CubicWeight(sy - (iy + yy));
        const uint8_t* row = LogicalRow(src, py);

        for (int xx = -1; xx <= 2; ++xx)
        {
            const int px = std::clamp(ix + xx, 0, w - 1);
            const double wx = CubicWeight(sx - (ix + xx));
            const double ww = wx * wy;

            const uint8_t* p = row + px * bytes;

            acc[0] += p[0] * ww;
            acc[1] += p[1] * ww;
            acc[2] += p[2] * ww;

            if (bpp == 32)
                acc[3] += p[3] * ww;

            weight += ww;
        }
    }

    uint8_t* p = drow + dx * bytes;

    p[0] = ClampU8(weight ? acc[0] / weight : acc[0]);
    p[1] = ClampU8(weight ? acc[1] / weight : acc[1]);
    p[2] = ClampU8(weight ? acc[2] / weight : acc[2]);

    if (bpp == 32)
        p[3] = ClampU8(weight ? acc[3] / weight : acc[3]);
}

static HANDLE RotateArbitrary(const dynarithmic::dib::LockedDib& src, float angleDeg)
{
    const uint32_t sw = src.Width();
    const uint32_t sh = src.Height();
    const uint16_t bpp = src.BitsPerPixel();

    const double radians = angleDeg * 3.14159265358979323846 / 180.0;
    const double c = std::cos(radians);
    const double s = std::sin(radians);

    const double cx = (static_cast<double>(sw) - 1.0) / 2.0;
    const double cy = (static_cast<double>(sh) - 1.0) / 2.0;

    const double corners[4][2] =
    {
        { -cx,       -cy        },
        { sw - 1 - cx, -cy     },
        { -cx,        sh - 1 - cy },
        { sw - 1 - cx, sh - 1 - cy }
    };

    double minX = 1e30, maxX = -1e30;
    double minY = 1e30, maxY = -1e30;

    for (const auto& p : corners)
    {
        const double rx = p[0] * c - p[1] * s;
        const double ry = p[0] * s + p[1] * c;

        minX = std::min(minX, rx);
        maxX = std::max(maxX, rx);
        minY = std::min(minY, ry);
        maxY = std::max(maxY, ry);
    }

    const uint32_t dw = static_cast<uint32_t>(std::ceil(maxX - minX + 1.0));
    const uint32_t dh = static_cast<uint32_t>(std::ceil(maxY - minY + 1.0));

    const uint32_t palEntries = bpp <= 8 ? src.PaletteEntries() : 0;

    HANDLE hDst = CreateCroppedDibLike(*src.Header(), dw, dh, bpp, palEntries);
    if (!hDst)
        return nullptr;

    dynarithmic::dib::LockedDib dst(hDst);
    if (!dst.IsValid())
    {
        ::GlobalFree(hDst);
        return nullptr;
    }

    if (bpp <= 8 && palEntries)
        std::memcpy(dst.PalettePtrMutable(), src.Palette(), palEntries * sizeof(RGBQUAD));

    FillDibWhite(dst);

    const double dcx = (static_cast<double>(dw) - 1.0) / 2.0;
    const double dcy = (static_cast<double>(dh) - 1.0) / 2.0;

    for (uint32_t y = 0; y < dh; ++y)
    {
        for (uint32_t x = 0; x < dw; ++x)
        {
            const double dx = static_cast<double>(x) - dcx;
            const double dy = static_cast<double>(y) - dcy;

            // inverse rotate destination point back into source
            const double sx = dx * c + dy * s + cx;
            const double sy = -dx * s + dy * c + cy;

            if (sx < 0.0 || sy < 0.0 ||
                sx > static_cast<double>(sw - 1) ||
                sy > static_cast<double>(sh - 1))
            {
                continue;
            }

            if (bpp == 1 || bpp == 4)
                SampleNearestToDst(src, sx, sy, dst, x, y);
            else
                SampleBicubicToDst(src, sx, sy, dst, x, y);
        }
    }

    return hDst;
}


HANDLE dynarithmic::dib::Rotate(HANDLE src, float angle)
{
    if (!src)
        return nullptr;

    dynarithmic::dib::LockedDib s(src);
    if (!s.IsValid())
        return nullptr;

    const auto* bih = s.Header();
    if (!bih || bih->biCompression != BI_RGB)
        return nullptr;

    const uint16_t bpp = s.BitsPerPixel();

    switch (bpp)
    {
        case 1:
        case 4:
        case 8:
        case 16:
        case 24:
        case 32:
            break;
        default:
            return nullptr;
    }

    auto normAngle = std::fmod(angle, 360.0f);
    if (normAngle < 0.0f)
        normAngle += 360.0f;

    auto nearAngle = [](float a, float b)
    {
        return std::fabs(a - b) < 0.001f;
    };

    if (nearAngle(normAngle, 0.0f) || nearAngle(normAngle, 360.0f))
        return dynarithmic::dib::CloneDibHandle(src);

    if (nearAngle(normAngle, 90.0f))
        return RotateRightAngle(s, 90);

    if (nearAngle(normAngle, 180.0f))
        return RotateRightAngle(s, 180);

    if (nearAngle(normAngle, 270.0f))
        return RotateRightAngle(s, 270);

    return RotateArbitrary(s, normAngle);
}
