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
#ifndef PNMWRITER_H
#define PNMWRITER_H

#include <memory>
#include <cstdio>
#include <cstdint>
#include "imagefilewriterbase.h"
#include "dibutil.h"

// ============================================================
// PNM model
// DTWAIN policy:
//   1  bpp -> PBM
//   8  bpp -> PGM
//   16 bpp -> PGM
//   24 bpp -> PPM
//   32 bpp -> PPM (alpha discarded)
// ============================================================

enum class PnmPixelFlavor
{
	BW1,
	Gray8,
	Gray16,
	Bgr24,
	Bgra32
};

struct PreparedPnmDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	PnmPixelFlavor pixelFlavor = PnmPixelFlavor::Gray8;
	const uint8_t* bits = nullptr;
};

struct PnmSessionOptions
{
	bool useRaw = true;
	bool fixBilevelPolarity = true;
	std::string comment;
};

// ============================================================
// PNM writer
// ============================================================

class PnmSessionWriter
{
	public:
		PnmSessionWriter() = default;
		~PnmSessionWriter();
		PnmSessionWriter(const PnmSessionWriter&) = delete;
		PnmSessionWriter& operator=(const PnmSessionWriter&) = delete;
		bool Open(const std::wstring& filename, const PnmSessionOptions& options);
		bool SetPageInfo(const PreparedPnmDibPage& page);
		bool WriteCurrentPage();
		void Close();
		bool IsOpen() const noexcept;
		static std::optional<PreparedPnmDibPage> MakePreparedPnmDibPage(const dynarithmic::DibPageView& view);

	private:
		static bool ValidatePage(const PreparedPnmDibPage& page);
		static uint8_t ReverseBits(uint8_t v);
		const char* Magic() const;
		uint32_t MaxValue() const;
		bool WriteHeader();
		bool WritePixels();
		const uint8_t* GetSourceRow(uint32_t y) const;
		bool WritePbmPlain();
		bool WritePbmRaw();
		bool WriteGray8Plain();
		bool WriteGray8Raw();
		bool WriteGray16Plain();
		bool WriteGray16Raw();
		bool WriteRgb24Plain();
		bool WriteRgb24Raw();
		bool WriteRgba32AsRgbPlain();
		bool WriteRgba32AsRgbRaw();
		bool WriteCommentLines(const std::string& text);

	private:
		FILE* file_ = nullptr;
		std::wstring filename_;
		PnmSessionOptions options_{};

		PreparedPnmDibPage currentPage_{};
		bool hasCurrentPage_ = false;

		std::vector<uint8_t> rowBuffer_;
		std::vector<uint8_t> packedRow_;
};

// ============================================================
// DTWAIN-style wrapper
// Model B for PNM:
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
class DTWAINPnmOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const PnmSessionOptions& options, const PreparedPnmDibPage& page);
		bool OnLastPage();
		bool IsOpen() const noexcept;

	private:
		std::unique_ptr<PnmSessionWriter> writer_;
};

#endif
