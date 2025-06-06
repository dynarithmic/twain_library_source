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
#ifndef CTLCONSTEXPRUTILS_H
#define CTLCONSTEXPRUTILS_H

#include "ctlconstexprfind.h"
#include "twain.h"
#include "ctlenum.h"
#include "dtwain_version.h"

namespace dynarithmic
{
    struct RawTwainTriplet
    {
        TW_UINT32    nDG;
        TW_UINT16    nDAT;
        TW_UINT16    nMSG;
        constexpr RawTwainTriplet(TW_UINT32 dg, TW_UINT16 dat, TW_UINT16 msg) : nDG(dg), nDAT(dat), nMSG(msg) {}
    };

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
        return generic_array_finder(EnumTwainFileFormats(), nFileFormat).first;
    }

    static constexpr LONG GetArrayTypeFromCapType(TW_UINT16 CapType) noexcept
    {
        if (IsTwainShortStringType(CapType) || IsTwainLongStringType(CapType))
            return DTWAIN_ARRAYANSISTRING;
        if ( IsTwainFrameType(CapType))
            return DTWAIN_ARRAYFRAME;
        if ( IsTwainFix32Type(CapType))
            return DTWAIN_ARRAYFLOAT;
        return DTWAIN_ARRAYLONG;
    }

    static constexpr LONG ExtImageInfoArrayType(LONG ExtType) noexcept
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

    static constexpr int GetCapMaskFromCap(TW_UINT16  Cap) noexcept
    {
        // Jump table
        constexpr int CapAll = CTL_CapMaskGET | CTL_CapMaskGETCURRENT | CTL_CapMaskGETDEFAULT |
            CTL_CapMaskSET | CTL_CapMaskRESET;

        constexpr int CapSupport = CTL_CapMaskGET | CTL_CapMaskGETCURRENT | CTL_CapMaskGETDEFAULT |
            CTL_CapMaskRESET;
        constexpr int CapAllGets = CTL_CapMaskGET | CTL_CapMaskGETCURRENT | CTL_CapMaskGETDEFAULT;

        switch (Cap)
        {
            case CAP_XFERCOUNT:
            case CAP_AUTOFEED:
            case CAP_CLEARPAGE:
            case CAP_REWINDPAGE:
                return CapAll;

            case ICAP_COMPRESSION:
            case ICAP_PIXELTYPE:
            case ICAP_UNITS:
            case ICAP_XFERMECH:
            case ICAP_BITDEPTH:
            case ICAP_BITORDER:
            case ICAP_XRESOLUTION:
            case ICAP_YRESOLUTION:
                return CapSupport;

            case CAP_UICONTROLLABLE:
            case CAP_SUPPORTEDCAPS:
                return CTL_CapMaskGET;

            case ICAP_PLANARCHUNKY:
            case ICAP_PHYSICALHEIGHT:
            case ICAP_PHYSICALWIDTH:
            case ICAP_PIXELFLAVOR:
            case CAP_FEEDERENABLED:
            case CAP_FEEDERLOADED:
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

    static constexpr LONG GetTwainGetType(LONG gettype) noexcept
    {
        switch (gettype)
        {
            case DTWAIN_CAPGETHELP:
                return MSG_GETHELP;

            case DTWAIN_CAPGETLABEL:
                return MSG_GETLABEL;

            case DTWAIN_CAPGETLABELENUM:
                return MSG_GETLABELENUM;
        }
        return gettype;
    }

    static constexpr bool IsMSGGetType(TW_UINT16 msgType) noexcept
    {
        return  msgType == MSG_GET ||
            msgType == MSG_GETCURRENT ||
            msgType == MSG_GETDEFAULT ||
            msgType == MSG_GETHELP ||
            msgType == MSG_GETLABEL ||
            msgType == MSG_GETLABELENUM;
    }

    static constexpr bool IsMSGSetType(TW_UINT16 msgType) noexcept
    {
        return msgType == MSG_SET || msgType == MSG_SETCONSTRAINT;
    }

    static constexpr bool IsMSGResetType(TW_UINT16 msgType) noexcept
    {
        return msgType == MSG_RESET || msgType == MSG_RESETALL;
    }

    static constexpr bool IsMSGSetOrResetType(TW_UINT16 msgType) noexcept
    {
        return IsMSGSetType(msgType) || IsMSGResetType(msgType);
    }

    static constexpr bool IsTwainDSM2(long DSMType) noexcept
    {
        return DSMType == DTWAIN_TWAINDSM_VERSION2 ||
               DSMType == DTWAIN_TWAINDSM_LATESTVERSION;
    }

    static constexpr std::pair<std::array<const char*, 4>, int> GetContainerNamesFromType(int nType) noexcept
    {
        using arrayret_type = std::array<const char*, 4>;
        using arrayintret_type = std::pair<arrayret_type, int>;
        using arrayall_type = std::array<arrayintret_type, 16>;
        
        constexpr arrayall_type retValues = {{
            {{""},0},
            {{"TW_ARRAY"},1},
            {{"TW_ENUMERATION"},1},
            {{"TW_ARRAY", "TW_ENUMERATION"},2},
            {{"TW_ONEVALUE"},1},
            {{"TW_ONEVALUE", "TW_ARRAY"}, 2},
            {{"TW_ONEVALUE", "TW_ENUMERATION"}, 2},
            {{"TW_ONEVALUE", "TW_ENUMERATION", "TW_ARRAY"}, 3},
            {{"TW_RANGE"}, 1},
            {{"TW_ARRAY", "TW_RANGE"}, 2},
            {{"TW_ENUMERATION", "TW_RANGE"}, 2},
            {{"TW_ARRAY", "TW_ENUMERATION", "TW_RANGE"}, 3},
            {{"TW_ONEVALUE", "TW_RANGE"}, 2},
            {{"TW_ARRAY", "TW_ONEVALUE", "TW_RANGE"}, 3},
            {{"TW_ONEVALUE", "TW_ENUMERATION", "TW_RANGE"}, 3},
            {{"TW_ARRAY", "TW_ENUMERATION", "TW_ONEVALUE", "TW_RANGE"}, 4}
            }};

        auto nShifted = nType >> 3;
        if (nShifted > 15)
            nShifted = 0;
        return retValues[nShifted];
    }

    static constexpr bool IsCapMaskOnGet(TW_UINT16 Cap, TW_UINT16 GetType) noexcept
    {
        return (GetCapMaskFromCap(Cap) & GetType);
    }


    static constexpr bool IsCapMaskOnSet(TW_UINT16 Cap, TW_UINT16 SetType) noexcept
    {
        return (GetCapMaskFromCap(Cap) & SetType);
    }

    static constexpr std::array<int, 3> GetDTWAINDLLVersionInfo() noexcept
    {
        constexpr std::array<int, 3> aDLLVersion = { DTWAIN_MAJOR_VERSION,DTWAIN_MINOR_VERSION, DTWAIN_PATCHLEVEL_VERSION };
        return aDLLVersion;
    }

    static constexpr bool IsTimeOutTripletIgnored(const RawTwainTriplet& trip)
    {
        constexpr std::array<RawTwainTriplet, 7> Trips = {
            {{DG_AUDIO, DAT_AUDIOFILEXFER, MSG_GET},
            {DG_AUDIO, DAT_AUDIONATIVEXFER, MSG_GET},
            {DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDS},
            {DG_CONTROL, DAT_USERINTERFACE, MSG_ENABLEDSUIONLY},
            {DG_IMAGE, DAT_IMAGEFILEXFER, MSG_GET},
            {DG_IMAGE, DAT_IMAGENATIVEXFER, MSG_GET},
            {DG_IMAGE, DAT_IMAGEMEMXFER, MSG_GET}} };
        
        auto isFoundPr = generic_array_finder_if(Trips, [&](auto& trip2)
            { return std::tie(trip.nDG, trip.nDAT, trip.nMSG) == std::tie(trip2.nDG, trip2.nDAT, trip2.nMSG); });
        return isFoundPr.first;
    }

    static constexpr LONG GetDTWAINVersionType() noexcept
    {
        LONG lVersionType = 0;
        #ifdef UNICODE
            lVersionType |= DTWAIN_UNICODE_VERSION;
        #endif

        #ifdef DTWAIN_DEBUG
            lVersionType |= DTWAIN_DEVELOP_VERSION;
        #endif

        #if defined (WIN64) || defined(_WIN64)
            lVersionType |= DTWAIN_64BIT_VERSION;
            #else
            #if defined (WIN32) || defined(_WIN32)
                lVersionType |= DTWAIN_32BIT_VERSION;
            #endif
        #endif

        #ifdef DTWAIN_DEVELOP_DLL
            lVersionType |= DTWAIN_DEVELOP_VERSION;
        #endif

        #if DTWAIN_BUILD_LOGCALLSTACK == 1
            lVersionType |= DTWAIN_CALLSTACK_LOGGING;
        #endif

        #if DTWAIN_BUILD_LOGCALLSTACK == 1 && DTWAIN_BUILD_LOGPOINTERS == 1
            lVersionType |= DTWAIN_CALLSTACK_LOGGING_PLUS;
        #endif
            return lVersionType;
    }
};

#endif