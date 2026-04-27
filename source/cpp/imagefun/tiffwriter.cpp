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
 #ifdef _MSC_VER
#pragma warning (disable : 4786)
#endif
#include <tiffio.h>
#include <windows.h>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "tiffwriter.h"
#include "dtwaindefs.h"
#include "dibutil.h"

// ============================================================
// Internal helpers
// ============================================================
static uint16_t to_libtiff_compression(TiffCompression c)
{
    switch (c)
    {
        case TiffCompression::None:     return COMPRESSION_NONE;
        case TiffCompression::Group3:   return COMPRESSION_CCITTFAX3;
        case TiffCompression::Group4:   return COMPRESSION_CCITTFAX4;
        case TiffCompression::Lzw:      return COMPRESSION_LZW;
        case TiffCompression::Flate:    return COMPRESSION_ADOBE_DEFLATE;
        case TiffCompression::PackBits: return COMPRESSION_PACKBITS;
        case TiffCompression::Jpeg:     return COMPRESSION_JPEG;
    }
    return COMPRESSION_NONE;
}

static std::string build_tiff_open_mode(const TiffSessionOptions& opt)
{
	std::string mode = "w";

	// ClassicTIFF vs BigTIFF
	mode += (opt.containerFormat == TiffContainerFormat::BigTiff) ? '8' : '4';

	// Optional endianness
	if (opt.littleEndian)
		mode += 'l';
	else if (opt.bigEndian)
		mode += 'b';

	// Fill-order mode flag
	mode += opt.fillOrderLsbToMsb ? 'L' : 'B';

	return mode;
}

static PageTagInfo describe_page_tags(const PreparedDibPage& page, const TiffPageSettings& settings)
{
    PageTagInfo info{};

    switch (page.pixelFlavor)
    {
        case PixelFlavor::BW1:
            info.samplesPerPixel = 1;
            info.bitsPerSample = 1;
			info.photometric = PHOTOMETRIC_MINISWHITE;
            info.writeColorMap = false;
            break;

        case PixelFlavor::Gray8:
            info.samplesPerPixel = 1;
            info.bitsPerSample = 8;
            info.photometric = PHOTOMETRIC_MINISBLACK;
            info.writeColorMap = false;
            break;

        case PixelFlavor::Palette8:
            info.samplesPerPixel = 1;
            info.bitsPerSample = 8;
            info.photometric = PHOTOMETRIC_PALETTE;
            info.writeColorMap = true;
            break;

        case PixelFlavor::Gray16:
            info.samplesPerPixel = 1;
            info.bitsPerSample = 16;
            info.photometric = PHOTOMETRIC_MINISBLACK;
            info.writeColorMap = false;
            break;

        case PixelFlavor::Bgr24:
        case PixelFlavor::Bgra32:
            info.samplesPerPixel = 3;
            info.bitsPerSample = 8;
            info.photometric = PHOTOMETRIC_RGB;
            info.writeColorMap = false;
            break;
    }

    return info;
}

static size_t calc_output_row_size(const PreparedDibPage& page, const PageTagInfo& tagInfo)
{
	if (tagInfo.bitsPerSample == 1 && tagInfo.samplesPerPixel == 1)
		return static_cast<size_t>((page.width + 7) / 8);

	return static_cast<size_t>(page.width) *
		static_cast<size_t>(tagInfo.samplesPerPixel) *
		static_cast<size_t>(tagInfo.bitsPerSample / 8);
}

static bool convert_row(const PreparedDibPage& page,
                        const TiffPageSettings& settings,
                        const uint8_t* src,
                        uint8_t* dst,
                        size_t dstSize)
{
    switch (page.pixelFlavor)
    {
        case PixelFlavor::BW1:
        {
            const size_t n = static_cast<size_t>((page.width + 7) / 8);
            if (dstSize < n)
                return false;

            std::memcpy(dst, src, n);

            if (settings.invertBilevelBits)
            {
                for (size_t i = 0; i < n; ++i)
                    dst[i] = static_cast<uint8_t>(~dst[i]);
            }
            return true;
        }

        case PixelFlavor::Gray8:
        case PixelFlavor::Palette8:
        {
            const size_t n = static_cast<size_t>(page.width);
            if (dstSize < n)
                return false;

            std::memcpy(dst, src, n);
            return true;
        }

        case PixelFlavor::Gray16:
        {
            const size_t n = static_cast<size_t>(page.width) * 2;
            if (dstSize < n)
                return false;

            std::memcpy(dst, src, n);
            return true;
        }

        case PixelFlavor::Bgr24:
        {
            const size_t n = static_cast<size_t>(page.width) * 3;
            if (dstSize < n)
                return false;

            for (uint32_t x = 0; x < page.width; ++x)
            {
                const uint8_t b = src[x * 3 + 0];
                const uint8_t g = src[x * 3 + 1];
                const uint8_t r = src[x * 3 + 2];

                dst[x * 3 + 0] = r;
                dst[x * 3 + 1] = g;
                dst[x * 3 + 2] = b;
            }
            return true;
        }

        case PixelFlavor::Bgra32:
        {
            const size_t n = static_cast<size_t>(page.width) * 3;
            if (dstSize < n)
                return false;

            for (uint32_t x = 0; x < page.width; ++x)
            {
                const uint8_t b = src[x * 4 + 0];
                const uint8_t g = src[x * 4 + 1];
                const uint8_t r = src[x * 4 + 2];
                // alpha ignored

                dst[x * 3 + 0] = r;
                dst[x * 3 + 1] = g;
                dst[x * 3 + 2] = b;
            }
            return true;
        }
    }
    return false;
}

// ============================================================
// TiffSessionWriter
// ============================================================

TiffSessionWriter::~TiffSessionWriter()
{
	Close();
}

TiffSessionWriter::TiffSessionWriter(TiffSessionWriter&& other) noexcept
		: tif_(other.tif_),
		filename_(std::move(other.filename_)),
		sessionOptions_(std::move(other.sessionOptions_)),
		pageIndex_(other.pageIndex_),
		currentPage_(other.currentPage_),
		currentPageSettings_(other.currentPageSettings_),
		hasCurrentPage_(other.hasCurrentPage_)
{
	other.tif_ = nullptr;
	other.pageIndex_ = 0;
	other.hasCurrentPage_ = false;
}

TiffSessionWriter& TiffSessionWriter::operator=(TiffSessionWriter&& other) noexcept
{
	if (this != &other)
	{
		Close();

		tif_ = other.tif_;
		filename_ = std::move(other.filename_);
		sessionOptions_ = std::move(other.sessionOptions_);
		pageIndex_ = other.pageIndex_;
		currentPage_ = other.currentPage_;
		currentPageSettings_ = other.currentPageSettings_;
		hasCurrentPage_ = other.hasCurrentPage_;

		other.tif_ = nullptr;
		other.pageIndex_ = 0;
		other.hasCurrentPage_ = false;
	}
	return *this;
}

bool TiffSessionWriter::Open(const std::wstring& filename, const TiffSessionOptions& sessionOptions)
{
	if (tif_)
		return false;

	const std::string mode = build_tiff_open_mode(sessionOptions);
	tif_ = TIFFOpenW(filename.c_str(), mode.c_str());
	if (!tif_)
		return false;

	filename_ = filename;
	sessionOptions_ = sessionOptions;
	pageIndex_ = 0;
	hasCurrentPage_ = false;
	return true;
}

bool TiffSessionWriter::SetPageInfo(const PreparedDibPage& page, const TiffPageSettings& pageSettings)
{
	if (!tif_)
		return false;

	currentPage_ = page;
	currentPageSettings_ = pageSettings;
	currentPageSettings_.pageIndex = static_cast<uint16_t>(pageIndex_);

	if (currentPage_.pixelFlavor == PixelFlavor::BW1)
	{
		currentPageSettings_.invertBilevelBits = currentPageSettings_.invertImage;
	}

	hasCurrentPage_ = true;
	return true;
}

bool TiffSessionWriter::WriteCurrentPage()
{
	if (!tif_ || !hasCurrentPage_)
		return false;

	if (!ValidateCurrentPage())
		return false;

	const PageTagInfo tagInfo = describe_page_tags(currentPage_, currentPageSettings_);

	if (!SetCommonTags(tagInfo))
		return false;

	if (!SetCompressionTags(tagInfo))
		return false;

	if (tagInfo.writeColorMap)
	{
		if (!SetPaletteTags(tagInfo.bitsPerSample))
			return false;
	}

	if (!WritePixels(tagInfo))
		return false;

	if (TIFFWriteDirectory(tif_) != 1)
		return false;

	++pageIndex_;
	hasCurrentPage_ = false;
	return true;
}

void TiffSessionWriter::Close()
{
	if (tif_)
	{
		TIFFClose(tif_);
		tif_ = nullptr;
	}

	filename_.clear();
	pageIndex_ = 0;
	hasCurrentPage_ = false;
}

bool TiffSessionWriter::IsOpen() const noexcept
{
	return tif_ != nullptr;
}

std::size_t TiffSessionWriter::GetPageIndex() const noexcept
{
	return pageIndex_;
}

bool TiffSessionWriter::ValidateCurrentPage() const
{
	if (!currentPage_.bits)
		return false;
	if (currentPage_.width == 0 || currentPage_.height == 0)
		return false;
	if (currentPage_.strideBytes == 0)
		return false;

	switch (currentPage_.pixelFlavor)
	{
	case PixelFlavor::BW1:
		return currentPage_.bitsPerPixel == 1;

	case PixelFlavor::Gray8:
		return currentPage_.bitsPerPixel == 8;

	case PixelFlavor::Palette8:
		return currentPage_.bitsPerPixel == 8 &&
			currentPage_.palette != nullptr &&
			currentPage_.paletteEntries > 0;

	case PixelFlavor::Gray16:
		return currentPage_.bitsPerPixel == 16;

	case PixelFlavor::Bgr24:
		return currentPage_.bitsPerPixel == 24;

	case PixelFlavor::Bgra32:
		return currentPage_.bitsPerPixel == 32;
	}

	return false;
}

bool TiffSessionWriter::SetCommonTags(const PageTagInfo& tagInfo)
{
	uint16_t photometric = tagInfo.photometric;

	TIFFSetField(tif_, TIFFTAG_IMAGEWIDTH, currentPage_.width);
	TIFFSetField(tif_, TIFFTAG_IMAGELENGTH, currentPage_.height);
	TIFFSetField(tif_, TIFFTAG_SAMPLESPERPIXEL, tagInfo.samplesPerPixel);
	TIFFSetField(tif_, TIFFTAG_BITSPERSAMPLE, tagInfo.bitsPerSample);
	TIFFSetField(tif_, TIFFTAG_PHOTOMETRIC, photometric);
	TIFFSetField(tif_, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif_, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);

	TIFFSetField(tif_, TIFFTAG_XRESOLUTION, static_cast<float>(currentPage_.xDpi));
	TIFFSetField(tif_, TIFFTAG_YRESOLUTION, static_cast<float>(currentPage_.yDpi));
	TIFFSetField(tif_, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);

	// JPEG overrides this later.
	TIFFSetField(tif_, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif_, 0));

	if (!sessionOptions_.software.empty())
		TIFFSetField(tif_, TIFFTAG_SOFTWARE, sessionOptions_.software.c_str());

	if (!sessionOptions_.copyright.empty())
		TIFFSetField(tif_, TIFFTAG_COPYRIGHT, sessionOptions_.copyright.c_str());

	if (currentPageSettings_.setPageNumber)
	{
		TIFFSetField(tif_, TIFFTAG_PAGENUMBER,
			currentPageSettings_.pageIndex,
			currentPageSettings_.pageCount);
	}

	if (currentPage_.pixelFlavor == PixelFlavor::BW1)
	{
		uint16_t fillOrder =
			sessionOptions_.fillOrderLsbToMsb ? FILLORDER_LSB2MSB
			: FILLORDER_MSB2LSB;

		if (currentPageSettings_.forceFillOrder)
			fillOrder = currentPageSettings_.forcedFillOrder;

		TIFFSetField(tif_, TIFFTAG_FILLORDER, fillOrder);
	}

	return true;
}

bool TiffSessionWriter::SetCompressionTags(const PageTagInfo& tagInfo)
{
	TIFFSetField(tif_, TIFFTAG_COMPRESSION,
		to_libtiff_compression(currentPageSettings_.compression));

	switch (currentPageSettings_.compression)
	{
	case TiffCompression::Group3:
		TIFFSetField(tif_, TIFFTAG_GROUP3OPTIONS, currentPageSettings_.group3Options);
		break;

	case TiffCompression::Group4:
		TIFFSetField(tif_, TIFFTAG_GROUP4OPTIONS, currentPageSettings_.group4Options);
		break;

	case TiffCompression::Lzw:
	{
		if (tagInfo.bitsPerSample == 1)
		{
			TIFFSetField(tif_, TIFFTAG_PREDICTOR, 1);
		}
		else if (!tagInfo.writeColorMap &&
			(tagInfo.photometric == PHOTOMETRIC_MINISBLACK ||
				tagInfo.photometric == PHOTOMETRIC_RGB) &&
			tagInfo.bitsPerSample >= 8)
		{
			TIFFSetField(tif_, TIFFTAG_PREDICTOR, 2);
		}
		else
		{
			TIFFSetField(tif_, TIFFTAG_PREDICTOR, 1);
		}
		break;
	}

	case TiffCompression::Flate:
	{
		if (!tagInfo.writeColorMap &&
			(tagInfo.photometric == PHOTOMETRIC_MINISBLACK ||
				tagInfo.photometric == PHOTOMETRIC_RGB) &&
			tagInfo.bitsPerSample >= 8)
		{
			TIFFSetField(tif_, TIFFTAG_PREDICTOR, 2);
		}
		else
		{
			TIFFSetField(tif_, TIFFTAG_PREDICTOR, 1);
		}
		break;
	}

	case TiffCompression::Jpeg:
	{
		TIFFSetField(tif_, TIFFTAG_JPEGQUALITY, currentPageSettings_.jpegQuality);

		uint32_t rowsperstrip = static_cast<uint32_t>(-1);
		rowsperstrip = TIFFDefaultStripSize(tif_, rowsperstrip);

		const uint32_t rem = rowsperstrip % 8;
		if (rem != 0)
			rowsperstrip += (8 - rem);

		TIFFSetField(tif_, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
		break;
	}

	case TiffCompression::None:
	case TiffCompression::PackBits:
		break;
	}

	return true;
}

bool TiffSessionWriter::SetPaletteTags(uint16_t bitsPerSample)
{
	if (!currentPage_.palette || currentPage_.paletteEntries == 0)
		return false;

	const uint32_t mapEntries = 1u << bitsPerSample;
	std::vector<uint16_t> red(mapEntries, 0);
	std::vector<uint16_t> green(mapEntries, 0);
	std::vector<uint16_t> blue(mapEntries, 0);

	const uint32_t n = (currentPage_.paletteEntries < mapEntries)
		? currentPage_.paletteEntries
		: mapEntries;

	for (uint32_t i = 0; i < n; ++i)
	{
		red[i] = static_cast<uint16_t>(currentPage_.palette[i].rgbRed) * 257u;
		green[i] = static_cast<uint16_t>(currentPage_.palette[i].rgbGreen) * 257u;
		blue[i] = static_cast<uint16_t>(currentPage_.palette[i].rgbBlue) * 257u;
	}

	TIFFSetField(tif_, TIFFTAG_COLORMAP, red.data(), green.data(), blue.data());
	return true;
}

bool TiffSessionWriter::EnsureRowBufferSize(size_t sizeNeeded)
{
	if (rowBuffer_.size() < sizeNeeded)
		rowBuffer_.resize(sizeNeeded);
	return true;
}

bool TiffSessionWriter::WritePixels(const PageTagInfo& tagInfo)
{
	const size_t outRowSize = calc_output_row_size(currentPage_, tagInfo);

	if (!EnsureRowBufferSize(outRowSize))
		return false;

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		if (!convert_row(currentPage_, currentPageSettings_, src, rowBuffer_.data(), outRowSize))
		{
			return false;
		}

		if (TIFFWriteScanline(tif_, rowBuffer_.data(), y, 0) < 0)
			return false;
	}

	return true;
}

std::optional<PreparedDibPage> TiffSessionWriter::MakePreparedDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedDibPage page{};
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;
	page.palette = view.palette;
	page.paletteEntries = view.paletteEntries;

	page.xDpi = view.xDPI > 0.0 ? view.xDPI : 200.0;
	page.yDpi = view.yDPI > 0.0 ? view.yDPI : 200.0;

	switch (page.bitsPerPixel)
	{
		case 1:  page.pixelFlavor = PixelFlavor::BW1; break;
		case 8:  page.pixelFlavor = page.palette ? PixelFlavor::Palette8 : PixelFlavor::Gray8; break;
		case 16: page.pixelFlavor = PixelFlavor::Gray16; break;
		case 24: page.pixelFlavor = PixelFlavor::Bgr24; break;
		case 32: page.pixelFlavor = PixelFlavor::Bgra32; break;
		default: return std::nullopt;
	}
	return page;
}

std::pair<bool, int> DTWAINTiffOutput::OnFirstPage(const std::wstring& filename, const TiffSessionOptions& sessionOptions, const PreparedDibPage& page,
		TiffPageSettings settings)
{
	if (writer_)
		return { false, DTWAIN_ERR_FILEWRITE };

	writer_ = std::make_unique<TiffSessionWriter>();
	if (!writer_->Open(filename, sessionOptions))
	{
		writer_.reset();
		return { false, DTWAIN_ERR_FILEOPEN };
	}

	pageIndex_ = 0;
	return write_page(page, settings);
}

std::pair<bool, int> DTWAINTiffOutput::OnNextPage(const PreparedDibPage& page, TiffPageSettings settings)
{
	if (!writer_)
		return { false, DTWAIN_ERR_FILEWRITE };

	return write_page(page, settings);
}

std::pair<bool, int> DTWAINTiffOutput::OnLastPage()
{
	if (!writer_)
		return { false, DTWAIN_ERR_FILEWRITE };

	writer_.reset(); // closes TIFF
	pageIndex_ = 0;
	return { true, DTWAIN_NO_ERROR };
}

bool DTWAINTiffOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}

std::pair<bool, int> DTWAINTiffOutput::write_page(const PreparedDibPage& page, TiffPageSettings settings)
{
	settings.pageIndex = static_cast<uint16_t>(pageIndex_);

	if (!writer_->SetPageInfo(page, settings))
		return { false, DTWAIN_ERR_FILEWRITE };

	if (!writer_->WriteCurrentPage())
		return { false, DTWAIN_ERR_FILEWRITE };

	++pageIndex_;
	return { true, DTWAIN_NO_ERROR };
}

