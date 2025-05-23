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
#ifndef CTLENUM_H
#define CTLENUM_H
#include "dtwain_filetypes.h"
#include "dtwaindefs.h"
#include "twain.h"

namespace dynarithmic
{
    enum CTL_TwainMessageEnum {   CTL_MsgSourceClosed = 1,
                                  CTL_MsgSourceClosing
    };

    enum CTL_TwainAcquireEnum {     TWAINAcquireType_Native = TWSX_NATIVE,
                                    TWAINAcquireType_File   = TWSX_FILE,
                                    TWAINAcquireType_Buffer   = TWSX_MEMORY,
                                    TWAINAcquireType_MemFile = TWSX_MEMFILE,
                                    TWAINAcquireType_AudioNative = 0xFA,
                                    TWAINAcquireType_AudioFile = 0xFB,
                                    TWAINAcquireType_FileUsingBuffered = 0xFC,
                                    TWAINAcquireType_FileUsingNative = 0xFD,
                                    TWAINAcquireType_Clipboard = 0xFE,
                                    TWAINAcquireType_Invalid
                            };

    enum CTL_TwainFileFormatEnum { TWAINFileFormat_Invalid = -1,
                                   TWAINFileFormat_TWAINBMP                  = DTWAIN_FF_BMP,
                                   TWAINFileFormat_TWAINJFIF                 = DTWAIN_FF_JFIF,
                                   TWAINFileFormat_TWAINSPIFF                = DTWAIN_FF_SPIFF,
                                   TWAINFileFormat_TWAINEXIF                 = DTWAIN_FF_EXIF,
                                   TWAINFileFormat_TWAINPNG                  = DTWAIN_FF_PNG,
                                   TWAINFileFormat_TWAINFPX                  = DTWAIN_FF_FPX,
                                   TWAINFileFormat_TWAINPICT                 = DTWAIN_FF_PICT,
                                   TWAINFileFormat_TWAINXBM                  = DTWAIN_FF_XBM,
                                   TWAINFileFormat_TWAINTIFF                 = DTWAIN_FF_TIFF,
                                   TWAINFileFormat_TWAINTIFFMULTI            = DTWAIN_FF_TIFFMULTI,
                                   TWAINFileFormat_BMP                       = DTWAIN_BMP,
                                   TWAINFileFormat_BMPRLE                    = DTWAIN_BMP_RLE,
                                   TWAINFileFormat_JPEG                      = DTWAIN_JPEG,
                                   TWAINFileFormat_PDF                       = DTWAIN_PDF,
                                   TWAINFileFormat_PDFMULTI                  = DTWAIN_PDFMULTI,

                                   TWAINFileFormat_PCX                       = DTWAIN_PCX,
                                   TWAINFileFormat_DCX                       = DTWAIN_DCX,
                                   TWAINFileFormat_TGA                       = DTWAIN_TGA,
                                   TWAINFileFormat_TGARLE                    = DTWAIN_TGA_RLE, 
                                   TWAINFileFormat_TIFFLZW                   = DTWAIN_TIFFLZW,
                                   TWAINFileFormat_TIFFNONE                  = DTWAIN_TIFFNONE,
                                   TWAINFileFormat_TIFFGROUP3                = DTWAIN_TIFFG3,
                                   TWAINFileFormat_TIFFGROUP4                = DTWAIN_TIFFG4,
                                   TWAINFileFormat_TIFFPACKBITS              = DTWAIN_TIFFPACKBITS,
                                   TWAINFileFormat_TIFFDEFLATE               = DTWAIN_TIFFDEFLATE,
                                   TWAINFileFormat_TIFFJPEG                  = DTWAIN_TIFFJPEG,
                                   TWAINFileFormat_TIFFPIXARLOG              = DTWAIN_TIFFPIXARLOG,
                                   TWAINFileFormat_TIFFPIXARLOGMULTI         = DTWAIN_TIFFPIXARLOGMULTI,
                                   TWAINFileFormat_TIFFNONEMULTI             = DTWAIN_TIFFNONEMULTI,
                                   TWAINFileFormat_TIFFGROUP3MULTI           = DTWAIN_TIFFG3MULTI, TWAINFileFormat_TIFFGROUP4MULTI
                                   = DTWAIN_TIFFG4MULTI, TWAINFileFormat_TIFFPACKBITSMULTI         = DTWAIN_TIFFPACKBITSMULTI,
                                   TWAINFileFormat_TIFFDEFLATEMULTI          = DTWAIN_TIFFDEFLATEMULTI,
                                   TWAINFileFormat_TIFFJPEGMULTI             = DTWAIN_TIFFJPEGMULTI, TWAINFileFormat_TIFFLZWMULTI
                                   = DTWAIN_TIFFLZWMULTI, TWAINFileFormat_POSTSCRIPT1               = DTWAIN_POSTSCRIPT1,
                                   TWAINFileFormat_POSTSCRIPT1MULTI          = DTWAIN_POSTSCRIPT1MULTI, TWAINFileFormat_POSTSCRIPT2
                                   = DTWAIN_POSTSCRIPT2, TWAINFileFormat_POSTSCRIPT2MULTI          = DTWAIN_POSTSCRIPT2MULTI,
                                   TWAINFileFormat_POSTSCRIPT3               = DTWAIN_POSTSCRIPT3, TWAINFileFormat_POSTSCRIPT3MULTI
                                   = DTWAIN_POSTSCRIPT3MULTI,
                                   TWAINFileFormat_TEXT                      = DTWAIN_TEXT,
                                   TWAINFileFormat_TEXTMULTI                 = DTWAIN_TEXTMULTI,
                                   TWAINFileFormat_WEBP                      = DTWAIN_WEBP,
                                   TWAINFileFormat_PBM                       = DTWAIN_PBM,

                                   TWAINFileFormat_WMF = DTWAIN_WMF,
                                   TWAINFileFormat_EMF = DTWAIN_EMF,
                                   TWAINFileFormat_GIF      = DTWAIN_GIF,
                                   TWAINFileFormat_PNG      =DTWAIN_PNG,
                                   TWAINFileFormat_PSD      =DTWAIN_PSD,
                                   TWAINFileFormat_JBIG     =5000,
                                   TWAINFileFormat_JPEG2000 = DTWAIN_JPEG2000,
                                   TWAINFileFormat_ICO      = DTWAIN_ICO,
                                   TWAINFileFormat_ICO_VISTA = DTWAIN_ICO_VISTA,
                                   TwainFileFormat_ICO_RESIZED = DTWAIN_ICO_RESIZED,
                                   TwainFileFormat_WBMP_RESIZED = DTWAIN_WBMP_RESIZED,
                                   TWAINFileFormat_WBMP     = DTWAIN_WBMP,

                                   TWAINFileFormat_BIGTIFFLZW = DTWAIN_BIGTIFFLZW,
                                   TWAINFileFormat_BIGTIFFLZWMULTI = DTWAIN_BIGTIFFLZWMULTI,
                                   TWAINFileFormat_BIGTIFFNONE = DTWAIN_BIGTIFFNONE,
                                   TWAINFileFormat_BIGTIFFNONEMULTI = DTWAIN_BIGTIFFNONEMULTI,
                                   TWAINFileFormat_BIGTIFFPACKBITS = DTWAIN_BIGTIFFPACKBITS,
                                   TWAINFileFormat_BIGTIFFPACKBITSMULTI = DTWAIN_BIGTIFFPACKBITSMULTI,
                                   TWAINFileFormat_BIGTIFFDEFLATE = DTWAIN_BIGTIFFDEFLATE,
                                   TWAINFileFormat_BIGTIFFDEFLATEMULTI = DTWAIN_BIGTIFFDEFLATEMULTI,
                                   TWAINFileFormat_BIGTIFFGROUP3 = DTWAIN_BIGTIFFG3,
                                   TWAINFileFormat_BIGTIFFGROUP3MULTI = DTWAIN_BIGTIFFG3MULTI,
                                   TWAINFileFormat_BIGTIFFGROUP4 = DTWAIN_BIGTIFFG4,
                                   TWAINFileFormat_BIGTIFFGROUP4MULTI = DTWAIN_BIGTIFFG4MULTI,
                                   TWAINFileFormat_BIGTIFFJPEG = DTWAIN_BIGTIFFJPEG,
                                   TWAINFileFormat_BIGTIFFJPEGMULTI = DTWAIN_BIGTIFFJPEGMULTI,
                                   TWAINFileFormat_RAW = 9999
                                };


    enum CTL_TwainFileFlags {  TWAINFileFlag_DEFAULTOPTIONS = 0,
                               TWAINFileFlag_USENATIVE  = 1,
                               TWAINFileFlag_USEBUFFERED  = 2,
                               TWAINFileFlag_USENAME    = 4,
                               TWAINFileFlag_PROMPT     = 8,
                               TWAINFileFlag_USELONGNAMES = 16,
                               TWAINFileFlag_USESOURCEMODE = 32,
                               TWAINFileFlag_USELIST    = 64 };

    enum CTL_TwainUnitEnum  {   TwainUnit_INVALID       =   -1,
                                TwainUnit_INCHES        =  TWUN_INCHES,
                                TwainUnit_CENTIMETERS   =  TWUN_CENTIMETERS,
                                TwainUnit_PICAS         =  TWUN_PICAS,
                                TwainUnit_POINTS        =  TWUN_POINTS,
                                TwainUnit_TWIPS         =  TWUN_TWIPS,
                                TwainUnit_PIXELS        =  TWUN_PIXELS
                            };

        enum CTL_EnumCapMask {  CTL_CapMaskGET          = 1,
                            CTL_CapMaskGETCURRENT   = 2,
                            CTL_CapMaskGETDEFAULT   = 4,
                            CTL_CapMaskSET          = 8,
                            CTL_CapMaskRESET        = 16,
                            CTL_CapMaskRESETALL     = 32,
                            CTL_CapMaskSETCONSTRAINT = 64,
                         };

    enum CTL_EnumTransferType { CTL_XferTypeNATIVE,
                                CTL_XferTypeMEMORY,
                                CTL_XferTypeFILE
                              };

    enum CTL_EnumTwainVersion { CTL_TwainVersion15 = 0,
                                CTL_TwainVersion16 = 1,
                                CTL_TwainVersion17 = 2,
                                CTL_TwainVersion18 = 3,
                                CTL_TwainVersion19 = 4,
                                CTL_TwainVersion20 = 5,
                                CTL_TwainVersion21 = 6,
                                CTL_TwainVersion22 = 7,
                                CTL_TwainVersion23 = 8,
    };
    
    #define  TwainCap_INVALID              -1

    enum CTL_EnumContainer {
             TwainContainer_ONEVALUE    = DTWAIN_CONTONEVALUE,
             TwainContainer_ENUMERATION = DTWAIN_CONTENUMERATION,
             TwainContainer_RANGE       = DTWAIN_CONTRANGE,
             TwainContainer_ARRAY       = DTWAIN_CONTARRAY,
             TwainContainer_INVALID     = 0
    };

    enum CTL_EnumTwainRange {
            TwainRange_MIN = 0,
            TwainRange_MAX,
            TwainRange_STEP,
            TwainRange_DEFAULT,
            TwainRange_CURRENT
    };
}
#endif

