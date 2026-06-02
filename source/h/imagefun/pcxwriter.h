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
#ifndef PCXWRITER_H
#define PCXWRITER_H

#include "dibutil.h"
#include <memory>
#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>
#include <optional>
#include <windows.h>
#include "imagefilewriterbase.h"

// ============================================================
// PCX / DCX model
// ============================================================

enum class PcxPixelFlavor
{
    BW1,
    Indexed8,
    Bgr24
};

struct PreparedPcxDibPage
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 0;
    uint32_t strideBytes = 0;
    bool bottomUp = true;

    PcxPixelFlavor pixelFlavor = PcxPixelFlavor::Indexed8;
    const uint8_t* bits = nullptr;

    const RGBQUAD* palette = nullptr;
    uint32_t paletteEntries = 0;

    uint16_t xDpi = 300;
    uint16_t yDpi = 300;
};

struct PcxSessionOptions
{
    bool writeDcx = false;   // false = PCX, true = DCX
};

// ============================================================
// On-disk helpers
// ============================================================
namespace dynarithmic::pcx
{
#pragma pack(push, 1)
    struct PcxHeader
    {
        uint8_t  manufacturer;   // 0x0A
        uint8_t  version;        // 5
        uint8_t  encoding;       // 1 = RLE
        uint8_t  bitsPerPixel;   // per plane
        uint16_t xmin;
        uint16_t ymin;
        uint16_t xmax;
        uint16_t ymax;
        uint16_t hDpi;
        uint16_t vDpi;
        uint8_t  egaPalette[48];
        uint8_t  reserved1;
        uint8_t  colorPlanes;
        uint16_t bytesPerLine;
        uint16_t paletteType;
        uint16_t hScreenSize;
        uint16_t vScreenSize;
        uint8_t  filler[54];
    };
#pragma pack(pop)

    static_assert(sizeof(PcxHeader) == 128, "PCX header must be 128 bytes");

    inline bool write_u32_le(FILE* f, uint32_t v)
    {
        const uint8_t b[4] =
        {
            static_cast<uint8_t>(v & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF),
            static_cast<uint8_t>((v >> 16) & 0xFF),
            static_cast<uint8_t>((v >> 24) & 0xFF)
        };
        return std::fwrite(b, 1, 4, f) == 4;
    }

    inline uint16_t even_pad(uint16_t n)
    {
        return static_cast<uint16_t>((n + 1u) & ~1u);
    }

    inline void emit_rle_byte(std::vector<uint8_t>& out, uint8_t value, uint8_t count)
    {
        if (count > 1 || (value & 0xC0) == 0xC0)
        {
            out.push_back(static_cast<uint8_t>(0xC0 | count));
            out.push_back(value);
        }
        else
        {
            out.push_back(value);
        }
    }

    inline void encode_rle_row(const uint8_t* src, uint32_t len, std::vector<uint8_t>& out)
    {
        uint32_t i = 0;
        while (i < len)
        {
            uint8_t value = src[i];
            uint8_t run = 1;
            while ((i + run) < len && run < 63 && src[i + run] == value)
                ++run;

            emit_rle_byte(out, value, run);
            i += run;
        }
    }
}

// ============================================================
// Session writer
// ============================================================
class PcxSessionWriter
{
    public:
        PcxSessionWriter() = default;
        ~PcxSessionWriter();
        PcxSessionWriter(const PcxSessionWriter&) = delete;
        PcxSessionWriter& operator=(const PcxSessionWriter&) = delete;
        bool Open(const std::wstring& filename, const PcxSessionOptions& options);
        bool WritePage(const PreparedPcxDibPage& page);
        bool Close();
        static std::optional<PreparedPcxDibPage> MakePreparedPcxDibPage(const dynarithmic::DibPageView& view);

    private:
        static bool ValidatePage(const PreparedPcxDibPage& page);
        bool PatchDcxDirectory();
        bool WriteOnePcx(const PreparedPcxDibPage& page);
        static void FillMonoPalette(dynarithmic::pcx::PcxHeader& hdr, const PreparedPcxDibPage& page);
        bool Write1Bpp(const PreparedPcxDibPage& page, uint16_t bytesPerLine);
        bool Write8Bpp(const PreparedPcxDibPage& page, uint16_t bytesPerLine);
        bool Write24Bpp(const PreparedPcxDibPage& page, uint16_t bytesPerLine);
        bool Write256Palette(const PreparedPcxDibPage& page);

    private:
        FILE* file_ = nullptr;
        std::wstring filename_;
        PcxSessionOptions options_{};
        bool hasOpen_ = false;
        bool isDcx_ = false;

        std::vector<uint32_t> pageOffsets_;
        std::vector<uint8_t> rowBuffer_;
        std::vector<uint8_t> planeBuffer_;
};

// ============================================================
// DTWAIN-style wrapper
// ============================================================

class DTWAINPcxDcxOutput
{
    public:
        bool OnFirstPage(const std::wstring& filename, const PcxSessionOptions& options, const PreparedPcxDibPage& page);
        bool OnNextPage(const PreparedPcxDibPage& page);
        bool OnLastPage();

    private:
        std::unique_ptr<PcxSessionWriter> writer_;
};

#endif
