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
#ifndef TGAWRITER_H
#define TGAWRITER_H

#include <string>
#include <vector>
#include <memory>
#include <cstdio>
#include <optional>
#include <windows.h>
#include "dibutil.h"
#include "imagefilewriterbase.h"

// ============================================================
// TGA model
// ============================================================

enum class TgaPixelFlavor
{
    Palette8,
    Gray8,
    Bgr24,
    Bgra32
};

struct PreparedTgaDibPage
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 0;
    uint32_t strideBytes = 0;
    bool bottomUp = true;

    TgaPixelFlavor pixelFlavor = TgaPixelFlavor::Bgr24;
    const uint8_t* bits = nullptr;

    const RGBQUAD* palette = nullptr;
    uint32_t paletteEntries = 0;
};

struct TgaSessionOptions
{
    bool useRle = false;
    std::string comment;
};

// ============================================================
// TGA header
// ============================================================

#pragma pack(push, 1)
struct TgaHeader
{
    uint8_t  id_length;
    uint8_t  color_map_type;
    uint8_t  image_type;
    uint16_t color_map_first_entry;
    uint16_t color_map_length;
    uint8_t  color_map_entry_size;
    uint16_t x_origin;
    uint16_t y_origin;
    uint16_t width;
    uint16_t height;
    uint8_t  pixel_depth;
    uint8_t  image_descriptor;
};
#pragma pack(pop)

static_assert(sizeof(TgaHeader) == 18, "TgaHeader must be 18 bytes");

// ============================================================
// TGA writer
// ============================================================
class TgaSessionWriter
{
public:
    TgaSessionWriter() = default;
    ~TgaSessionWriter();
    TgaSessionWriter(const TgaSessionWriter&) = delete;
    TgaSessionWriter& operator=(const TgaSessionWriter&) = delete;

    bool Open(const std::wstring& filename, const TgaSessionOptions& options);
    bool SetPageInfo(const PreparedTgaDibPage& page);
    bool WriteCurrentPage();
    void Close();
    bool IsOpen() const noexcept;
    static std::optional<PreparedTgaDibPage> MakePreparedTgaDibPage(const dynarithmic::DibPageView& view);

private:
    static bool ValidatePage(const PreparedTgaDibPage& page);
    TgaHeader BuildHeader() const;
    bool UsesColorMap() const;
    uint16_t GetColorMapLength() const;
    uint8_t GetImageType() const;
    uint8_t GetPixelDepth() const;
    uint8_t GetImageDescriptor() const;
    bool WriteColorMap();
    uint32_t PixelBytes() const;
    const uint8_t* GetRowPtr(uint32_t y) const;
    bool WriteImageRaw();
    bool WriteImageRle();
    bool PrepareRow(const uint8_t* src, uint8_t* dst, uint32_t rowBytes) const;
    static bool PixelsEqual(const uint8_t* a, const uint8_t* b, uint32_t pixelBytes);
    bool WriteRleRow(const uint8_t* row, uint32_t width, uint32_t pixelBytes);
    bool WriteExtensionAreaWithComments();

private:
    FILE* file_ = nullptr;
    std::wstring filename_;
    TgaSessionOptions options_{};

    PreparedTgaDibPage currentPage_{};
    bool hasCurrentPage_ = false;

    std::vector<uint8_t> rowBuffer_;
    std::vector<uint8_t> packetBuffer_;
};

// ============================================================
// DTWAIN-style wrapper
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================

class DTWAINTgaOutput
{
    public:
        bool OnFirstPage(const std::wstring& filename, const TgaSessionOptions& options, const PreparedTgaDibPage& page);
        bool OnLastPage();
        bool IsOpen() const noexcept;

    private:
        std::unique_ptr<TgaSessionWriter> writer_;
};


#endif
