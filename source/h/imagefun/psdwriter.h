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
#ifndef PSDWRITER_H
#define PSDWRITER_H

#include <memory>
#include <cstdio>
#include <cstdint>
#include <string>
#include <optional>
#include <vector>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// PSD model
// DTWAIN contract:
//   input DIB is always 24-bpp
//   flattened PSD only
//   raw image data only (compression = 0)
// ============================================================
struct PreparedPsdDibPage
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 24;
    uint32_t strideBytes = 0;
    bool bottomUp = true;
    const uint8_t* bits = nullptr;
};

struct PsdSessionOptions
{
    // Reserved for future use (e.g. PackBits RLE, image resources, metadata)
    bool useRle = false;

    // Optional PSD image-resource comment/caption.
    std::string comment;
};

// ============================================================
// PSD on-disk helpers
// ============================================================
namespace dynarithmic::psd
{
    inline bool write_u16_be(FILE* f, uint16_t v)
    {
        const uint8_t b[2] =
        {
            static_cast<uint8_t>((v >> 8) & 0xFF),
            static_cast<uint8_t>(v & 0xFF)
        };
        return std::fwrite(b, 1, 2, f) == 2;
    }

    inline bool write_u32_be(FILE* f, uint32_t v)
    {
        const uint8_t b[4] =
        {
            static_cast<uint8_t>((v >> 24) & 0xFF),
            static_cast<uint8_t>((v >> 16) & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF),
            static_cast<uint8_t>(v & 0xFF)
        };
        return std::fwrite(b, 1, 4, f) == 4;
    }
}

// ============================================================
// PSD writer
// 24-bpp DIB -> flattened PSD (RGB, 8 bits/channel, raw)
// ============================================================
class PsdSessionWriter
{
    public:
        PsdSessionWriter() = default;
        ~PsdSessionWriter();
        PsdSessionWriter(const PsdSessionWriter&) = delete;
        PsdSessionWriter& operator=(const PsdSessionWriter&) = delete;
        bool Open(const std::wstring& filename, const PsdSessionOptions& options);
        bool SetPageInfo(const PreparedPsdDibPage& page);
        bool WriteCurrentPage();
        void Close();
        bool IsOpen() const noexcept;
        static std::optional<PreparedPsdDibPage> MakePreparedPsdDibPage(const dynarithmic::DibPageView& view);

    private:
        static bool ValidatePage(const PreparedPsdDibPage& page);
        bool WriteHeader();
        bool WriteColorModeDataSection();
        bool WriteImageResourcesSection();
        bool WriteLayerAndMaskSection();
        bool WriteImageDataSection();
        bool WriteImageDataSectionRaw();
        bool WriteImageDataSectionRle();
        bool WriteChannelPlaneRaw(uint32_t bgrIndex);
        bool BuildChannelPlaneRle(uint32_t bgrIndex);
        void FillChannelRow(uint32_t y, uint32_t bgrIndex);
        static void EncodePackBitsRow(const uint8_t* src, uint32_t len,std::vector<uint8_t>& dst);

    private:
        FILE* file_ = nullptr;
        std::wstring filename_;
        PsdSessionOptions options_{};

        PreparedPsdDibPage currentPage_{};
        bool hasCurrentPage_ = false;

        std::vector<uint8_t> channelRow_;
        std::vector<uint8_t> packBitsRow_;
        std::vector<uint16_t> rleLengths_;
        std::vector<uint8_t> rleData_;
};

// ============================================================
// DTWAIN-style wrapper
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
class DTWAINPsdOutput
{
    public:
        bool OnFirstPage(const std::wstring& filename, const PsdSessionOptions& options, const PreparedPsdDibPage& page);
        bool OnLastPage();
        bool IsOpen() const noexcept;

    private:
        std::unique_ptr<PsdSessionWriter> writer_;
};
#endif
