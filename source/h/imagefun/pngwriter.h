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
#ifndef PNGWRITER_H
#define PNGWRITER_H

#include <png.h>
#include <windows.h>
#include <string>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <optional>
#include "dtwaindefs.h"
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// Prepared page (already resampled by DTWAIN before reaching PNG)
// Supported here:
//   8  bpp grayscale
//   16 bpp grayscale
//   24 bpp BGR DIB
// ============================================================
enum class PngPixelFlavor
{
    Gray8,
    Palette8,
    Gray16,
    Bgr24
};

struct PreparedPngDibPage
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 0;
    uint32_t strideBytes = 0;
    bool bottomUp = true;

    PngPixelFlavor pixelFlavor = PngPixelFlavor::Gray8;
    const uint8_t* bits = nullptr;

    // For Palette8 only
    const RGBQUAD* palette = nullptr;
    uint32_t paletteEntries = 0;

    double xDpi = 96.0;
    double yDpi = 96.0;
};

struct PngTextMetadata
{
    std::string software;
    std::string copyright;
    std::string author;
    std::string description;
    std::string comment;
};

struct PngSessionOptions
{
    PngTextMetadata text;
    int compressionLevel = PNG_Z_DEFAULT_COMPRESSION;
};

class PngSessionWriter
{
    public:
        PngSessionWriter() = default;
        ~PngSessionWriter();
        PngSessionWriter(const PngSessionWriter&) = delete;
        PngSessionWriter& operator=(const PngSessionWriter&) = delete;
        bool Open(const std::wstring& filename, const PngSessionOptions& sessionOptions);
        bool SetPageInfo(const PreparedPngDibPage& page);
        std::pair<bool,int> WriteCurrentPage();
        void Close();
        bool IsOpen() const noexcept;
        static std::optional<PreparedPngDibPage> MakePreparedPngDibPage(const dynarithmic::DibPageView& view);

    private:
        bool ValidateCurrentPage() const;
        void prepare_text_chunks();
        void prepare_row_pointers();
        bool write_current_page_guarded();
        void write_current_page_impl_noexcept();
        void prepare_palette();

    private:
        FILE* file_ = nullptr;
        png_structp png_ptr_ = nullptr;
        png_infop info_ptr_ = nullptr;

        std::wstring filename_;
        PngSessionOptions sessionOptions_{};
        bool isOpen_ = false;

        PreparedPngDibPage currentPage_{};
        bool hasCurrentPage_ = false;

        std::vector<png_bytep> rowPointers_;
        std::vector<png_text> textChunks_;
        std::vector<png_color> pngPalette_;
};

class DTWAINPngOutput
{
    public:
        std::pair<bool, int> OnFirstPage(const std::wstring& filename, const PngSessionOptions& sessionOptions,
                                         const PreparedPngDibPage& page);
        std::pair<bool, int> OnLastPage();
        bool IsOpen() const noexcept;
    private:
        std::unique_ptr<PngSessionWriter> writer_;
};
#endif
