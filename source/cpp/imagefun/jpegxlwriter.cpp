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

#include "jpegxlwriter.h"
#include <jxl/encode.h>
#include <jxl/color_encoding.h>
#include <jxl/thread_parallel_runner.h>
#include <limits>
#include <vector>
namespace dynarithmic 
{
    namespace 
    {
        struct EncoderDeleter { void operator()(JxlEncoder* p) const { JxlEncoderDestroy(p); } };
        struct RunnerDeleter { void operator()(void* p) const { JxlThreadParallelRunnerDestroy(p); } };

        bool MakePixels(const DibPageView& v, std::vector<uint8_t>& dst, uint32_t& channels, uint32_t& depth) 
        {
            if (!v.bits || !v.width || !v.height || !v.bih || v.bih->biCompression != BI_RGB) 
                return false;
            const auto bpp = v.bitsPerPixel;
            if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 16 && bpp != 24 && bpp != 32) 
                return false;
            bool gray = bpp == 16;
            if (bpp <= 8) 
            {
                gray = true;
                if (v.palette && v.paletteEntries) 
                {
                    for (uint32_t i = 0; i < v.paletteEntries; ++i)
                    {
                        if (v.palette[i].rgbRed != v.palette[i].rgbGreen || v.palette[i].rgbRed != v.palette[i].rgbBlue)
                        {
                            gray = false; break;
                        }
                    }
                }
            }
            channels = gray ? 1u : 3u;
            depth = bpp == 16 ? 16u : 8u;
            const uint64_t minStride = (uint64_t(v.width) * bpp + 7) / 8;
            const uint64_t bytes = uint64_t(v.width) * v.height * channels * (depth / 8);
            if (v.strideBytes < minStride || bytes > std::numeric_limits<size_t>::max()) return false;
            dst.resize(static_cast<size_t>(bytes));
            for (uint32_t y = 0; y < v.height; ++y) 
            {
                const auto* src = v.bits + size_t(v.bottomUp ? v.height - 1 - y : y) * v.strideBytes;
                auto* out = dst.data() + size_t(y) * v.width * channels * (depth / 8);
                if (bpp == 16)
                {
                    std::memcpy(out, src, size_t(v.width) * 2); 
                    continue;
                }
                for (uint32_t x = 0; x < v.width; ++x) 
                {
                    uint8_t r = 0, g = 0, b = 0;
                    if (bpp <= 8) 
                    {
                        uint32_t idx = bpp == 8 ? src[x] : bpp == 4 ? ((x & 1) ? (src[x / 2] & 15) : (src[x / 2] >> 4)) : ((src[x / 8] >> (7 - (x & 7))) & 1);
                        if (v.palette && idx < v.paletteEntries) 
                        {
                            const auto& c = v.palette[idx]; r = c.rgbRed; g = c.rgbGreen; b = c.rgbBlue;
                        } 
                        else if (!v.palette || !v.paletteEntries) 
                        {
                            r = g = b = bpp == 1 ? static_cast<uint8_t>(idx * 255) : bpp == 4 ? static_cast<uint8_t>(idx * 17) : static_cast<uint8_t>(idx);
                        } 
                        else 
                            return false;
                    } 
                    else
                    {
                        const auto* p = src + size_t(x) * (bpp / 8); b = p[0]; g = p[1]; r = p[2];
                    }
                    if (gray) 
                        out[x] = r;
                    else
                    {
                        out[x * 3] = r; 
                        out[x * 3 + 1] = g; 
                        out[x * 3 + 2] = b;
                    }
                }
            }
            return true;
        }
    }

    JxlSessionWriter::~JxlSessionWriter() { Close(); }
    
    bool JxlSessionWriter::Open(const std::wstring& filename, const JxlSessionOptions& options) 
    {
        if (file_ || options.effort < 1 || options.effort > 9 || options.distance < 0) 
            return false;
        file_ = _wfopen(filename.c_str(), L"wb");
        if (!file_) 
            return false;
        options_ = options; 
        hasPage_ = false; 
        return true;
    }

    std::optional<PreparedJxlDibPage> JxlSessionWriter::MakePreparedJxlPage(const DibPageView& v) 
    {
        if (!v.bits || !v.bih || !v.width || !v.height || v.bih->biCompression != BI_RGB) 
            return std::nullopt;
        switch (v.bitsPerPixel)
        {
            case 1: 
            case 4: 
            case 8: 
            case 16: 
            case 24: 
            case 32: 
                break; 
            default: 
                return std::nullopt;
        }
        return PreparedJxlDibPage{v};
    }

    bool JxlSessionWriter::SetPageInfo(const PreparedJxlDibPage& page) 
    {
        if (!file_ || hasPage_ || !MakePreparedJxlPage(page.view)) 
            return false;
        page_ = page; 
        hasPage_ = true; 
        return true;
    }

    bool JxlSessionWriter::WriteCurrentPage() 
    {
        if (!file_ || !hasPage_) 
            return false;
        std::vector<uint8_t> pixels;
        uint32_t channels = 0, depth = 0;
        if (!MakePixels(page_.view, pixels, channels, depth)) 
            return false;
        std::unique_ptr<JxlEncoder, EncoderDeleter> enc(JxlEncoderCreate(nullptr));
        if (!enc) 
            return false;
        std::unique_ptr<void, RunnerDeleter> runner(JxlThreadParallelRunnerCreate(nullptr, options_.numThreads > 0 ? size_t(options_.numThreads) : JxlThreadParallelRunnerDefaultNumWorkerThreads()));
        if (!runner || JxlEncoderSetParallelRunner(enc.get(), JxlThreadParallelRunner, runner.get()) != JXL_ENC_SUCCESS) 
            return false;
        JxlBasicInfo info;
        JxlEncoderInitBasicInfo(&info);
        info.xsize = page_.view.width; 
        info.ysize = page_.view.height;
        info.bits_per_sample = depth; 
        info.exponent_bits_per_sample = 0;
        info.num_color_channels = channels;
        info.uses_original_profile = options_.lossless ? JXL_TRUE : JXL_FALSE;
        if (JxlEncoderSetBasicInfo(enc.get(), &info) != JXL_ENC_SUCCESS) 
            return false;
        JxlColorEncoding color;
        JxlColorEncodingSetToSRGB(&color, channels == 1 ? JXL_TRUE : JXL_FALSE);
        if (JxlEncoderSetColorEncoding(enc.get(), &color) != JXL_ENC_SUCCESS) 
            return false;
        auto* frame = JxlEncoderFrameSettingsCreate(enc.get(), nullptr);
        if (!frame) 
            return false;
        if (JxlEncoderSetFrameLossless(frame, options_.lossless ? JXL_TRUE : JXL_FALSE) != JXL_ENC_SUCCESS) 
            return false;
        if (JxlEncoderSetFrameDistance(frame, options_.lossless ? 0.0f : options_.distance) != JXL_ENC_SUCCESS) 
            return false;
        if (JxlEncoderFrameSettingsSetOption(frame, JXL_ENC_FRAME_SETTING_EFFORT, options_.effort) != JXL_ENC_SUCCESS) 
            return false;
        JxlPixelFormat fmt{channels, depth == 16 ? JXL_TYPE_UINT16 : JXL_TYPE_UINT8, JXL_NATIVE_ENDIAN, 0};
        if (JxlEncoderAddImageFrame(frame, &fmt, pixels.data(), pixels.size()) != JXL_ENC_SUCCESS) 
            return false;
        JxlEncoderCloseInput(enc.get());
        std::vector<uint8_t> out(65536);
        for (;;) 
        {
            uint8_t* next = out.data();
            size_t avail = out.size();
            auto status = JxlEncoderProcessOutput(enc.get(), &next, &avail);
            const size_t produced = size_t(next - out.data());
            if (produced && std::fwrite(out.data(), 1, produced, file_) != produced) 
                return false;
            if (status == JXL_ENC_SUCCESS) 
                break;
            if (status != JXL_ENC_NEED_MORE_OUTPUT) 
                return false;
        }
        hasPage_ = false;
        return std::fflush(file_) == 0;
    }

    void JxlSessionWriter::Close()
    {
        if (file_)
        {
            std::fclose(file_); 
            file_ = nullptr;
        } 
        hasPage_ = false; 
        page_ = {};
    }

    bool DTWAINJxlOutput::OnFirstPage(const std::wstring& filename, const JxlSessionOptions& options, const PreparedJxlDibPage& page) 
    {
        if (writer_) 
            return false;
        writer_ = std::make_unique<JxlSessionWriter>();
        if (!writer_->Open(filename, options) || !writer_->SetPageInfo(page) || !writer_->WriteCurrentPage())
        {
            writer_.reset(); 
            return false;
        }
        return true;
    }

    bool DTWAINJxlOutput::OnLastPage()
    {
        if (!writer_) 
            return false; 
        writer_.reset(); 
        return true;
    }
}
