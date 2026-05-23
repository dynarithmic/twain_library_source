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
#include <cstring>
#include <array>
#include "tgawriter.h"

std::optional<PreparedTgaDibPage> TgaSessionWriter::MakePreparedTgaDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedTgaDibPage page{};
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;
	page.palette = view.palette;
	page.paletteEntries = view.paletteEntries;

	switch (page.bitsPerPixel)
	{
		case 8:
			if (page.palette && page.paletteEntries > 0)
				page.pixelFlavor = TgaPixelFlavor::Palette8;
			else
				page.pixelFlavor = TgaPixelFlavor::Gray8;
			break;

		case 24:
			page.pixelFlavor = TgaPixelFlavor::Bgr24;
			break;

		case 32:
			page.pixelFlavor = TgaPixelFlavor::Bgra32;
			break;

		default:
			return std::nullopt;
	}

	return page;
}

 // ============================================================
 // TGA writer
 // ============================================================
TgaSessionWriter::~TgaSessionWriter()
{
	Close();
}

bool TgaSessionWriter::Open(const std::wstring& filename, const TgaSessionOptions& options)
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

bool TgaSessionWriter::SetPageInfo(const PreparedTgaDibPage& page)
{
	if (!file_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool TgaSessionWriter::WriteCurrentPage()
{
	if (!file_ || !hasCurrentPage_)
		return false;

	const TgaHeader hdr = BuildHeader();

	if (std::fwrite(&hdr, sizeof(hdr), 1, file_) != 1)
		return false;

	if (!WriteColorMap())
		return false;

	if (options_.useRle)
		return WriteImageRle();
	else
		return WriteImageRaw();
}

bool TgaSessionWriter::WriteExtensionAreaWithComments()
{
	const uint32_t extensionOffset =
		static_cast<uint32_t>(std::ftell(file_));

	std::array<uint8_t, 495> ext{};
	std::memset(ext.data(), 0, ext.size());

	constexpr size_t AuthorCommentsOffset = 41;
	constexpr size_t AuthorCommentsSize = 324;

	const size_t copyLen =
		std::min(options_.comment.size(), AuthorCommentsSize - 1);

	std::memcpy(ext.data() + AuthorCommentsOffset,
				options_.comment.data(), copyLen);

	if (std::fwrite(ext.data(), 1, ext.size(), file_) != ext.size())
		return false;

	struct TgaFooter
	{
		uint32_t extensionOffset;
		uint32_t developerOffset;
		char signature[18];
	} footer{};

	footer.extensionOffset = extensionOffset;
	footer.developerOffset = 0;

	std::memcpy(footer.signature, "TRUEVISION-XFILE.", 17);
	footer.signature[17] = '\0';

	if (std::fwrite(&footer, sizeof(footer), 1, file_) != 1)
		return false;

	return true;
}

void TgaSessionWriter::Close()
{
	if (file_)
	{
		WriteExtensionAreaWithComments();
		std::fclose(file_);
		file_ = nullptr;
	}

	filename_.clear();
	rowBuffer_.clear();
	packetBuffer_.clear();
	hasCurrentPage_ = false;
}

bool TgaSessionWriter::IsOpen() const noexcept
{
	return file_ != nullptr;
}

bool TgaSessionWriter::ValidatePage(const PreparedTgaDibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 8:
		case 24:
		case 32:
			return true;
		default:
			return false;
	}
}

TgaHeader TgaSessionWriter::BuildHeader() const
{
	TgaHeader hdr{};
	hdr.id_length = 0;
	hdr.color_map_type = UsesColorMap() ? 1 : 0;
	hdr.image_type = GetImageType();
	hdr.color_map_first_entry = 0;
	hdr.color_map_length = UsesColorMap()
		? static_cast<uint16_t>(GetColorMapLength())
		: 0;
	hdr.color_map_entry_size = UsesColorMap() ? 24 : 0;
	hdr.x_origin = 0;
	hdr.y_origin = 0;
	hdr.width = static_cast<uint16_t>(currentPage_.width);
	hdr.height = static_cast<uint16_t>(currentPage_.height);
	hdr.pixel_depth = GetPixelDepth();
	hdr.image_descriptor = GetImageDescriptor();
	return hdr;
}

bool TgaSessionWriter::UsesColorMap() const
{
	return currentPage_.pixelFlavor == TgaPixelFlavor::Palette8;
}

uint16_t TgaSessionWriter::GetColorMapLength() const
{
	if (!UsesColorMap())
		return 0;
	if (currentPage_.paletteEntries == 0)
		return 256;
	return static_cast<uint16_t>(currentPage_.paletteEntries > 256 ? 256 : currentPage_.paletteEntries);
}

uint8_t TgaSessionWriter::GetImageType() const
{
	switch (currentPage_.pixelFlavor)
	{
		case TgaPixelFlavor::Palette8:
			return options_.useRle ? 9 : 1;
		case TgaPixelFlavor::Gray8:
			return options_.useRle ? 11 : 3;
		case TgaPixelFlavor::Bgr24:
		case TgaPixelFlavor::Bgra32:
			return options_.useRle ? 10 : 2;
	}
	return 2;
}

uint8_t TgaSessionWriter::GetPixelDepth() const
{
	switch (currentPage_.pixelFlavor)
	{
		case TgaPixelFlavor::Palette8:
		case TgaPixelFlavor::Gray8:
			return 8;
		case TgaPixelFlavor::Bgr24:
			return 24;
		case TgaPixelFlavor::Bgra32:
			return 32;
	}
	return 24;
}

uint8_t TgaSessionWriter::GetImageDescriptor() const
{
	uint8_t desc = 0;

	if (currentPage_.pixelFlavor == TgaPixelFlavor::Bgra32)
		desc |= 8; // alpha bits

	// Top-left origin. Makes row order match the normalized write path.
	desc |= 0x20;
	return desc;
}

bool TgaSessionWriter::WriteColorMap()
{
	if (!UsesColorMap())
		return true;

	const uint16_t n = GetColorMapLength();
	for (uint16_t i = 0; i < n; ++i)
	{
		uint8_t bgr[3] = { 0, 0, 0 };

		if (currentPage_.palette && i < currentPage_.paletteEntries)
		{
			bgr[0] = currentPage_.palette[i].rgbBlue;
			bgr[1] = currentPage_.palette[i].rgbGreen;
			bgr[2] = currentPage_.palette[i].rgbRed;
		}

		if (std::fwrite(bgr, sizeof(bgr), 1, file_) != 1)
			return false;
	}
	return true;
}

uint32_t TgaSessionWriter::PixelBytes() const
{
	switch (currentPage_.pixelFlavor)
	{
		case TgaPixelFlavor::Palette8:
		case TgaPixelFlavor::Gray8:
			return 1;
		case TgaPixelFlavor::Bgr24:
			return 3;
		case TgaPixelFlavor::Bgra32:
			return 4;
	}
	return 1;
}

const uint8_t* TgaSessionWriter::GetRowPtr(uint32_t y) const
{
	// TGA written here as top-left origin, so normalize to top-down rows.
	const uint32_t srcY = currentPage_.bottomUp
		? (currentPage_.height - 1 - y)
		: y;

	return currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;
}

bool TgaSessionWriter::WriteImageRaw()
{
	const uint32_t rowBytes = currentPage_.width * PixelBytes();
	rowBuffer_.resize(rowBytes);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint8_t* src = GetRowPtr(y);

		if (!PrepareRow(src, rowBuffer_.data(), rowBytes))
			return false;

		if (std::fwrite(rowBuffer_.data(), 1, rowBytes, file_) != rowBytes)
			return false;
	}

	hasCurrentPage_ = false;
	return true;
}

bool TgaSessionWriter::WriteImageRle()
{
	const uint32_t pixelBytes = PixelBytes();
	const uint32_t rowBytes = currentPage_.width * pixelBytes;
	rowBuffer_.resize(rowBytes);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint8_t* src = GetRowPtr(y);

		if (!PrepareRow(src, rowBuffer_.data(), rowBytes))
			return false;

		if (!WriteRleRow(rowBuffer_.data(), currentPage_.width, pixelBytes))
			return false;
	}

	hasCurrentPage_ = false;
	return true;
}

bool TgaSessionWriter::PrepareRow(const uint8_t* src, uint8_t* dst, uint32_t rowBytes) const
{
	switch (currentPage_.pixelFlavor)
	{
		case TgaPixelFlavor::Palette8:
		case TgaPixelFlavor::Gray8:
			std::memcpy(dst, src, rowBytes);
			return true;

		case TgaPixelFlavor::Bgr24:
			// DIB is already BGR, same as TGA true-color byte order.
			std::memcpy(dst, src, rowBytes);
			return true;

		case TgaPixelFlavor::Bgra32:
			// DIB is already BGRA, same as TGA true-color+alpha byte order.
			std::memcpy(dst, src, rowBytes);
			return true;
	}
	return false;
}

bool TgaSessionWriter::PixelsEqual(const uint8_t* a, const uint8_t* b, uint32_t pixelBytes)
{
	return std::memcmp(a, b, pixelBytes) == 0;
}

bool TgaSessionWriter::WriteRleRow(const uint8_t* row, uint32_t width, uint32_t pixelBytes)
{
	uint32_t x = 0;

	while (x < width)
	{
		// Look for a run packet first.
		uint32_t runLen = 1;
		while ((x + runLen) < width &&
			runLen < 128 &&
			PixelsEqual(row + x * pixelBytes,
				row + (x + runLen) * pixelBytes,
				pixelBytes))
		{
			++runLen;
		}

		if (runLen >= 2)
		{
			const uint8_t header = static_cast<uint8_t>(0x80 | (runLen - 1));
			if (std::fwrite(&header, 1, 1, file_) != 1)
				return false;

			if (std::fwrite(row + x * pixelBytes, 1, pixelBytes, file_) != pixelBytes)
				return false;

			x += runLen;
			continue;
		}

		// Raw packet: stop before a repeated run begins, or at 128 pixels.
		const uint32_t rawStart = x;
		uint32_t rawLen = 1;
		++x;

		while (x < width && rawLen < 128)
		{
			runLen = 1;
			while ((x + runLen) < width &&
				runLen < 128 &&
				PixelsEqual(row + x * pixelBytes,
					row + (x + runLen) * pixelBytes,
					pixelBytes))
			{
				++runLen;
			}

			if (runLen >= 2)
				break;

			++x;
			++rawLen;
		}

		const uint8_t header = static_cast<uint8_t>(rawLen - 1);
		if (std::fwrite(&header, 1, 1, file_) != 1)
			return false;

		const size_t rawBytes = static_cast<size_t>(rawLen) * pixelBytes;
		if (std::fwrite(row + rawStart * pixelBytes, 1, rawBytes, file_) != rawBytes)
			return false;
	}

	return true;
}

bool DTWAINTgaOutput::OnFirstPage(const std::wstring& filename, const TgaSessionOptions& options, const PreparedTgaDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<TgaSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINTgaOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINTgaOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}
