/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#ifndef RESAMPLEFACTORY_H
#define RESAMPLEFACTORY_H

#include <vector>
#include <map>
#include <memory>
#include <string>

namespace dynarithmic
{
    class CTL_TwainDib;
    class ImageResampler
    {
        protected:
            enum {
                NO_RESAMPLING,
                START_RESAMPLING,
                FAILED_RESAMPLING,
                SUCCESS_RESAMPLING
            };
            std::map<int, int> m_mapSampleFromTo;
            std::vector<int> m_vectNoSamplingDone;
            std::string m_ImageType;
            void LogMsg(int nWhich, int high, int low, HANDLE dataHandle = {}) const;

        public:
            virtual ~ImageResampler() = default;
            bool Resample(CTL_TwainDib&);
            ImageResampler(const std::vector<int>& vNoSamples = {}, const std::map<int, int>& mFromTo = {}, 
                std::string sImgType ="") :
                m_mapSampleFromTo(mFromTo), m_vectNoSamplingDone(vNoSamples), m_ImageType(sImgType) {}
    };

    class GIFResampler : public ImageResampler
    {
        public:
            GIFResampler() : ImageResampler({ 8 }, { {1,8}, {4,8}, {16,8}, {24,8}, {32,8} }, "GIF") {}
    };

    class JPEGResampler : public ImageResampler
    {
        public:
            JPEGResampler() : ImageResampler({ 8, 16, 24 }, { {1,24}, {4,24}, {32,24} }, "JPEG") {}
    };

    class JPEG2000Resampler : public ImageResampler
    {
    public:
        JPEG2000Resampler() : ImageResampler({ 8, 16, 24, 32 }, { {1,24}, {4,24} }, "JPEG-2000") {}
    };

    class PCXResampler : public ImageResampler
    {
        public:
            PCXResampler() : ImageResampler({ 1, 8, 24 }, { {4,8}, {16,24}, {32,24} }, "PCX") {}
    };

    class PNGResampler : public ImageResampler
    {
        public:
            PNGResampler() : ImageResampler({ 8, 16, 24 }, { {1,8}, {4,8}, {32, 24} }, "PNG") {}
    };

    class TIFFG3G4Resampler : public ImageResampler
    {
        public:
            TIFFG3G4Resampler() : ImageResampler({ 1 }, { {4,1}, {8,1}, {16,1}, {24,1}, {32, 1} }, "TIFF G3/G4") {}
    };

    class TIFFJPEGResampler : public ImageResampler
    {
        public:
            TIFFJPEGResampler() : ImageResampler({ 8, 24 }, { {1,8}, {4,8}, {16,24}, {32,24} }, "TIFF-JPEG") {}
    };

    class WEBPResampler : public ImageResampler
    {
        public:
            WEBPResampler() : ImageResampler({24, 32}, { {1,24}, {4,24}, {8,24}, {16,24} }, "WEBP") {}
    };

    class TGAResampler : public ImageResampler
    {
        public:
            TGAResampler() : ImageResampler({ 8, 24, 32 }, { {1,8}, {4,8}, {16,24} }, "Targa") {}
    };

    class WBMPResampler : public ImageResampler
    {
        public:
            WBMPResampler() : ImageResampler({ 1 }, { {4,1}, {8,1}, {16,1}, {24,1},{32,1} }, "WBMP") {}
    };

    class WMFResampler : public ImageResampler
    {
        public:
            WMFResampler() : ImageResampler({ 24 }, { {1,24}, {4,24}, {8,24}, {16,24},{32,24} }, "EMF/WMF") {}
    };

    class PSDResampler : public ImageResampler
    {
        public:
            PSDResampler() : ImageResampler({ 24 }, { {1,24}, {4,24}, {8,24}, {16,24},{32,24} }, "Adobe PSD") {}
    };

    class PDFResampler : public ImageResampler
    {
        public:
            PDFResampler() : ImageResampler({ 1, 8, 16, 24 }, { {32,24} }, "Adobe PDF") {}
    };

    class ResampleFactory
    {
        public:
            static std::unique_ptr<ImageResampler> GetResampler(int imageType);
    };
}
#endif

