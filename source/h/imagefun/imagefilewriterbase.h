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
#ifndef IMAGEFILEWRITERBASE_H
#define IMAGEFILEWRITERBASE_H

#include "dibutil.h"

namespace dynarithmic
{
	enum class DibPixelFlavor
	{
		BW1,
		Indexed4,
		Indexed8,
		Gray8,
		Gray16,
		RGB24,
		RGBA32,
		Unknown
	};

	struct DibPageView
	{
		uint32_t width = 0;
		uint32_t height = 0;
		uint16_t bitsPerPixel = 0;
		uint32_t strideBytes = 0;
		uint16_t xDPI = 0;
		uint16_t yDPI = 0;
		bool bottomUp = true;

		const uint8_t* bits = nullptr;
		const RGBQUAD* palette = nullptr;
		uint32_t paletteEntries = 0;

		const BITMAPINFOHEADER* bih = nullptr;
	};

	class LockedDibPage
	{
		public:
			explicit LockedDibPage(HANDLE hDib);
			bool IsValid() const noexcept { return valid_; }
			const DibPageView& GetView() const noexcept { return view_; }

		private:
			dib::LockedDib dib_;
			DibPageView view_{};
			bool valid_ = false;
	};

	inline DibPixelFlavor ClassifyPixelFlavor(const LockedDibPage& page)
	{
		const auto& theView = page.GetView();
		switch (theView.bitsPerPixel)
		{
			case 1:
				return DibPixelFlavor::BW1;

			case 4:
				return DibPixelFlavor::Indexed4;

			case 8:
			{
				if (theView.palette && dynarithmic::dib::is_grayscale_palette( theView.palette, theView.paletteEntries))
					return DibPixelFlavor::Gray8;
				return DibPixelFlavor::Indexed8;
			}

			case 16:
				return DibPixelFlavor::Gray16;

			case 24:
				return DibPixelFlavor::RGB24;

			case 32:
				return DibPixelFlavor::RGBA32;

			default:
				return DibPixelFlavor::Unknown;
		}
	}
}
#endif
