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

#ifndef CTLDIB32_H
#define CTLDIB32_H

//// Special bitmap routines
#include "imagefun/imgfunc.h"
#include <vector>
#include <utility>
#include <optional>
#include <boost/container/flat_map.hpp>
#include "dibmulti.h"
#include "dibinfox.h"
#include "fltrect.h"
#include <unordered_map>
#include "dtwain_filetypes.h"
#include "tiffwriter.h"
#include "pcxwriter.h"
#include "postscriptwriter.h"

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif
namespace dynarithmic
{
    class CTL_TwainDibArray;
    class CTL_TwainDib;

    //////////////////// Io Handler
    #define BYTEBUFFERSIZE      2048

    class CTL_ImageIOHandler
    {
        public:
            CTL_ImageIOHandler();
            CTL_ImageIOHandler( CTL_TwainDib *pDib );
            virtual ~CTL_ImageIOHandler() = default;
            void    SetDib( CTL_TwainDib *pDib ) { m_pDib = pDib; }
            virtual int WriteBitmap(LPCTSTR szFile, bool bOpenFile, int fh, DibMultiPageStruct* pMultiDibStruct = nullptr) = 0;
            std::shared_ptr<DibMultiPageData> GetMultiDibData() const { return pMultiDibData; }
            void SetMultiDibData(std::shared_ptr<DibMultiPageData> pData) { pMultiDibData = std::move(pData);  }
            void SetMultiDibInfo(const DibMultiPageStruct &s);
            DibMultiPageStruct GetMultiDibInfo() const;
            unsigned int GetNumPagesWritten() const { return m_nPage; }
            void SetNumPagesWritten(unsigned nPages) { m_nPage = nPages; }
            void SetOnePageWritten(bool bSet) { m_bOnePageWritten = bSet; }
            bool IsOnePageWritten() const { return m_bOnePageWritten; }
            virtual void SetImageInfo(const DTWAINImageInfoEx& /*ImageInfo*/) { }
            void SetBaseImageInfo(const DTWAINImageInfoEx& ImageInfo) { m_ImageInfo = ImageInfo; }
            const DTWAINImageInfoEx& GetBaseImageInfo() const { return m_ImageInfo; }
            CTL_TwainDib* GetDib() const { return m_pDib; }
            static bool IsValidBitDepth(LONG FileType, LONG bitDepth);
            static auto& GetSupportedBPPMap() { return s_supportedBitDepths; }
            const std::string& GetCopyrightString() const { return m_sCopyright; }
            int WriteBitmapImpl(LPCTSTR szFile, int nFormat, bool bOpenFile, int fh, DibMultiPageStruct* pMultiDibStruct = nullptr);

        protected:
            CTL_TwainDib *m_pDib;
            DTWAINImageInfoEx m_ImageInfo;
            std::shared_ptr<DibMultiPageData> pMultiDibData;
            DibMultiPageStruct m_DibMultiPageStruct;
            std::string m_sCopyright;
            unsigned m_nPage;
            bool m_bOnePageWritten;
            static boost::container::flat_map<LONG, std::vector<uint16_t>> s_supportedBitDepths;
    };
}
#endif
