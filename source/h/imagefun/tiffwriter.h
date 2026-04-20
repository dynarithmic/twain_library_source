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
#ifndef TIFFWRITER_H
#define TIFFWRITER_H

#include <string>
#include <memory>
#include <vector>
#include "tiffio.h"

enum class TiffContainerFormat
{
	ClassicTiff,
	BigTiff
};

enum class TiffCompression
{
	None,
	Group3,
	Group4,
	Lzw,
	Flate,
	PackBits,
	Jpeg
};

enum class PixelFlavor
{
	BW1,        // 1-bpp bilevel
	Gray8,      // 8-bpp grayscale
	Palette8,   // 8-bpp indexed/paletted
	Gray16,     // 16-bpp grayscale
	Bgr24,      // 24-bpp Windows DIB
	Bgra32      // 32-bpp Windows DIB, alpha ignored
};


struct TiffSessionOptions
{
	TiffContainerFormat containerFormat = TiffContainerFormat::ClassicTiff;

	// Optional TIFF open mode flags
	bool littleEndian = false;
	bool bigEndian = false;
	bool fillOrderLsbToMsb = false;

	// Metadata
	std::string software = "DTWAIN";
	std::string copyright;
};

struct TiffPageSettings
{
	TiffCompression compression = TiffCompression::None;

	// CCITT fax options
	uint32_t group3Options = 0;
	uint32_t group4Options = 0;

	// JPEG options
	int jpegQuality = 75;

	// LZW / Flate
	bool usePredictor = true;

	// Page numbering
	bool setPageNumber = true;
	uint16_t pageIndex = 0;
	uint16_t pageCount = 0;

	// 1-bpp TIFF meaning requested by the caller:
	// PHOTOMETRIC_MINISWHITE => 0 = white, 1 = black
	// PHOTOMETRIC_MINISBLACK => 0 = black, 1 = white
	uint16_t bilevelPhotometric = PHOTOMETRIC_MINISWHITE;

	// Low-level packed-bit inversion. This is derived automatically in
	// SetPageInfo() for BW1 based on the assumed source convention:
	// source BW1 DIB uses 0 = black, 1 = white.
	bool invertBilevelBits = false;

	// Optional per-page fill-order override
	bool forceFillOrder = false;
	uint16_t forcedFillOrder = FILLORDER_MSB2LSB;

	// Setting specific to DTWAIN for TIFF image inversion
	bool invertImage = false;
};

struct PreparedDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	PixelFlavor pixelFlavor = PixelFlavor::Bgr24;

	const uint8_t* bits = nullptr;

	// For Palette8 only
	const RGBQUAD* palette = nullptr;
	uint32_t paletteEntries = 0;

	// Resolution
	double xDpi = 200.0;
	double yDpi = 200.0;
};

struct PageTagInfo
{
	uint16_t samplesPerPixel = 1;
	uint16_t bitsPerSample = 1;
	uint16_t photometric = PHOTOMETRIC_MINISWHITE;
	bool writeColorMap = false;
};

// ============================================================
// RAII view for HANDLE-based DIBs
//
// The DIB remains GlobalLocked() for the lifetime of this object.
// That ensures PreparedDibPage pointers remain valid through
// SetPageInfo() + WriteCurrentPage().
// ============================================================
class LockedDibPage
{
	public:
		explicit LockedDibPage(HANDLE hDib);
		~LockedDibPage();
		LockedDibPage(const LockedDibPage&) = delete;
		LockedDibPage& operator=(const LockedDibPage&) = delete;
		LockedDibPage(LockedDibPage&& other) noexcept;
		LockedDibPage& operator=(LockedDibPage&& other) noexcept;
		bool IsValid() const noexcept;
		const PreparedDibPage& GetPage() const noexcept;
		PreparedDibPage& GetPageRef() { return page_; }

	private:
		HANDLE hDib_ = nullptr;
		void* locked_ = nullptr;
		PreparedDibPage page_{};
		bool valid_ = false;
};

class TiffSessionWriter
{
	public:
		TiffSessionWriter() = default;
		~TiffSessionWriter();
		TiffSessionWriter(const TiffSessionWriter&) = delete;
		TiffSessionWriter& operator=(const TiffSessionWriter&) = delete;
		TiffSessionWriter(TiffSessionWriter&& other) noexcept;
		TiffSessionWriter& operator=(TiffSessionWriter&& other) noexcept;
		bool Open(const std::wstring& filename, const TiffSessionOptions& sessionOptions);
		bool SetPageInfo(const PreparedDibPage& page, const TiffPageSettings& pageSettings);
		bool WriteCurrentPage();
		void Close();
		bool IsOpen() const noexcept;
		std::size_t GetPageIndex() const noexcept;

	private:
		bool ValidateCurrentPage() const;
		bool SetCommonTags(const PageTagInfo& tagInfo);
		bool SetCompressionTags(const PageTagInfo& tagInfo);
		bool SetPaletteTags(uint16_t bitsPerSample);
		bool WritePixels(const PageTagInfo& tagInfo);
		bool EnsureRowBufferSize(size_t sizeNeeded);

	private:
		TIFF* tif_ = nullptr;
		std::wstring filename_;
		TiffSessionOptions sessionOptions_{};
		std::size_t pageIndex_ = 0;

		PreparedDibPage currentPage_{};
		TiffPageSettings currentPageSettings_{};
		bool hasCurrentPage_ = false;
		std::vector<uint8_t> rowBuffer_;
};

class DTWAINTiffOutput
{
	public:
		std::pair<bool, int> OnFirstPage(const std::wstring& filename, const TiffSessionOptions& sessionOptions, const PreparedDibPage& page,
										 TiffPageSettings settings);
		std::pair<bool, int> OnNextPage(const PreparedDibPage& page, TiffPageSettings settings);
		std::pair<bool, int> OnLastPage();
		bool IsOpen() const noexcept;
	private:
		std::pair<bool, int> write_page(const PreparedDibPage& page, TiffPageSettings settings);

	private:
		std::unique_ptr<TiffSessionWriter> writer_;
		std::size_t pageIndex_ = 0;
};

#endif
