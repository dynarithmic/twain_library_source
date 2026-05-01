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
#ifndef JPEGXRWRITER_H
#define JPEGXRWRITER_H

#include <string>
#include <cstdint>
#include <memory>
#include "dibutil.h"
#include "JXRGlue.h"
#include "ctlstringconversion.h"
#include "imagefilewriterbase.h"

// ============================================================
// JPEG-XR model
// DTWAIN contract:
//   1, 8, 16, 24, 32 are preserved
//   4 bpp is resampled upstream to 8
// ============================================================

enum class JxrPixelFlavor
{
	BW1,
	Gray8,
	Gray16,
	Bgr24,
	Bgra32
};

struct PreparedJxrDibPage
{
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
	uint32_t strideBytes = 0;
	bool bottomUp = true;

	JxrPixelFlavor pixelFlavor = JxrPixelFlavor::Gray8;
	const uint8_t* bits = nullptr;

	double xDpi = 96.0;
	double yDpi = 96.0;
};

struct JxrTextMetadata
{
	std::string comment;
};

struct JxrSessionOptions
{
	bool lossless = false;

	// A practical quality scale for the wrapper:
	// 0.0 = smallest / lowest quality
	// 1.0 = highest quality
	// ignored when lossless == true
	float quality = 0.90f;

	bool progressive = false;

	JxrTextMetadata text;
};

// ============================================================
// Small RAII helpers for jxrlib glue objects
// ============================================================
namespace dynarithmic::jxr_adapter
{
	struct StreamDeleter
	{
		void operator()(WMPStream* p) const noexcept
		{
			if (p)
				p->Close(&p);
		}
	};

	struct EncoderDeleter
	{
		void operator()(PKImageEncode* p) const noexcept
		{
			if (p)
				p->Release(&p);
		}
	};

	using UniqueStream = std::unique_ptr<WMPStream, StreamDeleter>;
	using UniqueEncoder = std::unique_ptr<PKImageEncode, EncoderDeleter>;

	inline const GUID& PixelFormatGuid(JxrPixelFlavor flavor)
	{
		switch (flavor)
		{
			case JxrPixelFlavor::BW1:
				return GUID_PKPixelFormatBlackWhite;

			case JxrPixelFlavor::Gray8:
				return GUID_PKPixelFormat8bppGray;

			case JxrPixelFlavor::Gray16:
				return GUID_PKPixelFormat16bppGray;

			case JxrPixelFlavor::Bgr24:
				return GUID_PKPixelFormat24bppBGR;

			case JxrPixelFlavor::Bgra32:
				return GUID_PKPixelFormat32bppBGRA;
			}

			return GUID_PKPixelFormat24bppBGR;
	}

	inline ERR CreateFileStreamWrite(const wchar_t* filename, UniqueStream& out)
	{
		WMPStream* raw = nullptr;

		// Common jxrlib glue helper
		std::string narrowFilename = StringConversion::Convert_WidePtr_To_Ansi(filename);
		ERR err = CreateWS_File(&raw, narrowFilename.c_str(), "wb");
		if (err != WMP_errSuccess || !raw)
			return err;

		out.reset(raw);
		return WMP_errSuccess;
	}

	inline ERR CreateEncoder(UniqueEncoder& out)
	{
		PKImageEncode* raw = nullptr;

		// Common jxrlib glue helper for JPEG-XR encoder creation
		ERR err = PKImageEncode_Create_WMP(&raw);
		if (err != WMP_errSuccess || !raw)
			return err;

		out.reset(raw);
		return WMP_errSuccess;
	}
}

// ============================================================
// JPEG-XR writer
// ============================================================
class JxrSessionWriter
{
	public:
		~JxrSessionWriter();
		JxrSessionWriter() = default;
		JxrSessionWriter(const JxrSessionWriter&) = delete;
		JxrSessionWriter& operator=(const JxrSessionWriter&) = delete;
		bool Open(const std::wstring& filename, const JxrSessionOptions& options);
		bool SetPageInfo(const PreparedJxrDibPage& page);
		bool WriteCurrentPage();
		void Close();
		bool IsOpen() const noexcept;
		static std::optional<PreparedJxrDibPage> MakePreparedJxrPage(const dynarithmic::DibPageView& view);

	private:
		static bool ValidatePage(const PreparedJxrDibPage& page);
		uint32_t EffectiveRowBytes() const;
		void PrepareRow(const uint8_t* src, uint8_t* dst, uint32_t rowBytes) const;

	private:
		std::wstring filename_;
		JxrSessionOptions options_{};

		PreparedJxrDibPage currentPage_{};
		bool hasCurrentPage_ = false;
		bool isOpen_ = false;

		std::vector<uint8_t> rowBuffer_;
		std::vector<uint8_t> imageBuffer_;
};

// ============================================================
// DTWAIN-style wrapper
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
class DTWAINJxrOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const JxrSessionOptions& options, const PreparedJxrDibPage& page);
		bool OnLastPage();
		bool IsOpen() const noexcept;

	private:
		std::unique_ptr<JxrSessionWriter> writer_;
};

#endif
