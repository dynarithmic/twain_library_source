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
#ifndef GIFWRITER_H
#define GIFWRITER_H

#include <windows.h>
#include <gif_lib.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// Prepared page
// Supported here:
//   8-bpp indexed / grayscale DIB
// ============================================================

enum class GifPixelFlavor
{
    Indexed8,
    Gray8
};

struct PreparedGifDibPage
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 8;
    uint32_t strideBytes = 0;
    bool bottomUp = true;

    GifPixelFlavor pixelFlavor = GifPixelFlavor::Indexed8;
    const uint8_t* bits = nullptr;

    // Optional palette from DIB
    const RGBQUAD* palette = nullptr;
    uint32_t paletteEntries = 0;
};


struct GifTextMetadata
{
    std::string software;
    std::string copyright;
    std::string author;
    std::string description;
    std::string comment;
};

struct GifSessionOptions
{
    GifTextMetadata text;
};

// ============================================================
// GIF writer
// Single-image writer for DTWAIN-style first/last-page workflow
// ============================================================

class GifSessionWriter
{
    public:
        GifSessionWriter() = default;
        ~GifSessionWriter();
        GifSessionWriter(const GifSessionWriter&) = delete;
        GifSessionWriter& operator=(const GifSessionWriter&) = delete;

        bool Open(const std::wstring& filename, const GifSessionOptions& sessionOptions);
        bool SetPageInfo(const PreparedGifDibPage& page);
        bool WriteCurrentPage();
        void Close();
        bool IsOpen() const noexcept;
        static std::optional<PreparedGifDibPage> MakePreparedGifPage(const dynarithmic::DibPageView& view);

    private:
        static int gif_write_callback(GifFileType* gif, const GifByteType* data, int length);
        static bool ValidatePage(const PreparedGifDibPage& page);
        void prepare_color_map();
        std::string build_comment_text() const;
        bool write_comment_extensions();

    private:
        FILE* file_ = nullptr;
        GifFileType* gif_ = nullptr;
        ColorMapObject* colorMap_ = nullptr;

        std::wstring filename_;
        GifSessionOptions sessionOptions_{};

        PreparedGifDibPage currentPage_{};
        bool hasCurrentPage_ = false;

        std::vector<uint8_t> rowBuffer_;
};

class DTWAINGifOutput
{
    public:
        bool OnFirstPage(const std::wstring& filename, const GifSessionOptions& sessionOptions, const PreparedGifDibPage& page);
        bool OnLastPage();
        bool IsOpen() const noexcept;

    private:
        std::unique_ptr<GifSessionWriter> writer_;
};

#endif
