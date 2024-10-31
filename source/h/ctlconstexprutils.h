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
#ifndef CTLCONSTEXPRUTILS_H
#define CTLCONSTEXPRUTILS_H

#include "ctlconstexprfind.h"
#include "twain.h"
#include "ctlenum.h"

namespace dynarithmic
{
    static constexpr bool IsTwainStringType(TW_UINT16 nItemType)
    {
        switch (nItemType)
        {
            case TWTY_STR32:
            case TWTY_STR64:
            case TWTY_STR128:
            case TWTY_STR255:
            case TWTY_STR1024:
            case TWTY_UNI512:
                return true;
        }
        return false;
    }

    static constexpr bool IsTwainShortStringType(TW_UINT16 nItemType)
    {
        switch (nItemType)
        {
            case TWTY_STR32:
            case TWTY_STR64:
            case TWTY_STR128:
            case TWTY_STR255:
                return true;
        }
        return false;
    }

    static constexpr bool IsTwainLongStringType(TW_UINT16 nItemType)
    {
        switch (nItemType)
        {
            case TWTY_STR1024:
                return true;
        }
        return false;
    }

    static constexpr bool IsTwainUnicodeStringType(TW_UINT16 nItemType)
    {
        switch (nItemType)
        {
            case TWTY_UNI512:
                return true;
        }
        return false;
    }

    static constexpr bool IsTwainIntegralType(TW_UINT16 nItemType)
    {
        switch (nItemType)
        {
            case TWTY_INT8:
            case TWTY_INT16:
            case TWTY_INT32:
            case TWTY_UINT8:
            case TWTY_UINT16:
            case TWTY_UINT32:
            case TWTY_BOOL:
                return true;
        }
        return false;
    }

    static constexpr bool IsTwainHandleType(TW_UINT16 nItemType)
    {
        return nItemType == TWTY_HANDLE;
    }

    static constexpr bool IsTwainFix32Type(TW_UINT16 nItemType)
    {
        return nItemType == TWTY_FIX32;
    }

    static constexpr bool IsTwainFrameType(TW_UINT16 nItemType)
    {
        return nItemType == TWTY_FRAME;
    }

    static constexpr TW_UINT16 GetTwainItemSize(TW_UINT16 nItemType)
    {
        switch (nItemType)
        {
            case TWTY_INT8:
                return sizeof(TW_INT8);
            case TWTY_INT16:
                return sizeof(TW_INT16);
            case TWTY_INT32:
                return sizeof(TW_INT32);
            case TWTY_UINT8:
                return sizeof(TW_UINT8);
            case TWTY_UINT16:
                return sizeof(TW_UINT16);
            case TWTY_UINT32:
                return sizeof(TW_UINT32);
            case TWTY_BOOL:
                return sizeof(TW_BOOL);
            case TWTY_FIX32:
                return sizeof(TW_FIX32);
            case TWTY_FRAME:
                return sizeof(TW_FRAME);
            case TWTY_STR32:
                return sizeof(TW_STR32);
            case TWTY_STR64:
                return sizeof(TW_STR64);
            case TWTY_STR128:
                return sizeof(TW_STR128);
            case TWTY_STR255:
                return sizeof(TW_STR255);
            case TWTY_STR1024:
                return sizeof(TW_STR1024);
            case TWTY_UNI512:
                return sizeof(TW_UNI512);
            case TWTY_HANDLE:
                return sizeof(TW_HANDLE);
        }
        return 0;
    }

    static constexpr bool IsFileTypeMultiPage(CTL_TwainFileFormatEnum FileType)
    {
        return FileType == TWAINFileFormat_TIFFGROUP3MULTI ||
            FileType == TWAINFileFormat_TIFFGROUP4MULTI ||
            FileType == TWAINFileFormat_TIFFNONEMULTI ||
            FileType == TWAINFileFormat_TIFFJPEGMULTI ||
            FileType == TWAINFileFormat_TIFFPACKBITSMULTI ||
            FileType == TWAINFileFormat_TIFFDEFLATEMULTI ||
            FileType == TWAINFileFormat_PDFMULTI ||
            FileType == TWAINFileFormat_POSTSCRIPT1MULTI ||
            FileType == TWAINFileFormat_POSTSCRIPT2MULTI ||
            FileType == TWAINFileFormat_POSTSCRIPT3MULTI ||
            FileType == TWAINFileFormat_TIFFLZWMULTI ||
            FileType == TWAINFileFormat_TIFFPIXARLOGMULTI ||
            FileType == TWAINFileFormat_DCX ||
            FileType == TWAINFileFormat_TEXTMULTI ||
            FileType == TWAINFileFormat_BIGTIFFLZWMULTI ||
            FileType == TWAINFileFormat_BIGTIFFNONEMULTI ||
            FileType == TWAINFileFormat_BIGTIFFPACKBITSMULTI ||
            FileType == TWAINFileFormat_BIGTIFFDEFLATEMULTI ||
            FileType == TWAINFileFormat_BIGTIFFGROUP3MULTI ||
            FileType == TWAINFileFormat_BIGTIFFGROUP4MULTI ||
            FileType == TWAINFileFormat_BIGTIFFJPEGMULTI ||
            FileType == DTWAIN_FF_TIFFMULTI;
    }

    static constexpr bool IsFileTypeTIFF(CTL_TwainFileFormatEnum FileType)
    {
        constexpr std::array<CTL_TwainFileFormatEnum,29> setInfo = {
                                TWAINFileFormat_TIFFGROUP3MULTI,
                                TWAINFileFormat_TIFFGROUP4MULTI,
                                TWAINFileFormat_TIFFNONEMULTI,
                                TWAINFileFormat_TIFFJPEGMULTI,
                                TWAINFileFormat_TIFFPACKBITSMULTI,
                                TWAINFileFormat_TIFFDEFLATEMULTI,
                                TWAINFileFormat_TIFFLZWMULTI,
                                TWAINFileFormat_TIFFGROUP4,
                                TWAINFileFormat_TIFFGROUP3,
                                TWAINFileFormat_TIFFNONE,
                                TWAINFileFormat_TIFFJPEG,
                                TWAINFileFormat_TIFFPACKBITS,
                                TWAINFileFormat_TIFFDEFLATE,
                                TWAINFileFormat_TIFFPIXARLOG,
                                TWAINFileFormat_TIFFLZW,
                                TWAINFileFormat_BIGTIFFLZW,
                                TWAINFileFormat_BIGTIFFLZWMULTI,
                                TWAINFileFormat_BIGTIFFNONE,
                                TWAINFileFormat_BIGTIFFNONEMULTI,
                                TWAINFileFormat_BIGTIFFPACKBITS,
                                TWAINFileFormat_BIGTIFFPACKBITSMULTI,
                                TWAINFileFormat_BIGTIFFDEFLATE,
                                TWAINFileFormat_BIGTIFFDEFLATEMULTI,
                                TWAINFileFormat_BIGTIFFGROUP3,
                                TWAINFileFormat_BIGTIFFGROUP3MULTI,
                                TWAINFileFormat_BIGTIFFGROUP4,
                                TWAINFileFormat_BIGTIFFGROUP4MULTI,
                                TWAINFileFormat_BIGTIFFJPEG,
                                TWAINFileFormat_BIGTIFFJPEGMULTI };
        return generic_array_finder(setInfo, FileType).first;
    }

    static constexpr bool IsFileTypeBigTiff(CTL_TwainFileFormatEnum FileType)
    {
        constexpr std::array<CTL_TwainFileFormatEnum, 14> setInfo = {
                                TWAINFileFormat_BIGTIFFLZWMULTI,
                                TWAINFileFormat_BIGTIFFLZW,
                                TWAINFileFormat_BIGTIFFNONEMULTI,
                                TWAINFileFormat_BIGTIFFNONE,
                                TWAINFileFormat_BIGTIFFPACKBITS,
                                TWAINFileFormat_BIGTIFFPACKBITSMULTI,
                                TWAINFileFormat_BIGTIFFDEFLATE,
                                TWAINFileFormat_BIGTIFFDEFLATEMULTI,
                                TWAINFileFormat_BIGTIFFGROUP3,
                                TWAINFileFormat_BIGTIFFGROUP3MULTI,
                                TWAINFileFormat_BIGTIFFGROUP4,
                                TWAINFileFormat_BIGTIFFGROUP4MULTI,
                                TWAINFileFormat_BIGTIFFJPEG,
                                TWAINFileFormat_BIGTIFFJPEGMULTI };
        return generic_array_finder(setInfo, FileType).first;
    }

    static constexpr bool IsFileTypePostscript(CTL_TwainFileFormatEnum FileType)
    {
        return FileType == TWAINFileFormat_POSTSCRIPT1 ||
            FileType == TWAINFileFormat_POSTSCRIPT1MULTI ||
            FileType == TWAINFileFormat_POSTSCRIPT2 ||
            FileType == TWAINFileFormat_POSTSCRIPT2MULTI ||
            FileType == TWAINFileFormat_POSTSCRIPT3 ||
            FileType == TWAINFileFormat_POSTSCRIPT3MULTI;
    }

    static constexpr CTL_TwainFileFormatEnum GetMultiPageType(CTL_TwainFileFormatEnum FileType)
    {
        constexpr std::array<std::pair<CTL_TwainFileFormatEnum, CTL_TwainFileFormatEnum>, 21> pageMap =
        { {
            { TWAINFileFormat_TIFFGROUP3,      TWAINFileFormat_TIFFGROUP3MULTI },
            { TWAINFileFormat_TIFFGROUP4,      TWAINFileFormat_TIFFGROUP4MULTI },
            { TWAINFileFormat_TIFFNONE,        TWAINFileFormat_TIFFNONEMULTI },
            { TWAINFileFormat_TIFFJPEG,        TWAINFileFormat_TIFFJPEGMULTI },
            { TWAINFileFormat_TIFFPACKBITS,    TWAINFileFormat_TIFFPACKBITSMULTI },
            { TWAINFileFormat_TIFFDEFLATE,     TWAINFileFormat_TIFFDEFLATEMULTI },
            { TWAINFileFormat_PDF,             TWAINFileFormat_PDFMULTI },
            { TWAINFileFormat_POSTSCRIPT1,     TWAINFileFormat_POSTSCRIPT1MULTI },
            { TWAINFileFormat_POSTSCRIPT2,     TWAINFileFormat_POSTSCRIPT2MULTI },
            { TWAINFileFormat_POSTSCRIPT3,     TWAINFileFormat_POSTSCRIPT3MULTI },
            { TWAINFileFormat_TIFFLZW,         TWAINFileFormat_TIFFLZWMULTI },
            { TWAINFileFormat_BIGTIFFLZW,      TWAINFileFormat_BIGTIFFLZWMULTI },
            { TWAINFileFormat_BIGTIFFNONE,     TWAINFileFormat_BIGTIFFNONEMULTI },
            { TWAINFileFormat_BIGTIFFPACKBITS, TWAINFileFormat_BIGTIFFPACKBITSMULTI },
            { TWAINFileFormat_BIGTIFFDEFLATE,  TWAINFileFormat_BIGTIFFDEFLATEMULTI },
            { TWAINFileFormat_BIGTIFFGROUP3,   TWAINFileFormat_BIGTIFFGROUP3MULTI },
            { TWAINFileFormat_BIGTIFFGROUP4,   TWAINFileFormat_BIGTIFFGROUP4MULTI },
            { TWAINFileFormat_BIGTIFFJPEG,     TWAINFileFormat_BIGTIFFJPEGMULTI },
            { TWAINFileFormat_TIFFPIXARLOG,    TWAINFileFormat_TIFFPIXARLOGMULTI },
            { TWAINFileFormat_PCX,             TWAINFileFormat_DCX },
            { TWAINFileFormat_TEXT,            TWAINFileFormat_TEXTMULTI }
        } };

        const auto iter = generic_array_finder_if(pageMap, [&](auto& pr) { return pr.first == FileType; });
        if (iter.first)
            return pageMap[iter.second].second;
        return FileType;
    }

    /////////////// Supported formats for File transfers ////////////////
    static constexpr auto EnumTwainFileFormats()
    {
        constexpr std::array<int, 58> ca = {
            TWAINFileFormat_BMP, TWAINFileFormat_BMPRLE, TWAINFileFormat_PCX, TWAINFileFormat_DCX, TWAINFileFormat_TIFFLZW,
            TWAINFileFormat_PDF, TWAINFileFormat_PDFMULTI, TWAINFileFormat_TIFFNONE, TWAINFileFormat_TIFFGROUP3,TWAINFileFormat_TIFFGROUP4,
            TWAINFileFormat_TIFFPACKBITS, TWAINFileFormat_TIFFDEFLATE, TWAINFileFormat_TIFFJPEG, TWAINFileFormat_TIFFNONEMULTI,
            TWAINFileFormat_TIFFGROUP3MULTI, TWAINFileFormat_TIFFGROUP4MULTI, TWAINFileFormat_TIFFPACKBITSMULTI,TWAINFileFormat_TIFFDEFLATEMULTI,TWAINFileFormat_TIFFJPEGMULTI,
            TWAINFileFormat_TIFFLZWMULTI ,TWAINFileFormat_WMF,TWAINFileFormat_EMF,TWAINFileFormat_PSD,TWAINFileFormat_JPEG,TWAINFileFormat_TGA,
            TWAINFileFormat_JPEG2000,TWAINFileFormat_POSTSCRIPT1,TWAINFileFormat_POSTSCRIPT1MULTI,TWAINFileFormat_POSTSCRIPT2,TWAINFileFormat_POSTSCRIPT2MULTI,
            TWAINFileFormat_POSTSCRIPT3,TWAINFileFormat_POSTSCRIPT3MULTI,TWAINFileFormat_GIF,TWAINFileFormat_PNG,TWAINFileFormat_TEXT,
            TWAINFileFormat_TEXTMULTI,TWAINFileFormat_ICO,TWAINFileFormat_ICO_VISTA, TwainFileFormat_ICO_RESIZED, TwainFileFormat_WBMP_RESIZED,
            TWAINFileFormat_WBMP, TWAINFileFormat_WEBP, TWAINFileFormat_PBM, TWAINFileFormat_TGARLE, TWAINFileFormat_BIGTIFFLZW, TWAINFileFormat_BIGTIFFLZWMULTI,
            TWAINFileFormat_BIGTIFFNONE, TWAINFileFormat_BIGTIFFNONEMULTI, TWAINFileFormat_BIGTIFFPACKBITS, TWAINFileFormat_BIGTIFFPACKBITSMULTI,
            TWAINFileFormat_BIGTIFFDEFLATE, TWAINFileFormat_BIGTIFFDEFLATEMULTI, TWAINFileFormat_BIGTIFFGROUP3, TWAINFileFormat_BIGTIFFGROUP3MULTI,
            TWAINFileFormat_BIGTIFFGROUP4, TWAINFileFormat_BIGTIFFGROUP4MULTI, TWAINFileFormat_BIGTIFFJPEG, TWAINFileFormat_BIGTIFFJPEGMULTI };
        return ca;
    }

    static constexpr bool IsSupportedFileFormat(int nFileFormat)
    {
        auto iArray = EnumTwainFileFormats();
        if (iArray.empty())
            return nFileFormat != TWFF_BMP ? false : true;
        return generic_array_finder(iArray, nFileFormat).first;
    }

    static constexpr LONG GetArrayTypeFromCapType(TW_UINT16 CapType)
    {
        if (IsTwainShortStringType(CapType) || IsTwainLongStringType(CapType))
            return DTWAIN_ARRAYANSISTRING;
        if ( IsTwainFrameType(CapType))
            return DTWAIN_ARRAYFRAME;
        if ( IsTwainFix32Type(CapType))
            return DTWAIN_ARRAYFLOAT;
        return DTWAIN_ARRAYLONG;
    }

    static constexpr LONG ExtImageInfoArrayType(LONG ExtType)
    {
        TW_UINT16 actualType = static_cast<TW_UINT16>(ExtType);
        if (IsTwainIntegralType(actualType))
            return DTWAIN_ARRAYLONG;
        if (IsTwainStringType(actualType))
            return DTWAIN_ARRAYANSISTRING;
        if (IsTwainHandleType(actualType))
            return DTWAIN_ARRAYHANDLE;
        if (IsTwainFrameType(actualType))
            return DTWAIN_ARRAYFRAME;
        if (IsTwainFix32Type(actualType))
            return DTWAIN_ARRAYFLOAT;
        return DTWAIN_ARRAYLONG;
    }

    static constexpr int GetCapMaskFromCap(CTL_EnumCapability Cap)
    {
        // Jump table
        constexpr int CapAll = CTL_CapMaskGET | CTL_CapMaskGETCURRENT | CTL_CapMaskGETDEFAULT |
            CTL_CapMaskSET | CTL_CapMaskRESET;

        constexpr int CapSupport = CTL_CapMaskGET | CTL_CapMaskGETCURRENT | CTL_CapMaskGETDEFAULT |
            CTL_CapMaskRESET;
        constexpr int CapAllGets = CTL_CapMaskGET | CTL_CapMaskGETCURRENT | CTL_CapMaskGETDEFAULT;

        switch (Cap)
        {
            case TwainCap_XFERCOUNT:
            case TwainCap_AUTOFEED:
            case TwainCap_CLEARPAGE:
            case TwainCap_REWINDPAGE:
                return CapAll;

            case TwainCap_COMPRESSION:
            case TwainCap_PIXELTYPE:
            case TwainCap_UNITS:
            case TwainCap_XFERMECH:
            case TwainCap_BITDEPTH:
            case TwainCap_BITORDER:
            case TwainCap_XRESOLUTION:
            case TwainCap_YRESOLUTION:
                return CapSupport;

            case TwainCap_UICONTROLLABLE:
            case TwainCap_SUPPORTEDCAPS:
                return CTL_CapMaskGET;

            case TwainCap_PLANARCHUNKY:
            case TwainCap_PHYSICALHEIGHT:
            case TwainCap_PHYSICALWIDTH:
            case TwainCap_PIXELFLAVOR:
            case TwainCap_FEEDERENABLED:
            case TwainCap_FEEDERLOADED:
                return CapAllGets;
        }
        return 0;
    }

    static constexpr float Fix32ToFloat(TW_FIX32 Fix32) noexcept
    {
        return static_cast<float>(Fix32.Whole) + static_cast<float>(Fix32.Frac) / static_cast<float>(65536.0);
    }

    static constexpr TW_FIX32 FloatToFix32(float fnum) noexcept
    {
        TW_FIX32 fix32_value{};
        const bool sign = fnum < 0 ? true : false;
        auto value = static_cast<TW_INT32>(fnum * 65536.0 + (sign ? -0.5 : 0.5));
        fix32_value.Whole = static_cast<TW_INT16>(value >> 16);
        fix32_value.Frac = static_cast<TW_UINT16>(value & 0x0000ffffL);
        return fix32_value;
    }
};
#endif