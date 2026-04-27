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
#include <webp/mux.h>
#include "webpwriter.h"

static int WebPWriterCallback(const uint8_t* data, size_t data_size, const WebPPicture* picture)
{
	if (!data || !picture || !picture->custom_ptr)
		return 0;

	auto* ctx = static_cast<WebPMemoryWriterContext*>(picture->custom_ptr);
	ctx->data.insert(ctx->data.end(), data, data + data_size);
	return 1;
}

std::optional<PreparedWebPDibPage> WebPSessionWriter::MakePreparedWebPDibPage(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedWebPDibPage page{};
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;

	switch (page.bitsPerPixel)
	{
		case 24:
			page.pixelFlavor = WebPPixelFlavor::Bgr24;
			break;

		case 32:
			page.pixelFlavor = WebPPixelFlavor::Bgra32;
			break;

		default:
			return std::nullopt;
	}

	return page;
}

////////////////////////////////////////////////////////////////
WebPSessionWriter::~WebPSessionWriter()
{
	Close();
}
bool WebPSessionWriter::Open(const std::wstring& filename, const WebPSessionOptions& options)
{
	if (isOpen_)
		return false;

	filename_ = filename;
	options_ = options;
	hasCurrentPage_ = false;
	isOpen_ = true;
	return true;
}

bool WebPSessionWriter::SetPageInfo(const PreparedWebPDibPage& page)
{
	if (!isOpen_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

bool WebPSessionWriter::WriteCurrentPage()
{
	if (!isOpen_ || !hasCurrentPage_)
		return false;

	WebPConfig config;
	if (!WebPConfigPreset(&config, WEBP_PRESET_DEFAULT, options_.quality))
		return false;

	config.lossless = options_.lossless ? 1 : 0;
	config.quality = options_.quality;
	config.method = options_.method;
	config.exact = options_.exact ? 1 : 0;

	if (!WebPValidateConfig(&config))
		return false;

	WebPPicture picture;
	if (!WebPPictureInit(&picture))
		return false;

	picture.width = static_cast<int>(currentPage_.width);
	picture.height = static_cast<int>(currentPage_.height);

	WebPMemoryWriterContext output;
	picture.writer = WebPWriterCallback;
	picture.custom_ptr = &output;

	bool ok = false;

	if (currentPage_.pixelFlavor == WebPPixelFlavor::Bgr24)
		ok = ImportBgr24(picture);
	else
		ok = ImportBgra32(picture);

	if (!ok)
	{
		WebPPictureFree(&picture);
		return false;
	}

	ok = (WebPEncode(&config, &picture) != 0);

	WebPPictureFree(&picture);

	if (!ok)
		return false;

	std::vector<uint8_t> finalOutput;
	if (!AddMetadataWithMux(output.data, finalOutput))
		return false;

	if (!WriteOutputFile(finalOutput))
		return false;

	hasCurrentPage_ = false;
	return true;
}

void WebPSessionWriter::Close()
{
	filename_.clear();
	rowBuffer_.clear();
	hasCurrentPage_ = false;
	isOpen_ = false;
}

bool WebPSessionWriter::IsOpen() const noexcept
{
	return isOpen_;
}

bool WebPSessionWriter::ValidatePage(const PreparedWebPDibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 24:
		case 32:
			return true;
		default:
			return false;
	}
}

bool WebPSessionWriter::ImportBgr24(WebPPicture& picture)
{
	// Convert DIB top-down rows into a contiguous RGB buffer.
	const uint32_t rowBytes = currentPage_.width * 3;
	rgbBuffer_.resize(static_cast<size_t>(rowBytes) * currentPage_.height);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		uint8_t* dst =
			rgbBuffer_.data() + static_cast<size_t>(y) * rowBytes;

		for (uint32_t x = 0; x < currentPage_.width; ++x)
		{
			dst[x * 3 + 0] = src[x * 3 + 2]; // R
			dst[x * 3 + 1] = src[x * 3 + 1]; // G
			dst[x * 3 + 2] = src[x * 3 + 0]; // B
		}
	}

	return WebPPictureImportRGB(&picture, rgbBuffer_.data(), static_cast<int>(rowBytes)) != 0;
}

bool WebPSessionWriter::ImportBgra32(WebPPicture& picture)
{
	// Convert DIB top-down rows into a contiguous RGBA buffer.
	const uint32_t rowBytes = currentPage_.width * 4;
	rgbaBuffer_.resize(static_cast<size_t>(rowBytes) * currentPage_.height);

	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY = currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* src =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		uint8_t* dst =
			rgbaBuffer_.data() + static_cast<size_t>(y) * rowBytes;

		for (uint32_t x = 0; x < currentPage_.width; ++x)
		{
			dst[x * 4 + 0] = src[x * 4 + 2]; // R
			dst[x * 4 + 1] = src[x * 4 + 1]; // G
			dst[x * 4 + 2] = src[x * 4 + 0]; // B
			dst[x * 4 + 3] = src[x * 4 + 3]; // A
		}
	}

	return WebPPictureImportRGBA(&picture, rgbaBuffer_.data(), static_cast<int>(rowBytes)) != 0;
}

bool WebPSessionWriter::WriteOutputFile(const std::vector<uint8_t>& data) const
{
	FILE* f = _wfopen(filename_.c_str(), L"wb");
	if (!f)
		return false;

	const bool ok = std::fwrite(data.data(), 1, data.size(), f) == data.size();

	std::fclose(f);
	return ok;
}

bool WebPSessionWriter::HasMetadata(const WebPTextMetadata& text)
{
	return !text.comment.empty() ||
		!text.copyright.empty() ||
		!text.author.empty() ||
		!text.software.empty();
}

std::string WebPSessionWriter::XmlEscape(const std::string& s)
{
	std::string out;
	out.reserve(s.size());
	for (char ch : s)
	{
		switch (ch)
		{
		case '&':  out += "&amp;";  break;
		case '<':  out += "&lt;";   break;
		case '>':  out += "&gt;";   break;
		case '\"': out += "&quot;"; break;
		case '\'': out += "&apos;"; break;
		default:   out += ch;       break;
		}
	}
	return out;
}

std::string WebPSessionWriter::BuildXmpPacket() const
{
	if (!HasMetadata(options_.text))
		return {};

	std::string xmp;
	xmp += "<?xpacket begin=\"\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>";
	xmp += "<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">";
	xmp += "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">";
	xmp += "<rdf:Description xmlns:dc=\"http://purl.org/dc/elements/1.1/\" "
		"xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\" "
		"xmlns:xmpRights=\"http://ns.adobe.com/xap/1.0/rights/\">";

	if (!options_.text.comment.empty())
	{
		xmp += "<dc:description><rdf:Alt><rdf:li xml:lang=\"x-default\">";
		xmp += XmlEscape(options_.text.comment);
		xmp += "</rdf:li></rdf:Alt></dc:description>";
	}

	if (!options_.text.author.empty())
	{
		xmp += "<dc:creator><rdf:Seq><rdf:li>";
		xmp += XmlEscape(options_.text.author);
		xmp += "</rdf:li></rdf:Seq></dc:creator>";
	}

	if (!options_.text.software.empty())
	{
		xmp += "<xmp:CreatorTool>";
		xmp += XmlEscape(options_.text.software);
		xmp += "</xmp:CreatorTool>";
	}

	if (!options_.text.copyright.empty())
	{
		xmp += "<dc:rights><rdf:Alt><rdf:li xml:lang=\"x-default\">";
		xmp += XmlEscape(options_.text.copyright);
		xmp += "</rdf:li></rdf:Alt></dc:rights>";

		xmp += "<xmpRights:Marked>True</xmpRights:Marked>";
	}

	xmp += "</rdf:Description>";
	xmp += "</rdf:RDF>";
	xmp += "</x:xmpmeta>";
	xmp += "<?xpacket end=\"w\"?>";
	return xmp;
}

bool WebPSessionWriter::AddMetadataWithMux(const std::vector<uint8_t>& encodedImage, std::vector<uint8_t>& finalImage) const
{
	finalImage.clear();

	if (!HasMetadata(options_.text))
	{
		finalImage = encodedImage;
		return true;
	}

	const std::string xmp = BuildXmpPacket();
	if (xmp.empty())
	{
		finalImage = encodedImage;
		return true;
	}

	WebPData imageData;
	WebPDataInit(&imageData);
	imageData.bytes = encodedImage.data();
	imageData.size = encodedImage.size();

	WebPMux* mux = WebPMuxNew();
	if (!mux)
		return false;

	constexpr int copyData = 1;

	if (WebPMuxSetImage(mux, &imageData, copyData) != WEBP_MUX_OK)
	{
		WebPMuxDelete(mux);
		return false;
	}

	WebPData xmpData;
	WebPDataInit(&xmpData);
	xmpData.bytes = reinterpret_cast<const uint8_t*>(xmp.data());
	xmpData.size = xmp.size();

	if (WebPMuxSetChunk(mux, "XMP ", &xmpData, copyData) != WEBP_MUX_OK)
	{
		WebPMuxDelete(mux);
		return false;
	}

	WebPData outputData;
	WebPDataInit(&outputData);

	if (WebPMuxAssemble(mux, &outputData) != WEBP_MUX_OK)
	{
		WebPMuxDelete(mux);
		return false;
	}

	finalImage.assign(outputData.bytes, outputData.bytes + outputData.size);

	WebPDataClear(&outputData);
	WebPMuxDelete(mux);
	return true;
}

/////////////////////////////////////////////////////////////
// ============================================================
// DTWAIN-style wrapper
// Model B for WebP:
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
bool DTWAINWebPOutput::OnFirstPage(const std::wstring& filename, const WebPSessionOptions& options, const PreparedWebPDibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<WebPSessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINWebPOutput::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINWebPOutput::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}
