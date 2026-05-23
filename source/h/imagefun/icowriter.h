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
#ifndef ICOWRITER_H
#define ICOWRITER_H

#include <vector>
#include <string>
#include <optional>
#include <windows.h>
#include <png.h>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// ICO prepared page
// ============================================================

enum class IcoPixelFlavor
{
	BW1,
	Palette4,
	Palette8,
	Gray8,
	Bgr16,
	Bgr24,
	Bgra32
};

enum class IcoMode
{
	Classic,
	VistaPng
};

struct PreparedIcoDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	IcoPixelFlavor pixelFlavor = IcoPixelFlavor::Bgra32;
	const uint8_t* bits = nullptr;

	const RGBQUAD* palette = nullptr;
	uint32_t paletteEntries = 0;
};

struct IcoSessionOptions
{
	IcoMode mode = IcoMode::Classic;
};

// ============================================================
// PNG-ready intermediate
// ============================================================

enum class PngReadyFlavor
{
	Palette1,
	Palette4,
	Palette8,
	Gray8,
	Rgb24,
	Rgba32
};

struct PngReadyImage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint32_t strideBytes = 0;
	PngReadyFlavor flavor = PngReadyFlavor::Rgb24;
	const uint8_t* bits = nullptr;
	std::vector<uint8_t> ownedBits;
	std::vector<RGBQUAD> palette;
};

// ============================================================
// ICO on-disk structures
// ============================================================

#pragma pack(push, 1)
struct ICONDIR
{
	uint16_t idReserved;
	uint16_t idType;
	uint16_t idCount;
};

struct ICONDIRENTRY
{
	uint8_t  bWidth;
	uint8_t  bHeight;
	uint8_t  bColorCount;
	uint8_t  bReserved;
	uint16_t wPlanes;
	uint16_t wBitCount;
	uint32_t dwBytesInRes;
	uint32_t dwImageOffset;
};
#pragma pack(pop)

static_assert(sizeof(ICONDIR) == 6, "ICONDIR must be 6 bytes");
static_assert(sizeof(ICONDIRENTRY) == 16, "ICONDIRENTRY must be 16 bytes");

// ============================================================
// Memory PNG encoder for Vista-style ICO payloads
// Reuses non-FreeImage libpng logic
// ============================================================
class IcoMemoryPngEncoder
{
	public:
		bool Encode(const PreparedIcoDibPage& page, std::vector<uint8_t>& outPng);
	private:
		static void png_write_callback(png_structp png_ptr, png_bytep data, png_size_t length);
		static bool build_png_ready_image(const PreparedIcoDibPage& page, PngReadyImage& out);
		static void normalize_top_down_bytes(const PreparedIcoDibPage& page, uint32_t outStride, std::vector<uint8_t>& out);
};

// ============================================================
// ICO writer
// ============================================================
class IcoSessionWriter
{
public:
	IcoSessionWriter() = default;
	~IcoSessionWriter();
	IcoSessionWriter(const IcoSessionWriter&) = delete;
	IcoSessionWriter& operator=(const IcoSessionWriter&) = delete;
	bool Open(const std::wstring& filename, const IcoSessionOptions& options);
	bool SetPageInfo(const PreparedIcoDibPage& page);
	bool WriteCurrentPage();
	void Close();
	static std::optional<PreparedIcoDibPage> MakePreparedIcoDibPage(const dynarithmic::DibPageView& view);

private:
	static bool ValidatePage(const PreparedIcoDibPage& page);
	bool build_classic_icon_image();
	bool write_ico_file();
	void append_bytes(const void* p, size_t n);

private:
	FILE* file_ = nullptr;
	std::wstring filename_;
	IcoSessionOptions options_{};
	PreparedIcoDibPage currentPage_{};
	bool hasCurrentPage_ = false;
	std::vector<uint8_t> imageData_;
};

// ============================================================
// DTWAIN-style wrapper
// ============================================================

class DTWAINIcoOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const IcoSessionOptions& options, const PreparedIcoDibPage& page);
		bool OnLastPage();
	private:
		std::unique_ptr<IcoSessionWriter> writer_;
};

#endif
