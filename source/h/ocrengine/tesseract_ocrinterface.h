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
#ifndef TESSERACT_OCRINTERFACE_H
#define TESSERACT_OCRINTERFACE_H

#include <string>
#include "ocrinterface.h"
#include <tesseract\capi.h>

namespace dynarithmic
{
    struct TessFunctions
    {
        decltype(&TessVersion) TessVersion = nullptr;
        decltype(&TessBaseAPICreate) TessBaseAPICreate = nullptr;
        decltype(&TessBaseAPIDelete) TessBaseAPIDelete = nullptr;
        decltype(&TessBaseAPIInit3) TessBaseAPIInit3 = nullptr;
        decltype(&TessBaseAPIEnd) TessBaseAPIEnd = nullptr;

        decltype(&TessBaseAPISetImage) TessBaseAPISetImage = nullptr;
        decltype(&TessBaseAPISetSourceResolution) TessBaseAPISetSourceResolution = nullptr;

        decltype(&TessBaseAPIRecognize) TessBaseAPIRecognize = nullptr;
        decltype(&TessBaseAPIGetUTF8Text) TessBaseAPIGetUTF8Text = nullptr;
        decltype(&TessDeleteText) TessDeleteText = nullptr;
    };

    class TesseractOCR : public OCREngine
    {
    public:
        explicit TesseractOCR(CTL_TwainDLLHandle* pHandle);
        ~TesseractOCR() override;

        bool IsInitialized() const override;

        LONG StartupOCREngine() override;
        LONG StartOCR() override;
        LONG StartOCR(CTL_StringType filename) override;
        LONG StartOCR(HANDLE hDib) override;

        std::string GetOCRVersionInfo() override;
        bool SetOCRVersionIdentity() override;
        bool ShutdownOCR(int& status) override;
        OCRInputType GetInputType() const override { return OCRInputType::DIB; }
        std::string GetOCRText(LONG nPage = 0) override;

    private:

        void DestroyOCRSession() noexcept;
        void UnloadTesseract() noexcept;

        HMODULE m_hTessDLL = nullptr;
        bool m_bInitialized = false;
        TessFunctions m_TessFunctions{};
        TessBaseAPI* m_pTessAPI = nullptr;
    };
}
#endif