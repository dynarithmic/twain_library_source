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
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "svgwriter.h"
#include "dtwaindefs.h"
#include "ctlstringconversion.h"
#include "base64encode.h"
#include <gdiplus.h>
#include <stdexcept>
#include <fstream>
#include "dtwain_raii.h"

#pragma comment(lib, "gdiplus.lib")

extern "C" {
#include "miniz.h"
}

#ifdef _MSC_VER
	#pragma warning (disable : 4786)
	#pragma warning (disable : 4611)
#endif

using namespace dynarithmic;
//////////////////////////////////////////////////////////////////////////////////////////
class GdiplusInit
{
	public:
		GdiplusInit() { Gdiplus::GdiplusStartup(&token, &input, nullptr); }
		~GdiplusInit() { Gdiplus::GdiplusShutdown(token); }
	private:
		ULONG_PTR token{};
		Gdiplus::GdiplusStartupInput input{};
};

static bool DIBToPNGBytes(const BITMAPINFOHEADER& bih, const uint8_t* dibBase, std::vector<uint8_t>& out)
{
	GdiplusInit gdiplus;

	const bool hasPalette = (bih.biCompression == BI_RGB && bih.biBitCount <= 8);

	uint32_t paletteEntries = 0;
	if (hasPalette)
	{
		paletteEntries = bih.biClrUsed ? bih.biClrUsed : (1u << bih.biBitCount);
	}

	const size_t bmiSize =
		sizeof(BITMAPINFOHEADER) + paletteEntries * sizeof(RGBQUAD);

	std::vector<uint8_t> bmiStorage(bmiSize, 0);
	auto* bi = reinterpret_cast<BITMAPINFO*>(bmiStorage.data());

	// Copy header
	std::memcpy(&bi->bmiHeader, &bih, sizeof(BITMAPINFOHEADER));

	// Copy palette if present
	if (paletteEntries > 0)
	{
		const auto* srcPalette =
			reinterpret_cast<const RGBQUAD*>(
				reinterpret_cast<const uint8_t*>(&bih) + sizeof(BITMAPINFOHEADER));

		std::memcpy(bi->bmiColors,
			srcPalette,
			paletteEntries * sizeof(RGBQUAD));
	}

	Gdiplus::Bitmap bmp(bi, const_cast<uint8_t*>(dibBase));

	CLSID pngClsid{};
	UINT num = 0, size = 0;
	Gdiplus::GetImageEncodersSize(&num, &size);

	std::vector<uint8_t> buf(size);
	auto* encoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buf.data());
	Gdiplus::GetImageEncoders(num, size, encoders);

	bool foundPng = false;
	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(encoders[i].MimeType, L"image/png") == 0)
		{
			pngClsid = encoders[i].Clsid;
			foundPng = true;
			break;
		}
	}

	if (!foundPng)
		return false;

	IStream* stream = nullptr;
	if (CreateStreamOnHGlobal(nullptr, TRUE, &stream) != S_OK)
		return false;

	const auto saveStatus = bmp.Save(stream, &pngClsid, nullptr);
	if (saveStatus != Gdiplus::Ok)
	{
		stream->Release();
		return false;
	}

	STATSTG stat{};
	if (stream->Stat(&stat, STATFLAG_NONAME) != S_OK)
	{
		stream->Release();
		return false;
	}

	out.resize(static_cast<std::size_t>(stat.cbSize.QuadPart));

	LARGE_INTEGER zero{};
	if (stream->Seek(zero, STREAM_SEEK_SET, nullptr) != S_OK)
	{
		stream->Release();
		return false;
	}

	ULONG read = 0;
	if (stream->Read(out.data(), static_cast<ULONG>(out.size()), &read) != S_OK ||
		read != out.size())
	{
		stream->Release();
		return false;
	}

	stream->Release();
	return true;
}

static bool IsLikelyLineArt(const BITMAPINFOHEADER& bih)
{
	if (bih.biBitCount == 1) return true;
	if (bih.biBitCount <= 8 && bih.biClrUsed <= 16) return true;
	return false;
}

static std::string CreateMetaData(std::string& comment)
{
	std::string sMetaData = "    <metadata>\n";
	sMetaData += "      <rdf:RDF>\n";
	sMetaData += "        <rdf:Description>\n";
	sMetaData += "          <dc:creator>" + comment + "</dc:creator>\n";
	sMetaData += "        </rdf:Description>\n";
	sMetaData += "      </rdf:RDF>\n";
	sMetaData += "    </metadata>";
	return sMetaData;
}

std::string VectorizeMonochromeToSVG(const uint8_t* bits, int width, int height, int stride, bool bottomUp, std::string& comment)
{
	std::ostringstream svg;

	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\"" <<
		" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"" <<
		" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" " <<
		"width = \"" << width << "\" height=\"" << height << "\">\n";
	svg << CreateMetaData(comment) + "\n";
	for (int y = 0; y < height; ++y)
	{
		// DIBs are usually bottom-up when biHeight > 0
		int srcY = bottomUp ? (height - 1 - y) : y;
		const uint8_t* row = bits + srcY * stride;

		int x = 0;
		while (x < width)
		{
			int byteIndex = x / 8;
			int bitIndex = 7 - (x % 8); // MSB is leftmost in DIB 1bpp

			// invert colors: 1 = black, 0 = white
			bool black = !((row[byteIndex] >> bitIndex) & 1);

			if (!black)
			{
				++x;
				continue;
			}

			int start = x;

			while (x < width)
			{
				int bi = x / 8;
				int bj = 7 - (x % 8);
				bool pixelBlack = !((row[bi] >> bj) & 1);
				if (!pixelBlack)
					break;
				++x;
			}

			int run = x - start;

			svg << "<rect x=\"" << start
				<< "\" y=\"" << y
				<< "\" width=\"" << run
				<< "\" height=\"1\" fill=\"black\"/>\n";
		}
	}

	svg << "</svg>";
	return svg.str();
}

std::string RasterToSVG(const BITMAPINFOHEADER& bih, const uint8_t* bits, std::string& comment)
{
	std::vector<uint8_t> png;
	if (!DIBToPNGBytes(bih, bits, png))
		return {};

	std::string b64;
	Base64Encode(png.data(), b64, png.size());

	std::ostringstream svg;

	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\"" <<
		" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"" <<
		" xmlns:dc=\"http://purl.org/dc/elements/1.1/\" " <<
		"width = \"" << bih.biWidth << "\" height=\"" << abs(bih.biHeight) << "\">\n";

	svg << CreateMetaData(comment) << "\n";

	svg << "<image width=\"100%\" height=\"100%\" href=\"data:image/png;base64,"
		<< b64 << "\"/>";

	svg << "</svg>";
	return svg.str();
}

static void WriteLE32(std::vector<unsigned char>& out, uint32_t v)
{
	out.push_back((unsigned char)(v & 0xFF));
	out.push_back((unsigned char)((v >> 8) & 0xFF));
	out.push_back((unsigned char)((v >> 16) & 0xFF));
	out.push_back((unsigned char)((v >> 24) & 0xFF));
}

struct mz_DestroyTraits
{
	static void Destroy(void* p)
	{
		if (p)
			mz_free(p);
	}
};

typedef DTWAIN_RAII<void*, mz_DestroyTraits> mz_RAII;

std::pair<bool, int> SaveSVGZ(const std::string& svgText, const std::string& filename)
{
	size_t deflateSize = 0;

	// Compress entire SVG text to RAW DEFLATE in heap memory
	void* pDeflateHeap = tdefl_compress_mem_to_heap(
		svgText.data(), // input buffer
		svgText.size(), // input size
		&deflateSize, // output size pointer
		9 // compression level, RAW DEFLATE
	);

	if (!pDeflateHeap || deflateSize == 0)
		return { false, DTWAIN_ERR_MEM };

	mz_RAII raii(pDeflateHeap);

	std::vector<unsigned char> out;

	// --- GZIP header ---
	out.push_back(0x1F); // ID1
	out.push_back(0x8B); // ID2
	out.push_back(0x08); // CM = deflate
	out.push_back(0x00); // FLG
	WriteLE32(out, 0); // MTIME
	out.push_back(0x00); // XFL
	out.push_back(0xFF); // OS

	// --- Append raw DEFLATE payload ---
	out.insert(out.end(), (unsigned char*)pDeflateHeap, (unsigned char*)pDeflateHeap + deflateSize);

	// --- CRC32 of original SVG text ---
	uint32_t crc = mz_crc32(0, reinterpret_cast<const unsigned char*>(svgText.data()), svgText.size());
	WriteLE32(out, crc);

	// --- ISIZE (original input size modulo 2^32) ---
	WriteLE32(out, static_cast<uint32_t>(svgText.size()));

	// --- Write to file ---
	std::ofstream f(filename, std::ios::binary);
	if (!f)
		return { false, DTWAIN_ERR_FILEWRITE };

	f.write(reinterpret_cast<const char*>(out.data()), out.size());
	if (f.good())
		return { true, DTWAIN_NO_ERROR };
	return { false, DTWAIN_ERR_FILEWRITE };
}
std::optional<PreparedSvgDibPage> SvgSessionWriter::MakePreparedSvgPage(const DibPageView& view)
{
	if (!view.bih || !view.bits)
		return std::nullopt;

	switch (view.bitsPerPixel)
	{
		case 1:
		case 8:
		case 16:
		case 24:
		case 32:
			break;

		default:
			return std::nullopt;
	}

	PreparedSvgDibPage page{};
	page.header = const_cast<LPBITMAPINFOHEADER>(view.bih);
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.offsetToBitsData = static_cast<uint32_t>(reinterpret_cast<const uint8_t*>(view.bits) - reinterpret_cast<const uint8_t*>(view.bih));
	return page;
}

bool SvgSessionWriter::Open(const std::wstring& filename, const SvgSessionOptions& options)
{
	if (isOpen_)
		return false;

	filename_ = filename;
	options_ = options;
	wrotePage_ = false;
	isOpen_ = true;
	return true;
}

bool SvgSessionWriter::WritePage()
{
	if (!isOpen_ || wrotePage_)
		return false;

	if (!ValidatePage(currentPage_))
		return false;

	const std::string filenameA = StringConversion::Convert_Native_To_Ansi(filename_.c_str());

	LPBITMAPINFOHEADER header = currentPage_.header;

	auto bitsOffsetVal = reinterpret_cast<BYTE*>(header) + currentPage_.offsetToBitsData;
	const bool ok = SaveDIBAsSVGEx(*header, bitsOffsetVal,
									filenameA.c_str(), options_.type == SvgOutputType::Svgz, options_.comment);

	if (!ok)
		return false;

	wrotePage_ = true;
	return true;
}

bool SvgSessionWriter::Close()
{
	filename_.clear();
	options_ = {};
	wrotePage_ = false;
	isOpen_ = false;
	return true;
}

bool SvgSessionWriter::IsOpen() const noexcept
{
	return isOpen_;
}

bool SvgSessionWriter::SetPageInfo(const PreparedSvgDibPage& page)
{
	if (filename_.empty())
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	return true;
}

bool SvgSessionWriter::ValidatePage(const PreparedSvgDibPage& page)
{
	if (!page.header || page.offsetToBitsData == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 1:
		case 8:
		case 16:
		case 24:
		case 32:
			return page.width > 0 && page.height > 0;

		default:
			return false;
	}
}

bool SvgSessionWriter::SaveDIBAsSVGEx(const BITMAPINFOHEADER& bih, const uint8_t* bits, const std::string filename, bool isSVGZ,
								  	  std::string& comment)
{
	std::string svg;
	if (IsLikelyLineArt(bih) && bih.biBitCount == 1)
	{
		int stride = ((bih.biWidth + 31) / 32) * 4;
		bool bottomUp = (bih.biHeight > 0);
		svg = VectorizeMonochromeToSVG(bits, bih.biWidth, abs(bih.biHeight), stride, bottomUp, comment);
	}
	else
	{
		svg = RasterToSVG(bih, bits, comment);
	}

	if (svg.empty())
		return false;

	std::pair<bool, int> retValue = {};
	if (isSVGZ)
		retValue = SaveSVGZ(svg, filename);
	else
	{

		std::ofstream f(filename, std::ios::binary);
		f.write(svg.data(), svg.size());
		if (f.good())
			return true;
		return false;
	}
	return true;
}

