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
#ifndef JPEGWRITER_H
#define JPEGWRITER_H

#include <string>
#include <vector>
#include <memory>
#include <cstdio>
#include <windows.h>
#include <jpeglib.h>
#include <optional>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// JPEG page model
// Supports only:
//   8 bpp  -> grayscale JPEG
//   16 bpp -> grayscale JPEG (down-converted to 8-bit samples)
//   24 bpp -> RGB JPEG
//
// Other source depths are assumed to have been resampled upstream
// before reaching this writer.
// ============================================================
enum class JpegPixelFlavor
{
	Gray8,
	Gray16,
	Bgr24
};

struct PreparedJpegDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	JpegPixelFlavor pixelFlavor = JpegPixelFlavor::Gray8;
	const uint8_t* bits = nullptr;
};

struct JpegTextMetadata
{
	std::string software;
	std::string copyright;
	std::string author;
	std::string description;
	std::string comment;
};

struct JpegSessionOptions
{
	int quality = 75;
	bool progressive = false;
	JpegTextMetadata text;
};

// ============================================================
// JPEG writer
// ============================================================
class JpegSessionWriter
{
	public:
		JpegSessionWriter() = default;
		~JpegSessionWriter();
		JpegSessionWriter(const JpegSessionWriter&) = delete;
		JpegSessionWriter& operator=(const JpegSessionWriter&) = delete;
		bool Open(const std::wstring& filename, const JpegSessionOptions& options);
		bool SetPageInfo(const PreparedJpegDibPage& page);
		bool WriteCurrentPage();
		void Close();
		bool IsOpen() const noexcept;
		static std::optional<PreparedJpegDibPage> MakePreparedJpegPage(const dynarithmic::DibPageView& view);

	private:
		static bool ValidatePage(const PreparedJpegDibPage& page);
		static void append_metadata_line(std::string& out, const char* key, const std::string& value);
		std::string build_comment_text() const;
		void write_comment_markers(jpeg_compress_struct& cinfo);

	private:
		FILE* file_ = nullptr;
		std::wstring filename_;
		JpegSessionOptions options_{};
		PreparedJpegDibPage currentPage_{};
		bool hasCurrentPage_ = false;
		std::vector<uint8_t> rowBuffer_;
};

class DTWAINJpegOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const JpegSessionOptions& options, const PreparedJpegDibPage& page);
		bool OnLastPage();
	private:
		std::unique_ptr<JpegSessionWriter> writer_;
};

#endif
