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
#ifndef DTWAIN_PDFTEXT_H
#define DTWAIN_PDFTEXT_H

/* PDF Text options */

/* Which page to print text */

/* Start of DTWAIN constant definitions */
#define DTWAIN_PDFTEXT_ALLPAGES       0x00000001
#define DTWAIN_PDFTEXT_EVENPAGES      0x00000002
#define DTWAIN_PDFTEXT_ODDPAGES       0x00000004
#define DTWAIN_PDFTEXT_FIRSTPAGE      0x00000008
#define DTWAIN_PDFTEXT_LASTPAGE       0x00000010
#define DTWAIN_PDFTEXT_CURRENTPAGE    0x00000020
#define DTWAIN_PDFTEXT_DISABLED       0x00000040  // text is ignored

/* Where to print the text (overrides absolute coordinates) */
#define DTWAIN_PDFTEXT_TOPLEFT        0x00000100
#define DTWAIN_PDFTEXT_TOPRIGHT       0x00000200
#define DTWAIN_PDFTEXT_HORIZCENTER    0x00000400
#define DTWAIN_PDFTEXT_VERTCENTER     0x00000800
#define DTWAIN_PDFTEXT_BOTTOMLEFT     0x00001000
#define DTWAIN_PDFTEXT_BOTTOMRIGHT    0x00002000
#define DTWAIN_PDFTEXT_BOTTOMCENTER   0x00004000
#define DTWAIN_PDFTEXT_TOPCENTER      0x00008000
#define DTWAIN_PDFTEXT_XCENTER        0x00010000
#define DTWAIN_PDFTEXT_YCENTER        0x00020000

/* Which parameters to use when calling DTWAIN_AddPDFText */
#define DTWAIN_PDFTEXT_NOSCALING      0x00100000
#define DTWAIN_PDFTEXT_NOCHARSPACING  0x00200000
#define DTWAIN_PDFTEXT_NOWORDSPACING  0x00400000
#define DTWAIN_PDFTEXT_NOSTROKEWIDTH  0x00800000
#define DTWAIN_PDFTEXT_NORENDERMODE   0x01000000
#define DTWAIN_PDFTEXT_NORGBCOLOR     0x02000000
#define DTWAIN_PDFTEXT_NOFONTSIZE     0x04000000
#define DTWAIN_PDFTEXT_NOABSPOSITION  0x08000000
#define DTWAIN_PDFTEXT_IGNOREALL      0xFFF00000

#define DTWAIN_FONT_COURIER              0
#define DTWAIN_FONT_COURIERBOLD          1
#define DTWAIN_FONT_COURIERBOLDOBLIQUE   2
#define DTWAIN_FONT_COURIEROBLIQUE       3
#define DTWAIN_FONT_HELVETICA            4
#define DTWAIN_FONT_HELVETICABOLD        5
#define DTWAIN_FONT_HELVETICABOLDOBLIQUE 6
#define DTWAIN_FONT_HELVETICAOBLIQUE     7
#define DTWAIN_FONT_TIMESBOLD            8
#define DTWAIN_FONT_TIMESBOLDITALIC      9
#define DTWAIN_FONT_TIMESROMAN          10
#define DTWAIN_FONT_TIMESITALIC         11
#define DTWAIN_FONT_SYMBOL              12
#define DTWAIN_FONT_ZAPFDINGBATS        13

/* Render modes */
#define DTWAIN_PDFRENDER_FILL       0
#define DTWAIN_PDFRENDER_STROKE     1
#define DTWAIN_PDFRENDER_FILLSTROKE 2
#define DTWAIN_PDFRENDER_INVISIBLE  3  // the text exists, but is not visible

/* Text element */
#define DTWAIN_PDFTEXTELEMENT_SCALINGXY     0
#define DTWAIN_PDFTEXTELEMENT_FONTHEIGHT    1
#define DTWAIN_PDFTEXTELEMENT_WORDSPACING   2
#define DTWAIN_PDFTEXTELEMENT_POSITION      3
#define DTWAIN_PDFTEXTELEMENT_COLOR         4
#define DTWAIN_PDFTEXTELEMENT_STROKEWIDTH   5
#define DTWAIN_PDFTEXTELEMENT_DISPLAYFLAGS  6
#define DTWAIN_PDFTEXTELEMENT_FONTNAME      7
#define DTWAIN_PDFTEXTELEMENT_TEXT          8
#define DTWAIN_PDFTEXTELEMENT_RENDERMODE    9
#define DTWAIN_PDFTEXTELEMENT_CHARSPACING   10
#define DTWAIN_PDFTEXTELEMENT_ROTATIONANGLE 11
#define DTWAIN_PDFTEXTELEMENT_LEADING       12
#define DTWAIN_PDFTEXTELEMENT_SCALING       13
#define DTWAIN_PDFTEXTELEMENT_TEXTLENGTH    14
#define DTWAIN_PDFTEXTELEMENT_SKEWANGLES    15
#define DTWAIN_PDFTEXTELEMENT_TRANSFORMORDER 16

/* Text transformations */
#define DTWAIN_PDFTEXTTRANSFORM_TSRK       0
#define DTWAIN_PDFTEXTTRANSFORM_TSKR       1
#define DTWAIN_PDFTEXTTRANSFORM_TKSR       2
#define DTWAIN_PDFTEXTTRANSFORM_TKRS       3
#define DTWAIN_PDFTEXTTRANSFORM_TRSK       4
#define DTWAIN_PDFTEXTTRANSFORM_TRKS       5

#define DTWAIN_PDFTEXTTRANSFORM_STRK       6
#define DTWAIN_PDFTEXTTRANSFORM_STKR       7
#define DTWAIN_PDFTEXTTRANSFORM_SKTR       8
#define DTWAIN_PDFTEXTTRANSFORM_SKRT       9
#define DTWAIN_PDFTEXTTRANSFORM_SRTK       10
#define DTWAIN_PDFTEXTTRANSFORM_SRKT       11

#define DTWAIN_PDFTEXTTRANSFORM_RSTK       12
#define DTWAIN_PDFTEXTTRANSFORM_RSKT       13
#define DTWAIN_PDFTEXTTRANSFORM_RTSK       14
#define DTWAIN_PDFTEXTTRANSFORM_RTKT       15
#define DTWAIN_PDFTEXTTRANSFORM_RKST       16
#define DTWAIN_PDFTEXTTRANSFORM_RKTS       17

#define DTWAIN_PDFTEXTTRANSFORM_KSTR        18
#define DTWAIN_PDFTEXTTRANSFORM_KSRT        19
#define DTWAIN_PDFTEXTTRANSFORM_KRST        20
#define DTWAIN_PDFTEXTTRANSFORM_KRTS        21
#define DTWAIN_PDFTEXTTRANSFORM_KTSR        22
#define DTWAIN_PDFTEXTTRANSFORM_KTRS        23

#define DTWAIN_PDFTEXTTRANFORM_LAST         DTWAIN_PDFTEXTTRANSFORM_KTRS

#endif
