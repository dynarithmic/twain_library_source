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
#ifndef DIBUTIL_H
#define DIBUTIL_H

#include <windows.h>
#include <cstdint>
#include <memory>

namespace dynarithmic::dib
{
	struct DibHandleDeleter
	{
		void operator()(HGLOBAL h) const noexcept
		{
			if (h)
				::GlobalFree(h);
		}
	};

	using unique_dib = std::unique_ptr<std::remove_pointer_t<HGLOBAL>, DibHandleDeleter>;

	static constexpr uint32_t calc_stride_bytes(uint32_t width, uint16_t bitsPerPixel)
	{
		const uint64_t bitsPerRow = static_cast<uint64_t>(width) * bitsPerPixel;
		const uint64_t alignedBits = (bitsPerRow + 31ULL) & ~31ULL;
		return static_cast<uint32_t>(alignedBits / 8ULL);
	}

	inline uint32_t palette_entries(const BITMAPINFOHEADER& bih)
	{
		if (bih.biClrUsed != 0)
			return static_cast<uint32_t>(bih.biClrUsed);

		if (bih.biBitCount <= 8)
			return (1u << bih.biBitCount);

		return 0;
	}

	static constexpr uint32_t effective_palette_entries(uint16_t bpp)
	{
		if (bpp >= 1 && bpp <= 8)
			return 1 << bpp;
		return 0;
	}


	static constexpr uint32_t effective_width(uint32_t width, uint16_t bpp)
	{
		return (width * bpp + 31) / 32 * 4;
	}

	inline const RGBQUAD* palette_ptr(const BITMAPINFOHEADER* pBih)
	{
		if (!pBih)
			return nullptr;

		if (pBih->biBitCount > 8)
			return nullptr;

		const uint32_t palEntries = palette_entries(*pBih);
		if (palEntries == 0)
			return nullptr;

		const uint8_t* p = reinterpret_cast<const uint8_t*>(pBih);
		p += pBih->biSize;
		return reinterpret_cast<const RGBQUAD*>(p);
	}

	inline RGBQUAD* palette_ptr_mutable(const BITMAPINFOHEADER* pBih)
	{
		return const_cast<RGBQUAD*>(palette_ptr(pBih));
	}

	inline const uint8_t* bits_ptr(const BITMAPINFOHEADER* pBih)
	{
		if (!pBih)
			return nullptr;

		const uint8_t* p = reinterpret_cast<const uint8_t*>(pBih);
		p += pBih->biSize;

		if (pBih->biBitCount <= 8)
			p += palette_entries(*pBih) * sizeof(RGBQUAD);

		return p;
	}

	inline bool is_bottom_up(const BITMAPINFOHEADER& bih)
	{
		return bih.biHeight > 0;
	}

	inline uint32_t image_width(const BITMAPINFOHEADER& bih)
	{
		return static_cast<uint32_t>(bih.biWidth);
	}

	inline uint32_t image_height(const BITMAPINFOHEADER& bih)
	{
		return static_cast<uint32_t>(bih.biHeight > 0 ? bih.biHeight : -bih.biHeight);
	}

	inline double pels_per_meter_to_dpi(LONG ppm)
	{
		return ppm > 0 ? static_cast<double>(ppm) * 0.0254 : 0.0;
	}

	inline bool is_grayscale_palette(const RGBQUAD* palette, uint32_t entries)
	{
		if (!palette || entries == 0)
			return false;

		for (uint32_t i = 0; i < entries; ++i)
		{
			if (palette[i].rgbRed != palette[i].rgbGreen ||
				palette[i].rgbRed != palette[i].rgbBlue)
			{
				return false;
			}
		}
		return true;
	}

	inline bool is_identity_grayscale_palette(const RGBQUAD* palette, uint32_t entries)
	{
		if (!palette || entries == 0)
			return false;

		for (uint32_t i = 0; i < entries; ++i)
		{
			const uint8_t expected = static_cast<uint8_t>(i & 0xFF);

			if (palette[i].rgbRed != expected ||
				palette[i].rgbGreen != expected ||
				palette[i].rgbBlue != expected)
			{
				return false;
			}
		}
		return true;
	}

	class LockedDib
	{
	public:
		explicit LockedDib(HANDLE hDib)
			: hDib_(hDib)
		{
			if (!hDib_)
				return;

			locked_ = ::GlobalLock(hDib_);
			if (!locked_)
				return;

			bih_ = static_cast<const BITMAPINFOHEADER*>(locked_);
			if (!bih_ || bih_->biSize < sizeof(BITMAPINFOHEADER))
			{
				bih_ = nullptr;
				return;
			}

			valid_ = true;
		}

		~LockedDib()
		{
			if (locked_)
				::GlobalUnlock(hDib_);
		}

		LockedDib(const LockedDib&) = delete;
		LockedDib& operator=(const LockedDib&) = delete;

		LockedDib(LockedDib&& other) noexcept
			: hDib_(other.hDib_),
			locked_(other.locked_),
			bih_(other.bih_),
			valid_(other.valid_)
		{
			other.hDib_ = nullptr;
			other.locked_ = nullptr;
			other.bih_ = nullptr;
			other.valid_ = false;
		}

		LockedDib& operator=(LockedDib&& other) noexcept
		{
			if (this != &other)
			{
				if (locked_)
					::GlobalUnlock(hDib_);

				hDib_ = other.hDib_;
				locked_ = other.locked_;
				bih_ = other.bih_;
				valid_ = other.valid_;

				other.hDib_ = nullptr;
				other.locked_ = nullptr;
				other.bih_ = nullptr;
				other.valid_ = false;
			}
			return *this;
		}

		bool IsValid() const noexcept
		{
			return valid_;
		}

		const BITMAPINFOHEADER* Header() const noexcept
		{
			return bih_;
		}

		const BYTE* HeaderAsBytePtr() noexcept
		{
			return reinterpret_cast<const BYTE *>(bih_);
		}

		BITMAPINFOHEADER* HeaderMutable() noexcept
		{
			return const_cast<BITMAPINFOHEADER *>(bih_);
		}

		const RGBQUAD* Palette() const noexcept
		{
			return palette_ptr(bih_);
		}

		RGBQUAD* PalettePtrMutable() noexcept
		{
			return palette_ptr_mutable(bih_);
		}

		uint32_t PaletteEntries() const noexcept
		{
			return bih_ ? palette_entries(*bih_) : 0;
		}

		uint8_t* Bits() noexcept
		{
			return const_cast<uint8_t*>(bits_ptr(bih_));
		}

		const uint8_t* Bits() const noexcept
		{
			return const_cast<uint8_t*>(bits_ptr(bih_));
		}

		uint32_t Width() const noexcept
		{
			return bih_ ? image_width(*bih_) : 0;
		}

		uint32_t Height() const noexcept
		{
			return bih_ ? image_height(*bih_) : 0;
		}

		uint16_t BitsPerPixel() const noexcept
		{
			return bih_ ? static_cast<uint16_t>(bih_->biBitCount) : 0;
		}

		uint32_t Pitch() const noexcept
		{
			return effective_width(Width(), BitsPerPixel()) + 3 & ~3;
		}

		uint32_t StrideBytes() const noexcept
		{
			return bih_ ? calc_stride_bytes(Width(), BitsPerPixel()) : 0;
		}

		uint32_t ColorsUsed() const noexcept
		{
			return bih_ ? bih_->biClrUsed : 0;
		}

		bool BottomUp() const noexcept
		{
			return bih_ ? is_bottom_up(*bih_) : true;
		}

		double XDpi() const noexcept
		{
			return bih_ ? pels_per_meter_to_dpi(bih_->biXPelsPerMeter) : 0.0;
		}

		double YDpi() const noexcept
		{
			return bih_ ? pels_per_meter_to_dpi(bih_->biYPelsPerMeter) : 0.0;
		}

		uint32_t EffectiveWidth() const noexcept
		{
			return bih_ ? effective_width(image_width(*bih_), static_cast<uint16_t>(bih_->biBitCount)): 0;
		}

	private:
		HANDLE hDib_ = nullptr;
		void* locked_ = nullptr;
		const BITMAPINFOHEADER* bih_ = nullptr;
		bool valid_ = false;
	};
}
#endif