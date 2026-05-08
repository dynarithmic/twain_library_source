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

#include "jpeg2kwriter.h"
#include "ctlstringconversion.h"

#ifdef _MSC_VER
	#pragma warning (disable : 4244)
#endif

std::optional<PreparedJpeg2000DibPage> Jpeg2000SessionWriter::MakePreparedJpeg2000Page(const dynarithmic::DibPageView& view)
{
	if (!view.bits)
		return std::nullopt;

	PreparedJpeg2000DibPage page{};
	page.width = view.width;
	page.height = view.height;
	page.bitsPerPixel = view.bitsPerPixel;
	page.strideBytes = view.strideBytes;
	page.bottomUp = view.bottomUp;
	page.bits = view.bits;

	switch (page.bitsPerPixel)
	{
		case 8:
			page.pixelFlavor = Jpeg2000PixelFlavor::Gray8;
			break;

		case 16:
			page.pixelFlavor = Jpeg2000PixelFlavor::Gray16;
			break;

		case 24:
			page.pixelFlavor = Jpeg2000PixelFlavor::Bgr24;
			break;

		case 32:
			page.pixelFlavor = Jpeg2000PixelFlavor::Bgra32;
			break;

		default:
			return std::nullopt;
	}

	return page;
}

Jpeg2000SessionWriter::~Jpeg2000SessionWriter()
{
	Close();
}

bool Jpeg2000SessionWriter::Open(const std::wstring& filename, const Jpeg2000SessionOptions& options)
{
	if (isOpen_)
		return false;

	filename_ = filename;
	options_ = options;
	hasCurrentPage_ = false;
	isOpen_ = true;
	return true;
}

bool Jpeg2000SessionWriter::SetPageInfo(const PreparedJpeg2000DibPage& page)
{
	if (!isOpen_)
		return false;

	if (!ValidatePage(page))
		return false;

	currentPage_ = page;
	hasCurrentPage_ = true;
	return true;
}

std::string Jpeg2000SessionWriter::BuildJpeg2000Comment() const
{
	std::string s;
	auto add_line = [&](const char* key, const std::string& value)
		{
			if (value.empty())
				return;
			if (!s.empty())
				s += "\n";
			s += key;
			s += ": ";
			s += value;
		};

	add_line("Software", options_.text.software);
	add_line("Copyright", options_.text.copyright);
	add_line("Author", options_.text.author);
	add_line("Comment", options_.text.comment);
	return s;
}

bool Jpeg2000SessionWriter::WriteCurrentPage()
{
	if (!isOpen_ || !hasCurrentPage_)
		return false;

	opj_cparameters_t params{};
	opj_set_default_encoder_parameters(&params);

	if (options_.compressionRate <= 0.0f)
		params.tcp_rates[0] = 16.0f;
	else
		params.tcp_rates[0] = options_.compressionRate;

	params.tcp_numlayers = 1;
	params.cp_disto_alloc = 1;
	params.numresolution = options_.numResolutions;

	// Match FreeImage behavior for 3-component images
	params.tcp_mct =
		(currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Bgr24 ||
			currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Bgra32) ? 1 : 0;

	const OPJ_CODEC_FORMAT format = options_.useJP2Container ? OPJ_CODEC_JP2 : OPJ_CODEC_J2K;

	opj_codec_t* codec = opj_create_compress(format);
	if (!codec)
		return false;

	// Optional message handlers can be added here later:
	// opj_set_info_handler(codec, ...)
	// opj_set_warning_handler(codec, ...)
	// opj_set_error_handler(codec, ...)

	opj_image_t* image = CreateOpenJpegImage();
	if (!image)
	{
		opj_destroy_codec(codec);
		return false;
	}

	std::string commentStorage = BuildJpeg2000Comment();
	if (!commentStorage.empty())
		params.cp_comment = const_cast<char*>(commentStorage.c_str());

	if (!opj_setup_encoder(codec, &params, image))
	{
		opj_image_destroy(image);
		opj_destroy_codec(codec);
		return false;
	}

	if (options_.numThreads > 0)
	{
		opj_codec_set_threads(codec, options_.numThreads);
	}

	const std::string narrow = dynarithmic::StringConversion::Convert_Wide_To_Ansi(filename_);
	opj_stream_t* stream =
		opj_stream_create_default_file_stream(narrow.c_str(), OPJ_FALSE);

	if (!stream)
	{
		opj_image_destroy(image);
		opj_destroy_codec(codec);
		return false;
	}

	bool ok = true;

	if (!opj_start_compress(codec, image, stream))
		ok = false;

	if (ok && !opj_encode(codec, stream))
		ok = false;

	if (ok && !opj_end_compress(codec, stream))
		ok = false;

	opj_stream_destroy(stream);
	opj_image_destroy(image);
	opj_destroy_codec(codec);

	if (!ok)
		return false;

	hasCurrentPage_ = false;
	return true;
}

void Jpeg2000SessionWriter::Close()
{
	filename_.clear();
	hasCurrentPage_ = false;
	isOpen_ = false;
}

bool Jpeg2000SessionWriter::IsOpen() const noexcept
{
	return isOpen_;
}

bool Jpeg2000SessionWriter::ValidatePage(const PreparedJpeg2000DibPage& page)
{
	if (page.width == 0 || page.height == 0 || !page.bits || page.strideBytes == 0)
		return false;

	switch (page.bitsPerPixel)
	{
		case 8:
		case 16:
		case 24:
		case 32:
			return true;
		default:
			return false;
	}
}

opj_image_t* Jpeg2000SessionWriter::CreateOpenJpegImage() const
{
	const bool isColor =
		currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Bgr24 ||
		currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Bgra32;

	const OPJ_COLOR_SPACE colorSpace =
		isColor ? OPJ_CLRSPC_SRGB : OPJ_CLRSPC_GRAY;

	const int numComponents = isColor ? 3 : 1;
	std::vector<opj_image_cmptparm_t> cmpt(static_cast<size_t>(numComponents));
	std::memset(cmpt.data(), 0, cmpt.size() * sizeof(opj_image_cmptparm_t));

	for (int i = 0; i < numComponents; ++i)
	{
		cmpt[i].dx = 1;
		cmpt[i].dy = 1;
		cmpt[i].w = currentPage_.width;
		cmpt[i].h = currentPage_.height;
		cmpt[i].sgnd = 0;

		switch (currentPage_.pixelFlavor)
		{
			case Jpeg2000PixelFlavor::Gray8:
				cmpt[i].prec = 8;
				break;
			case Jpeg2000PixelFlavor::Gray16:
				cmpt[i].prec = 16;
				break;
			case Jpeg2000PixelFlavor::Bgr24:
			case Jpeg2000PixelFlavor::Bgra32:
				cmpt[i].prec = 8;
				break;
		}
	}

	opj_image_t* image = opj_image_create(numComponents, cmpt.data(), colorSpace);
	if (!image)
		return nullptr;

	image->x0 = 0;
	image->y0 = 0;
	image->x1 = currentPage_.width;
	image->y1 = currentPage_.height;

	FillOpenJpegImage(image);
	return image;
}

void Jpeg2000SessionWriter::FillOpenJpegImage(opj_image_t* image) const
{
	for (uint32_t y = 0; y < currentPage_.height; ++y)
	{
		const uint32_t srcY =
			currentPage_.bottomUp
			? (currentPage_.height - 1 - y)
			: y;

		const uint8_t* row =
			currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;

		if (currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Gray8)
		{
			int* gray = image->comps[0].data;
			for (uint32_t x = 0; x < currentPage_.width; ++x)
				gray[y * currentPage_.width + x] = row[x];
		}
		else if (currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Gray16)
		{
			int* gray = image->comps[0].data;
			const uint16_t* row16 = reinterpret_cast<const uint16_t*>(row);
			for (uint32_t x = 0; x < currentPage_.width; ++x)
				gray[y * currentPage_.width + x] = row16[x];
		}
		else if (currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Bgr24)
		{
			int* r = image->comps[0].data;
			int* g = image->comps[1].data;
			int* b = image->comps[2].data;

			for (uint32_t x = 0; x < currentPage_.width; ++x)
			{
				const uint8_t* px = row + x * 3;
				b[y * currentPage_.width + x] = px[0];
				g[y * currentPage_.width + x] = px[1];
				r[y * currentPage_.width + x] = px[2];
			}
		}
		else if (currentPage_.pixelFlavor == Jpeg2000PixelFlavor::Bgra32)
		{
			int* r = image->comps[0].data;
			int* g = image->comps[1].data;
			int* b = image->comps[2].data;

			for (uint32_t x = 0; x < currentPage_.width; ++x)
			{
				const uint8_t* px = row + x * 4;
				b[y * currentPage_.width + x] = px[0];
				g[y * currentPage_.width + x] = px[1];
				r[y * currentPage_.width + x] = px[2];
			}
		}
	}
}


bool DTWAINJpeg2000Output::OnFirstPage(const std::wstring& filename,const Jpeg2000SessionOptions& options,const PreparedJpeg2000DibPage& page)
{
	if (writer_)
		return false;

	writer_ = std::make_unique<Jpeg2000SessionWriter>();
	if (!writer_->Open(filename, options))
	{
		writer_.reset();
		return false;
	}

	if (!writer_->SetPageInfo(page))
		return false;

	return writer_->WriteCurrentPage();
}

bool DTWAINJpeg2000Output::OnLastPage()
{
	if (!writer_)
		return false;

	writer_.reset();
	return true;
}

bool DTWAINJpeg2000Output::IsOpen() const noexcept
{
	return writer_ != nullptr && writer_->IsOpen();
}

