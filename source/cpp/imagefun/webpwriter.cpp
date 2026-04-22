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
#include "webpwriter.h"
#include <windows.h>

static int WebPWriterCallback(const uint8_t* data, size_t data_size, const WebPPicture* picture)
{
	if (!data || !picture || !picture->custom_ptr)
		return 0;

	auto* ctx = static_cast<WebPMemoryWriterContext*>(picture->custom_ptr);
	ctx->data.insert(ctx->data.end(), data, data + data_size);
	return 1;
}

 // ============================================================
 // Locked page wrapper
 // ============================================================
LockedWebPDibPage::LockedWebPDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	const auto* bih = dib_.Header();
	if (!bih || bih->biWidth <= 0 || bih->biHeight == 0)
		return;

	PreparedWebPDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = dib_.BitsPerPixel();
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();

	switch (page.bitsPerPixel)
	{
		case 24:
			page.pixelFlavor = WebPPixelFlavor::Bgr24;
			break;

		case 32:
			page.pixelFlavor = WebPPixelFlavor::Bgra32;
			break;

		default:
			return;
	}

	page_ = page;
	valid_ = true;
}

bool LockedWebPDibPage::IsValid() const noexcept
{
	return valid_;
}

const PreparedWebPDibPage& LockedWebPDibPage::GetPage() const noexcept
{
	return page_;
}

////////////////////////////////////////////////////////////////
WebPSessionWriter::~WebPSessionWriter()
{
	Close();
}
bool WebPSessionWriter::Open(const std::wstring& filename, const WebPSessionOptions& options)
{
	if (isOpen_)
		return false;

	filename_ = filename;
	options_ = options;
	hasCurrentPage_ = false;
	isOpen_ = true;
	return true;
}

bool WebPSessionWriter::SetPageInfo(const PreparedWebPDibPage& page)
{
	if (!isOpen_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool WebPSessionWriter::WriteCurrentPage()
{
	if (!isOpen_ || !hasCurrentPage_)
		return false;

	WebPConfig config;
	if (!WebPConfigPreset(&config, WEBP_PRESET_DEFAULT, options_.quality))
		return false;

	config.lossless = options_.lossless ? 1 : 0;
	config.quality = options_.quality;
	config.method = options_.method;
	config.exact = options_.exact ? 1 : 0;

	if (!WebPValidateConfig(&config))
		return false;

	WebPPicture picture;
	if (!WebPPictureInit(&picture))
		return false;

	picture.width = static_cast<int>(currentPage_.width);
	picture.height = static_cast<int>(currentPage_.height);

	WebPMemoryWriterContext output;
	picture.writer = WebPWriterCallback;
	picture.custom_ptr = &output;

	bool ok = false;

	if (currentPage_.pixelFlavor == WebPPixelFlavor::Bgr24)
		ok = ImportBgr24(picture);
	else
		ok = ImportBgra32(picture);

	if (!ok)
	{
		WebPPictureFree(&picture);
		return false;
	}

	ok = (WebPEncode(&config, &picture) != 0);

	WebPPictureFree(&picture);

	if (!ok)
		return false;

	const bool writeOk = WriteOutputFile(output.data);
	if (!writeOk)
		return false;

	hasCurrentPage_ = false;
	return true;
}

void WebPSessionWriter::Close()
{
	filename_.clear();
	rowBuffer_.clear();
	hasCurrentPage_ = false;
	isOpen_ = false;
}

bool WebPSessionWriter::IsOpen() const noexcept
{
	return isOpen_;
}

bool WebPSessionWriter::ValidatePage(const PreparedWebPDibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 24:
		case 32:
			return true;
		default:
			return false;
	}
}

bool WebPSessionWriter::ImportBgr24(WebPPicture& picture)
{
	// Convert DIB top-down rows into a contiguous RGB buffer.
	const uint32_t rowBytes = currentPage_.width * 3;
	rgbBuffer_.resize(static_cast<size_t>(rowBytes) * currentPage_.height);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		uint8_t* dst =
			rgbBuffer_.data() + static_cast<size_t>(y) * rowBytes;

		for (uint32_t x = 0; x < currentPage_.width; ++x)
		{
			dst[x * 3 + 0] = src[x * 3 + 2]; // R
			dst[x * 3 + 1] = src[x * 3 + 1]; // G
			dst[x * 3 + 2] = src[x * 3 + 0]; // B
		}
	}

	return WebPPictureImportRGB(&picture, rgbBuffer_.data(), static_cast<int>(rowBytes)) != 0;
}

bool WebPSessionWriter::ImportBgra32(WebPPicture& picture)
{
	// Convert DIB top-down rows into a contiguous RGBA buffer.
	const uint32_t rowBytes = currentPage_.width * 4;
	rgbaBuffer_.resize(static_cast<size_t>(rowBytes) * currentPage_.height);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		uint8_t* dst =
			rgbaBuffer_.data() + static_cast<size_t>(y) * rowBytes;

		for (uint32_t x = 0; x < currentPage_.width; ++x)
		{
			dst[x * 4 + 0] = src[x * 4 + 2]; // R
			dst[x * 4 + 1] = src[x * 4 + 1]; // G
			dst[x * 4 + 2] = src[x * 4 + 0]; // B
			dst[x * 4 + 3] = src[x * 4 + 3]; // A
		}
	}

	return WebPPictureImportRGBA(&picture, rgbaBuffer_.data(), static_cast<int>(rowBytes)) != 0;
}

bool WebPSessionWriter::WriteOutputFile(const std::vector<uint8_t>& data) const
{
	FILE* f = _wfopen(filename_.c_str(), L"wb");
	if (!f)
		return false;

	const bool ok = std::fwrite(data.data(), 1, data.size(), f) == data.size();

	std::fclose(f);
	return ok;
}

/////////////////////////////////////////////////////////////
// ============================================================
// DTWAIN-style wrapper
// Model B for WebP:
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
bool DTWAINWebPOutput::OnFirstPage(const std::wstring& filename, const WebPSessionOptions& options, const PreparedWebPDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<WebPSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINWebPOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINWebPOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}
