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
#ifndef SVGWRITER_H
#define SVGWRITER_H

#include <windows.h>
#include <string>
#include <cstdint>
#include <optional>
#include "imagefilewriterbase.h"

enum class SvgOutputType
{
	Svg,
	Svgz
};

struct SvgSessionOptions
{
	SvgOutputType type = SvgOutputType::Svg;
	std::string comment;
};

struct PreparedSvgDibPage
{
	LPBITMAPINFOHEADER header = nullptr;
	uint32_t offsetToBitsData = 0;

	uint32_t width = 0;
	uint32_t height = 0;
	uint16_t bitsPerPixel = 0;
};

class SvgSessionWriter
{
	public:
		bool Open(const std::wstring& filename, const SvgSessionOptions& options);
		bool WritePage();
		bool Close();
		bool IsOpen() const noexcept;
		bool SetPageInfo(const PreparedSvgDibPage& page);
		static std::optional<PreparedSvgDibPage> MakePreparedSvgPage(const dynarithmic::DibPageView& view);

	private:
		static bool ValidatePage(const PreparedSvgDibPage& page);
		bool SaveDIBAsSVGEx(const BITMAPINFOHEADER& bih, const uint8_t* bits, const std::string filename, bool isSVGZ,
							std::string& comment);
		PreparedSvgDibPage currentPage_{};
		std::wstring filename_;
		SvgSessionOptions options_{};
		bool isOpen_ = false;
		bool wrotePage_ = false;
};

#endif
