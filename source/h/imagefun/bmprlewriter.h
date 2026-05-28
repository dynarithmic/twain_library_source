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
#ifndef BMPRLEWRITER_H
#define BMPRLEWRITER_H

#include <windows.h>

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
// Prepared 8-bpp DIB page for BMP-RLE8 output
// ============================================================

struct PreparedBmpRle8Page
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint16_t bitsPerPixel = 8;
    uint32_t strideBytes = 0;
    bool bottomUp = true;

    const uint8_t* bits = nullptr;

    const RGBQUAD* palette = nullptr;
    uint32_t paletteEntries = 0;

    int32_t xPelsPerMeter = 0;
    int32_t yPelsPerMeter = 0;
};

// ============================================================
// BMP RLE8 encoder
// ============================================================
class BmpRle8Writer
{
public:
    BmpRle8Writer() = default;

    ~BmpRle8Writer();
    BmpRle8Writer(const BmpRle8Writer&) = delete;
    BmpRle8Writer& operator=(const BmpRle8Writer&) = delete;

    bool Open(const std::wstring& filename);
    bool SetPageInfo(const PreparedBmpRle8Page& page);
    bool WriteCurrentPage();
    void Close();
    static std::optional<PreparedBmpRle8Page> MakePreparedBmpRle8Page(const dynarithmic::DibPageView& view);

private:
    static bool ValidatePage(const PreparedBmpRle8Page& page);
    void emit_byte(uint8_t b);
    void emit_word(uint8_t a, uint8_t b);
    bool encode_rle8();
    int RLEEncodeLineLikeFreeImage(uint8_t* target, const uint8_t* source, int size);
    bool write_bmp_file();

private:
	FILE* file_ = nullptr;
	std::wstring filename_;

	PreparedBmpRle8Page currentPage_{};
	bool hasPage_ = false;

	std::vector<uint8_t> encodedData_;
	std::vector<uint8_t> lineEncodeBuffer_;
};

// ============================================================
// Optional DTWAIN-style wrapper
//   FirstPage = open + write image
//   LastPage  = close
// ============================================================
class DTWAINBmpRle8Output
{
    public:
        bool OnFirstPage(const std::wstring& filename, const PreparedBmpRle8Page& page);
        bool OnLastPage();

    private:
        std::unique_ptr<BmpRle8Writer> writer_;
};

#if 0
/*
Example usage:

// Direct helper
if (!WriteOneDibHandleToBmpRle8(L"output_rle8.bmp", hDib))
    return false;

// DTWAIN-style Model B wrapper
{
    LockedBmpRle8Page locked(hDib);
    if (!locked.IsValid())
        return false;

    DTWAINBmpRle8Output output;
    if (!output.OnFirstPage(L"output_rle8.bmp", locked.GetPage()))
        return false;
    if (!output.OnLastPage())
        return false;
}
*/
#endif
#endif
