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
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "pngwriter.h"
#include "dtwaindefs.h"

#ifdef _MSC_VER
	#pragma warning (disable : 4786)
	#pragma warning (disable : 4611)
#endif

std::optional<PreparedPngDibPage> PngSessionWriter::MakePreparedPngDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedPngDibPage page{};

	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;

	switch (page.bitsPerPixel)
	{
		case 8:
			if (page.palette && page.paletteEntries > 0 &&
				!dynarithmic::dib::is_identity_grayscale_palette(page.palette, page.paletteEntries))
				page.pixelFlavor = PngPixelFlavor::Palette8;
			else
				page.pixelFlavor = PngPixelFlavor::Gray8;
			break;

		case 16:
			page.pixelFlavor = PngPixelFlavor::Gray16;
			break;

		case 24:
			page.pixelFlavor = PngPixelFlavor::Bgr24;
			break;

		default:
			return std::nullopt;
	}

	page.xDpi = view.xDPI > 0.0 ? view.xDPI : 96.0;
	page.yDpi = view.yDPI > 0.0 ? view.yDPI : 96.0;

	return page;
}

PngSessionWriter::~PngSessionWriter()
{
	Close();
}

bool PngSessionWriter::Open(const std::wstring& filename, const PngSessionOptions& sessionOptions)
{
	if (isOpen_)
		return false;

	file_ = _wfopen(filename.c_str(), L"wb");
	if (!file_)
		return false;

	png_ptr_ = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
	if (!png_ptr_)
	{
		Close();
		return false;
	}

	info_ptr_ = png_create_info_struct(png_ptr_);
	if (!info_ptr_)
	{
		Close();
		return false;
	}

	png_init_io(png_ptr_, file_);
	png_set_compression_level(png_ptr_, sessionOptions.compressionLevel);

	filename_ = filename;
	sessionOptions_ = sessionOptions;
	isOpen_ = true;
	hasCurrentPage_ = false;
	return true;
}

bool PngSessionWriter::SetPageInfo(const PreparedPngDibPage& page)
{
	if (!isOpen_)
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

std::pair<bool, int> PngSessionWriter::WriteCurrentPage()
{
	if (!isOpen_ || !hasCurrentPage_)
		return { false, DTWAIN_ERR_FILEWRITE };

	if (!ValidateCurrentPage())
		return { false, DTWAIN_ERR_FILEWRITE };

	prepare_row_pointers();
	prepare_text_chunks();
	prepare_palette();

	if (!write_current_page_guarded())
		return { false, DTWAIN_ERR_FILEWRITE };

	hasCurrentPage_ = false;
	return { true, DTWAIN_NO_ERROR };
}


static void push_text_chunk(std::vector<png_text>& chunks,const char* key,const std::string& value)
{
	if (value.empty())
		return;

	png_text t{};
	t.compression = PNG_TEXT_COMPRESSION_NONE;
	t.key = const_cast<png_charp>(key);
	t.text = const_cast<png_charp>(value.c_str());
	t.text_length = value.size();
	chunks.push_back(t);
}

void PngSessionWriter::prepare_text_chunks()
{
	textChunks_.clear();
	textChunks_.reserve(5);

	push_text_chunk(textChunks_, "Software", sessionOptions_.text.software);
	push_text_chunk(textChunks_, "Copyright", sessionOptions_.text.copyright);
	push_text_chunk(textChunks_, "Author", sessionOptions_.text.author);
	push_text_chunk(textChunks_, "Description", sessionOptions_.text.description);
	push_text_chunk(textChunks_, "Comment", sessionOptions_.text.comment);
}

void PngSessionWriter::prepare_palette()
{
	pngPalette_.clear();

	if (currentPage_.pixelFlavor != PngPixelFlavor::Palette8 ||
		!currentPage_.palette ||
		currentPage_.paletteEntries == 0)
	{
		return;
	}

	const uint32_t n = currentPage_.paletteEntries > 256 ? 256 : currentPage_.paletteEntries;
	pngPalette_.resize(n);

	for (uint32_t i = 0; i < n; ++i)
	{
		pngPalette_[i].red = currentPage_.palette[i].rgbRed;
		pngPalette_[i].green = currentPage_.palette[i].rgbGreen;
		pngPalette_[i].blue = currentPage_.palette[i].rgbBlue;
	}
}

void PngSessionWriter::Close()
{
	if (png_ptr_)
		png_destroy_write_struct(&png_ptr_, &info_ptr_);

	png_ptr_ = nullptr;
	info_ptr_ = nullptr;

	if (file_)
	{
		std::fclose(file_);
		file_ = nullptr;
	}

	rowPointers_.clear();
	textChunks_.clear();
	pngPalette_.clear();

	filename_.clear();
	isOpen_ = false;
	hasCurrentPage_ = false;
}

bool PngSessionWriter::IsOpen() const noexcept
{
	return isOpen_;
}

bool PngSessionWriter::ValidateCurrentPage() const
{
	if (!currentPage_.bits)
		return false;
	if (currentPage_.width == 0 || currentPage_.height == 0)
		return false;
	if (currentPage_.strideBytes == 0)
		return false;

	switch (currentPage_.pixelFlavor)
	{
		case PngPixelFlavor::Gray8:
			return currentPage_.bitsPerPixel == 8;

		case PngPixelFlavor::Palette8:
			return currentPage_.bitsPerPixel == 8 &&
				currentPage_.palette != nullptr &&
				currentPage_.paletteEntries > 0;

		case PngPixelFlavor::Gray16:
			return currentPage_.bitsPerPixel == 16;

		case PngPixelFlavor::Bgr24:
			return currentPage_.bitsPerPixel == 24;
	}
	return false;
}

void PngSessionWriter::prepare_row_pointers()
{
	rowPointers_.assign(currentPage_.height, nullptr);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		rowPointers_[y] = const_cast<png_bytep>(
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes);
	}
}

bool PngSessionWriter::write_current_page_guarded()
{
	if (!png_ptr_ || !info_ptr_)
		return false;

	if (setjmp(png_jmpbuf(png_ptr_)) != 0)
		return false;

	// From here down: no local STL containers / no important RAII objects.
	write_current_page_impl_noexcept();
	return true;
}

void PngSessionWriter::write_current_page_impl_noexcept()
{
	int colorType = PNG_COLOR_TYPE_GRAY;
	int bitDepth = 8;

	switch (currentPage_.pixelFlavor)
	{
		case PngPixelFlavor::Gray8:
			colorType = PNG_COLOR_TYPE_GRAY;
			bitDepth = 8;
			break;

		case PngPixelFlavor::Gray16:
			colorType = PNG_COLOR_TYPE_GRAY;
			bitDepth = 16;
			break;

		case PngPixelFlavor::Bgr24:
			colorType = PNG_COLOR_TYPE_RGB;
			bitDepth = 8;
			break;
	}

	png_set_IHDR(png_ptr_,
		info_ptr_,
		currentPage_.width,
		currentPage_.height,
		bitDepth,
		colorType,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE,
		PNG_FILTER_TYPE_BASE);

	const png_uint_32 xppm =
		static_cast<png_uint_32>(currentPage_.xDpi / 0.0254 + 0.5);
	const png_uint_32 yppm =
		static_cast<png_uint_32>(currentPage_.yDpi / 0.0254 + 0.5);

	png_set_pHYs(png_ptr_, info_ptr_, xppm, yppm, PNG_RESOLUTION_METER);

	if (!textChunks_.empty())
	{
		png_set_text(png_ptr_,
			info_ptr_,
			textChunks_.data(),
			static_cast<int>(textChunks_.size()));
	}

	// Windows 24-bpp DIB = BGR
	if (currentPage_.pixelFlavor == PngPixelFlavor::Bgr24)
		png_set_bgr(png_ptr_);

	// Windows 16-bit grayscale is little-endian in memory
	if (currentPage_.pixelFlavor == PngPixelFlavor::Gray16)
		png_set_swap(png_ptr_);

	png_write_info(png_ptr_, info_ptr_);
	png_write_image(png_ptr_, rowPointers_.data());
	png_write_end(png_ptr_, info_ptr_);
}

std::pair<bool, int> DTWAINPngOutput::OnFirstPage(const std::wstring& filename, const PngSessionOptions& sessionOptions,
												  const PreparedPngDibPage& page)
{
	if (writer_)
		return { false, DTWAIN_ERR_FILEWRITE };

	writer_ = std::make_unique<PngSessionWriter>();
	if (!writer_->Open(filename, sessionOptions))
	{
		writer_.reset();
		return { false, DTWAIN_ERR_FILEOPEN };
	}

	if (!writer_->SetPageInfo(page))
		return { false, DTWAIN_ERR_FILEWRITE };

	return writer_->WriteCurrentPage();
}

std::pair<bool, int> DTWAINPngOutput::OnLastPage()
{
	if (!writer_)
		return { false, DTWAIN_ERR_FILEWRITE };

	writer_.reset();
	return { true, DTWAIN_NO_ERROR };
}

bool DTWAINPngOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}
