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
#include "pnmwriter.h"

std::optional<PreparedPnmDibPage> PnmSessionWriter::MakePreparedPnmDibPage(const dynarithmic::DibPageView& view)
{
    if (!view.bits)
        return std::nullopt;

    PreparedPnmDibPage page{};
    page.width = view.width;
    page.height = view.height;
    page.bitsPerPixel = view.bitsPerPixel;
    page.strideBytes = view.strideBytes;
    page.bottomUp = view.bottomUp;
    page.bits = view.bits;

    switch (page.bitsPerPixel)
    {
        case 1:
            page.pixelFlavor = PnmPixelFlavor::BW1;
            break;
        case 8:
            page.pixelFlavor = PnmPixelFlavor::Gray8;
            break;
        case 16:
            page.pixelFlavor = PnmPixelFlavor::Gray16;
            break;
        case 24:
            page.pixelFlavor = PnmPixelFlavor::Bgr24;
            break;
        case 32:
            page.pixelFlavor = PnmPixelFlavor::Bgra32;
            break;
        default:
            return std::nullopt;
    }
    return page;
}

 ///////////////////////////////////////////////////////////
// ============================================================
// PNM writer
// ============================================================

PnmSessionWriter::~PnmSessionWriter()
{
    Close();
}

bool PnmSessionWriter::Open(const std::wstring& filename, const PnmSessionOptions& options)
{
    if (file_)
        return false;

    file_ = _wfopen(filename.c_str(), L"wb");
    if (!file_)
        return false;

    filename_ = filename;
    options_ = options;
    hasCurrentPage_ = false;
    return true;
}

bool PnmSessionWriter::SetPageInfo(const PreparedPnmDibPage& page)
{
    if (!file_)
        return false;

    if (!ValidatePage(page))
        return false;

    currentPage_ = page;
    hasCurrentPage_ = true;
    return true;
}

bool PnmSessionWriter::WriteCurrentPage()
{
    if (!file_ || !hasCurrentPage_)
        return false;

    if (!WriteHeader())
        return false;

    if (!WritePixels())
        return false;

    hasCurrentPage_ = false;
    return true;
}

void PnmSessionWriter::Close()
{
    if (file_)
    {
        std::fclose(file_);
        file_ = nullptr;
    }

    filename_.clear();
    rowBuffer_.clear();
    packedRow_.clear();
    hasCurrentPage_ = false;
}

bool PnmSessionWriter::IsOpen() const noexcept
{
    return file_ != nullptr;
}

bool PnmSessionWriter::ValidatePage(const PreparedPnmDibPage& page)
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

uint8_t PnmSessionWriter::ReverseBits(uint8_t v)
{
    v = static_cast<uint8_t>(((v & 0xF0) >> 4) | ((v & 0x0F) << 4));
    v = static_cast<uint8_t>(((v & 0xCC) >> 2) | ((v & 0x33) << 2));
    v = static_cast<uint8_t>(((v & 0xAA) >> 1) | ((v & 0x55) << 1));
    return v;
}

const char* PnmSessionWriter::Magic() const
{
    switch (currentPage_.pixelFlavor)
    {
        case PnmPixelFlavor::BW1:
            return options_.useRaw ? "P4" : "P1";
        case PnmPixelFlavor::Gray8:
        case PnmPixelFlavor::Gray16:
            return options_.useRaw ? "P5" : "P2";
        case PnmPixelFlavor::Bgr24:
        case PnmPixelFlavor::Bgra32:
            return options_.useRaw ? "P6" : "P3";
    }
    return "P6";
}

uint32_t PnmSessionWriter::MaxValue() const
{
    switch (currentPage_.pixelFlavor)
    {
        case PnmPixelFlavor::Gray16:
            return 65535;
        case PnmPixelFlavor::BW1:
            return 1;
        default:
            return 255;
    }
}

bool PnmSessionWriter::WriteHeader()
{
    if (std::fprintf(file_, "%s\n", Magic()) < 0)
        return false;

    if (!options_.comment.empty())
    {
        if (!WriteCommentLines(options_.comment))
            return false;
    }

    if (std::fprintf(file_, "%u %u\n",
        currentPage_.width,
        currentPage_.height) < 0)
    {
        return false;
    }

    if (currentPage_.pixelFlavor != PnmPixelFlavor::BW1)
    {
        if (std::fprintf(file_, "%u\n", MaxValue()) < 0)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteCommentLines(const std::string& text)
{
    size_t start = 0;

    while (start <= text.size())
    {
        size_t end = text.find_first_of("\r\n", start);
        std::string line;

        if (end == std::string::npos)
        {
            line = text.substr(start);
            start = text.size() + 1;
        }
        else
        {
            line = text.substr(start, end - start);

            // Handle CRLF as one newline.
            if (text[end] == '\r' &&
                end + 1 < text.size() &&
                text[end + 1] == '\n')
            {
                start = end + 2;
            }
            else
            {
                start = end + 1;
            }
        }

        if (line.empty())
            continue;

        // Avoid accidentally creating malformed nested comments.
        // Netpbm comments begin with '#'; we emit the marker ourselves.
        while (!line.empty() && line.front() == '#')
            line.erase(line.begin());

        if (std::fprintf(file_, "# %s\n", line.c_str()) < 0)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WritePixels()
{
    switch (currentPage_.pixelFlavor)
    {
        case PnmPixelFlavor::BW1:
            return options_.useRaw ? WritePbmRaw() : WritePbmPlain();

        case PnmPixelFlavor::Gray8:
            return options_.useRaw ? WriteGray8Raw() : WriteGray8Plain();

        case PnmPixelFlavor::Gray16:
            return options_.useRaw ? WriteGray16Raw() : WriteGray16Plain();

        case PnmPixelFlavor::Bgr24:
            return options_.useRaw ? WriteRgb24Raw() : WriteRgb24Plain();

        case PnmPixelFlavor::Bgra32:
            return options_.useRaw ? WriteRgba32AsRgbRaw() : WriteRgba32AsRgbPlain();
    }

    return false;
}

const uint8_t* PnmSessionWriter::GetSourceRow(uint32_t y) const
{
    const uint32_t srcY =
        currentPage_.bottomUp
        ? (currentPage_.height - 1 - y)
        : y;

    return currentPage_.bits + static_cast<size_t>(srcY) * currentPage_.strideBytes;
}

bool PnmSessionWriter::WritePbmPlain()
{
//    const uint32_t packedBytes = (currentPage_.width + 7u) / 8u;

    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            const uint8_t byte = src[x / 8];
            const uint8_t bit = static_cast<uint8_t>((byte >> (7 - (x % 8))) & 1);

            // PBM semantics:
            // 1 = black, 0 = white
            //
            // Typical DIB semantics for 1-bpp text pages:
            // 1 = white, 0 = black
            //
            // fixBilevelPolarity=true converts to visually correct PBM output.
            const uint8_t outVal = options_.fixBilevelPolarity
                ? static_cast<uint8_t>(1 - bit)
                : bit;

            if (std::fprintf(file_, "%u", static_cast<unsigned>(outVal)) < 0)
                return false;

            if (x + 1 < currentPage_.width)
            {
                if (std::fputc(' ', file_) == EOF)
                    return false;
            }
        }

        if (std::fputc('\n', file_) == EOF)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WritePbmRaw()
{
    const uint32_t packedBytes = (currentPage_.width + 7u) / 8u;
    packedRow_.resize(packedBytes);

    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        if (options_.fixBilevelPolarity)
        {
            for (uint32_t i = 0; i < packedBytes; ++i)
                packedRow_[i] = static_cast<uint8_t>(~src[i]);
        }
        else
        {
            std::memcpy(packedRow_.data(), src, packedBytes);
        }

        if (std::fwrite(packedRow_.data(), 1, packedBytes, file_) != packedBytes)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteGray8Plain()
{
    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            if (std::fprintf(file_, "%u", static_cast<unsigned>(src[x])) < 0)
                return false;

            if (x + 1 < currentPage_.width)
            {
                if (std::fputc(' ', file_) == EOF)
                    return false;
            }
        }

        if (std::fputc('\n', file_) == EOF)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteGray8Raw()
{
    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        if (std::fwrite(src, 1, currentPage_.width, file_) != currentPage_.width)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteGray16Plain()
{
    const uint16_t* src16 = nullptr;

    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);
        src16 = reinterpret_cast<const uint16_t*>(src);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            if (std::fprintf(file_, "%u", static_cast<unsigned>(src16[x])) < 0)
                return false;

            if (x + 1 < currentPage_.width)
            {
                if (std::fputc(' ', file_) == EOF)
                    return false;
            }
        }

        if (std::fputc('\n', file_) == EOF)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteGray16Raw()
{
    rowBuffer_.resize(static_cast<size_t>(currentPage_.width) * 2);

    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);
        const uint16_t* src16 = reinterpret_cast<const uint16_t*>(src);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            const uint16_t v = src16[x];

            // Netpbm raw 16-bit samples are big-endian.
            rowBuffer_[x * 2 + 0] = static_cast<uint8_t>((v >> 8) & 0xFF);
            rowBuffer_[x * 2 + 1] = static_cast<uint8_t>(v & 0xFF);
        }

        if (std::fwrite(rowBuffer_.data(), 1, rowBuffer_.size(), file_) != rowBuffer_.size())
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteRgb24Plain()
{
    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            const uint8_t b = src[x * 3 + 0];
            const uint8_t g = src[x * 3 + 1];
            const uint8_t r = src[x * 3 + 2];

            if (std::fprintf(file_, "%u %u %u",
                static_cast<unsigned>(r),
                static_cast<unsigned>(g),
                static_cast<unsigned>(b)) < 0)
            {
                return false;
            }

            if (x + 1 < currentPage_.width)
            {
                if (std::fputc(' ', file_) == EOF)
                    return false;
            }
        }

        if (std::fputc('\n', file_) == EOF)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteRgb24Raw()
{
    rowBuffer_.resize(static_cast<size_t>(currentPage_.width) * 3);

    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            rowBuffer_[x * 3 + 0] = src[x * 3 + 2];
            rowBuffer_[x * 3 + 1] = src[x * 3 + 1];
            rowBuffer_[x * 3 + 2] = src[x * 3 + 0];
        }

        if (std::fwrite(rowBuffer_.data(), 1, rowBuffer_.size(), file_) != rowBuffer_.size())
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteRgba32AsRgbPlain()
{
    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            const uint8_t b = src[x * 4 + 0];
            const uint8_t g = src[x * 4 + 1];
            const uint8_t r = src[x * 4 + 2];

            if (std::fprintf(file_, "%u %u %u",
                static_cast<unsigned>(r),
                static_cast<unsigned>(g),
                static_cast<unsigned>(b)) < 0)
            {
                return false;
            }

            if (x + 1 < currentPage_.width)
            {
                if (std::fputc(' ', file_) == EOF)
                    return false;
            }
        }

        if (std::fputc('\n', file_) == EOF)
            return false;
    }

    return true;
}

bool PnmSessionWriter::WriteRgba32AsRgbRaw()
{
    rowBuffer_.resize(static_cast<size_t>(currentPage_.width) * 3);

    for (uint32_t y = 0; y < currentPage_.height; ++y)
    {
        const uint8_t* src = GetSourceRow(y);

        for (uint32_t x = 0; x < currentPage_.width; ++x)
        {
            rowBuffer_[x * 3 + 0] = src[x * 4 + 2];
            rowBuffer_[x * 3 + 1] = src[x * 4 + 1];
            rowBuffer_[x * 3 + 2] = src[x * 4 + 0];
        }

        if (std::fwrite(rowBuffer_.data(), 1, rowBuffer_.size(), file_) != rowBuffer_.size())
            return false;
    }

    return true;
}
///////////////////////////////////////////////
// ============================================================
// DTWAIN-style wrapper
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
bool DTWAINPnmOutput::OnFirstPage(const std::wstring& filename, const PnmSessionOptions& options, const PreparedPnmDibPage& page)
{
    if (writer_)
        return false;

    writer_ = std::make_unique<PnmSessionWriter>();
    if (!writer_->Open(filename, options))
    {
        writer_.reset();
        return false;
    }

    if (!writer_->SetPageInfo(page))
        return false;

    return writer_->WriteCurrentPage();
}

bool DTWAINPnmOutput::OnLastPage()
{
    if (!writer_)
        return false;

    writer_.reset();
    return true;
}

bool DTWAINPnmOutput::IsOpen() const noexcept
{
    return writer_ != nullptr && writer_->IsOpen();
}

