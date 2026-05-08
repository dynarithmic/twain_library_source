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
#include "metafilewriter.h"
#include <windows.h>

#pragma pack(push, 1)
struct AldusPlaceableHeader
{
	DWORD key = 0x9AC6CDD7;
	WORD  hmf = 0;
	SHORT left = 0;
	SHORT top = 0;
	SHORT right = 0;
	SHORT bottom = 0;
	WORD  inch = 1440;
	DWORD reserved = 0;
	WORD  checksum = 0;
};
#pragma pack(pop)

static WORD ComputePlaceableChecksum(const AldusPlaceableHeader& h)
{
	const WORD* p = reinterpret_cast<const WORD*>(&h);
	WORD sum = 0;

	// checksum covers first 10 WORDs, excluding checksum itself
	for (int i = 0; i < 10; ++i)
		sum ^= p[i];

	return sum;
}

std::optional<PreparedMetafileDibPage> MetafileSessionWriter::MakePreparedMetafileDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedMetafileDibPage page{};
	page.width = view.width;
	page.height = view.height;

	auto bih = view.bih;
	if (!bih || bih->biSize < sizeof(BITMAPINFOHEADER))
		return std::nullopt;

	page.bih = view.bih;
	page.bpp = view.bitsPerPixel;
	page.bottomUp = view.height > 0;

	if (bih->biXPelsPerMeter > 0)
		page.xDpi = bih->biXPelsPerMeter * 0.0254;
	if (bih->biYPelsPerMeter > 0)
		page.yDpi = bih->biYPelsPerMeter * 0.0254; 

	page.bits = view.bits;
	return page;
}

///////////////////////////////////////////////////////////////////////////
bool MetafileSessionWriter::Open(const std::wstring& filename, const MetafileSessionOptions& options)
{
	if (isOpen_)
		return false;

	filename_ = filename;
	options_ = options;
	wrotePage_ = false;
	isOpen_ = true;
	return true;
}

bool MetafileSessionWriter::WritePage(const PreparedMetafileDibPage& page)
{
	if (!isOpen_ || wrotePage_)
		return false;

	if (!page.bih || !page.bits || page.width == 0 || page.height == 0)
		return false;

	bool ok = false;

	if (options_.type == MetafileType::Emf)
		ok = WriteEmf(page);
	else
		ok = WriteWmfViaEmf(page);

	wrotePage_ = ok;
	return ok;
}

bool MetafileSessionWriter::Close()
{
	filename_.clear();
	isOpen_ = false;
	wrotePage_ = false;
	return true;
}

int MetafileSessionWriter::To01mm(double pixels, double dpi)
{
	if (dpi <= 0.0)
		dpi = 300.0;

	// inches = pixels / dpi
	// millimeters = inches * 25.4
	// .01mm = mm * 100
	return static_cast<int>((pixels / dpi) * 25.4 * 100.0 + 0.5);
}

bool MetafileSessionWriter::WriteEmf(const PreparedMetafileDibPage& page)
{
	HDC refDC = GetDC(nullptr);
	if (!refDC)
		return false;

	const int horzResMM = GetDeviceCaps(refDC, HORZSIZE);
	const int vertResMM = GetDeviceCaps(refDC, VERTSIZE);
	const int horzResPx = GetDeviceCaps(refDC, HORZRES);
	const int vertResPx = GetDeviceCaps(refDC, VERTRES);

	RECT frame{};
	frame.left = 0;
	frame.top = 0;
	frame.right =
		static_cast<LONG>((static_cast<double>(page.width) *
			horzResMM * 100.0) / horzResPx + 0.5);
	frame.bottom =
		static_cast<LONG>((static_cast<double>(page.height) *
			vertResMM * 100.0) / vertResPx + 0.5);

	HDC emfDC = CreateEnhMetaFileW(
		refDC,
		filename_.c_str(),
		&frame,
		options_.description.empty() ? nullptr : options_.description.c_str());

	ReleaseDC(nullptr, refDC);

	if (!emfDC)
		return false;

	SetMapMode(emfDC, MM_ISOTROPIC);
	SetWindowExtEx(emfDC, 96, 96, nullptr);
	SetViewportExtEx(emfDC, 96, 96, nullptr);

	const int ret = StretchDIBits(
		emfDC,
		0,
		0,
		static_cast<int>(page.width),
		static_cast<int>(page.height),
		0,
		0,
		static_cast<int>(page.width),
		static_cast<int>(page.height),
		page.bits,
		reinterpret_cast<const BITMAPINFO*>(page.bih),
		DIB_RGB_COLORS,
		SRCCOPY);

	HENHMETAFILE hemf = CloseEnhMetaFile(emfDC);
	if (!hemf)
		return false;

	DeleteEnhMetaFile(hemf);

	return ret != GDI_ERROR;
}

bool MetafileSessionWriter::WriteWmfViaEmf(const PreparedMetafileDibPage& page)
{
	HENHMETAFILE hemf = CreateRasterEmfInMemory(page);
	if (!hemf)
		return false;

	HDC refDC = GetDC(nullptr);
	if (!refDC)
	{
		DeleteEnhMetaFile(hemf);
		return false;
	}

	const UINT bytesNeeded = GetWinMetaFileBits(
		hemf,
		0,
		nullptr,
		MM_ANISOTROPIC,
		refDC);

	std::vector<BYTE> wmfBits(bytesNeeded);

	const UINT bytesWritten = GetWinMetaFileBits(
		hemf,
		bytesNeeded,
		wmfBits.data(),
		MM_ANISOTROPIC,
		refDC);

	ReleaseDC(nullptr, refDC);
	DeleteEnhMetaFile(hemf);

	if (bytesNeeded == 0 || bytesWritten != bytesNeeded)
		return false;

	return WritePlaceableWmfFile(filename_, page, wmfBits);
}

HENHMETAFILE MetafileSessionWriter::CreateRasterEmfInMemory(const PreparedMetafileDibPage& page)
{
	if (!page.bih || !page.bits || page.width == 0 || page.height == 0)
		return nullptr;

	HDC refDC = GetDC(nullptr);
	if (!refDC)
		return nullptr;

	const int horzSizeMM = GetDeviceCaps(refDC, HORZSIZE);
	const int vertSizeMM = GetDeviceCaps(refDC, VERTSIZE);
	const int horzResPx = GetDeviceCaps(refDC, HORZRES);
	const int vertResPx = GetDeviceCaps(refDC, VERTRES);

	RECT frame{};
	frame.left = 0;
	frame.top = 0;
	frame.right =
		static_cast<LONG>((static_cast<double>(page.width) *
			horzSizeMM * 100.0) / horzResPx + 0.5);
	frame.bottom =
		static_cast<LONG>((static_cast<double>(page.height) *
			vertSizeMM * 100.0) / vertResPx + 0.5);

	HDC emfDC = CreateEnhMetaFileW(
		refDC,
		nullptr, // nullptr = memory-based EMF
		&frame,
		L"DTWAIN\0Raster EMF\0");

	ReleaseDC(nullptr, refDC);

	if (!emfDC)
		return nullptr;

	SetMapMode(emfDC, MM_ISOTROPIC);
	SetWindowExtEx(emfDC, 96, 96, nullptr);
	SetViewportExtEx(emfDC, 96, 96, nullptr);

	const int ret = StretchDIBits(
		emfDC,
		0,
		0,
		static_cast<int>(page.width),
		static_cast<int>(page.height),
		0,
		0,
		static_cast<int>(page.width),
		static_cast<int>(page.height),
		page.bits,
		reinterpret_cast<const BITMAPINFO*>(page.bih),
		DIB_RGB_COLORS,
		SRCCOPY);

	HENHMETAFILE hemf = CloseEnhMetaFile(emfDC);

	if (!hemf)
		return nullptr;

	if (ret == GDI_ERROR)
	{
		DeleteEnhMetaFile(hemf);
		return nullptr;
	}

	return hemf;
}
bool MetafileSessionWriter::WritePlaceableWmfFile(const std::wstring& filename, const PreparedMetafileDibPage& page, const std::vector<BYTE>& wmfBits)
{
	FILE* f = _wfopen(filename.c_str(), L"wb");
	if (!f)
		return false;

	AldusPlaceableHeader hdr{};

	hdr.left = 0;
	hdr.top = 0;

	// Use 1440 twips/inch.
	// Convert pixels to twips using image DPI if available.
	const double dpiX = page.xDpi > 0.0 ? page.xDpi : 96.0;
	const double dpiY = page.yDpi > 0.0 ? page.yDpi : 96.0;

	hdr.right =
		static_cast<SHORT>((static_cast<double>(page.width) / dpiX) * 1440.0 + 0.5);

	hdr.bottom =
		static_cast<SHORT>((static_cast<double>(page.height) / dpiY) * 1440.0 + 0.5);

	hdr.checksum = ComputePlaceableChecksum(hdr);

	bool ok = true;

	if (std::fwrite(&hdr, 1, sizeof(hdr), f) != sizeof(hdr))
		ok = false;

	if (ok && !wmfBits.empty())
	{
		if (std::fwrite(wmfBits.data(), 1, wmfBits.size(), f) != wmfBits.size())
			ok = false;
	}

	std::fclose(f);
	return ok;
}
///////////////////////////////////////////////////////////////////////////
bool DTWAINMetafileOutput::OnFirstPage(const std::wstring& filename, const MetafileSessionOptions& options, const PreparedMetafileDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<MetafileSessionWriter>();

	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	return writer_->WritePage(page);
}

bool DTWAINMetafileOutput::OnLastPage()
{
	if (!writer_)
		return false;

	const bool ok = writer_->Close();
	writer_.reset();
	return ok;
}
