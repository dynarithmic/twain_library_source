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
#include <map>

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
            std::map<uint16_t, uint16_t> m_mapSampleFromTo;
            std::vector<uint16_t> m_vectNoSamplingDone;
            std::string m_ImageType;
            void LogMsg(int nWhich, int high, int low, HANDLE dataHandle = {}) const;

        public:
            bool Resample(CTL_TwainDib&);
            ImageResampler(const std::vector<uint16_t>& vNoSamples = {}, const std::map<uint16_t, uint16_t>& mFromTo = {},
                std::string sImgType ="") :
                m_mapSampleFromTo(mFromTo), m_vectNoSamplingDone(vNoSamples), m_ImageType(sImgType) {}
    };

    class ResampleFactory
    {
        public:
            static std::unique_ptr<ImageResampler> GetResampler(int imageType);
    };
}
#endif

