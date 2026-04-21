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
#include "gifwriter.h"

 // ============================================================
 // HANDLE-based DIB lock helper
 // ============================================================
LockedGifDibPage::LockedGifDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	if (dib_.BitsPerPixel() != 8)
		return;

	PreparedGifDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = 8;
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();
	page.palette = dib_.Palette();
	page.paletteEntries = dib_.PaletteEntries();

	if (page.palette && page.paletteEntries > 0 &&
		dynarithmic::dib::is_grayscale_palette(page.palette, page.paletteEntries))
		page.pixelFlavor = GifPixelFlavor::Gray8;
	else if (page.palette && page.paletteEntries > 0)
		page.pixelFlavor = GifPixelFlavor::Indexed8;
	else
		page.pixelFlavor = GifPixelFlavor::Gray8;

	page_ = page;
	valid_ = true;
}

bool LockedGifDibPage::IsValid() const noexcept { return valid_; }
const PreparedGifDibPage& LockedGifDibPage::GetPage() const noexcept { return page_; }

// ============================================================
// GIF writer
// Single-image writer for DTWAIN-style first/last-page workflow
// ============================================================

GifSessionWriter::~GifSessionWriter()
{
	Close();
}

bool GifSessionWriter::Open(const std::wstring& filename, const GifSessionOptions& sessionOptions)
{
	if (gif_)
		return false;

	file_ = _wfopen(filename.c_str(), L"wb");
	if (!file_)
		return false;

	int errorCode = 0;
	gif_ = EGifOpen(file_, &GifSessionWriter::gif_write_callback, &errorCode);
	if (!gif_)
	{
		Close();
		return false;
	}

	filename_ = filename;
	sessionOptions_ = sessionOptions;
	hasCurrentPage_ = false;
	return true;
}

bool GifSessionWriter::SetPageInfo(const PreparedGifDibPage& page)
{
	if (!gif_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool GifSessionWriter::WriteCurrentPage()
{
	if (!gif_ || !hasCurrentPage_)
		return false;

	prepare_color_map();
	if (!colorMap_)
		return false;

	const int width = static_cast<int>(currentPage_.width);
	const int height = static_cast<int>(currentPage_.height);

	if (EGifPutScreenDesc(gif_, width, height, 8, 0, colorMap_) == GIF_ERROR)
	{
		return false;
	}

	if (!write_comment_extensions())
		return false;

	if (EGifPutImageDesc(gif_, 0, 0, width, height, false, nullptr) == GIF_ERROR)
	{
		return false;
	}

	rowBuffer_.resize(currentPage_.width);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src = currentPage_.bits +
			static_cast<size_t>(srcY) * currentPage_.strideBytes;

		std::memcpy(rowBuffer_.data(), src, currentPage_.width);

		if (EGifPutLine(gif_,
			reinterpret_cast<GifPixelType*>(rowBuffer_.data()),
			static_cast<int>(currentPage_.width)) == GIF_ERROR)
		{
			return false;
		}
	}

	hasCurrentPage_ = false;
	return true;
}

void GifSessionWriter::Close()
{
	if (gif_)
	{
		int errorCode = 0;
		EGifCloseFile(gif_, &errorCode);
		gif_ = nullptr;
	}

	if (colorMap_)
	{
		GifFreeMapObject(colorMap_);
		colorMap_ = nullptr;
	}

	if (file_)
	{
		std::fclose(file_);
		file_ = nullptr;
	}

	rowBuffer_.clear();
	filename_.clear();
	hasCurrentPage_ = false;
}

bool GifSessionWriter::IsOpen() const noexcept
{
	return gif_ != nullptr;
}

int GifSessionWriter::gif_write_callback(GifFileType* gif, const GifByteType* data, int length)
{
	FILE* file = static_cast<FILE*>(gif->UserData);
	if (!file || !data || length < 0)
		return 0;

	return static_cast<int>(std::fwrite(data, 1, static_cast<size_t>(length), file));
}

static void append_metadata_line(std::string& out,const char* key, const std::string& value)
{
	if (value.empty())
		return;

	out += key;
	out += ": ";
	out += value;
	out += "\n";
}

std::string GifSessionWriter::build_comment_text() const
{
	std::string text;
	append_metadata_line(text, "Software", sessionOptions_.text.software);
	append_metadata_line(text, "Copyright", sessionOptions_.text.copyright);
	append_metadata_line(text, "Author", sessionOptions_.text.author);
	append_metadata_line(text, "Description", sessionOptions_.text.description);
	append_metadata_line(text, "Comment", sessionOptions_.text.comment);

	if (!text.empty() && text.back() == '\n')
		text.pop_back();

	return text;
}

bool GifSessionWriter::write_comment_extensions()
{
	const std::string text = build_comment_text();
	if (text.empty())
		return true;

	const uint8_t* p = reinterpret_cast<const uint8_t*>(text.data());
	size_t remaining = text.size();

	while (remaining > 0)
	{
		const int chunkSize = static_cast<int>(remaining > 255 ? 255 : remaining);

		if (EGifPutExtensionLeader(gif_, COMMENT_EXT_FUNC_CODE) == GIF_ERROR)
			return false;

		if (EGifPutExtensionBlock(gif_,
			chunkSize,
			const_cast<uint8_t*>(p)) == GIF_ERROR)
		{
			return false;
		}

		if (EGifPutExtensionTrailer(gif_) == GIF_ERROR)
			return false;

		p += chunkSize;
		remaining -= static_cast<size_t>(chunkSize);
	}

	return true;
}

bool GifSessionWriter::ValidatePage(const PreparedGifDibPage& page)
{
	return page.width > 0 &&
		page.height > 0 &&
		page.bitsPerPixel == 8 &&
		page.bits != nullptr &&
		page.strideBytes > 0;
}

void GifSessionWriter::prepare_color_map()
{
	if (colorMap_)
	{
		GifFreeMapObject(colorMap_);
		colorMap_ = nullptr;
	}

	colorMap_ = GifMakeMapObject(256, nullptr);
	if (!colorMap_)
		return;

	if (currentPage_.pixelFlavor == GifPixelFlavor::Indexed8 &&
		currentPage_.palette &&
		currentPage_.paletteEntries > 0)
	{
		const uint32_t n = currentPage_.paletteEntries > 256 ? 256 : currentPage_.paletteEntries;

		for (uint32_t i = 0; i < n; ++i)
		{
			colorMap_->Colors[i].Red = currentPage_.palette[i].rgbRed;
			colorMap_->Colors[i].Green = currentPage_.palette[i].rgbGreen;
			colorMap_->Colors[i].Blue = currentPage_.palette[i].rgbBlue;
		}

		for (uint32_t i = n; i < 256; ++i)
		{
			colorMap_->Colors[i].Red = 0;
			colorMap_->Colors[i].Green = 0;
			colorMap_->Colors[i].Blue = 0;
		}
	}
	else
	{
		for (int i = 0; i < 256; ++i)
		{
			colorMap_->Colors[i].Red = static_cast<GifByteType>(i);
			colorMap_->Colors[i].Green = static_cast<GifByteType>(i);
			colorMap_->Colors[i].Blue = static_cast<GifByteType>(i);
		}
	}
}

bool DTWAINGifOutput::OnFirstPage(const std::wstring& filename, const GifSessionOptions& sessionOptions, const PreparedGifDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<GifSessionWriter>();
	if (!writer_->Open(filename, sessionOptions))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINGifOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINGifOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}
