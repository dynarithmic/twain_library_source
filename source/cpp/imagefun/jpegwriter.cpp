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

#include "jpegwriter.h"
#include <jpeglib.h>

 // ============================================================
 // Locked page wrapper
 // ============================================================
LockedJpegDibPage::LockedJpegDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	const auto* bih = dib_.Header();
	if (!bih || bih->biWidth <= 0 || bih->biHeight == 0)
		return;

	PreparedJpegDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = dib_.BitsPerPixel();
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();

	switch (page.bitsPerPixel)
	{
		case 8:
		{
			const RGBQUAD* pal = dib_.Palette();
			const uint32_t palEntries = dib_.PaletteEntries();

			// If an 8-bpp palette exists, require it to be grayscale.
			if (pal && palEntries > 0 &&
				!dynarithmic::dib::is_grayscale_palette(pal, palEntries))
			{
				return;
			}

			page.pixelFlavor = JpegPixelFlavor::Gray8;
			break;
		}

		case 16:
			page.pixelFlavor = JpegPixelFlavor::Gray16;
			break;

		case 24:
			page.pixelFlavor = JpegPixelFlavor::Bgr24;
			break;

		default:
			return;
	}

	page_ = page;
	valid_ = true;
}

bool LockedJpegDibPage::IsValid() const noexcept
{
	return valid_;
}

const PreparedJpegDibPage& LockedJpegDibPage::GetPage() const noexcept
{
	return page_;
}

// ============================================================
// JPEG writer
// ============================================================
JpegSessionWriter::~JpegSessionWriter()
{
	Close();
}

bool JpegSessionWriter::Open(const std::wstring& filename, const JpegSessionOptions& options)
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

bool JpegSessionWriter::SetPageInfo(const PreparedJpegDibPage& page)
{
	if (!file_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool JpegSessionWriter::WriteCurrentPage()
{
	if (!file_ || !hasCurrentPage_)
		return false;

	jpeg_compress_struct cinfo{};
	jpeg_error_mgr jerr{};

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, file_);

	cinfo.image_width = currentPage_.width;
	cinfo.image_height = currentPage_.height;

	if (currentPage_.pixelFlavor == JpegPixelFlavor::Bgr24)
	{
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;
	}
	else
	{
		cinfo.input_components = 1;
		cinfo.in_color_space = JCS_GRAYSCALE;
	}

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, options_.quality, TRUE);

	if (options_.progressive)
		jpeg_simple_progression(&cinfo);

	jpeg_start_compress(&cinfo, TRUE);

	const uint32_t rowBytes =
		(currentPage_.pixelFlavor == JpegPixelFlavor::Bgr24)
		? currentPage_.width * 3
		: currentPage_.width;

	rowBuffer_.resize(rowBytes);

	while (cinfo.next_scanline < cinfo.image_height)
	{
		const uint32_t y = static_cast<uint32_t>(cinfo.next_scanline);
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		if (currentPage_.pixelFlavor == JpegPixelFlavor::Gray8)
		{
			std::memcpy(rowBuffer_.data(), src, currentPage_.width);
		}
		else if (currentPage_.pixelFlavor == JpegPixelFlavor::Gray16)
		{
			// Down-convert 16-bit grayscale to 8-bit grayscale for JPEG.
			const uint16_t* src16 = reinterpret_cast<const uint16_t*>(src);
			for (uint32_t x = 0; x < currentPage_.width; ++x)
				rowBuffer_[x] = static_cast<uint8_t>(src16[x] >> 8);
		}
		else
		{
			// Convert BGR DIB row -> RGB JPEG row
			for (uint32_t x = 0; x < currentPage_.width; ++x)
			{
				rowBuffer_[x * 3 + 0] = src[x * 3 + 2];
				rowBuffer_[x * 3 + 1] = src[x * 3 + 1];
				rowBuffer_[x * 3 + 2] = src[x * 3 + 0];
			}
		}

		JSAMPROW rowPtr = rowBuffer_.data();
		if (jpeg_write_scanlines(&cinfo, &rowPtr, 1) != 1)
		{
			jpeg_finish_compress(&cinfo);
			jpeg_destroy_compress(&cinfo);
			return false;
		}
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	hasCurrentPage_ = false;
	return true;
}

void JpegSessionWriter::Close()
{
	if (file_)
	{
		std::fclose(file_);
		file_ = nullptr;
	}

	filename_.clear();
	rowBuffer_.clear();
	hasCurrentPage_ = false;
}

bool JpegSessionWriter::IsOpen() const noexcept
{
	return file_ != nullptr;
}

bool JpegSessionWriter::ValidatePage(const PreparedJpegDibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 8:
		case 16:
		case 24:
			return true;
		default:
			return false;
	}
	return false;
}

bool DTWAINJpegOutput::OnFirstPage(const std::wstring& filename, const JpegSessionOptions& options, const PreparedJpegDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<JpegSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINJpegOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}
