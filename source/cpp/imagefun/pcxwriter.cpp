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
#include "pcxwriter.h"

 // ============================================================
 // Locked page wrapper
 // ============================================================

LockedPcxDibPage::LockedPcxDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	const auto* bih = dib_.Header();
	if (!bih || bih->biWidth <= 0 || bih->biHeight == 0)
		return;

	PreparedPcxDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = dib_.BitsPerPixel();
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();
	page.palette = dib_.Palette();
	page.paletteEntries = dib_.PaletteEntries();

	if (bih->biXPelsPerMeter > 0)
		page.xDpi = static_cast<uint16_t>(bih->biXPelsPerMeter * 0.0254 + 0.5);
	if (bih->biYPelsPerMeter > 0)
		page.yDpi = static_cast<uint16_t>(bih->biYPelsPerMeter * 0.0254 + 0.5);

	switch (page.bitsPerPixel)
	{
		case 1:
			page.pixelFlavor = PcxPixelFlavor::BW1;
			break;
		case 8:
			page.pixelFlavor = PcxPixelFlavor::Indexed8;
			break;
		case 24:
			page.pixelFlavor = PcxPixelFlavor::Bgr24;
			break;
		default:
			return;
	}

	page_ = page;
	valid_ = true;
}

bool LockedPcxDibPage::IsValid() const noexcept { return valid_; }
const PreparedPcxDibPage& LockedPcxDibPage::GetPage() const noexcept { return page_; }

////////////////////////////////////////////////////////////////////////
// ============================================================
// Session writer
// ============================================================

PcxSessionWriter::~PcxSessionWriter()
{
	Close();
}

bool PcxSessionWriter::Open(const std::wstring& filename, const PcxSessionOptions& options)
{
	if (file_)
		return false;

	file_ = _wfopen(filename.c_str(), L"wb+");
	if (!file_)
		return false;

	filename_ = filename;
	options_ = options;
	isDcx_ = options_.writeDcx;

	if (isDcx_)
	{
		// DCX directory:
		// magic + 1024 DWORD offsets (offset[0..1022] pages, zero terminator)
		pageOffsets_.clear();
		if (!dynarithmic::pcx::write_u32_le(file_, 0x3ADE68B1u))
			return false;
		for (int i = 0; i < 1024; ++i)
		{
			if (!dynarithmic::pcx::write_u32_le(file_, 0))
				return false;
		}
	}

	hasOpen_ = true;
	return true;
}

bool PcxSessionWriter::WritePage(const PreparedPcxDibPage& page)
{
	if (!hasOpen_ || !ValidatePage(page))
		return false;

	if (isDcx_)
	{
		if (pageOffsets_.size() >= 1023)
			return false;

		const long pos = std::ftell(file_);
		if (pos < 0)
			return false;

		pageOffsets_.push_back(static_cast<uint32_t>(pos));
	}
	else if (!pageOffsets_.empty())
	{
		// Single-page PCX only accepts one page
		return false;
	}
	else
	{
		pageOffsets_.push_back(0);
	}

	return WriteOnePcx(page);
}

bool PcxSessionWriter::Close()
{
	if (!file_)
		return true;

	bool ok = true;

	if (isDcx_)
	{
		ok = PatchDcxDirectory();
	}

	std::fclose(file_);
	file_ = nullptr;
	hasOpen_ = false;
	pageOffsets_.clear();
	rowBuffer_.clear();
	planeBuffer_.clear();
	return ok;
}

bool PcxSessionWriter::ValidatePage(const PreparedPcxDibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	return page.bitsPerPixel == 1 || page.bitsPerPixel == 8 || page.bitsPerPixel == 24;
}

bool PcxSessionWriter::PatchDcxDirectory()
{
	if (std::fseek(file_, 4, SEEK_SET) != 0)
		return false;

	for (size_t i = 0; i < pageOffsets_.size(); ++i)
	{
		if (!dynarithmic::pcx::write_u32_le(file_, pageOffsets_[i]))
			return false;
	}

	// terminating zero
	if (!dynarithmic::pcx::write_u32_le(file_, 0))
		return false;

	return true;
}

bool PcxSessionWriter::WriteOnePcx(const PreparedPcxDibPage& page)
{
	dynarithmic::pcx::PcxHeader hdr{};
	hdr.manufacturer = 0x0A;
	hdr.version = 5;
	hdr.encoding = 1;
	hdr.xmin = 0;
	hdr.ymin = 0;
	hdr.xmax = static_cast<uint16_t>(page.width - 1);
	hdr.ymax = static_cast<uint16_t>(page.height - 1);
	hdr.hDpi = page.xDpi;
	hdr.vDpi = page.yDpi;
	hdr.reserved1 = 0;
	hdr.paletteType = 1;
	hdr.hScreenSize = static_cast<uint16_t>(page.width);
	hdr.vScreenSize = static_cast<uint16_t>(page.height);

	switch (page.pixelFlavor)
	{
		case PcxPixelFlavor::BW1:
			hdr.bitsPerPixel = 1;
			hdr.colorPlanes = 1;
			hdr.bytesPerLine = dynarithmic::pcx::even_pad(
				static_cast<uint16_t>((page.width + 7u) / 8u));
			FillMonoPalette(hdr, page);
			break;

		case PcxPixelFlavor::Indexed8:
			hdr.bitsPerPixel = 8;
			hdr.colorPlanes = 1;
			hdr.bytesPerLine = dynarithmic::pcx::even_pad(
				static_cast<uint16_t>(page.width));
			break;

		case PcxPixelFlavor::Bgr24:
			hdr.bitsPerPixel = 8;
			hdr.colorPlanes = 3;
			hdr.bytesPerLine = dynarithmic::pcx::even_pad(
				static_cast<uint16_t>(page.width));
			break;
	}

	if (std::fwrite(&hdr, 1, sizeof(hdr), file_) != sizeof(hdr))
		return false;

	switch (page.pixelFlavor)
	{
		case PcxPixelFlavor::BW1:
			if (!Write1Bpp(page, hdr.bytesPerLine))
				return false;
			break;

		case PcxPixelFlavor::Indexed8:
			if (!Write8Bpp(page, hdr.bytesPerLine))
				return false;
			if (!Write256Palette(page))
				return false;
			break;

		case PcxPixelFlavor::Bgr24:
			if (!Write24Bpp(page, hdr.bytesPerLine))
				return false;
			break;
	}

	return true;
}

void PcxSessionWriter::FillMonoPalette(dynarithmic::pcx::PcxHeader& hdr, const PreparedPcxDibPage& page)
{
	// Prefer DIB mono palette if present; otherwise default to black/white.
	RGBQUAD c0{ 0,0,0,0 };
	RGBQUAD c1{ 255,255,255,0 };

	if (page.palette && page.paletteEntries >= 2)
	{
		c0 = page.palette[0];
		c1 = page.palette[1];
	}

	hdr.egaPalette[0] = c0.rgbRed;
	hdr.egaPalette[1] = c0.rgbGreen;
	hdr.egaPalette[2] = c0.rgbBlue;

	hdr.egaPalette[3] = c1.rgbRed;
	hdr.egaPalette[4] = c1.rgbGreen;
	hdr.egaPalette[5] = c1.rgbBlue;
}

bool PcxSessionWriter::Write1Bpp(const PreparedPcxDibPage& page, uint16_t bytesPerLine)
{
	rowBuffer_.assign(bytesPerLine, 0);
	planeBuffer_.clear();

	const uint32_t srcPacked = (page.width + 7u) / 8u;

	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
		const uint8_t* src = page.bits + static_cast<size_t>(srcY) * page.strideBytes;

		std::memset(rowBuffer_.data(), 0, rowBuffer_.size());
		std::memcpy(rowBuffer_.data(), src, srcPacked);

		planeBuffer_.clear();
		dynarithmic::pcx::encode_rle_row(rowBuffer_.data(), bytesPerLine, planeBuffer_);

		if (std::fwrite(planeBuffer_.data(), 1, planeBuffer_.size(), file_) != planeBuffer_.size())
			return false;
	}

	return true;
}

bool PcxSessionWriter::Write8Bpp(const PreparedPcxDibPage& page, uint16_t bytesPerLine)
{
	rowBuffer_.assign(bytesPerLine, 0);
	planeBuffer_.clear();

	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
		const uint8_t* src = page.bits + static_cast<size_t>(srcY) * page.strideBytes;

		std::memset(rowBuffer_.data(), 0, rowBuffer_.size());
		std::memcpy(rowBuffer_.data(), src, page.width);

		planeBuffer_.clear();
		dynarithmic::pcx::encode_rle_row(rowBuffer_.data(), bytesPerLine, planeBuffer_);

		if (std::fwrite(planeBuffer_.data(), 1, planeBuffer_.size(), file_) != planeBuffer_.size())
			return false;
	}

	return true;
}

bool PcxSessionWriter::Write24Bpp(const PreparedPcxDibPage& page, uint16_t bytesPerLine)
{
	rowBuffer_.assign(bytesPerLine, 0);
	planeBuffer_.clear();

	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
		const uint8_t* src = page.bits + static_cast<size_t>(srcY) * page.strideBytes;

		// PCX truecolor stores row by plane: R row, G row, B row
		for (int plane = 0; plane < 3; ++plane)
		{
			std::memset(rowBuffer_.data(), 0, rowBuffer_.size());

			for (uint32_t x = 0; x < page.width; ++x)
			{
				const uint8_t b = src[x * 3 + 0];
				const uint8_t g = src[x * 3 + 1];
				const uint8_t r = src[x * 3 + 2];

				rowBuffer_[x] = (plane == 0) ? r : (plane == 1 ? g : b);
			}

			planeBuffer_.clear();
			dynarithmic::pcx::encode_rle_row(rowBuffer_.data(), bytesPerLine, planeBuffer_);

			if (std::fwrite(planeBuffer_.data(), 1, planeBuffer_.size(), file_) != planeBuffer_.size())
				return false;
		}
	}

	return true;
}

bool PcxSessionWriter::Write256Palette(const PreparedPcxDibPage& page)
{
	static const uint8_t marker = 0x0C;
	if (std::fwrite(&marker, 1, 1, file_) != 1)
		return false;

	for (uint32_t i = 0; i < 256; ++i)
	{
		uint8_t rgb[3] = { 0, 0, 0 };

		if (page.palette && i < page.paletteEntries)
		{
			rgb[0] = page.palette[i].rgbRed;
			rgb[1] = page.palette[i].rgbGreen;
			rgb[2] = page.palette[i].rgbBlue;
		}
		else
		{
			// grayscale fallback
			rgb[0] = rgb[1] = rgb[2] = static_cast<uint8_t>(i);
		}

		if (std::fwrite(rgb, 1, 3, file_) != 3)
			return false;
	}
	return true;
}

///////////////////////////////////////////////////////////
// ============================================================
// DTWAIN-style wrapper
// ============================================================
bool DTWAINPcxDcxOutput::OnFirstPage(const std::wstring& filename, const PcxSessionOptions& options, const PreparedPcxDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<PcxSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	return writer_->WritePage(page);
}

bool DTWAINPcxDcxOutput::OnNextPage(const PreparedPcxDibPage& page)
{
	if (!writer_)
		return false;
	return writer_->WritePage(page);
}

bool DTWAINPcxDcxOutput::OnLastPage()
{
	if (!writer_)
		return false;

	const bool ok = writer_->Close();
	writer_.reset();
	return ok;
}

