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
#ifndef WBMPWRITER_H
#define WBMPWRITER_H

#include "dibutil.h"
#include <memory>

// ============================================================
// WBMP model
// Input is always 1-bpp
// ============================================================

struct PreparedWbmpDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 1;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	const uint8_t* bits = nullptr;
};

struct WbmpSessionOptions
{
	// WBMP Type 0 only
	// If true, reverse bits in each byte before writing.
	// Use this only if your incoming 1-bpp DIB rows are LSB-first.
	bool reverseBitOrder = false;
};

// ============================================================
// Locked page wrapper
// ============================================================
class LockedWbmpDibPage
{
	public:
		explicit LockedWbmpDibPage(HANDLE hDib);
		bool IsValid() const noexcept;
		const PreparedWbmpDibPage& GetPage() const noexcept;

	private:
		dynarithmic::dib::LockedDib dib_;
		PreparedWbmpDibPage page_{};
		bool valid_ = false;
};

// ============================================================
// WBMP writer
// Type 0 WBMP:
//   byte 0 = TypeField = 0
//   byte 1 = FixHeaderField = 0
//   width  = multi-byte integer
//   height = multi-byte integer
//   image data = packed 1-bpp rows, MSB = leftmost pixel
// ============================================================
class WbmpSessionWriter
{
public:
	WbmpSessionWriter() = default;
	~WbmpSessionWriter();
	WbmpSessionWriter(const WbmpSessionWriter&) = delete;
	WbmpSessionWriter& operator=(const WbmpSessionWriter&) = delete;
	bool Open(const std::wstring& filename, const WbmpSessionOptions& options);
	bool SetPageInfo(const PreparedWbmpDibPage& page);
	bool WriteCurrentPage();
	void Close();
	bool IsOpen() const noexcept;

private:
	static bool ValidatePage(const PreparedWbmpDibPage& page);
	static uint8_t ReverseBits(uint8_t v);
	bool WriteByte(uint8_t b);
	bool WriteMultiByteUInt(uint32_t value);
	bool WriteHeader();
	bool WriteBitmapData();

private:
	FILE* file_ = nullptr;
	std::wstring filename_;
	WbmpSessionOptions options_{};

	PreparedWbmpDibPage currentPage_{};
	bool hasCurrentPage_ = false;

	std::vector<uint8_t> rowBuffer_;
};

class DTWAINWbmpOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const WbmpSessionOptions& options, const PreparedWbmpDibPage& page);
		bool OnLastPage();
		bool IsOpen() const noexcept;

	private:
		std::unique_ptr<WbmpSessionWriter> writer_;
};

#endif
