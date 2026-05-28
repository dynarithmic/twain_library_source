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
#ifndef METAFILEWRITER_H
#define METAFILEWRITER_H

#include <string>
#include <memory>
#include <optional>
#include <vector>
#include <windows.h>
#include "dibutil.h"
#include "imagefilewriterbase.h"

enum class MetafileType
{
	Emf,
	Wmf
};

struct MetafileSessionOptions
{
	MetafileType type = MetafileType::Emf;

	// Logical output size in .01 mm for EMF frame.
	// If zero, calculated from DIB DPI.
	int frameWidth01mm = 0;
	int frameHeight01mm = 0;

	// WMF mapping target in twips-like logical units.
	int wmfWidth = 0;
	int wmfHeight = 0;

	std::wstring description;
};

struct PreparedMetafileDibPage
{
	const BITMAPINFOHEADER* bih = nullptr;
	const void* bits = nullptr;
	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bpp = 0;
	bool bottomUp = true;
	double xDpi = 300.0;
	double yDpi = 300.0;
};

class MetafileSessionWriter
{
	public:
		bool Open(const std::wstring& filename, const MetafileSessionOptions& options);
		bool WritePage(const PreparedMetafileDibPage& page);
		bool Close();
		static std::optional<PreparedMetafileDibPage> MakePreparedMetafileDibPage(const dynarithmic::DibPageView& view);

	private:
		static int To01mm(double pixels, double dpi);
		bool WriteEmf(const PreparedMetafileDibPage& page);
		bool WriteWmfViaEmf(const PreparedMetafileDibPage& page);
		bool WritePlaceableWmfFile(const std::wstring& filename, const PreparedMetafileDibPage& page,
			                       const std::vector<BYTE>& wmfBits);
		HENHMETAFILE CreateRasterEmfInMemory(const PreparedMetafileDibPage& page);


	private:
		std::wstring filename_;
		MetafileSessionOptions options_{};
		bool isOpen_ = false;
		bool wrotePage_ = false;
};

class DTWAINMetafileOutput
{
	public:
		bool OnFirstPage(const std::wstring& filename, const MetafileSessionOptions& options, const PreparedMetafileDibPage& page);
		bool OnLastPage();

	private:
		std::unique_ptr<MetafileSessionWriter> writer_;
};

#endif
