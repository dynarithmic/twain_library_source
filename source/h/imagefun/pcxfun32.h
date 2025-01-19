/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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
#ifndef PCXFUN32_H
#define PCXFUN32_H

#ifndef WINBIT32_H
#include "winbit32.h"
#endif

#ifndef WINBIT32_H
    #include "winbit32.h"
#endif

#include <vector>
#include <fstream>
#include <memory>

#include "dibinfox.h"

#define DCXHEADER_ID 0x3ADE68B1
namespace dynarithmic
{
    struct PCXHEAD
    {
        char manufacturer;
        char version;
        char encoding;
        char bits;
        short int xmin,ymin;
        short int xmax,ymax;
        short int hres;
        short int vres;
        char palette[48];
        char reserved;
        char colour_planes;
        short int bytes_per_line;
        short int palette_type;
        char filler[58];
    };

    struct DCXHEADER
    {
        DWORD Id;
        DWORD nOffsets[1024];
    };

    struct DCXINFO : DibMultiPageData
    {
        DCXHEADER DCXHeader;
        std::unique_ptr<std::ofstream> fh;
        int nCurrentPage;
        int nCurrentOffset;
    };

    class CPCXImageHandler : public CDibInterface
    {
        public:
            CPCXImageHandler(const DTWAINImageInfoEx &ImageInfoEx) : m_bWriteOk(true), m_pDCXInfo(nullptr), m_ImageInfoEx(ImageInfoEx) {}
            // Virtual interface
            std::string GetFileExtension() const  override;
            HANDLE  GetFileInformation(LPCSTR path) override;
            int     WriteGraphicFile(CTL_ImageIOHandler* pThis, LPCTSTR path, HANDLE bitmap, void *pUserInfo= nullptr) override;
            int     WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht,
                               UINT32 bpp, UINT32 cpal, RGBQUAD *pPal, void *pUserInfo= nullptr) override;


            void SetMultiPageStatus(DibMultiPageStruct *pStruct) override;
            void GetMultiPageStatus(DibMultiPageStruct *pStruct) override;

        protected:
            void DestroyAllObjects() override;
            bool OpenOutputFile(LPCTSTR pFileName) override;
            bool CloseOutputFile() override;
            WORD PCXWriteLine(LPSTR p, std::ofstream& fh,int n);

        private:
            bool m_bWriteOk;
            std::vector<CHAR> m_plinebuffer;
            std::vector<CHAR> m_pextrabuffer;
            std::shared_ptr<DibMultiPageData> m_pDCXInfo;
            std::unique_ptr<std::ofstream> m_hFile;
            DTWAINImageInfoEx m_ImageInfoEx;
    };
}
#endif

