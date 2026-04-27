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
#ifndef JPEG2KWRITER_H
#define JPEG2KWRITER_H

#include <cstdint>
#include <string>
#include <memory>
#include <openjpeg.h>
#include "dibutil.h"
#include "imagefilewriterbase.h"

enum class Jpeg2000PixelFlavor
{
	Gray8,
	Gray16,
	Bgr24,
	Bgra32
};

struct Jpeg2000TextMetadata
{
	std::string software;
	std::string copyright;
	std::string author;
	std::string description;
	std::string comment;
};

struct Jpeg2000SessionOptions
{
	bool useJP2Container = true;  // true = .jp2, false = raw codestream .j2k/.j2c
	float compressionRate = 0.0f; // 0.0 = lossless
	int numResolutions = 6;
	int numThreads = 0;           // 0 = default
	Jpeg2000TextMetadata text;
};

struct PreparedJpeg2000DibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	Jpeg2000PixelFlavor pixelFlavor = Jpeg2000PixelFlavor::Gray8;
	const uint8_t* bits = nullptr;
};


class Jpeg2000SessionWriter
{
	public:
		Jpeg2000SessionWriter() = default;
		~Jpeg2000SessionWriter();
		Jpeg2000SessionWriter(const Jpeg2000SessionWriter&) = delete;
		Jpeg2000SessionWriter& operator=(const Jpeg2000SessionWriter&) = delete;

		bool Open(const std::wstring& filename, const Jpeg2000SessionOptions& options);
		bool SetPageInfo(const PreparedJpeg2000DibPage& page);
		bool WriteCurrentPage();
		void Close();
		bool IsOpen() const noexcept;
		static std::optional<PreparedJpeg2000DibPage> MakePreparedJpeg2000Page(const dynarithmic::DibPageView& view);

	private:
		static bool ValidatePage(const PreparedJpeg2000DibPage& page);
		static std::string NarrowFilename(const std::wstring& ws);
		opj_image_t* CreateOpenJpegImage() const;
		void FillOpenJpegImage(opj_image_t* image) const;
		std::string BuildJpeg2000Comment() const;

	private:
		std::wstring filename_;
		Jpeg2000SessionOptions options_{};

		PreparedJpeg2000DibPage currentPage_{};
		bool hasCurrentPage_ = false;
		bool isOpen_ = false;
};

class DTWAINJpeg2000Output
{
	public:
		bool OnFirstPage(const std::wstring& filename, const Jpeg2000SessionOptions& options, const PreparedJpeg2000DibPage& page);
		bool OnLastPage();
		bool IsOpen() const noexcept;

	private:
		std::unique_ptr<Jpeg2000SessionWriter> writer_;
};

#endif
