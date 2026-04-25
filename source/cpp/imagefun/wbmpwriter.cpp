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
#include "wbmpwriter.h"

 // ============================================================
 // Locked page wrapper
 // ============================================================
LockedWbmpDibPage::LockedWbmpDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	const auto* bih = dib_.Header();
	if (!bih || bih->biWidth <= 0 || bih->biHeight == 0)
		return;

	if (dib_.BitsPerPixel() != 1)
		return;

	PreparedWbmpDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = 1;
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();

	page_ = page;
	valid_ = true;
}

bool LockedWbmpDibPage::IsValid() const noexcept
{
	return valid_;
}

const PreparedWbmpDibPage& LockedWbmpDibPage::GetPage() const noexcept
{
	return page_;
}

// ============================================================
// WBMP writer
// Type 0 WBMP:
//   byte 0 = TypeField = 0
//   byte 1 = FixHeaderField = 0
//   width  = multi-byte integer
//   height = multi-byte integer
//   image data = packed 1-bpp rows, MSB = leftmost pixel
// ============================================================
WbmpSessionWriter::~WbmpSessionWriter()
{
	Close();
}

bool WbmpSessionWriter::Open(const std::wstring& filename, const WbmpSessionOptions& options)
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

bool WbmpSessionWriter::SetPageInfo(const PreparedWbmpDibPage& page)
{
	if (!file_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool WbmpSessionWriter::WriteCurrentPage()
{
	if (!file_ || !hasCurrentPage_)
		return false;

	if (!WriteHeader())
		return false;

	if (!WriteBitmapData())
		return false;

	hasCurrentPage_ = false;
	return true;
}

void WbmpSessionWriter::Close()
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

bool WbmpSessionWriter::IsOpen() const noexcept
{
	return file_ != nullptr;
}

bool WbmpSessionWriter::ValidatePage(const PreparedWbmpDibPage& page)
{
	return page.width > 0 &&
		page.height > 0 &&
		page.bitsPerPixel == 1 &&
		page.bits != nullptr &&
		page.strideBytes > 0;
}

uint8_t WbmpSessionWriter::ReverseBits(uint8_t v)
{
	v = static_cast<uint8_t>(((v & 0xF0) >> 4) | ((v & 0x0F) << 4));
	v = static_cast<uint8_t>(((v & 0xCC) >> 2) | ((v & 0x33) << 2));
	v = static_cast<uint8_t>(((v & 0xAA) >> 1) | ((v & 0x55) << 1));
	return v;
}

bool WbmpSessionWriter::WriteByte(uint8_t b)
{
	return std::fwrite(&b, 1, 1, file_) == 1;
}

bool WbmpSessionWriter::WriteMultiByteUInt(uint32_t value)
{
	// WBMP uses big-endian variable-length 7-bit groups.
	uint8_t tmp[5]{};
	int count = 0;

	do
	{
		tmp[count++] = static_cast<uint8_t>(value & 0x7F);
		value >>= 7;
	} while (value != 0 && count < 5);

	for (int i = count - 1; i >= 0; --i)
	{
		uint8_t b = tmp[i];
		if (i != 0)
			b |= 0x80;

		if (!WriteByte(b))
			return false;
	}

	return true;
}

bool WbmpSessionWriter::WriteHeader()
{
	// TypeField = 0, FixHeaderField = 0
	if (!WriteByte(0))
		return false;
	if (!WriteByte(0))
		return false;

	if (!WriteMultiByteUInt(currentPage_.width))
		return false;
	if (!WriteMultiByteUInt(currentPage_.height))
		return false;

	return true;
}

bool WbmpSessionWriter::WriteBitmapData()
{
	const uint32_t rowBytes = static_cast<uint32_t>((currentPage_.width + 7) / 8);
	rowBuffer_.resize(rowBytes);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY =
			currentPage_.bottomUp ? (currentPage_.height - 1 - y) : y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		std::memcpy(rowBuffer_.data(), src, rowBytes);

		if (options_.reverseBitOrder)
		{
			for (uint32_t i = 0; i < rowBytes; ++i)
				rowBuffer_[i] = ReverseBits(rowBuffer_[i]);
		}

		if (std::fwrite(rowBuffer_.data(), 1, rowBytes, file_) != rowBytes)
			return false;
	}

	return true;
}

bool DTWAINWbmpOutput::OnFirstPage(const std::wstring& filename, const WbmpSessionOptions& options, const PreparedWbmpDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<WbmpSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINWbmpOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINWbmpOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}

