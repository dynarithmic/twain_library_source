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
#include "postscriptwriter.h"
#include "a85encode.h"
#include "flateencode.h"
#include "zlib.h"

static void PsRunLengthEncode(std::string_view input, std::string& output)
{
	output.clear();

	const auto* src = reinterpret_cast<const uint8_t*>(input.data());
	const size_t n = input.size();

	size_t i = 0;

	while (i < n)
	{
		// Detect repeated run, max 128 bytes.
		size_t run = 1;
		while (i + run < n && run < 128 && src[i + run] == src[i])
			++run;

		if (run >= 3)
		{
			// Repeated run:
			// length byte = 257 - run length
			// followed by repeated byte
			output.push_back(static_cast<char>(257 - run));
			output.push_back(static_cast<char>(src[i]));
			i += run;
		}
		else
		{
			// Literal run:
			// collect up to 128 bytes, stopping before next repeated run >= 3
			const size_t literalStart = i;
			size_t literalLen = 0;

			while (i < n && literalLen < 128)
			{
				run = 1;
				while (i + run < n && run < 128 && src[i + run] == src[i])
					++run;

				if (run >= 3)
					break;

				++i;
				++literalLen;
			}

			output.push_back(static_cast<char>(literalLen - 1));
			output.append(reinterpret_cast<const char*>(src + literalStart), literalLen);
		}
	}

	// PostScript RunLengthDecode EOD marker.
	output.push_back(static_cast<char>(128));
}

std::optional<PreparedPsDibPage> PsSessionWriter::MakePreparedPsDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedPsDibPage page{};
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;
	page.palette = view.palette;
	page.paletteEntries = view.paletteEntries;

	if (view.xDPI > 0)
		page.xDpi = view.xDPI * 0.0254;
	if (view.yDPI > 0)
		page.yDpi = view.yDPI * 0.0254;

	switch (page.bitsPerPixel)
	{
		case 1:
			page.pixelFlavor = PsPixelFlavor::BW1;
			break;
		case 8:
			page.pixelFlavor = PsPixelFlavor::Gray8;
			break;
		case 16:
			page.pixelFlavor = PsPixelFlavor::Gray16;
			break;
		case 24:
			page.pixelFlavor = PsPixelFlavor::Bgr24;
			break;
		case 32:
			page.pixelFlavor = PsPixelFlavor::Bgra32;
			break;
		default:
			return page;
	}
	return page;
}

////////////////////////////////////////////////////////////////
PsSessionWriter::~PsSessionWriter()
{
	Close();
}

bool PsSessionWriter::Open(const std::wstring& filename, const PsSessionOptions& options)
{
	if (file_)
		return false;

	file_ = _wfopen(filename.c_str(), L"wb");
	if (!file_)
		return false;

	options_ = options;
	pageCount_ = 0;
	opened_ = true;

	return WriteDocumentHeader();
}

bool PsSessionWriter::WritePage(const PreparedPsDibPage& page)
{
	if (!opened_ || !ValidatePage(page))
		return false;

	++pageCount_;

	if (!WritePageHeader(page))
		return false;

	if (!WritePageSetup(page))
		return false;

	if (options_.level == PsLevel::Level1)
	{
		if (!WriteLevel1Image(page))
			return false;
	}
	else
	{
		if (!WriteLevel2Or3Image(page))
			return false;
	}

	if (std::fprintf(file_, "\ngrestore\nshowpage\n%%%%PageTrailer\n") < 0)
		return false;

	return true;
}

bool PsSessionWriter::Close()
{
	if (!file_)
		return true;

	bool ok = true;
	if (std::fprintf(file_, "%%%%Trailer\n%%%%Pages: %u\n%%%%EOF\n", pageCount_) < 0)
		ok = false;

	std::fclose(file_);
	file_ = nullptr;
	opened_ = false;
	pageCount_ = 0;
	hexLineCount_ = 0;
	rowBuffer_.clear();
	return ok;
}

bool PsSessionWriter::ValidatePage(const PreparedPsDibPage& page)
{
	if (!page.width || !page.height || !page.bits || !page.strideBytes)
		return false;

	return page.bitsPerPixel == 1 ||
		page.bitsPerPixel == 8 ||
		page.bitsPerPixel == 16 ||
		page.bitsPerPixel == 24 ||
		page.bitsPerPixel == 32;
}

bool PsSessionWriter::WriteDocumentHeader()
{
	const char* levelText = options_.level == PsLevel::Level1 ? "1.0" : "2.0";

	if (std::fprintf(file_,
		"%%!PS-Adobe-3.0\n"
		"%%%%Creator: %s\n"
		"%%%%LanguageLevel: %s\n"
		"%%%%Pages: (atend)\n"
		"%%%%BoundingBox: 0 0 %.0f %.0f\n"
		"%%%%EndComments\n\n",
		options_.creator.c_str(),
		levelText,
		options_.pageWidthPts,
		options_.pageHeightPts) < 0)
	{
		return false;
	}

	return true;
}

bool PsSessionWriter::WritePageHeader(const PreparedPsDibPage&)
{
	return std::fprintf(file_,
		"%%%%Page: %u %u\n",
		pageCount_, pageCount_) >= 0;
}

bool PsSessionWriter::WritePageSetup(const PreparedPsDibPage& page)
{
	const double imageWpts = (static_cast<double>(page.width) / page.xDpi) * 72.0;
	const double imageHpts = (static_cast<double>(page.height) / page.yDpi) * 72.0;

	const double availW = std::max(1.0, options_.pageWidthPts - options_.marginLeftPts - options_.marginRightPts);
	const double availH = std::max(1.0, options_.pageHeightPts - options_.marginBottomPts - options_.marginTopPts);

	double drawW = imageWpts;
	double drawH = imageHpts;

	if (options_.preserveAspectRatio)
	{
		const double scale = std::min(availW / imageWpts, availH / imageHpts);
		drawW = imageWpts * scale;
		drawH = imageHpts * scale;
	}
	else
	{
		drawW = availW;
		drawH = availH;
	}

	double x = options_.marginLeftPts;
	double y = options_.marginBottomPts;

	if (options_.centerImage)
	{
		x += (availW - drawW) / 2.0;
		y += (availH - drawH) / 2.0;
	}

	return std::fprintf(file_,
		"gsave\n"
		"%.4f %.4f translate\n"
		"%.4f %.4f scale\n",
		x, y, drawW, drawH) >= 0;
}

const char* PsSessionWriter::ColorSpaceName(const PreparedPsDibPage& page) const
{
	switch (page.pixelFlavor)
	{
		case PsPixelFlavor::BW1:
		case PsPixelFlavor::Gray8:
		case PsPixelFlavor::Gray16:
			return "/DeviceGray";

		case PsPixelFlavor::Bgr24:
		case PsPixelFlavor::Bgra32:
			return "/DeviceRGB";
	}
	return "/DeviceRGB";
}

uint32_t PsSessionWriter::Components(const PreparedPsDibPage& page) const
{
	switch (page.pixelFlavor)
	{
		case PsPixelFlavor::BW1:
		case PsPixelFlavor::Gray8:
		case PsPixelFlavor::Gray16:
			return 1;

		case PsPixelFlavor::Bgr24:
		case PsPixelFlavor::Bgra32:
			return 3;
	}
	return 3;
}

uint32_t PsSessionWriter::BitsPerComponent(const PreparedPsDibPage& page) const
{
	switch (page.pixelFlavor)
	{
		case PsPixelFlavor::BW1:
			return 1;

		case PsPixelFlavor::Gray16:
			return 16;

		default:
			return 8;
	}
}

const char* PsSessionWriter::DecodeArray(const PreparedPsDibPage& page) const
{
	if (page.pixelFlavor == PsPixelFlavor::BW1)
		return options_.invert1bpp ? "[0 1]" : "[1 0]";

	if (page.pixelFlavor == PsPixelFlavor::Gray8)
		return "[0 1]";

	return "[0 1 0 1 0 1]";
}

bool PsSessionWriter::WriteLevel2Or3Image(const PreparedPsDibPage& page)
{
	const char* colorSpace =
		(page.pixelFlavor == PsPixelFlavor::Bgr24 ||
			page.pixelFlavor == PsPixelFlavor::Bgra32)
		? "/DeviceRGB"
		: "/DeviceGray";

	const char* filterChain = nullptr;

	if (options_.level == PsLevel::Level3)
	{
		filterChain = "/ASCII85Decode filter /FlateDecode filter";
	}
	else
	{
		filterChain = options_.useRunLength
			? "/ASCII85Decode filter /RunLengthDecode filter"
			: "/ASCII85Decode filter";
	}

	if (std::fprintf(file_,
		"%s setcolorspace\n"
		"<<\n"
		"  /ImageType 1\n"
		"  /Width %u\n"
		"  /Height %u\n"
		"  /BitsPerComponent %u\n"
		"  /Decode %s\n"
		"  /ImageMatrix [%u 0 0 -%u 0 %u]\n"
		"  /DataSource currentfile %s\n"
		">> image\n",
		colorSpace,
		page.width,
		page.height,
		BitsPerComponent(page),
		DecodeArray(page),
		page.width,
		page.height,
		page.height,
		filterChain) < 0)
	{
		return false;
	}

	if (options_.level == PsLevel::Level3)
		return WriteAscii85FlateImageData(page);

	if (options_.useRunLength)
		return WriteAscii85RunLengthImageData(page);

	return WriteAscii85ImageData(page);
}

bool PsSessionWriter::WriteAscii85FlateImageData(const PreparedPsDibPage& page)
{
	std::string raw;
	if (!BuildRawImageData(page, raw))
		return false;

	std::string flate;
	if (FlateEncode(std::string_view(raw.data(), raw.size()), flate) != Z_OK)
		return false;

	std::string encoded;
	if (ASCII85Encode(std::string_view(flate.data(), flate.size()), encoded) != 1)
		return false;

	if (!encoded.empty())
	{
		if (std::fwrite(encoded.data(), 1, encoded.size(), file_) != encoded.size())
			return false;
	}

	if (encoded.empty() || encoded.back() != '\n')
	{
		if (std::fputc('\n', file_) == EOF)
			return false;
	}

	return true;
}

bool PsSessionWriter::WriteLevel1Image(const PreparedPsDibPage& page)
{
	const uint32_t components = Components(page);
	const uint32_t bpc = BitsPerComponent(page);

	uint32_t rowBytes = 0;

	if (page.pixelFlavor == PsPixelFlavor::BW1)
		rowBytes = (page.width + 7u) / 8u;
	else
		rowBytes = page.width * components * (bpc / 8u);

	if (std::fprintf(file_,
		"/picstr %u string def\n"
		"%u %u %u\n"
		"[%u 0 0 -%u 0 %u]\n",
		rowBytes,
		page.width,
		page.height,
		bpc,
		page.width,
		page.height,
		page.height) < 0)
	{
		return false;
	}

	if (components == 1)
	{
		if (std::fprintf(file_,
			"{ currentfile picstr readhexstring pop } image\n") < 0)
			return false;
	}
	else
	{
		if (std::fprintf(file_,
			"{ currentfile picstr readhexstring pop } false 3 colorimage\n") < 0)
			return false;
	}

	return WriteHexImageData(page);
}
const uint8_t* PsSessionWriter::SourceRow(const PreparedPsDibPage& page, uint32_t y) const
{
	const uint32_t srcY = page.bottomUp ? (page.height - 1 - y) : y;
	return page.bits + static_cast<size_t>(srcY) * page.strideBytes;
}

bool PsSessionWriter::WriteHexByte(uint8_t v)
{
	static constexpr char hex[] = "0123456789ABCDEF";

	if (std::fputc(hex[(v >> 4) & 0x0F], file_) == EOF)
		return false;
	if (std::fputc(hex[v & 0x0F], file_) == EOF)
		return false;

	hexLineCount_ += 2;
	if (hexLineCount_ >= 78)
	{
		if (std::fputc('\n', file_) == EOF)
			return false;
		hexLineCount_ = 0;
	}

	return true;
}

bool PsSessionWriter::EndHexData()
{
	if (hexLineCount_ != 0)
	{
		if (std::fputc('\n', file_) == EOF)
			return false;
		hexLineCount_ = 0;
	}

	if (options_.level == PsLevel::Level2)
	{
		if (std::fputc('>', file_) == EOF)
			return false;
		if (std::fputc('\n', file_) == EOF)
			return false;
	}

	return true;
}

bool PsSessionWriter::WriteHexImageData(const PreparedPsDibPage& page)
{
	hexLineCount_ = 0;

	switch (page.pixelFlavor)
	{
		case PsPixelFlavor::BW1:
			if (!Write1bppHex(page))
				return false;
			break;

		case PsPixelFlavor::Gray8:
			if (!Write8bppGrayHex(page))
				return false;
			break;

		case PsPixelFlavor::Gray16:
			if (!Write16bppGrayHex(page))
				return false;
			break;

		case PsPixelFlavor::Bgr24:
			if (!Write24bppRgbHex(page))
				return false;
			break;

		case PsPixelFlavor::Bgra32:
			if (!Write32bppRgbHex(page))
				return false;
			break;
	}

	return EndHexData();
}

bool PsSessionWriter::Write1bppHex(const PreparedPsDibPage& page)
{
	const uint32_t rowBytes = (page.width + 7u) / 8u;

	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint8_t* src = SourceRow(page, y);

		for (uint32_t i = 0; i < rowBytes; ++i)
		{
			if (!WriteHexByte(src[i]))
				return false;
		}
	}

	return true;
}

bool PsSessionWriter::Write8bppGrayHex(const PreparedPsDibPage& page)
{
	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint8_t* src = SourceRow(page, y);

		for (uint32_t x = 0; x < page.width; ++x)
		{
			uint8_t gray = src[x];

			// If the 8-bpp DIB has a palette, honor it.
			if (page.palette && page.paletteEntries > gray)
			{
				const RGBQUAD& q = page.palette[gray];
				gray = static_cast<uint8_t>(
					(77u * q.rgbRed + 150u * q.rgbGreen + 29u * q.rgbBlue + 128u) >> 8);
			}

			if (!WriteHexByte(gray))
				return false;
		}
	}

	return true;
}

bool PsSessionWriter::Write16bppGrayHex(const PreparedPsDibPage& page)
{
	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint8_t* src = SourceRow(page, y);
		const auto* src16 = reinterpret_cast<const uint16_t*>(src);

		for (uint32_t x = 0; x < page.width; ++x)
		{
			const uint16_t v = src16[x];

			// PostScript 16-bit samples should be big-endian.
			const uint8_t hi = static_cast<uint8_t>((v >> 8) & 0xFF);
			const uint8_t lo = static_cast<uint8_t>(v & 0xFF);

			if (!WriteHexByte(hi)) return false;
			if (!WriteHexByte(lo)) return false;
		}
	}

	return true;
}

bool PsSessionWriter::Write24bppRgbHex(const PreparedPsDibPage& page)
{
	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint8_t* src = SourceRow(page, y);

		for (uint32_t x = 0; x < page.width; ++x)
		{
			const uint8_t b = src[x * 3 + 0];
			const uint8_t g = src[x * 3 + 1];
			const uint8_t r = src[x * 3 + 2];

			if (!WriteHexByte(r)) return false;
			if (!WriteHexByte(g)) return false;
			if (!WriteHexByte(b)) return false;
		}
	}

	return true;
}

bool PsSessionWriter::Write32bppRgbHex(const PreparedPsDibPage& page)
{
	for (uint32_t y = 0; y < page.height; ++y)
	{
		const uint8_t* src = SourceRow(page, y);

		for (uint32_t x = 0; x < page.width; ++x)
		{
			const uint8_t b = src[x * 4 + 0];
			const uint8_t g = src[x * 4 + 1];
			const uint8_t r = src[x * 4 + 2];

			if (!WriteHexByte(r)) return false;
			if (!WriteHexByte(g)) return false;
			if (!WriteHexByte(b)) return false;
		}
	}

	return true;
}

bool PsSessionWriter::BuildRawImageData(const PreparedPsDibPage& page, std::string& raw)
{
	raw.clear();

	switch (page.pixelFlavor)
	{
		case PsPixelFlavor::BW1:
		{
			const uint32_t rowBytes = (page.width + 7u) / 8u;
			raw.reserve(static_cast<size_t>(rowBytes) * page.height);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint8_t* src = SourceRow(page, y);
				raw.append(reinterpret_cast<const char*>(src), rowBytes);
			}
			return true;
		}

		case PsPixelFlavor::Gray8:
		{
			raw.reserve(static_cast<size_t>(page.width) * page.height);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint8_t* src = SourceRow(page, y);

				for (uint32_t x = 0; x < page.width; ++x)
				{
					uint8_t gray = src[x];

					if (page.palette && page.paletteEntries > gray)
					{
						const RGBQUAD& q = page.palette[gray];
						gray = static_cast<uint8_t>(
							(77u * q.rgbRed +
								150u * q.rgbGreen +
								29u * q.rgbBlue +
								128u) >> 8);
					}

					raw.push_back(static_cast<char>(gray));
				}
			}
			return true;
		}

		case PsPixelFlavor::Gray16:
		{
			raw.reserve(static_cast<size_t>(page.width) * page.height * 2);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint8_t* src = SourceRow(page, y);
				const auto* src16 = reinterpret_cast<const uint16_t*>(src);

				for (uint32_t x = 0; x < page.width; ++x)
				{
					const uint16_t v = src16[x];

					// PostScript wants 16-bit samples big-endian.
					raw.push_back(static_cast<char>((v >> 8) & 0xFF));
					raw.push_back(static_cast<char>(v & 0xFF));
				}
			}
			return true;
		}

		case PsPixelFlavor::Bgr24:
		{
			raw.reserve(static_cast<size_t>(page.width) * page.height * 3);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint8_t* src = SourceRow(page, y);

				for (uint32_t x = 0; x < page.width; ++x)
				{
					const uint8_t b = src[x * 3 + 0];
					const uint8_t g = src[x * 3 + 1];
					const uint8_t r = src[x * 3 + 2];

					raw.push_back(static_cast<char>(r));
					raw.push_back(static_cast<char>(g));
					raw.push_back(static_cast<char>(b));
				}
			}
			return true;
		}

		case PsPixelFlavor::Bgra32:
		{
			raw.reserve(static_cast<size_t>(page.width) * page.height * 3);

			for (uint32_t y = 0; y < page.height; ++y)
			{
				const uint8_t* src = SourceRow(page, y);

				for (uint32_t x = 0; x < page.width; ++x)
				{
					const uint8_t b = src[x * 4 + 0];
					const uint8_t g = src[x * 4 + 1];
					const uint8_t r = src[x * 4 + 2];

					raw.push_back(static_cast<char>(r));
					raw.push_back(static_cast<char>(g));
					raw.push_back(static_cast<char>(b));
				}
			}
			return true;
		}
	}

	return false;
}

bool PsSessionWriter::WriteAscii85ImageData(const PreparedPsDibPage& page)
{
	std::string raw;
	if (!BuildRawImageData(page, raw))
		return false;

	std::string encoded;
	if (!ASCII85Encode(std::string_view(raw.data(), raw.size()), encoded) != 0)
		return false;

	if (!encoded.empty())
	{
		if (std::fwrite(encoded.data(), 1, encoded.size(), file_) != encoded.size())
			return false;
	}

	if (encoded.empty() || encoded.back() != '\n')
	{
		if (std::fputc('\n', file_) == EOF)
			return false;
	}

	return true;
}

bool PsSessionWriter::WriteAscii85RunLengthImageData(const PreparedPsDibPage& page)
{
	std::string raw;
	if (!BuildRawImageData(page, raw))
		return false;

	std::string rle;
	PsRunLengthEncode(std::string_view(raw.data(), raw.size()), rle);

	std::string encoded;
	if (!ASCII85Encode(std::string_view(rle.data(), rle.size()), encoded) != 0)
		return false;

	if (!encoded.empty())
	{
		if (std::fwrite(encoded.data(), 1, encoded.size(), file_) != encoded.size())
			return false;
	}

	if (encoded.empty() || encoded.back() != '\n')
	{
		if (std::fputc('\n', file_) == EOF)
			return false;
	}

	return true;
}
////////////////////////////////////////////////////////
// ============================================================
// DTWAIN-style wrapper
// ============================================================

bool DTWAINPsOutput::OnFirstPage(const std::wstring& filename, const PsSessionOptions& options, const PreparedPsDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<PsSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	return writer_->WritePage(page);
}

bool DTWAINPsOutput::OnNextPage(const PreparedPsDibPage& page)
{
	if (!writer_)
		return false;

	return writer_->WritePage(page);
}

bool DTWAINPsOutput::OnLastPage()
{
	if (!writer_)
		return false;

	const bool ok = writer_->Close();
	writer_.reset();
	return ok;
}
