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
#include "jpegxrwriter.h"

 // ============================================================
 // Locked page wrapper
 // ============================================================
LockedJxrDibPage::LockedJxrDibPage(HANDLE hDib) : dib_(hDib)
{
	if (!dib_.IsValid())
		return;

	const auto* bih = dib_.Header();
	if (!bih || bih->biWidth <= 0 || bih->biHeight == 0)
		return;

	PreparedJxrDibPage page{};
	page.width = dib_.Width();
	page.height = dib_.Height();
	page.bitsPerPixel = dib_.BitsPerPixel();
	page.strideBytes = dib_.StrideBytes();
	page.bottomUp = dib_.BottomUp();
	page.bits = dib_.Bits();
	page.xDpi = dib_.XDpi();
	page.yDpi = dib_.YDpi();

	switch (page.bitsPerPixel)
	{
		case 1:
			page.pixelFlavor = JxrPixelFlavor::BW1;
			break;
		case 8:
			page.pixelFlavor = JxrPixelFlavor::Gray8;
			break;
		case 16:
			page.pixelFlavor = JxrPixelFlavor::Gray16;
			break;
		case 24:
			page.pixelFlavor = JxrPixelFlavor::Bgr24;
			break;
		case 32:
			page.pixelFlavor = JxrPixelFlavor::Bgra32;
			break;
		default:
			return;
	}

	page_ = page;
	valid_ = true;
}

bool LockedJxrDibPage::IsValid() const noexcept
{
	return valid_;
}

const PreparedJxrDibPage& LockedJxrDibPage::GetPage() const noexcept
{
	return page_;
}

/////////////////////////////////////////////////////////
// ============================================================
// JPEG-XR writer
// ============================================================

JxrSessionWriter::~JxrSessionWriter()
{
	Close();
}

bool JxrSessionWriter::Open(const std::wstring& filename, const JxrSessionOptions& options)
{
	if (isOpen_)
		return false;

	filename_ = filename;
	options_ = options;
	hasCurrentPage_ = false;
	isOpen_ = true;
	return true;
}

bool JxrSessionWriter::SetPageInfo(const PreparedJxrDibPage& page)
{
	if (!isOpen_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool JxrSessionWriter::WriteCurrentPage()
{
	if (!isOpen_ || !hasCurrentPage_)
		return false;

	using namespace dynarithmic::jxr_adapter;

	// --------------------------------------------------------
	// Create output stream
	// --------------------------------------------------------
	UniqueStream stream;
	if (CreateFileStreamWrite(filename_.c_str(), stream) != WMP_errSuccess)
		return false;

	// --------------------------------------------------------
	// Create encoder
	// --------------------------------------------------------
	UniqueEncoder encoder;
	if (CreateEncoder(encoder) != WMP_errSuccess)
		return false;

	// --------------------------------------------------------
	// Initialize encoder using its own internal codec-param block
	// --------------------------------------------------------
	if (encoder->Initialize(encoder.get(),
		stream.get(),
		&encoder->WMP.wmiSCP,
		sizeof(CWMIStrCodecParam)) != WMP_errSuccess)
	{
		return false;
	}

	// Encoder owns the stream after Initialize()
	stream.release();

	// --------------------------------------------------------
	// Configure codec params IN PLACE
	// IMPORTANT: do NOT memset the whole struct here
	// --------------------------------------------------------
	CWMIStrCodecParam& codecParam = encoder->WMP.wmiSCP;

	codecParam.bfBitstreamFormat = SPATIAL;
	codecParam.bProgressiveMode =
		(!options_.lossless && options_.progressive) ? TRUE : FALSE;
	codecParam.olOverlap = OL_ONE;

	codecParam.cfColorFormat =
		(currentPage_.pixelFlavor == JxrPixelFlavor::BW1 ||
			currentPage_.pixelFlavor == JxrPixelFlavor::Gray8 ||
			currentPage_.pixelFlavor == JxrPixelFlavor::Gray16)
		? Y_ONLY
		: YUV_444;

	codecParam.bdBitDepth = BD_LONG;

	codecParam.uAlphaMode =
		(currentPage_.pixelFlavor == JxrPixelFlavor::Bgra32) ? 2 : 0;

	if (options_.lossless)
	{
		codecParam.uiDefaultQPIndex = 0;
	}
	else
	{
		float q = options_.quality;
		if (q < 0.0f) q = 0.0f;
		if (q > 1.0f) q = 1.0f;

		float qp = (1.0f - q) * 63.0f + 0.5f;
		if (qp < 0.0f) qp = 0.0f;
		if (qp > 63.0f) qp = 63.0f;

		codecParam.uiDefaultQPIndex = static_cast<U8>(qp);
	}

	// --------------------------------------------------------
	// Set image size
	// --------------------------------------------------------
	if (encoder->SetSize(encoder.get(),
		currentPage_.width,
		currentPage_.height) != WMP_errSuccess)
	{
		return false;
	}

	// --------------------------------------------------------
	// Set pixel format
	// --------------------------------------------------------
	GUID pixelFormat = PixelFormatGuid(currentPage_.pixelFlavor);
	if (encoder->SetPixelFormat(encoder.get(), pixelFormat) != WMP_errSuccess)
		return false;

	// --------------------------------------------------------
	// Set DPI
	// --------------------------------------------------------
	if (encoder->SetResolution(encoder.get(),
		static_cast<Float>(currentPage_.xDpi),
		static_cast<Float>(currentPage_.yDpi)) != WMP_errSuccess)
	{
		return false;
	}

	// --------------------------------------------------------
	// Build a top-down image buffer using the full DIB stride
	// --------------------------------------------------------
	const uint32_t stride = currentPage_.strideBytes;
	imageBuffer_.resize(static_cast<size_t>(stride) * currentPage_.height);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY =
			currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		uint8_t* dst =
			imageBuffer_.data() + static_cast<size_t>(y) * stride;

		PrepareRow(src, dst, stride);
	}

	// --------------------------------------------------------
	// Write all rows in one call, matching plugin behavior
	// --------------------------------------------------------
	if (encoder->WritePixels(encoder.get(),
		currentPage_.height,
		imageBuffer_.data(),
		stride) != WMP_errSuccess)
	{
		return false;
	}

	// --------------------------------------------------------
	// Finalize
	// --------------------------------------------------------
	if (encoder->Terminate(encoder.get()) != WMP_errSuccess)
		return false;

	hasCurrentPage_ = false;
	return true;
}

void JxrSessionWriter::Close()
{
	filename_.clear();
	rowBuffer_.clear();
	hasCurrentPage_ = false;
	isOpen_ = false;
}

bool JxrSessionWriter::IsOpen() const noexcept
{
	return isOpen_;
}

bool JxrSessionWriter::ValidatePage(const PreparedJxrDibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 1:
		case 8:
		case 16:
		case 24:
		case 32:
			return true;
		default:
			return false;
	}
}

uint32_t JxrSessionWriter::EffectiveRowBytes() const
{
	switch (currentPage_.pixelFlavor)
	{
		case JxrPixelFlavor::BW1:
			return static_cast<uint32_t>((currentPage_.width + 7) / 8);
		case JxrPixelFlavor::Gray8:
			return currentPage_.width;
		case JxrPixelFlavor::Gray16:
			return currentPage_.width * 2;
		case JxrPixelFlavor::Bgr24:
			return currentPage_.width * 3;
		case JxrPixelFlavor::Bgra32:
			return currentPage_.width * 4;
	}
	return currentPage_.strideBytes;
}

void JxrSessionWriter::PrepareRow(const uint8_t* src, uint8_t* dst, uint32_t rowBytes) const
{
	// For the supported formats here, the in-memory Windows DIB layout is already
	// what jxrlib expects for these pixel formats: BW1, Gray8, Gray16, BGR24, BGRA32.
	// The only normalization we need is top-down row order.
	std::memcpy(dst, src, rowBytes);
}

// ============================================================
// DTWAIN-style wrapper
// Model B for JPEG-XR:
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================

bool DTWAINJxrOutput::OnFirstPage(const std::wstring& filename, const JxrSessionOptions& options, const PreparedJxrDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<JxrSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINJxrOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINJxrOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}
