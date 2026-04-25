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
#include "psdwriter.h"

 // ============================================================
 // Locked page wrapper
 // ============================================================
LockedPsdDibPage::LockedPsdDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	const auto* bih = dib_.Header();
	if (!bih || bih->biWidth <= 0 || bih->biHeight == 0)
		return;

	if (dib_.BitsPerPixel() != 24)
		return;

	PreparedPsdDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = 24;
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();

	page_ = page;
	valid_ = true;
}

bool LockedPsdDibPage::IsValid() const noexcept
{
	return valid_;
}

const PreparedPsdDibPage& LockedPsdDibPage::GetPage() const noexcept
{
	return page_;
}

///////////////////////////////////////////////////////////////
PsdSessionWriter::~PsdSessionWriter()
{
	Close();
}

bool PsdSessionWriter::Open(const std::wstring& filename, const PsdSessionOptions& options)
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

bool PsdSessionWriter::SetPageInfo(const PreparedPsdDibPage& page)
{
	if (!file_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool PsdSessionWriter::WriteCurrentPage()
{
	if (!file_ || !hasCurrentPage_)
		return false;

	if (!WriteHeader())
		return false;

	if (!WriteColorModeDataSection())
		return false;

	if (!WriteImageResourcesSection())
		return false;

	if (!WriteLayerAndMaskSection())
		return false;

	if (!WriteImageDataSection())
		return false;

	hasCurrentPage_ = false;
	return true;
}

void PsdSessionWriter::Close()
{
	if (file_)
	{
		std::fclose(file_);
		file_ = nullptr;
	}

	filename_.clear();
	channelRow_.clear();
	packBitsRow_.clear();
	rleLengths_.clear();
	rleData_.clear();
	hasCurrentPage_ = false;
}

bool PsdSessionWriter::IsOpen() const noexcept
{
	return file_ != nullptr;
}

bool PsdSessionWriter::ValidatePage(const PreparedPsdDibPage& page)
{
	return page.width > 0 &&
		page.height > 0 &&
		page.bitsPerPixel == 24 &&
		page.bits != nullptr &&
		page.strideBytes > 0;
}

bool PsdSessionWriter::WriteHeader()
{
	static const uint8_t sig[4] = { '8', 'B', 'P', 'S' };
	if (std::fwrite(sig, 1, 4, file_) != 4)
		return false;

	if (!dynarithmic::psd::write_u16_be(file_, 1))
		return false;

	const uint8_t reserved[6] = { 0, 0, 0, 0, 0, 0 };
	if (std::fwrite(reserved, 1, 6, file_) != 6)
		return false;

	if (!dynarithmic::psd::write_u16_be(file_, 3))
		return false;

	if (!dynarithmic::psd::write_u32_be(file_, currentPage_.height))
		return false;

	if (!dynarithmic::psd::write_u32_be(file_, currentPage_.width))
		return false;

	if (!dynarithmic::psd::write_u16_be(file_, 8))
		return false;

	if (!dynarithmic::psd::write_u16_be(file_, 3))
		return false;

	return true;
}

bool PsdSessionWriter::WriteColorModeDataSection()
{
	return dynarithmic::psd::write_u32_be(file_, 0);
}

bool PsdSessionWriter::WriteImageResourcesSection()
{
	if (options_.comment.empty())
		return dynarithmic::psd::write_u32_be(file_, 0);

	std::vector<uint8_t> resources;

	auto write_u16_be_vec = [](std::vector<uint8_t>& v, uint16_t x)
	{
		v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
		v.push_back(static_cast<uint8_t>(x & 0xFF));
	};

	auto write_u32_be_vec = [](std::vector<uint8_t>& v, uint32_t x)
	{
		v.push_back(static_cast<uint8_t>((x >> 24) & 0xFF));
		v.push_back(static_cast<uint8_t>((x >> 16) & 0xFF));
		v.push_back(static_cast<uint8_t>((x >> 8) & 0xFF));
		v.push_back(static_cast<uint8_t>(x & 0xFF));
	};

	auto append_resource = [&](uint16_t resourceId,
		const std::string& name,
		const std::vector<uint8_t>& data)
	{
		// Signature: '8BIM'
		resources.push_back('8');
		resources.push_back('B');
		resources.push_back('I');
		resources.push_back('M');

		// Resource ID
		write_u16_be_vec(resources, resourceId);

		// Pascal string name, padded to even size including length byte.
		const size_t nameLen = std::min<size_t>(name.size(), 255);
		resources.push_back(static_cast<uint8_t>(nameLen));
		resources.insert(resources.end(), name.begin(), name.begin() + nameLen);

		if (((1 + nameLen) & 1) != 0)
			resources.push_back(0);

		// Data size
		write_u32_be_vec(resources, static_cast<uint32_t>(data.size()));

		// Data
		resources.insert(resources.end(), data.begin(), data.end());

		// Data padded to even size
		if ((data.size() & 1) != 0)
			resources.push_back(0);
	};

	// Resource ID 1008 / 0x03F0 = Caption.
	std::vector<uint8_t> captionData(options_.comment.begin(),
		options_.comment.end());

	append_resource(1008, "Caption", captionData);

	if (!dynarithmic::psd::write_u32_be(file_,
		static_cast<uint32_t>(resources.size())))
	{
		return false;
	}

	if (!resources.empty())
	{
		if (std::fwrite(resources.data(), 1, resources.size(), file_) != resources.size())
			return false;
	}
	return true;
}

bool PsdSessionWriter::WriteLayerAndMaskSection()
{
	return dynarithmic::psd::write_u32_be(file_, 0);
}

bool PsdSessionWriter::WriteImageDataSection()
{
	if (options_.useRle)
		return WriteImageDataSectionRle();
	return WriteImageDataSectionRaw();
}

bool PsdSessionWriter::WriteImageDataSectionRaw()
{
	if (!dynarithmic::psd::write_u16_be(file_, 0))
		return false;

	channelRow_.resize(currentPage_.width);

	if (!WriteChannelPlaneRaw(2))
		return false;
	if (!WriteChannelPlaneRaw(1))
		return false;
	if (!WriteChannelPlaneRaw(0))
		return false;

	return true;
}

bool PsdSessionWriter::WriteImageDataSectionRle()
{
	if (!dynarithmic::psd::write_u16_be(file_, 1))
		return false;

	channelRow_.resize(currentPage_.width);
	packBitsRow_.clear();
	rleLengths_.clear();
	rleData_.clear();

	const size_t totalRows = static_cast<size_t>(currentPage_.height) * 3;
	rleLengths_.reserve(totalRows);

	if (!BuildChannelPlaneRle(2))
		return false;
	if (!BuildChannelPlaneRle(1))
		return false;
	if (!BuildChannelPlaneRle(0))
		return false;

	// PSD stores one 16-bit big-endian byte count per row per channel.
	for (uint16_t len : rleLengths_)
	{
		if (!dynarithmic::psd::write_u16_be(file_, len))
			return false;
	}

	if (!rleData_.empty())
	{
		if (std::fwrite(rleData_.data(), 1, rleData_.size(), file_) != rleData_.size())
			return false;
	}

	return true;
}

bool PsdSessionWriter::WriteChannelPlaneRaw(uint32_t bgrIndex)
{
	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		FillChannelRow(y, bgrIndex);

		if (std::fwrite(channelRow_.data(), 1, channelRow_.size(), file_) != channelRow_.size())
			return false;
	}

	return true;
}

bool PsdSessionWriter::BuildChannelPlaneRle(uint32_t bgrIndex)
{
	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		FillChannelRow(y, bgrIndex);

		packBitsRow_.clear();
		EncodePackBitsRow(channelRow_.data(),
			static_cast<uint32_t>(channelRow_.size()),
			packBitsRow_);

		if (packBitsRow_.size() > 65535)
			return false;

		rleLengths_.push_back(static_cast<uint16_t>(packBitsRow_.size()));
		rleData_.insert(rleData_.end(), packBitsRow_.begin(), packBitsRow_.end());
	}

	return true;
}

void PsdSessionWriter::FillChannelRow(uint32_t y, uint32_t bgrIndex)
{
	const uint32_t srcY =
		currentPage_.bottomUp
		? (currentPage_.height - 1 - y)
		: y;

	const uint8_t* srcRow =
		currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

	for (uint32_t x = 0; x < currentPage_.width; ++x)
		channelRow_[x] = srcRow[x * 3 + bgrIndex];
}

void PsdSessionWriter::EncodePackBitsRow(const uint8_t* src, uint32_t len, std::vector<uint8_t>& dst)
{
	uint32_t i = 0;

	while (i < len)
	{
		// Count run length of identical bytes, up to 128.
		uint32_t run = 1;
		while ((i + run) < len &&
			run < 128 &&
			src[i] == src[i + run])
		{
			++run;
		}

		if (run >= 3)
		{
			// Replicated run: header = 257 - run
			dst.push_back(static_cast<uint8_t>(257 - run));
			dst.push_back(src[i]);
			i += run;
			continue;
		}

		// Literal run. Accumulate until next long-enough repeated run or 128 bytes.
		const uint32_t literalStart = i;
		uint32_t literalLen = 0;

		while (i < len && literalLen < 128)
		{
			run = 1;
			while ((i + run) < len &&
				run < 128 &&
				src[i] == src[i + run])
			{
				++run;
			}

			if (run >= 3)
				break;

			++i;
			++literalLen;
		}

		// Header = literalLen - 1
		dst.push_back(static_cast<uint8_t>(literalLen - 1));
		dst.insert(dst.end(), src + literalStart, src + literalStart + literalLen);
	}
}

///////////////////////////////////////////////
bool DTWAINPsdOutput::OnFirstPage(const std::wstring& filename, const PsdSessionOptions& options, const PreparedPsdDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<PsdSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINPsdOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINPsdOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}

