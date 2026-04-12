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
#include <windows.h>
#include <cstdint>
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
#include <math.h>
#include <sstream>
#include "winbit32.h"
#include "ctliface.h"

using namespace dynarithmic;

namespace
{
	struct DibContext
	{
		const BITMAPINFOHEADER* bih = nullptr;
		const RGBQUAD* palette = nullptr;
		const BYTE* bits = nullptr;

		int width = 0;
		int height = 0;
		int bpp = 0;
		int stride = 0;
		int bytesPerPixel = 0;
		bool topDown = false;

		int left = 0;
		int top = 0;
		int right = 0;
		int bottom = 0;
	};

	int ComputeGray(BYTE r, BYTE g, BYTE b)
	{
		return (299 * static_cast<int>(r) +
			587 * static_cast<int>(g) +
			114 * static_cast<int>(b)) / 1000;
	}

	int ComputeStride(int width, int bpp)
	{
		return ((width * bpp + 31) / 32) * 4;
	}

	bool IsSupportedHeader(const BITMAPINFOHEADER* bih)
    {
        if (!bih)
            return false;
        if (bih->biSize < sizeof(BITMAPINFOHEADER))
            return false;
        if (bih->biPlanes != 1)
            return false;
        if (bih->biWidth <= 0)
            return false;
        if (bih->biHeight == 0)
            return false;

        switch (bih->biBitCount)
        {
            case 1:
            case 4:
            case 8:
            case 16:
            case 24:
            case 32:
                break;
            default:
                return false;
        }

        // Not handling BI_BITFIELDS here.
        if (bih->biCompression != BI_RGB)
            return false;

        return true;
	}

	int GetAllPaletteEntries(const BITMAPINFOHEADER* bih)
	{
		if (!bih)
			return 0;

		if (bih->biBitCount > 8)
			return 0;

		if (bih->biClrUsed != 0)
			return static_cast<int>(bih->biClrUsed);

		return 1 << bih->biBitCount;
	}

	const RGBQUAD* GetPalettePtr(const BITMAPINFOHEADER* bih)
	{
		return reinterpret_cast<const RGBQUAD*>(
			reinterpret_cast<const BYTE*>(bih) + bih->biSize);
	}

	const BYTE* GetBitsPtr(const BITMAPINFOHEADER* bih)
	{
		const int paletteEntries = GetAllPaletteEntries(bih);
		const BYTE* p = reinterpret_cast<const BYTE*>(bih) + bih->biSize;
		p += paletteEntries * sizeof(RGBQUAD);
		return p;
	}

	bool MakeDibContext(const BITMAPINFOHEADER* bih, DibContext& ctx)
	{
		if (!IsSupportedHeader(bih))
			return false;

		ctx.bih = bih;
		ctx.width = static_cast<int>(bih->biWidth);
		ctx.height = static_cast<int>(std::abs(bih->biHeight));
		ctx.topDown = (bih->biHeight < 0);
		ctx.bpp = bih->biBitCount;
		ctx.stride = ComputeStride(ctx.width, ctx.bpp);
		ctx.bytesPerPixel = (ctx.bpp >= 8) ? (ctx.bpp / 8) : 0;
		ctx.palette = GetPalettePtr(bih);
		ctx.bits = GetBitsPtr(bih);

		const int borderX = std::max(2, ctx.width / 100);
		const int borderY = std::max(2, ctx.height / 100);

		ctx.left = std::clamp(borderX, 0, ctx.width);
		ctx.top = std::clamp(borderY, 0, ctx.height);
		ctx.right = std::clamp(ctx.width - borderX, 0, ctx.width);
		ctx.bottom = std::clamp(ctx.height - borderY, 0, ctx.height);

		if (ctx.left >= ctx.right || ctx.top >= ctx.bottom)
		{
			ctx.left = 0;
			ctx.top = 0;
			ctx.right = ctx.width;
			ctx.bottom = ctx.height;
		}

		return true;
	}

	const BYTE* GetRowPointer(const DibContext& ctx, int y)
	{
		if (ctx.topDown)
			return ctx.bits + static_cast<std::ptrdiff_t>(y) * ctx.stride;

		const int physicalRow = ctx.height - 1 - y;
		return ctx.bits + static_cast<std::ptrdiff_t>(physicalRow) * ctx.stride;
	}

	int FindDominantLightPeak(const std::array<std::uint64_t, 256>& hist)
	{
		int bestIndex = 255;
		std::uint64_t bestCount = 0;

		for (int i = 128; i < 256; ++i)
		{
			if (hist[i] > bestCount)
			{
				bestCount = hist[i];
				bestIndex = i;
			}
		}

		if (bestCount != 0)
			return bestIndex;

		for (int i = 0; i < 256; ++i)
		{
			if (hist[i] > bestCount)
			{
				bestCount = hist[i];
				bestIndex = i;
			}
		}
		return bestIndex;
	}

	int Determine1BPPBackgroundBit(const RGBQUAD* pal)
	{
		if (!pal)
			return 1;

		const int lum0 = ComputeGray(pal[0].rgbRed, pal[0].rgbGreen, pal[0].rgbBlue);
		const int lum1 = ComputeGray(pal[1].rgbRed, pal[1].rgbGreen, pal[1].rgbBlue);
		return (lum1 >= lum0) ? 1 : 0;
	}

	BYTE Nibble4BPP(const BYTE* row, int x)
	{
		const BYTE byteVal = row[x / 2];
		return (x % 2 == 0)
			? static_cast<BYTE>((byteVal >> 4) & 0x0F)
			: static_cast<BYTE>(byteVal & 0x0F);
	}

	void Decode16BPP_RGB555(std::uint16_t pix, BYTE& r, BYTE& g, BYTE& b)
	{
		const int rr = (pix >> 10) & 0x1F;
		const int gg = (pix >> 5) & 0x1F;
		const int bb = pix & 0x1F;

		r = static_cast<BYTE>((rr << 3) | (rr >> 2));
		g = static_cast<BYTE>((gg << 3) | (gg >> 2));
		b = static_cast<BYTE>((bb << 3) | (bb >> 2));
	}

	struct BlankPageOptions
	{
		bool enableForegroundNoiseFilter = true;
		int minForegroundNeighbors = 2; // 0..8, good starting range: 1..3
	};

	struct BinaryMask
	{
		int width = 0;
		int height = 0;
		std::vector<std::uint8_t> pixels; // 0 = background, 1 = foreground

		BinaryMask() = default;

		BinaryMask(int w, int h)
			: width(w), height(h), pixels(static_cast<size_t>(w)* h, 0)
		{}

		std::uint8_t& at(int x, int y)
		{
			return pixels[static_cast<size_t>(y) * width + x];
		}

		std::uint8_t at(int x, int y) const
		{
			return pixels[static_cast<size_t>(y) * width + x];
		}
	};

	BinaryMask FilterIsolatedForeground(const BinaryMask& src, int minForegroundNeighbors)
	{
		BinaryMask dst(src.width, src.height);

		for (int y = 0; y < src.height; ++y)
		{
			for (int x = 0; x < src.width; ++x)
			{
				if (!src.at(x, y))
					continue;

				int count = 0;

				for (int yy = std::max(0, y - 1); yy <= std::min(src.height - 1, y + 1); ++yy)
				{
					for (int xx = std::max(0, x - 1); xx <= std::min(src.width - 1, x + 1); ++xx)
					{
						if (xx == x && yy == y)
							continue;

						if (src.at(xx, yy))
							++count;
					}
				}

				if (count >= minForegroundNeighbors)
					dst.at(x, y) = 1;
			}
		}

		return dst;
	}

	double ComputeBlankPercentFromMask(const BinaryMask& mask)
	{
		const std::uint64_t total =
			static_cast<std::uint64_t>(mask.width) * static_cast<std::uint64_t>(mask.height);

		if (total == 0)
			return 0.0;

		std::uint64_t foreground = 0;
		for (auto v : mask.pixels)
			foreground += (v != 0);

		const std::uint64_t background = total - foreground;
		return (100.0 * static_cast<double>(background)) / static_cast<double>(total);
	}

	BinaryMask BuildForegroundMask1BPP(const DibContext& ctx)
	{
		BinaryMask mask(ctx.right - ctx.left, ctx.bottom - ctx.top);
		const int bgBit = Determine1BPPBackgroundBit(ctx.palette);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const int byteIndex = x / 8;
				const int bitIndex = 7 - (x % 8);
				const int bit = (row[byteIndex] >> bitIndex) & 1;

				const int mx = x - ctx.left;
				const int my = y - ctx.top;

				mask.at(mx, my) = (bit == bgBit) ? 0 : 1;
			}
		}

		return mask;
	}

	BinaryMask BuildForegroundMask4BPP(const DibContext& ctx)
	{
		std::array<std::uint64_t, 256> hist{};
		hist.fill(0);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const BYTE idx = Nibble4BPP(row, x);
				const RGBQUAD& q = ctx.palette[idx];
				const int gray = ComputeGray(q.rgbRed, q.rgbGreen, q.rgbBlue);
				++hist[gray];
			}
		}

		const int bgGray = FindDominantLightPeak(hist);
		const int tol = 8;

		BinaryMask mask(ctx.right - ctx.left, ctx.bottom - ctx.top);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const BYTE idx = Nibble4BPP(row, x);
				const RGBQUAD& q = ctx.palette[idx];
				const int gray = ComputeGray(q.rgbRed, q.rgbGreen, q.rgbBlue);

				const bool isBackground = (std::abs(gray - bgGray) <= tol);
				mask.at(x - ctx.left, y - ctx.top) = isBackground ? 0 : 1;
			}
		}
		return mask;
	}

	BinaryMask BuildForegroundMask8BPP(const DibContext& ctx)
	{
		std::array<std::uint64_t, 256> hist{};
		hist.fill(0);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const BYTE idx = row[x];
				const RGBQUAD& q = ctx.palette[idx];
				const int gray = ComputeGray(q.rgbRed, q.rgbGreen, q.rgbBlue);
				++hist[gray];
			}
		}

		const int bgGray = FindDominantLightPeak(hist);
		const int tol = 8;

		BinaryMask mask(ctx.right - ctx.left, ctx.bottom - ctx.top);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const BYTE idx = row[x];
				const RGBQUAD& q = ctx.palette[idx];
				const int gray = ComputeGray(q.rgbRed, q.rgbGreen, q.rgbBlue);

				const bool isBackground = (std::abs(gray - bgGray) <= tol);
				mask.at(x - ctx.left, y - ctx.top) = isBackground ? 0 : 1;
			}
		}

		return mask;
	}

	BinaryMask BuildForegroundMask16BPP(const DibContext& ctx)
	{
		std::array<std::uint64_t, 256> hist{};
		hist.fill(0);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const auto* pPix = reinterpret_cast<const std::uint16_t*>(row + static_cast<std::ptrdiff_t>(x) * 2);
				const std::uint16_t pix = *pPix;

				BYTE r = 0, g = 0, b = 0;
				Decode16BPP_RGB555(pix, r, g, b);

				const int gray = ComputeGray(r, g, b);
				++hist[gray];
			}
		}

		const int bgGray = FindDominantLightPeak(hist);
		const int tol = 12;

		BinaryMask mask(ctx.right - ctx.left, ctx.bottom - ctx.top);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const auto* pPix = reinterpret_cast<const std::uint16_t*>(row + static_cast<std::ptrdiff_t>(x) * 2);
				const std::uint16_t pix = *pPix;

				BYTE r = 0, g = 0, b = 0;
				Decode16BPP_RGB555(pix, r, g, b);

				const int gray = ComputeGray(r, g, b);
				const bool isBackground = (std::abs(gray - bgGray) <= tol);

				mask.at(x - ctx.left, y - ctx.top) = isBackground ? 0 : 1;
			}
		}

		return mask;
	}

	BinaryMask BuildForegroundMask24Or32BPP(const DibContext& ctx)
	{
		std::array<std::uint64_t, 256> hist{};
		hist.fill(0);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const BYTE* px = row + static_cast<std::ptrdiff_t>(x) * ctx.bytesPerPixel;
				const int gray = ComputeGray(px[2], px[1], px[0]);
				++hist[gray];
			}
		}

		const int bgGray = FindDominantLightPeak(hist);
		const int tol = 12;

		BinaryMask mask(ctx.right - ctx.left, ctx.bottom - ctx.top);

		for (int y = ctx.top; y < ctx.bottom; ++y)
		{
			const BYTE* row = GetRowPointer(ctx, y);
			for (int x = ctx.left; x < ctx.right; ++x)
			{
				const BYTE* px = row + static_cast<std::ptrdiff_t>(x) * ctx.bytesPerPixel;
				const int gray = ComputeGray(px[2], px[1], px[0]);

				const bool isBackground = (std::abs(gray - bgGray) <= tol);
				mask.at(x - ctx.left, y - ctx.top) = isBackground ? 0 : 1;
			}
		}

		return mask;
	}

	CDibInterface::BlankDIBInfo IsDibBlankImpl(const BITMAPINFOHEADER* bih, double threshold)
	{
		DibContext ctx;
		if (!MakeDibContext(bih, ctx))
			return { false, {-1, -1 } };

		BlankPageOptions opts;

		BinaryMask mask;
		switch (ctx.bpp)
		{
			case 1:
				mask = BuildForegroundMask1BPP(ctx);
				break;

			case 4:
				mask = BuildForegroundMask4BPP(ctx);
				break;

			case 8:
				mask = BuildForegroundMask8BPP(ctx);
				break;

			case 16:
				mask = BuildForegroundMask16BPP(ctx);
				break;

			case 24:
			case 32:
				mask = BuildForegroundMask24Or32BPP(ctx);
				break;

			default:
				return { false, {-1, -1} };
		}

		if (opts.enableForegroundNoiseFilter)
		{
			// Enable stronger filtering by calling the filter twice
			mask = FilterIsolatedForeground(mask, opts.minForegroundNeighbors);
			mask = FilterIsolatedForeground(mask, opts.minForegroundNeighbors);
		}

		const double blankPercent = ComputeBlankPercentFromMask(mask);
		return { blankPercent >= threshold, {blankPercent, threshold } };
	}
}


CDibInterface::BlankDIBInfo CDibInterface::IsBlankDIBEx(HANDLE hDib, double threshold)
{
	if (!hDib)
		return { false, {-1, -1} };

	if (threshold < 0.0 || threshold > 100.0)
		return { false, {-1, -1} };

	const auto* pBI = static_cast<const BITMAPINFOHEADER*>(::GlobalLock(hDib));
	if (!pBI)
		return { false, {-1, -1} };

	DTWAINGlobalHandle_RAII handler(hDib);
	auto result = IsDibBlankImpl(pBI, threshold);

	return result;
}