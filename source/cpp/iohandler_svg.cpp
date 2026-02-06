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
#include <vector>
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <string>
#include <sstream>
#include <fstream>
#include <gdiplus.h>
#include <stdexcept>
#pragma comment(lib, "gdiplus.lib")
#include "ctldib.h"
#include "ctliface.h"
#include "base64encode.h"

extern "C" {
#include "miniz.h"
}

using namespace dynarithmic;

static std::pair<bool, int> SaveDIBAsSVG(const BITMAPINFOHEADER& bih, const uint8_t* bits, const std::string filename, bool isSVGZ);
static std::pair<bool, int> SaveSVGZ(const std::string& svgText, const std::string& filename);

int CTL_SVGIOHandler::WriteBitmap(LPCTSTR szFile, bool /*bOpenFile*/, int /*fhFile*/, DibMultiPageStruct* )
{
    HANDLE hDib = {};
    if (!m_pDib || !(hDib = m_pDib->GetHandle()))
        return DTWAIN_ERR_DIB;

    if (!IsValidBitDepth(DTWAIN_SVG, m_pDib->GetBitsPerPixel()))
        return DTWAIN_ERR_INVALID_BITDEPTH;

	DTWAINGlobalHandle_RAII handler(hDib);
	auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hDib));
	auto bitsOffsetVal = (BYTE *)pbi + m_pDib->GetBitsOffset().get();
	auto ret = SaveDIBAsSVG(*pbi, bitsOffsetVal, StringConversion::Convert_NativePtr_To_Ansi(szFile).c_str(), m_isSVGZ);
	return ret.second;
}

class GdiplusInit
{
	public:
		GdiplusInit() { Gdiplus::GdiplusStartup(&token, &input, nullptr); }
		~GdiplusInit() { Gdiplus::GdiplusShutdown(token); }
	private:
		ULONG_PTR token{};
		Gdiplus::GdiplusStartupInput input{};
};

bool DIBToPNGBytes(const BITMAPINFOHEADER& bih, const uint8_t* bits, std::vector<uint8_t>& out)
{
	GdiplusInit gdiplus;

	BITMAPINFO bi{};
	memcpy(&bi.bmiHeader, &bih, sizeof(BITMAPINFOHEADER));

	Gdiplus::Bitmap bmp(&bi, const_cast<uint8_t*>(bits));

	CLSID pngClsid{};
	UINT num = 0, size = 0;
	Gdiplus::GetImageEncodersSize(&num, &size);

	std::vector<uint8_t> buf(size);
	auto* encoders = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buf.data());
	Gdiplus::GetImageEncoders(num, size, encoders);

	for (UINT i = 0; i < num; ++i)
	{
		if (wcscmp(encoders[i].MimeType, L"image/png") == 0)
		{
			pngClsid = encoders[i].Clsid;
			break;
		}
	}

	IStream* stream = nullptr;
	if (CreateStreamOnHGlobal(nullptr, TRUE, &stream) != S_OK)
		return false;

	if (bmp.Save(stream, &pngClsid, nullptr) != Gdiplus::Ok)
	{
		stream->Release();
		return false;
	}

	STATSTG stat{};
	stream->Stat(&stat, STATFLAG_NONAME);

	out.resize(static_cast<std::size_t>(stat.cbSize.QuadPart));

	LARGE_INTEGER zero{};
	stream->Seek(zero, STREAM_SEEK_SET, nullptr);

	ULONG read = 0;
	stream->Read(out.data(), static_cast<ULONG>(out.size()), &read);
	stream->Release();

	return true;
}

bool IsLikelyLineArt(const BITMAPINFOHEADER& bih)
{
	if (bih.biBitCount == 1) return true;
	if (bih.biBitCount <= 8 && bih.biClrUsed <= 16) return true;
	return false;
}

std::string VectorizeMonochromeToSVG(const uint8_t* bits, int width, int height, int stride, bool bottomUp)
{
	std::ostringstream svg;

	svg << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""
		<< width << "\" height=\"" << height << "\">\n";

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

std::string RasterToSVG(const BITMAPINFOHEADER& bih, const uint8_t* bits)
{
	std::vector<uint8_t> png;
	if (!DIBToPNGBytes(bih, bits, png))
		return {};

	std::string b64;
	Base64Encode(png.data(), b64, png.size());

	std::ostringstream s;
	s << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\""
		<< bih.biWidth << "\" height=\"" << abs(bih.biHeight) << "\">";

	s << "<image width=\"100%\" height=\"100%\" href=\"data:image/png;base64,"
		<< b64 << "\"/>";

	s << "</svg>";
	return s.str();
}

std::pair<bool, int> SaveDIBAsSVG(const BITMAPINFOHEADER& bih, const uint8_t* bits, const std::string filename, bool isSVGZ)
{
	std::string svg;

	if (IsLikelyLineArt(bih) && bih.biBitCount == 1)
	{
		int stride = ((bih.biWidth + 31) / 32) * 4;
		bool bottomUp = (bih.biHeight > 0);
		svg = VectorizeMonochromeToSVG(bits, bih.biWidth, abs(bih.biHeight), stride, bottomUp);
	}
	else
	{
		svg = RasterToSVG(bih, bits);
	}

	if (svg.empty()) 
		return { false, DTWAIN_ERR_FILEWRITE };

	std::pair<bool, int> retValue = {};
	if (isSVGZ)
		retValue = SaveSVGZ(svg, filename);
	else
	{
		std::ofstream f(filename, std::ios::binary);
		f.write(svg.data(), svg.size());
		if (f.good())
			return { true, DTWAIN_NO_ERROR };
		return { false, DTWAIN_ERR_FILEWRITE };
	}

	return retValue;
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
	static void Destroy(void *p)
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
	if ( f.good())
		return { true, DTWAIN_NO_ERROR }; 
	return { false, DTWAIN_ERR_FILEWRITE };
}

#if 0
// ---------------- Example usage ----------------
#ifdef SVGZ_MINIZ_TEST
#include <iostream>

int main()
{
	std::string svg = R"(
<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100">
  <rect x="10" y="10" width="80" height="80" fill="black"/>
</svg>
)";

	if (SaveSVGZ(svg, L"test.svgz"))
		std::cout << "SVGZ written successfully\n";
	else
		std::cout << "Failed to write SVGZ\n";
}
#endif
#endif