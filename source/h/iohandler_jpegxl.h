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

#ifndef IOHANDLERJPEGXL_H
#define IOHANDLERJPEGXL_H
#include "ctldib32.h"
#include "jpegxlwriter.h"
#include "jpegxlloader.h"

namespace dynarithmic 
{
    class CTL_JPegXLIOHandler : public CTL_ImageIOHandler 
    {
        public:
            struct JxlLoadError
            {
                DWORD exceptionCode = 0;
                DWORD windowsError = 0;
            };

            explicit CTL_JPegXLIOHandler(const JxlSessionOptions& options = {}) : options_(options)
            {
                InitializeJXLSupport();
            }

            CTL_JPegXLIOHandler(CTL_TwainDib* dib, const DTWAINImageInfoEx& ImageInfo) : 
                            CTL_ImageIOHandler(dib), 
                            m_ImageInfoEx(ImageInfo)
            {
                InitializeJXLSupport();
            }

            bool IsJxlAvailable() const { return m_bJxlAvailable; }
            int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pDibStruct = nullptr) override;
            JxlLoadError GetJXLLoadError() const { return jxlError; }

        private:
            JxlLoadError jxlError{};
            void InitializeJXLSupport();
            JxlSessionOptions options_{};
            bool m_bJxlAvailable{};
            DTWAINImageInfoEx m_ImageInfoEx;
    };
}
#endif
