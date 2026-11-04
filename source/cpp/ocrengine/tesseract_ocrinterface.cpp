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
#include <windows.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <limits>
#include <tchar.h>
#include "capconst.h"
#include "tesseract_ocrinterface.h"
#include "ctlfileutils.h"
#include "ctlwindowsimpl.h"
#include "errorcheck.h"

using namespace dynarithmic;

namespace
{
    struct TesseractImage
    {
        std::vector<unsigned char> pixels;
        int width = 0;
        int height = 0;
        int bytesPerPixel = 0;
        int bytesPerLine = 0;
    };

    bool ConvertDIBForTesseract(HANDLE hDib, TesseractImage& output)
    {
        output = {};

        if (!hDib)
            return false;

        auto* pDib = static_cast<const BYTE*>(GlobalLock(hDib));

        if (!pDib)
            return false;

        struct UnlockDIB
        {
            HANDLE handle;
            ~UnlockDIB() { GlobalUnlock(handle); }
        } unlock{ hDib };

        if (GlobalSize(hDib) < sizeof(BITMAPINFOHEADER))
            return false;

        const auto* bih = reinterpret_cast<const BITMAPINFOHEADER*>(pDib);

        if (bih->biSize != sizeof(BITMAPINFOHEADER) ||
            bih->biPlanes != 1 ||
            bih->biCompression != BI_RGB ||
            bih->biWidth <= 0 ||
            bih->biHeight == 0 ||
            bih->biHeight == std::numeric_limits<LONG>::min())
        {
            return false;
        }

        const int width = bih->biWidth;
        const int height = bih->biHeight < 0
            ? -bih->biHeight
            : bih->biHeight;

        const int bpp = bih->biBitCount;

        if (bpp != 8 && bpp != 24 && bpp != 32)
            return false;

        const bool bottomUp = bih->biHeight > 0;

        const uint64_t sourceStride = ((static_cast<uint64_t>(width) * bpp + 31) / 32) * 4;
        const uint64_t sourceBytes = sourceStride * height;
        const uint64_t paletteEntries = bpp == 8
            ? (bih->biClrUsed ? bih->biClrUsed : 256)
            : 0;

        if (paletteEntries > 256)
            return false;

        const uint64_t pixelOffset = static_cast<uint64_t>(bih->biSize) +
            paletteEntries * sizeof(RGBQUAD);

        if (pixelOffset > GlobalSize(hDib) ||
            sourceBytes > GlobalSize(hDib) - pixelOffset)
        {
            return false;
        }

        const auto* palette = reinterpret_cast<const RGBQUAD*>(pDib + bih->biSize);

        const BYTE* sourcePixels = pDib + pixelOffset;

        // For 8-bit images, use one byte per pixel if the
        // palette is grayscale. Otherwise expand to RGB.
        bool grayscale = bpp == 8;

        if (bpp == 8)
        {
            for (uint64_t i = 0; i < paletteEntries; ++i)
            {
                const RGBQUAD& c = palette[i];

                if (c.rgbRed != c.rgbGreen ||
                    c.rgbGreen != c.rgbBlue)
                {
                    grayscale = false;
                    break;
                }
            }
        }

        const int outputBpp = grayscale ? 1 : 3;
        const uint64_t outputStride = static_cast<uint64_t>(width) * outputBpp;
        const uint64_t outputBytes = outputStride * height;

        if (outputStride > std::numeric_limits<int>::max() ||
            outputBytes > std::numeric_limits<size_t>::max())
        {
            return false;
        }

        output.width = width;
        output.height = height;
        output.bytesPerPixel = outputBpp;
        output.bytesPerLine = static_cast<int>(outputStride);
        output.pixels.resize(static_cast<size_t>(outputBytes));

        for (int y = 0; y < height; ++y)
        {
            const int sourceY = bottomUp ? height - 1 - y : y;

            const BYTE* src = sourcePixels + sourceY * sourceStride;

            BYTE* dst = output.pixels.data() + y * outputStride;

            for (int x = 0; x < width; ++x)
            {
                if (bpp == 8)
                {
                    const BYTE index = src[x];

                    if (index >= paletteEntries)
                        return false;

                    const RGBQUAD& c = palette[index];

                    if (grayscale)
                    {
                        dst[x] = c.rgbRed;
                    }
                    else
                    {
                        dst[x * 3 + 0] = c.rgbRed;
                        dst[x * 3 + 1] = c.rgbGreen;
                        dst[x * 3 + 2] = c.rgbBlue;
                    }
                }
                else if (bpp == 24)
                {
                    const BYTE* pixel = src + x * 3;

                    dst[x * 3 + 0] = pixel[2];
                    dst[x * 3 + 1] = pixel[1];
                    dst[x * 3 + 2] = pixel[0];
                }
                else // 32-bit BGRX
                {
                    const BYTE* pixel = src + x * 4;

                    dst[x * 3 + 0] = pixel[2];
                    dst[x * 3 + 1] = pixel[1];
                    dst[x * 3 + 2] = pixel[0];
                }
            }
        }

        return true;
    }

    std::wstring GetTesseractDLLName()
    {
        // Load the resources
        auto* customProfile = CTL_StaticData::GetINIInterface();
        const char* defName = "Tesseract";
        if (customProfile)
            defName = customProfile->GetValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_OCRLIBRARY_KEY).data(), defName);
        auto val = stringconversion::Convert_AnsiPtr_To_Wide(defName);
        if (val.empty())
            val = _T("libtesseract-5.dll");
        return val;
    }
}


TesseractOCR::TesseractOCR(CTL_TwainDLLHandle* pHandle) : OCREngine(pHandle) 
{
    SetOCREngineType(OCREngineType::Tesseract);

    // Load the Tesseract DLL.

    // Get the configured path
    auto strPath = CTL_StaticData::GetComponentPath(DTWAIN_COMPONENT_TESSERACT);
    std::wstring sDLLPath = stringconversion::Convert_Native_To_Wide(strPath);
    sDLLPath = WindowsAPIImplDef::AddBackslashToDirectory(sDLLPath);

    // Get the Tesseract DLL name to load
    auto sTotalPath = sDLLPath + GetTesseractDLLName();

    m_hTessDLL = LoadLibraryExW(sTotalPath.c_str(), nullptr, 
        LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!m_hTessDLL)
        return;

    // Resolve a Tesseract C API function.
    auto resolve = [this](auto& func, const char* name) -> bool
    {
        func = reinterpret_cast<std::remove_reference_t<decltype(func)>>(GetProcAddress(m_hTessDLL, name));
        return func != nullptr;
    };

    bool success = true;

    success &= resolve(m_TessFunctions.TessVersion,"TessVersion");
    success &= resolve(m_TessFunctions.TessBaseAPICreate,"TessBaseAPICreate");
    success &= resolve(m_TessFunctions.TessBaseAPIDelete,"TessBaseAPIDelete");
    success &= resolve(m_TessFunctions.TessBaseAPIInit3,"TessBaseAPIInit3");
    success &= resolve(m_TessFunctions.TessBaseAPIEnd,"TessBaseAPIEnd");
    success &= resolve(m_TessFunctions.TessBaseAPISetImage,"TessBaseAPISetImage");
    success &= resolve(m_TessFunctions.TessBaseAPISetSourceResolution,"TessBaseAPISetSourceResolution");
    success &= resolve(m_TessFunctions.TessBaseAPIRecognize,"TessBaseAPIRecognize");
    success &= resolve(m_TessFunctions.TessBaseAPIGetUTF8Text,"TessBaseAPIGetUTF8Text");
    success &= resolve(m_TessFunctions.TessDeleteText,"TessDeleteText");

    if (!success)
    {
        FreeLibrary(m_hTessDLL);
        m_hTessDLL = nullptr;
        m_TessFunctions = {};
        return;
    }

    m_bInitialized = true;
}


bool TesseractOCR::SetOCRVersionIdentity()
{
    OCRVersionIdentity identity{};

    identity.Manufacturer  = "Tesseract OCR Project";
    identity.ProductFamily = "Tesseract OCR";
    identity.ProductName   = "Tesseract OCR";

    identity.Version.Language = TWLG_ENGLISH_USA;
    identity.Version.Country  = TWCY_USA;

    const char* version = m_TessFunctions.TessVersion();

    if (version)
    {
        identity.Version.Info = version;

        int major = 0;
        int minor = 0;

        if (std::sscanf(version, "%d.%d", &major, &minor) == 2)
        {
            identity.Version.MajorNum = major;
            identity.Version.MinorNum = minor;
        }
    }

    return OCREngine::SetOCRVersionIdentity(identity);
}

bool TesseractOCR::IsInitialized() const
{
    return m_bInitialized;
}

void TesseractOCR::UnloadTesseract() noexcept
{
    // Release the OCR session first.
    DestroyOCRSession();

    // Unload the Tesseract library.
    if (m_hTessDLL)
    {
        FreeLibrary(m_hTessDLL);
        m_hTessDLL = nullptr;
    }

    // Invalidate the function pointers.
    m_TessFunctions = {};

    m_bInitialized = false;
}

bool TesseractOCR::ShutdownOCR(int& status)
{
    DestroyOCRSession();

    status = 0;
    return true;
}

void TesseractOCR::DestroyOCRSession() noexcept
{
    if (m_pTessAPI)
    {
        m_TessFunctions.TessBaseAPIEnd(m_pTessAPI);
        m_TessFunctions.TessBaseAPIDelete(m_pTessAPI);

        m_pTessAPI = nullptr;
    }
}

TesseractOCR::~TesseractOCR()
{
    UnloadTesseract();
}


LONG TesseractOCR::StartupOCREngine()
{
    // The Tesseract DLL must have been loaded
    // successfully by the constructor.
    if (!m_bInitialized)
        return -1;

    // Already started.
    if (m_pTessAPI)
        return 0;

    // Create the Tesseract API instance.
    TessBaseAPI* pAPI = m_TessFunctions.TessBaseAPICreate();

    if (!pAPI)
        return -1;

    // Initialize Tesseract for English OCR.
    //
    // nullptr: Use Tesseract's default tessdata
    //           lookup mechanism.
    // "eng":   English language.
    const int result = m_TessFunctions.TessBaseAPIInit3(pAPI,"d:/dtwain_bin/tessdata","eng");

    if (result != 0)
    {
        // Initialization failed.
        m_TessFunctions.TessBaseAPIEnd(pAPI);
        m_TessFunctions.TessBaseAPIDelete(pAPI);
        return -1;
    }

    // Initialization succeeded.
    m_pTessAPI = pAPI;

    return 0;
}

std::string TesseractOCR::GetOCRVersionInfo()
{
    const auto identity = GetOCRVersionIdentity();

    return identity.ProductName + " " + identity.Version.Info;
}

LONG TesseractOCR::StartOCR() { return 0; }
LONG TesseractOCR::StartOCR(CTL_StringType filename) { return 0; }

LONG TesseractOCR::StartOCR(HANDLE hDib)
{
    // Replace these with DTWAIN's actual OCR error codes.
    constexpr LONG OCR_SUCCESS = 0;
    constexpr LONG OCR_FAILURE = -1;

    // Clear the previous recognition result.
    SetCachedText("");

    if (!m_bInitialized || !hDib)
    {
        SetLastError(OCR_FAILURE);
        return OCR_FAILURE;
    }

    TesseractImage image;

    if (!ConvertDIBForTesseract(hDib, image))
    {
        SetLastError(OCR_FAILURE);
        return OCR_FAILURE;
    }

    // Supply the converted pixels to Tesseract.
    m_TessFunctions.TessBaseAPISetImage(
        m_pTessAPI,
        image.pixels.data(),
        image.width,
        image.height,
        image.bytesPerPixel,
        image.bytesPerLine);

    // Perform recognition.
    const int result = m_TessFunctions.TessBaseAPIRecognize(m_pTessAPI, nullptr);

    if (result != 0)
    {
        SetLastError(OCR_FAILURE);
        return OCR_FAILURE;
    }

    // Retrieve the recognized UTF-8 text.
    char* pText = m_TessFunctions.TessBaseAPIGetUTF8Text(m_pTessAPI);

    if (!pText)
    {
        SetLastError(OCR_FAILURE);
        return OCR_FAILURE;
    }

    // Copy text into DTWAIN's OCR cache before
    // releasing the Tesseract-owned allocation.
    SetCachedText(std::string(pText));

    m_TessFunctions.TessDeleteText(pText);

    SetLastError(OCR_SUCCESS);
    return OCR_SUCCESS;
}

std::string TesseractOCR::GetOCRText(LONG nPage)
{
    return GetCachedText();
}
