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
#include <math.h>
#include <sstream>
#include "winbit32.h"
#include "ctliface.h"
#include "ctltwmgr.h"
#include "../cximage/ximage.h"
#include "resamplefactory.h"

namespace dynarithmic
{
    void ImageResampler::LogMsg(int nWhich, int fromRes, int toRes, HANDLE dataHandle) const
    {
        if (!AnyLoggerExists())
            return;
        if (!(CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS))
            return;
        switch (nWhich)
        {
            case NO_RESAMPLING:
                CTL_TwainAppMgr::WriteLogInfoA("No resampling done for " + m_ImageType);
            break;

            case START_RESAMPLING:
                CTL_TwainAppMgr::WriteLogInfoA("Resampling bitmap data for image type " + m_ImageType + 
                                ".  From " + std::to_string(fromRes) + " bpp to " + std::to_string(toRes) + " bpp ...");
            break;

            case FAILED_RESAMPLING:
                CTL_TwainAppMgr::WriteLogInfoA("Resampling bitmap data for image type " + m_ImageType + " failed...");
            break;

            case SUCCESS_RESAMPLING:
                CTL_TwainAppMgr::WriteLogInfoA("Resampling bitmap data for image type " + m_ImageType + " success...");
                std::string sOut = CTL_ErrorStructDecoder::DecodeBitmap(dataHandle);
                CTL_TwainAppMgr::WriteLogInfoA(sOut);
            break;
        }
    }

    bool ImageResampler::Resample(CTL_TwainDib& CurDib)
    {
        const int depth = CurDib.GetDepth();
        if (std::find(m_vectNoSamplingDone.begin(), m_vectNoSamplingDone.end(), depth) != m_vectNoSamplingDone.end())
        {
            LogMsg(NO_RESAMPLING, 0, 0);
            return false; // no sampling done
        }

        // Get the sampling to be done
        auto iter = m_mapSampleFromTo.find(depth);
        bool bOk = false;
        if (iter != m_mapSampleFromTo.end())
        {
            int toDepth = iter->second;
            LogMsg(START_RESAMPLING, depth, toDepth);
            if (depth < toDepth)
                bOk = CurDib.IncreaseBpp(toDepth);
            else
                bOk = CurDib.DecreaseBpp(toDepth);
            if (bOk)
                LogMsg(SUCCESS_RESAMPLING, depth, toDepth, CurDib.GetHandle());
            else
                LogMsg(FAILED_RESAMPLING, depth, toDepth);
            return bOk;
        }
        return false;
    }

    std::unique_ptr<ImageResampler> ResampleFactory::GetResampler(int imageType)
    {
        switch (imageType)
        {
            case DTWAIN_GIF:
                return std::make_unique<GIFResampler>();

            case DTWAIN_JPEG:
                return std::make_unique<JPEGResampler>();

            case DTWAIN_JPEG2000:
                return std::make_unique<JPEG2000Resampler>();

            case DTWAIN_PNG:
                return std::make_unique<PNGResampler>();

            case DTWAIN_PCX:
            case DTWAIN_DCX:
                return std::make_unique<PCXResampler>();

            case DTWAIN_TGA:
                return std::make_unique<TGAResampler>();

            case DTWAIN_EMF:
            case DTWAIN_WMF:
                return std::make_unique<WMFResampler>();

            case DTWAIN_TIFFG3:
            case DTWAIN_TIFFG3MULTI:
            case DTWAIN_TIFFG4:
            case DTWAIN_TIFFG4MULTI:
                return std::make_unique<TIFFG3G4Resampler>();

            case DTWAIN_TIFFJPEG:
            case DTWAIN_TIFFJPEGMULTI:
                return std::make_unique<TIFFJPEGResampler>();

            case DTWAIN_WEBP:
                return std::make_unique<WEBPResampler>();

            case DTWAIN_WBMP:
                return std::make_unique<WBMPResampler>();

            case DTWAIN_PSD:
                return std::make_unique<PSDResampler>();

            case DTWAIN_PDF:
            case DTWAIN_PDFMULTI:
                return std::make_unique<PDFResampler>();
        }
        return nullptr;
    }
}
