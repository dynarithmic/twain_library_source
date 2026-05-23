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
#ifndef POSTSCRIPTWRITER_H
#define POSTSCRIPTWRITER_H

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// PostScript writer
// ============================================================

enum class PsPixelFlavor
{
	BW1,
	Gray8,
	Gray16,
	Bgr24,
	Bgra32
};

enum class PsLevel
{
	Level1,
	Level2,
	Level3
};

struct PreparedPsDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	PsPixelFlavor pixelFlavor = PsPixelFlavor::Bgr24;
	const uint8_t* bits = nullptr;
	const RGBQUAD* palette = nullptr;
	uint32_t paletteEntries = 0;

	double xDpi = 300.0;
	double yDpi = 300.0;
};

struct PsSessionOptions
{
	PsLevel level = PsLevel::Level2;

	// Page size in PostScript points. 8.5 x 11 by default.
	double pageWidthPts = 612.0;
	double pageHeightPts = 792.0;

	// Margin in points.
	double marginLeftPts = 36.0;
	double marginBottomPts = 36.0;
	double marginRightPts = 36.0;
	double marginTopPts = 36.0;

	bool preserveAspectRatio = true;
	bool centerImage = true;

	// For 1-bpp:
	// false usually means DIB 0=black, 1=white becomes normal paper output.
	// true reverses the Decode array.
	bool invert1bpp = false;
	bool useRunLength = true;

	std::string creator;
};

class PsSessionWriter
{
	public:
		PsSessionWriter() = default;
		~PsSessionWriter();
		PsSessionWriter(const PsSessionWriter&) = delete;
		PsSessionWriter& operator=(const PsSessionWriter&) = delete;

		bool Open(const std::wstring& filename, const PsSessionOptions& options);
		bool WritePage(const PreparedPsDibPage& page);
		bool Close();
		static std::optional<PreparedPsDibPage> MakePreparedPsDibPage(const dynarithmic::DibPageView& view);

	private:
		static bool ValidatePage(const PreparedPsDibPage& page);
		bool WriteDocumentHeader();
		bool WritePageHeader(const PreparedPsDibPage&);
		bool WritePageSetup(const PreparedPsDibPage& page);
		const char* ColorSpaceName(const PreparedPsDibPage& page) const;
		uint32_t Components(const PreparedPsDibPage& page) const;
		uint32_t BitsPerComponent(const PreparedPsDibPage& page) const;
		const char* DecodeArray(const PreparedPsDibPage& page) const;
		bool WriteLevel1Image(const PreparedPsDibPage& page);
		bool WriteLevel2Or3Image(const PreparedPsDibPage& page);
		const uint8_t* SourceRow(const PreparedPsDibPage& page, uint32_t y) const;
		bool WriteHexByte(uint8_t v);
		bool EndHexData();
		bool WriteHexImageData(const PreparedPsDibPage& page);
		bool Write1bppHex(const PreparedPsDibPage& page);
		bool Write8bppGrayHex(const PreparedPsDibPage& page);
		bool Write16bppGrayHex(const PreparedPsDibPage& page);
		bool Write24bppRgbHex(const PreparedPsDibPage& page);
		bool Write32bppRgbHex(const PreparedPsDibPage& page);

	private:
		bool BuildRawImageData(const PreparedPsDibPage& page, std::string& raw);
		bool WriteAscii85ImageData(const PreparedPsDibPage& page);
		bool WriteAscii85RunLengthImageData(const PreparedPsDibPage& page);
		bool WriteAscii85FlateImageData(const PreparedPsDibPage& page);

		FILE* file_ = nullptr;
		PsSessionOptions options_{};
		bool opened_ = false;
		uint32_t pageCount_ = 0;
		uint32_t hexLineCount_ = 0;
		std::vector<uint8_t> rowBuffer_;
};

// ============================================================
// DTWAIN-style wrapper
// ============================================================

class DTWAINPsOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const PsSessionOptions& options, const PreparedPsDibPage& page);
		bool OnNextPage(const PreparedPsDibPage& page);
		bool OnLastPage();

	private:
		std::unique_ptr<PsSessionWriter> writer_;
};

#endif
