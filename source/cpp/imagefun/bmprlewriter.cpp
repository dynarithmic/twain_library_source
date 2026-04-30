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
#include "bmprlewriter.h"

std::optional<PreparedBmpRle8Page> BmpRle8Writer::MakePreparedBmpRle8Page(const dynarithmic::DibPageView& view)
{
	if (view.bitsPerPixel != 8 || !view.palette || view.paletteEntries == 0)
		return std::nullopt;

	PreparedBmpRle8Page page{};
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = 8;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;
	page.palette = view.palette;
	page.paletteEntries = view.paletteEntries;

	const auto bih = view.bih;
	page.xPelsPerMeter = bih ? bih->biXPelsPerMeter : 0;
	page.yPelsPerMeter = bih ? bih->biYPelsPerMeter : 0;
	return page;
}

BmpRle8Writer::~BmpRle8Writer()
{
	Close();
}

bool BmpRle8Writer::Open(const std::wstring& filename)
{
	if (file_)
		return false;

	file_ = _wfopen(filename.c_str(), L"wb");
	if (!file_)
		return false;

	filename_ = filename;
	hasPage_ = false;
	return true;
}

bool BmpRle8Writer::SetPageInfo(const PreparedBmpRle8Page& page)
{
	if (!file_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasPage_ = true;
	return true;
}

bool BmpRle8Writer::WriteCurrentPage()
{
	if (!file_ || !hasPage_)
		return false;

	encodedData_.clear();
	lineEncodeBuffer_.clear();

	if (!encode_rle8())
		return false;

	if (!write_bmp_file())
		return false;

	hasPage_ = false;
	return true;
}

void BmpRle8Writer::Close()
{
	if (file_)
	{
		std::fclose(file_);
		file_ = nullptr;
	}

	filename_.clear();
	encodedData_.clear();
	lineEncodeBuffer_.clear();
	hasPage_ = false;
}

bool BmpRle8Writer::ValidatePage(const PreparedBmpRle8Page& page)
{
	return page.width > 0 &&
		page.height > 0 &&
		page.bitsPerPixel == 8 &&
		page.bits != nullptr &&
		page.palette != nullptr &&
		page.paletteEntries > 0 &&
		page.strideBytes > 0;
}

void BmpRle8Writer::emit_byte(uint8_t b)
{
	encodedData_.push_back(b);
}

void BmpRle8Writer::emit_word(uint8_t a, uint8_t b)
{
	encodedData_.push_back(a);
	encodedData_.push_back(b);
}

// FreeImage-style RLE line encoder:
// - encoded runs only if run length > 3
// - literal buffer flush has special handling for lengths 1 and 2
// - literal buffer flushes at 254 bytes
int BmpRle8Writer::RLEEncodeLineLikeFreeImage(uint8_t* target, const uint8_t* source, int size)
{
	uint8_t buffer[256];
	int buffer_size = 0;
	int target_pos = 0;

	auto flush_literal_buffer = [&]() {
		switch (buffer_size)
		{
		case 0:
			break;

		case 1:
			target[target_pos++] = 1;
			target[target_pos++] = buffer[0];
			break;

		case 2:
			target[target_pos++] = 1;
			target[target_pos++] = buffer[0];
			target[target_pos++] = 1;
			target[target_pos++] = buffer[1];
			break;

		default:
			target[target_pos++] = 0;
			target[target_pos++] = static_cast<uint8_t>(buffer_size);
			std::memcpy(target + target_pos, buffer, static_cast<size_t>(buffer_size));
			target_pos += buffer_size;

			if (buffer_size & 1)
				target[target_pos++] = 0;
			break;
		}
		buffer_size = 0;
		};

	for (int i = 0; i < size; ++i)
	{
		if ((i < size - 1) && (source[i] == source[i + 1]))
		{
			int j = i + 1;
			const int jmax = i + 254;

			while ((j < size - 1) && (j < jmax) && (source[j] == source[j + 1]))
				++j;

			const int run_len = (j - i) + 1;

			// FreeImage uses encoded mode only when run length > 3
			if (run_len > 3)
			{
				flush_literal_buffer();
				target[target_pos++] = static_cast<uint8_t>(run_len);
				target[target_pos++] = source[i];
			}
			else
			{
				for (int k = 0; k < run_len; ++k)
				{
					buffer[buffer_size++] = source[i + k];
					if (buffer_size == 254)
						flush_literal_buffer();
				}
			}

			i = j;
		}
		else
		{
			buffer[buffer_size++] = source[i];
			if (buffer_size == 254)
				flush_literal_buffer();
		}
	}

	flush_literal_buffer();
	return target_pos;
}

bool BmpRle8Writer::encode_rle8()
{
	// Reserve something reasonable; exact size is not known in advance
	encodedData_.reserve(static_cast<size_t>(currentPage_.width) * currentPage_.height / 2);

	// FreeImage notes target can be same size as source line.
	lineEncodeBuffer_.resize(currentPage_.width > 0 ? currentPage_.width : 1);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		// BMP file rows are bottom-up when biHeight > 0.
		// We emit rows in file order from bottom row to top row.
		const uint32_t srcY = currentPage_.bottomUp
			? y
			: (currentPage_.height - 1 - y);

		const uint8_t* row =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		const int encodedSize =
			RLEEncodeLineLikeFreeImage(lineEncodeBuffer_.data(),
				row,
				static_cast<int>(currentPage_.width));

		if (encodedSize < 0)
			return false;

		encodedData_.insert(encodedData_.end(),
			lineEncodeBuffer_.begin(),
			lineEncodeBuffer_.begin() + encodedSize);

		// End of line
		emit_word(0, 0);
	}

	// End of bitmap
	emit_word(0, 1);
	return true;
}


bool BmpRle8Writer::write_bmp_file()
{
	static_assert(sizeof(BITMAPFILEHEADER) == 14, "BITMAPFILEHEADER must be 14 bytes");
	static_assert(sizeof(BITMAPINFOHEADER) == 40, "BITMAPINFOHEADER must be 40 bytes");

	const uint32_t paletteBytes =
		static_cast<uint32_t>(currentPage_.paletteEntries * sizeof(RGBQUAD));
	const uint32_t infoHeaderSize = sizeof(BITMAPINFOHEADER);
	const uint32_t fileHeaderSize = sizeof(BITMAPFILEHEADER);
	const uint32_t offBits = fileHeaderSize + infoHeaderSize + paletteBytes;
	const uint32_t imageSize = static_cast<uint32_t>(encodedData_.size());
	const uint32_t fileSize = offBits + imageSize;

	BITMAPFILEHEADER bfh{};
	bfh.bfType = 0x4D42; // 'BM'
	bfh.bfSize = fileSize;
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = offBits;

	BITMAPINFOHEADER bih{};
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = static_cast<LONG>(currentPage_.width);
	bih.biHeight = static_cast<LONG>(currentPage_.height); // bottom-up BMP
	bih.biPlanes = 1;
	bih.biBitCount = 8;
	bih.biCompression = BI_RLE8;
	bih.biSizeImage = imageSize;
	bih.biXPelsPerMeter = currentPage_.xPelsPerMeter;
	bih.biYPelsPerMeter = currentPage_.yPelsPerMeter;
	bih.biClrUsed = currentPage_.paletteEntries;
	bih.biClrImportant = currentPage_.paletteEntries;

	if (std::fwrite(&bfh, sizeof(bfh), 1, file_) != 1)
		return false;

	if (std::fwrite(&bih, sizeof(bih), 1, file_) != 1)
		return false;

	if (std::fwrite(currentPage_.palette,
		sizeof(RGBQUAD),
		currentPage_.paletteEntries,
		file_) != currentPage_.paletteEntries)
	{
		return false;
	}

	if (!encodedData_.empty())
	{
		if (std::fwrite(encodedData_.data(), 1, encodedData_.size(), file_) != encodedData_.size())
			return false;
	}

	return true;
}

bool DTWAINBmpRle8Output::OnFirstPage(const std::wstring& filename, const PreparedBmpRle8Page& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<BmpRle8Writer>();
	if (!writer_->Open(filename))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINBmpRle8Output::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

