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
#ifndef WEBPWRITER_H
#define WEBPWRITER_H

#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <webp/encode.h>
#include <optional>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// WebP model
// DTWAIN contract:
//   input DIB is always 24-bpp or 32-bpp
// ============================================================

enum class WebPPixelFlavor
{
    Bgr24,
    Bgra32
};

struct PreparedWebPDibPage
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 0;
    uint32_t strideBytes = 0;
    bool bottomUp = true;

    WebPPixelFlavor pixelFlavor = WebPPixelFlavor::Bgr24;
    const uint8_t* bits = nullptr;
};

struct WebPTextMetadata
{
    std::string comment;
    std::string copyright;
    std::string author;
    std::string software;
};

struct WebPSessionOptions
{
    // Match FreeImage defaults
    bool lossless = false;
    float quality = 75.0f;
    int method = 4;
    bool exact = false;

    // Optional new DTWAIN metadata
    WebPTextMetadata text;
};

// ============================================================
// WebP memory sink
// ============================================================
struct WebPMemoryWriterContext
{
    std::vector<uint8_t> data;
};

// ============================================================
// WebP writer
// ============================================================
class WebPSessionWriter
{
    public:
        WebPSessionWriter() = default;
        ~WebPSessionWriter();
        WebPSessionWriter(const WebPSessionWriter&) = delete;
        WebPSessionWriter& operator=(const WebPSessionWriter&) = delete;
        bool Open(const std::wstring& filename, const WebPSessionOptions& options);
        bool SetPageInfo(const PreparedWebPDibPage& page);
        bool WriteCurrentPage();
        void Close();
        bool IsOpen() const noexcept;
        static std::optional<PreparedWebPDibPage> MakePreparedWebPDibPage(const dynarithmic::DibPageView& view);

    private:
        static bool ValidatePage(const PreparedWebPDibPage& page);
        bool ImportBgr24(WebPPicture& picture);
        bool ImportBgra32(WebPPicture& picture);
        bool WriteOutputFile(const std::vector<uint8_t>& data) const;
        static bool HasMetadata(const WebPTextMetadata& text);
        static std::string XmlEscape(const std::string& s);
        std::string BuildXmpPacket() const;
        bool AddMetadataWithMux(const std::vector<uint8_t>& encodedImage, std::vector<uint8_t>& finalImage) const;

    private:
        std::wstring filename_;
        WebPSessionOptions options_{};

        PreparedWebPDibPage currentPage_{};
        bool hasCurrentPage_ = false;
        bool isOpen_ = false;

        std::vector<uint8_t> rowBuffer_;
        std::vector<uint8_t> rgbBuffer_;
        std::vector<uint8_t> rgbaBuffer_;
};

// ============================================================
// DTWAIN-style wrapper
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
class DTWAINWebPOutput
{
    public:
        bool OnFirstPage(const std::wstring& filename, const WebPSessionOptions& options, const PreparedWebPDibPage& page);
        bool OnLastPage();
        bool IsOpen() const noexcept;

    private:
        std::unique_ptr<WebPSessionWriter> writer_;
};

#endif
