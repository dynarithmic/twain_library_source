/*
This file is part of the Dynarithmic TWAIN Library (DTWAIN).
Copyright (c) 2002-2024 Dynarithmic Software.

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
#ifndef TIFFUN32_H
#define TIFFUN32_H

// These #include files are for the TIFFLIB DLL library
#include <utility>

#include "dibinfox.h"
#include "winbit32.h"
#include "dtwain_filetypes.h"

// Includes stuff from the LIBTIFF library
namespace dynarithmic
{
    struct TiffMultiPageData : DibMultiPageData
    {
        FIMULTIBITMAP *fp = nullptr;
    };

    class CTIFFImageHandler : public CDibInterface
    {
            enum { TiffFormatLZW = DTWAIN_TIFFLZW,
                   TiffFormatNONE = DTWAIN_TIFFNONE,
                   TiffFormatGROUP3 = DTWAIN_TIFFG3,
                   TiffFormatGROUP4 = DTWAIN_TIFFG4,
                   TiffFormatPACKBITS = DTWAIN_TIFFPACKBITS,
                   TiffFormatDEFLATE = DTWAIN_TIFFDEFLATE,
                   TiffFormatJPEG = DTWAIN_TIFFJPEG,
                   TiffFormatPIXARLOG = DTWAIN_TIFFPIXARLOG,
                   TiffFormatNONEMULTI = DTWAIN_TIFFNONEMULTI,
                   TiffFormatGROUP3MULTI = DTWAIN_TIFFG3MULTI,
                   TiffFormatGROUP4MULTI  = DTWAIN_TIFFG4MULTI,
                   TiffFormatPACKBITSMULTI = DTWAIN_TIFFPACKBITSMULTI,
                   TiffFormatDEFLATEMULTI = DTWAIN_TIFFDEFLATEMULTI,
                   TiffFormatJPEGMULTI = DTWAIN_TIFFJPEGMULTI,
                   TiffFormatLZWMULTI = DTWAIN_TIFFLZWMULTI,
                   TiffFormatPIXARLOGMULTI = DTWAIN_TIFFPIXARLOGMULTI,
                   BigTiffFormatLZW = DTWAIN_BIGTIFFLZW,
                   BigTiffformatLZWMULTI = DTWAIN_BIGTIFFLZWMULTI,
                   BigTiffFormatNONE = DTWAIN_BIGTIFFNONE,
                   BigTiffformatNONEMULTI = DTWAIN_BIGTIFFNONEMULTI,
                   BigTiffFormatPACKBITS = DTWAIN_BIGTIFFPACKBITS,
                   BigTiffformatPACKBITSMULTI = DTWAIN_BIGTIFFPACKBITSMULTI,
                   BigTiffFormatDEFLATE = DTWAIN_BIGTIFFDEFLATE,
                   BigTiffformatDEFLATEMULTI = DTWAIN_BIGTIFFDEFLATEMULTI,
                   BigTiffFormatGROUP3 = DTWAIN_BIGTIFFG3,
                   BigTiffFormatGROUP3MULTI = DTWAIN_BIGTIFFG3MULTI,
                   BigTiffformatGROUP4 = DTWAIN_BIGTIFFG4,
                   BigTiffformatGROUP4MULTI = DTWAIN_BIGTIFFG4MULTI,
                   BigTiffformatJPEG = DTWAIN_BIGTIFFJPEG,
                   BigTiffformatJPEGMULTI = DTWAIN_BIGTIFFJPEGMULTI,
     };

        private:
            UINT32 m_nFormat;
            DTWAINImageInfoEx m_ImageInfoEx;
            TCHAR m_FileName[255];
            bool   m_bWriteOk;
            bool    m_nError;

            static CTL_StringType s_AppInfo;

        public:
            CTIFFImageHandler(UINT32 nFormat, DTWAINImageInfoEx ImageInfoEx) :
                m_nFormat(nFormat),
                m_ImageInfoEx(std::move(ImageInfoEx)),
                m_FileName{},
                m_bWriteOk(false),
                m_nError(false)
                {
                    m_MultiPageStruct.pUserData = nullptr;
                    m_MultiPageStruct.Stage = 0;
                }

            // Virtual interface
            std::string GetFileExtension() const override;
            HANDLE  GetFileInformation(LPCSTR path) override;
            int     WriteGraphicFile(CTL_ImageIOHandler* ptrHandler, LPCTSTR path, HANDLE bitmap, void *pUserInfo= nullptr) override;
            int     WriteImage(CTL_ImageIOHandler* ptrHandler, BYTE *pImage2, UINT32 wid, UINT32 ht,
                                       UINT32 bpp, UINT32 cpal, RGBQUAD *pPal, void *pUserInfo= nullptr) override;

            bool OpenOutputFile(LPCTSTR pFileName) override;
            bool CloseOutputFile() override;
            void SetMultiPageStatus(DibMultiPageStruct *pStruct) override;
            void GetMultiPageStatus(DibMultiPageStruct *pStruct) override;
            LONG GetErrorCode() const { return m_nError; }
            static int  Tiff2PS(LPCTSTR szFileIn, LPCTSTR szFileOut, LONG PSType,
                                LPCTSTR szTitle, bool IsEncapsulated);
            void DestroyAllObjects() override;

        protected:
            int ProcessCompressionType(fipImage& im, unsigned long&) const;
    };
}
#endif
