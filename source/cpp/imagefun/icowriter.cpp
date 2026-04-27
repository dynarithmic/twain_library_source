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
#include <png.h>
#include "imagefilewriterbase.h"
#include "icowriter.h"

#ifdef _MSC_VER
	#pragma warning (disable : 4786)
	#pragma warning (disable : 4611)
#endif


std::optional<PreparedIcoDibPage> IcoSessionWriter::MakePreparedIcoDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedIcoDibPage page{};

	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;

	switch (page.bitsPerPixel)
	{
		case 1:
			page.pixelFlavor = IcoPixelFlavor::BW1;
			break;

		case 4:
			page.pixelFlavor = IcoPixelFlavor::Palette4;
			break;

		case 8:
			if (page.palette && page.paletteEntries > 0 &&
				dynarithmic::dib::is_grayscale_palette(page.palette, page.paletteEntries))
			{
				page.pixelFlavor = IcoPixelFlavor::Gray8;
			}
			else if (page.palette && page.paletteEntries > 0)
			{
				page.pixelFlavor = IcoPixelFlavor::Palette8;
			}
			else
			{
				page.pixelFlavor = IcoPixelFlavor::Gray8;
			}
			break;

		case 16:
			page.pixelFlavor = IcoPixelFlavor::Bgr16;
			break;

		case 24:
			page.pixelFlavor = IcoPixelFlavor::Bgr24;
			break;

		case 32:
			page.pixelFlavor = IcoPixelFlavor::Bgra32;
			break;

		default:
			return page;
	}
	return page;
}

// ============================================================
// Memory PNG encoder for Vista-style ICO payloads
// Reuses non-FreeImage libpng logic
// ============================================================
bool IcoMemoryPngEncoder::Encode(const PreparedIcoDibPage& page, std::vector<uint8_t>& outPng)
{
	outPng.clear();

	PngReadyImage ready;
	if (!build_png_ready_image(page, ready))
		return false;

	png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr)
		return false;

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_write_struct(&png_ptr, nullptr);
		return false;
	}

	if (setjmp(png_jmpbuf(png_ptr)))
	{
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return false;
	}

	png_set_write_fn(png_ptr, &outPng, &IcoMemoryPngEncoder::png_write_callback, nullptr);

	int colorType = PNG_COLOR_TYPE_RGB;
	int bitDepth = 8;

	switch (ready.flavor)
	{
		case PngReadyFlavor::Palette1:
			colorType = PNG_COLOR_TYPE_PALETTE;
			bitDepth = 1;
			break;

		case PngReadyFlavor::Palette4:
			colorType = PNG_COLOR_TYPE_PALETTE;
			bitDepth = 4;
			break;

		case PngReadyFlavor::Palette8:
			colorType = PNG_COLOR_TYPE_PALETTE;
			bitDepth = 8;
			break;

		case PngReadyFlavor::Gray8:
			colorType = PNG_COLOR_TYPE_GRAY;
			bitDepth = 8;
			break;

		case PngReadyFlavor::Rgb24:
			colorType = PNG_COLOR_TYPE_RGB;
			bitDepth = 8;
			break;

		case PngReadyFlavor::Rgba32:
			colorType = PNG_COLOR_TYPE_RGB_ALPHA;
			bitDepth = 8;
			break;
	}

	png_set_IHDR(png_ptr,
		info_ptr,
		ready.width,
		ready.height,
		bitDepth,
		colorType,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);

	std::vector<png_color> pngPalette;
	if (!ready.palette.empty())
	{
		pngPalette.resize(ready.palette.size());
		for (size_t i = 0; i < ready.palette.size(); ++i)
		{
			pngPalette[i].red = ready.palette[i].rgbRed;
			pngPalette[i].green = ready.palette[i].rgbGreen;
			pngPalette[i].blue = ready.palette[i].rgbBlue;
		}
		png_set_PLTE(png_ptr, info_ptr, pngPalette.data(), static_cast<int>(pngPalette.size()));
	}

	if (ready.flavor == PngReadyFlavor::Rgb24 || ready.flavor == PngReadyFlavor::Rgba32)
		png_set_bgr(png_ptr);

	png_write_info(png_ptr, info_ptr);

	std::vector<png_bytep> rows(ready.height);
	for (uint32_t y = 0; y < ready.height; ++y)
	{
		rows[y] = const_cast<png_bytep>(
			ready.bits + static_cast<size_t>(y) * ready.strideBytes);
	}

	png_write_image(png_ptr, rows.data());
	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	return true;
}

void IcoMemoryPngEncoder::png_write_callback(png_structp png_ptr, png_bytep data, png_size_t length)
{
	auto* out = static_cast<std::vector<uint8_t>*>(png_get_io_ptr(png_ptr));
	out->insert(out->end(), data, data + length);
}

bool IcoMemoryPngEncoder::build_png_ready_image(const PreparedIcoDibPage& page, PngReadyImage& out)
{
	out = {};
	out.width = page.width;
	out.height = page.height;

	switch (page.pixelFlavor)
	{
		case IcoPixelFlavor::BW1:
		{
			out.flavor = PngReadyFlavor::Palette1;
			out.strideBytes = dynarithmic::dib::calc_stride_bytes(page.width, 1);
			out.palette.resize(2);
			if (page.palette && page.paletteEntries >= 2)
			{
				out.palette[0] = page.palette[0];
				out.palette[1] = page.palette[1];
			}
			else
			{
				out.palette[0] = RGBQUAD{ 0,0,0,0 };
				out.palette[1] = RGBQUAD{ 255,255,255,0 };
			}

			normalize_top_down_bytes(page, out.strideBytes, out.ownedBits);
			out.bits = out.ownedBits.data();
			return true;
		}

		case IcoPixelFlavor::Palette4:
		{
			out.flavor = PngReadyFlavor::Palette4;
			out.strideBytes = static_cast<uint32_t>((page.width + 1) / 2);
			out.palette.resize(16);
			for (size_t i = 0; i < out.palette.size(); ++i)
				out.palette[i] = RGBQUAD{ 0,0,0,0 };

			if (page.palette && page.paletteEntries > 0)
			{
				const uint32_t n = page.paletteEntries > 16 ? 16 : page.paletteEntries;
				for (uint32_t i = 0; i < n; ++i)
					out.palette[i] = page.palette[i];
			}

			normalize_top_down_bytes(page, out.strideBytes, out.ownedBits);
			out.bits = out.ownedBits.data();
			return true;
		}

		case IcoPixelFlavor::Palette8:
		{
			out.flavor = PngReadyFlavor::Palette8;
			out.strideBytes = page.width;
			out.palette.resize(256);
			for (size_t i = 0; i < out.palette.size(); ++i)
				out.palette[i] = RGBQUAD{ 0,0,0,0 };

			if (page.palette && page.paletteEntries > 0)
			{
				const uint32_t n = page.paletteEntries > 256 ? 256 : page.paletteEntries;
				for (uint32_t i = 0; i < n; ++i)
					out.palette[i] = page.palette[i];
			}

			normalize_top_down_bytes(page, out.strideBytes, out.ownedBits);
			out.bits = out.ownedBits.data();
			return true;
		}

		case IcoPixelFlavor::Gray8:
		{
			out.flavor = PngReadyFlavor::Gray8;
			out.strideBytes = page.width;

			// If this is actually palette-indexed gray, expand through palette to true Gray8.
			out.ownedBits.resize(static_cast<size_t>(out.strideBytes) * out.height);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
				const uint8_t* src = page.bits + static_cast<size_t>(srcY) * page.strideBytes;
				uint8_t* dst = out.ownedBits.data() + static_cast<size_t>(y) * out.strideBytes;

				if (page.palette && page.paletteEntries > 0)
				{
					for (uint32_t x = 0; x < page.width; ++x)
					{
						const uint8_t idx = src[x];
						if (idx < page.paletteEntries)
							dst[x] = page.palette[idx].rgbRed;
						else
							dst[x] = 0;
					}
				}
				else
				{
					std::memcpy(dst, src, page.width);
				}
			}

			out.bits = out.ownedBits.data();
			return true;
		}

		case IcoPixelFlavor::Bgr16:
		{
			out.flavor = PngReadyFlavor::Rgb24;
			out.strideBytes = page.width * 3;
			out.ownedBits.resize(static_cast<size_t>(out.strideBytes) * out.height);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
				const uint8_t* src = page.bits + static_cast<size_t>(srcY) * page.strideBytes;
				uint8_t* dst = out.ownedBits.data() + static_cast<size_t>(y) * out.strideBytes;

				for (uint32_t x = 0; x < page.width; ++x)
				{
					const uint16_t px = *reinterpret_cast<const uint16_t*>(src + x * 2);

					// Assume RGB565. If your pipeline needs 555 too, this is the one place to branch.
					const uint8_t r = static_cast<uint8_t>(((px >> 11) & 0x1F) * 255 / 31);
					const uint8_t g = static_cast<uint8_t>(((px >> 5) & 0x3F) * 255 / 63);
					const uint8_t b = static_cast<uint8_t>((px & 0x1F) * 255 / 31);

					dst[x * 3 + 0] = b;
					dst[x * 3 + 1] = g;
					dst[x * 3 + 2] = r;
				}
			}

			out.bits = out.ownedBits.data();
			return true;
		}

		case IcoPixelFlavor::Bgr24:
		{
			out.flavor = PngReadyFlavor::Rgb24;
			out.strideBytes = page.width * 3;
			normalize_top_down_bytes(page, out.strideBytes, out.ownedBits);
			out.bits = out.ownedBits.data();
			return true;
		}

		case IcoPixelFlavor::Bgra32:
		{
			out.flavor = PngReadyFlavor::Rgba32;
			out.strideBytes = page.width * 4;
			normalize_top_down_bytes(page, out.strideBytes, out.ownedBits);
			out.bits = out.ownedBits.data();
			return true;
		}
	}

	return false;
}

void IcoMemoryPngEncoder::normalize_top_down_bytes(const PreparedIcoDibPage& page, uint32_t outStride, std::vector<uint8_t>& out)
{
	out.resize(static_cast<size_t>(outStride) * page.height);

	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
		const uint8_t* src = page.bits + static_cast<size_t>(srcY) * page.strideBytes;
		uint8_t* dst = out.data() + static_cast<size_t>(y) * outStride;
		std::memcpy(dst, src, outStride);
	}
}

IcoSessionWriter::~IcoSessionWriter()
{
	Close();
}

bool IcoSessionWriter::Open(const std::wstring& filename, const IcoSessionOptions& options)
{
	if (file_)
		return false;

	file_ = _wfopen(filename.c_str(), L"wb");
	if (!file_)
		return false;

	filename_ = filename;
	options_ = options;
	hasCurrentPage_ = false;
	return true;
}

bool IcoSessionWriter::SetPageInfo(const PreparedIcoDibPage& page)
{
	if (!file_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool IcoSessionWriter::WriteCurrentPage()
{
	if (!file_ || !hasCurrentPage_)
		return false;

	imageData_.clear();

	if (options_.mode == IcoMode::VistaPng)
	{
		IcoMemoryPngEncoder encoder;
		if (!encoder.Encode(currentPage_, imageData_))
			return false;
	}
	else
	{
		if (!build_classic_icon_image())
			return false;
	}

	return write_ico_file();
}

void IcoSessionWriter::Close()
{
	if (file_)
	{
		std::fclose(file_);
		file_ = nullptr;
	}

	filename_.clear();
	imageData_.clear();
	hasCurrentPage_ = false;
}

bool IcoSessionWriter::ValidatePage(const PreparedIcoDibPage& page)
{
	switch (page.bitsPerPixel)
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

	return page.width > 0 &&
		page.height > 0 &&
		page.bits != nullptr &&
		page.strideBytes > 0;
}

bool IcoSessionWriter::build_classic_icon_image()
{
	if (currentPage_.width > 255 || currentPage_.height > 255)
		return false;

	const uint32_t xorStride = currentPage_.strideBytes;
	const uint32_t andStride = dynarithmic::dib::calc_stride_bytes(currentPage_.width, 1);
	const uint32_t andSize = andStride * currentPage_.height;

	BITMAPINFOHEADER bih{};
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = static_cast<LONG>(currentPage_.width);
	bih.biHeight = static_cast<LONG>(currentPage_.height * 2);
	bih.biPlanes = 1;
	bih.biBitCount = currentPage_.bitsPerPixel;
	bih.biCompression = BI_RGB;
	bih.biSizeImage = xorStride * currentPage_.height + andSize;
	bih.biClrUsed = currentPage_.paletteEntries;
	bih.biClrImportant = currentPage_.paletteEntries;

	append_bytes(&bih, sizeof(bih));

	if (currentPage_.bitsPerPixel <= 8 &&
		currentPage_.palette &&
		currentPage_.paletteEntries > 0)
	{
		append_bytes(currentPage_.palette,
			currentPage_.paletteEntries * sizeof(RGBQUAD));
	}

	// Classic ICO stores DIB-style XOR data bottom-up.
	if (currentPage_.bottomUp)
	{
		append_bytes(currentPage_.bits, xorStride * currentPage_.height);
	}
	else
	{
		for (uint32_t y = 0; y < currentPage_.height; ++y)
		{
			const uint32_t srcY = currentPage_.height - 1 - y;
			const uint8_t* row =
				currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;
			append_bytes(row, xorStride);
		}
	}

	// AND mask. For simplicity, write fully opaque.
	std::vector<uint8_t> andMask(andSize, 0);
	append_bytes(andMask.data(), andMask.size());

	return true;
}

bool IcoSessionWriter::write_ico_file()
{
	ICONDIR dir{};
	dir.idReserved = 0;
	dir.idType = 1;
	dir.idCount = 1;

	ICONDIRENTRY entry{};
	entry.bWidth = currentPage_.width >= 256 ? 0 : static_cast<uint8_t>(currentPage_.width);
	entry.bHeight = currentPage_.height >= 256 ? 0 : static_cast<uint8_t>(currentPage_.height);

	if (currentPage_.bitsPerPixel <= 8 &&
		currentPage_.paletteEntries > 0 &&
		currentPage_.paletteEntries < 256)
	{
		entry.bColorCount = static_cast<uint8_t>(currentPage_.paletteEntries);
	}
	else
	{
		entry.bColorCount = 0;
	}

	entry.bReserved = 0;
	entry.wPlanes = 1;
	entry.wBitCount = currentPage_.bitsPerPixel;
	entry.dwBytesInRes = static_cast<uint32_t>(imageData_.size());
	entry.dwImageOffset = sizeof(ICONDIR) + sizeof(ICONDIRENTRY);

	if (std::fwrite(&dir, sizeof(dir), 1, file_) != 1)
		return false;

	if (std::fwrite(&entry, sizeof(entry), 1, file_) != 1)
		return false;

	if (!imageData_.empty())
	{
		if (std::fwrite(imageData_.data(), 1, imageData_.size(), file_) != imageData_.size())
			return false;
	}

	hasCurrentPage_ = false;
	return true;
}

void IcoSessionWriter::append_bytes(const void* p, size_t n)
{
	const auto* b = static_cast<const uint8_t*>(p);
	imageData_.insert(imageData_.end(), b, b + n);
}

// ============================================================
// DTWAIN-style wrapper
// ============================================================

bool DTWAINIcoOutput::OnFirstPage(const std::wstring& filename,const IcoSessionOptions& options, const PreparedIcoDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<IcoSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINIcoOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}
