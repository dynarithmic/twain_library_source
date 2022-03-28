/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
/* $Header: /cvsroot/osrs/libtiff/tools/tiff2ps.c,v 1.17 2003/05/07 08:12:30 dron Exp $ */

/*
 * Copyright (c) 1988-1997 Sam Leffler
 * Copyright (c) 1991-1997 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>         /* for atof */
#include <time.h>
#include <string.h>
#include "tiff.h"
#include "tiffio.h"
#include "ctliface.h"
#include "ctlobstr.h"
using namespace dynarithmic;
#ifdef _MSC_VER
#pragma warning (disable:4244 4505)
#endif/*
 * Revision history
 *
 * 2001-Mar-21
 *    I (Bruce A. Mallett) added this revision history comment ;)
 *
 *    Fixed PS_Lvl2page() code which outputs non-ASCII85 raw
 *    data.  Moved test for when to output a line break to
 *    *after* the output of a character.  This just serves
 *    to fix an eye-nuisance where the first line of raw
 *    data was one character shorter than subsequent lines.
 *
 *    Added an experimental ASCII85 encoder which can be used
 *    only when there is a single buffer of bytes to be encoded.
 *    This version is much faster at encoding a straight-line
 *    buffer of data because it can avoid alot of the loop
 *    overhead of the byte-by-bye version.  To use this version
 *    you need to define EXP_ASCII85ENCODER (experimental ...).
 *
 *    Added bug fix given by Michael Schmidt to PS_Lvl2page()
 *    in which an end-of-data marker ('>') was not being output
 *    when producing non-ASCII85 encoded PostScript Level 2
 *    data.
 *
 *    Fixed PS_Lvl2colorspace() so that it no longer assumes that
 *    a TIFF having more than 2 planes is a CMYK.  This routine
 *    no longer looks at the samples per pixel but instead looks
 *    at the "photometric" value.  This change allows support of
 *    CMYK TIFFs.
 *
 *    Modified the PostScript L2 imaging loop so as to test if
 *    the input stream is still open before attempting to do a
 *    flushfile on it.  This was done because some RIPs close
 *    the stream after doing the image operation.
 *
 *    Got rid of the realloc() being done inside a loop in the
 *    PSRawDataBW() routine.  The code now walks through the
 *    byte-size array outside the loop to determine the largest
 *    size memory block that will be needed.
 *
 *    Added "-m" switch to ask tiff2ps to, where possible, use the
 *    "imagemask" operator instead of the "image" operator.
 *
 *    Added the "-i #" switch to allow interpolation to be disabled.
 *
 *    Unrolled a loop or two to improve performance.
 */

/*
 * Define EXP_ASCII85ENCODER if you want to use an experimental
 * version of the ASCII85 encoding routine.  The advantage of
 * using this routine is that tiff2ps will convert to ASCII85
 * encoding at between 3 and 4 times the speed as compared to
 * using the old (non-experimental) encoder.  The disadvantage
 * is that you will be using a new (and unproven) encoding
 * routine.  So user beware, you have been warned!
 */

#define EXP_ASCII85ENCODER

/*
 * NB: this code assumes uint32 works with printf's %l[ud].
 */
#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

int ascii85 = FALSE;        /* use ASCII85 encoding */
int interpolate = TRUE;     /* interpolate level2 image */
int level2 = FALSE;         /* generate PostScript level 2 */
int level3 = FALSE;         /* generate PostScript level 3 */
int printAll = FALSE;       /* print all images in file */
int generateEPSF = TRUE;        /* generate Encapsulated PostScript */
int PSduplex = FALSE;       /* enable duplex printing */
int PStumble = FALSE;       /* enable top edge binding */
int PSavoiddeadzone = TRUE;     /* enable avoiding printer deadzone */
float   maxPageHeight = 0;      /* maximum size to fit on page */
float   splitOverlap = 0;       /* amount for split pages to overlag */
int rotate = FALSE;         /* rotate image by 180 degrees */
const char  *filename;          /* input filename */
int useImagemask = FALSE;       /* Use imagemask instead of image operator */
uint16  res_unit = 0;           /* Resolution units: 1 - inches, 2 - cm*/

/*
 * ASCII85 Encoding Support.
 */
unsigned char ascii85buf[10];
int ascii85count;
int ascii85breaklen;

static int  TIFF2PS(FILE*, TIFF*, float, float, double, double, int, int&, const char *,
                    const char *);
static void PSpage(FILE*, TIFF*, uint32, uint32);
static void PSColorContigPreamble(FILE*, uint32, uint32, int);
static void PSColorSeparatePreamble(FILE*, uint32, uint32, int);
static void PSDataColorContig(FILE*, TIFF*, uint32, uint32, int);
static void PSDataColorSeparate(FILE*, TIFF*, uint32, uint32, int);
static void PSDataPalette(FILE*, TIFF*, uint32, uint32);
static void PSDataBW(FILE*, TIFF*, uint32, uint32);
static void PSRawDataBW(FILE*, TIFF*, uint32, uint32);
static void Ascii85Init(void);
static void Ascii85Put(unsigned char code, FILE* fd);
static void Ascii85Flush(FILE* fd);
static void PSHead(FILE*, TIFF*, uint32, uint32, float, float, float, float, const char *outfile,
                   const char *szTitle);
static void PSTail(FILE*, int);

void init_getopt();
int getopt(int nargc, const char** nargv, const char* ostr);

#if defined( EXP_ASCII85ENCODER)
int Ascii85EncodeBlock( uint8 * ascii85_p, unsigned f_eod, const uint8 * raw_p, int raw_l );
#endif

static  void usage(int);

int PostscriptMain(int argc, const char** argv, const char* szTitle)
{
    char outfile[1024];
    int dirnum = -1, c, np = 0;
    int centered = 0;
    float bottommargin = 0;
    float leftmargin = 0;
    float pageWidth = 0;
    float pageHeight = 0;
    uint32 diroff = 0;
    #ifdef _WIN32
    extern const char *optarg;
    #else
    extern char *optarg;
    #endif
    extern int optind;
    FILE* output = stdout;
    init_getopt();
    while ((c = getopt(argc, argv, "b:d:h:H:L:i:w:l:o:O:acelmrxyzps1238DT")) != -1)
        switch (c) {
        case 'b':
            bottommargin = static_cast<float>(atof(optarg));
            break;
        case 'c':
            centered = 1;
            break;
        case 'd':
            dirnum = atoi(optarg);
            break;
        case 'D':
            PSduplex = TRUE;
            break;
        case 'i':
            interpolate = atoi(optarg) ? TRUE:FALSE;
            break;
        case 'T':
            PStumble = TRUE;
            break;
        case 'e':
            PSavoiddeadzone = FALSE;
            generateEPSF = TRUE;
            break;
        case 'h':
            pageHeight = static_cast<float>(atof(optarg));
            break;
        case 'H':
            maxPageHeight = static_cast<float>(atof(optarg));
            if (pageHeight==0)
                pageHeight = maxPageHeight;
            break;
        case 'L':
            splitOverlap = static_cast<float>(atof(optarg));
            break;
        case 'm':
            useImagemask = TRUE;
            break;
        case 'o':
            diroff = static_cast<uint32>(strtoul(optarg, nullptr, 0));
            break;
        case 'O':       /* XXX too bad -o is already taken */
            output = fopen(optarg, "w");
            strcpy(outfile, optarg);
            if (output == nullptr) {
                return -2;
            }
            break;
        case 'l':
            leftmargin = static_cast<float>(atof(optarg));
            break;
        case 'a':
            printAll = TRUE;
            /* fall thru... */
        case 'p':
            generateEPSF = FALSE;
            break;
        case 'r':
            rotate = TRUE;
            break;
        case 's':
            printAll = FALSE;
            break;
        case 'w':
            pageWidth = static_cast<float>(atof(optarg));
            break;
        case 'z':
            PSavoiddeadzone = FALSE;
            break;
        case '1':
            level2 = FALSE;
            level3 = FALSE;
            ascii85 = FALSE;
            break;
        case '2':
            level2 = TRUE;
            ascii85 = TRUE;         /* default to yes */
            break;
        case '3':
            level3 = TRUE;
            ascii85 = TRUE;         /* default to yes */
            break;
        case '8':
            ascii85 = FALSE;
            break;
        case 'x':
            res_unit = RESUNIT_CENTIMETER;
            break;
        case 'y':
            res_unit = RESUNIT_INCH;
            break;
        case '?':
            usage(-1);
        }
    int nPage = 0;
    for (; argc - optind > 0; optind++) {
        TIFF* tif = TIFFOpen(filename = argv[optind], "r");
        if (tif != nullptr) {
            if (dirnum != -1 && !TIFFSetDirectory(tif, static_cast<tdir_t>(dirnum)))
                return -1;
            else if (diroff != 0 &&
                !TIFFSetSubDirectory(tif, diroff))
                return -1;
            np = TIFF2PS(output, tif, pageWidth, pageHeight,
                leftmargin, bottommargin, centered, nPage, static_cast<const char*>(outfile),
                szTitle);
            TIFFClose(tif);
        }
    }
    if (np)
        PSTail(output, np);
/*  else
        usage(-1);*/
    if (output != stdout)
        fclose(output);
    return 0;
}

static  uint16 samplesperpixel;
static  uint16 bitspersample;
static  uint16 planarconfiguration;
static  uint16 photometric;
static  uint16 compression;
static  uint16 extrasamples;
static  int alpha;

static int
checkImage(TIFF* tif)
{
    switch (photometric) {
    case PHOTOMETRIC_YCBCR:
        if ((compression == COMPRESSION_JPEG || compression == COMPRESSION_OJPEG)
            && planarconfiguration == PLANARCONFIG_CONTIG) {
            /* can rely on libjpeg to convert to RGB */
            TIFFSetField(tif, TIFFTAG_JPEGCOLORMODE,
                     JPEGCOLORMODE_RGB);
            photometric = PHOTOMETRIC_RGB;
        } else {
            if (level2 || level3)
                break;
            TIFFError(filename, "Can not handle image with %s",
                "PhotometricInterpretation=YCbCr");
            return 0;
        }
        /* fall thru... */
    case PHOTOMETRIC_RGB:
        if (alpha && bitspersample != 8) {
            TIFFError(filename,
                "Can not handle %d-bit/sample RGB image with alpha",
                bitspersample);
            return 0;
        }
        /* fall thru... */
    case PHOTOMETRIC_SEPARATED:
    case PHOTOMETRIC_PALETTE:
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_MINISWHITE:
        break;
    case PHOTOMETRIC_LOGL:
    case PHOTOMETRIC_LOGLUV:
        if (compression != COMPRESSION_SGILOG &&
            compression != COMPRESSION_SGILOG24) {
            TIFFError(filename,
            "Can not handle %s data with compression other than SGILog",
                photometric == PHOTOMETRIC_LOGL ?
                "LogL" : "LogLuv"
            );
            return 0;
        }
        /* rely on library to convert to RGB/greyscale */
        TIFFSetField(tif, TIFFTAG_SGILOGDATAFMT, SGILOGDATAFMT_8BIT);
        photometric = photometric == PHOTOMETRIC_LOGL ?
            PHOTOMETRIC_MINISBLACK : PHOTOMETRIC_RGB;
        bitspersample = 8;
        break;
    case PHOTOMETRIC_CIELAB:
        /* fall thru... */
    default:
        TIFFError(filename,
            "Can not handle image with PhotometricInterpretation=%d",
            photometric);
        return 0;
    }
    switch (bitspersample) {
    case 1: case 2:
    case 4: case 8:
        break;
    default:
        TIFFError(filename, "Can not handle %d-bit/sample image",
            bitspersample);
        return 0;
    }
    if (planarconfiguration == PLANARCONFIG_SEPARATE && extrasamples > 0)
        TIFFWarning(filename, "Ignoring extra samples");
    return 1;
}

#define PS_UNIT_SIZE    72.0
#define PSUNITS(npix,res)   ((npix) * (PS_UNIT_SIZE / (res)))

static  char RGBcolorimage[] = "\
/bwproc {\n\
    rgbproc\n\
    dup length 3 idiv string 0 3 0\n\
    5 -1 roll {\n\
    add 2 1 roll 1 sub dup 0 eq {\n\
        pop 3 idiv\n\
        3 -1 roll\n\
        dup 4 -1 roll\n\
        dup 3 1 roll\n\
        5 -1 roll put\n\
        1 add 3 0\n\
    } { 2 1 roll } ifelse\n\
    } forall\n\
    pop pop pop\n\
} def\n\
/colorimage where {pop} {\n\
    /colorimage {pop pop /rgbproc exch def {bwproc} image} bind def\n\
} ifelse\n\
";

/*
 * Adobe Photoshop requires a comment line of the form:
 *
 * %ImageData: <cols> <rows> <depth>  <main channels> <pad channels>
 *  <block size> <1 for binary|2 for hex> "data start"
 *
 * It is claimed to be part of some future revision of the EPS spec.
 */
static void
PhotoshopBanner(FILE* fd, uint32 w, uint32 h, int bs, int nc, const char* startline)
{
    fprintf(fd, "%%ImageData: %ld %ld %d %d 0 %d 2 \"",
        static_cast<long>(w), static_cast<long>(h), bitspersample, nc, bs);
    fprintf(fd, startline, nc);
    fprintf(fd, "\"\n");
}

/*
 *   pw : image width in pixels
 *   ph : image height in pixels
 * pprw : image width in PS units (72 dpi)
 * pprh : image height in PS units (72 dpi)
 */
static void
setupPageState(TIFF* tif, uint32* pw, uint32* ph, float* pprw, float* pprh)
{
    float xres, yres;

    TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, pw);
    TIFFGetField(tif, TIFFTAG_IMAGELENGTH, ph);
    if (res_unit == 0)
        TIFFGetFieldDefaulted(tif, TIFFTAG_RESOLUTIONUNIT, &res_unit);
    /*
     * Calculate printable area.
     */
    if (!TIFFGetField(tif, TIFFTAG_XRESOLUTION, &xres) || !xres)
        xres = PS_UNIT_SIZE;
    if (!TIFFGetField(tif, TIFFTAG_YRESOLUTION, &yres) || !yres)
        yres = PS_UNIT_SIZE;
    switch (res_unit) {
    case RESUNIT_CENTIMETER:
        xres *= 2.54F, yres *= 2.54F;
        break;
    case RESUNIT_INCH:
        break;
    case RESUNIT_NONE:
    default:
        xres *= PS_UNIT_SIZE, yres *= PS_UNIT_SIZE;
        break;
    }
    *pprh = static_cast<float>(PSUNITS(*ph, yres));
    *pprw = static_cast<float>(PSUNITS(*pw, xres));
}

static int
isCCITTCompression(TIFF* tif)
{
    uint16 compress;
    TIFFGetField(tif, TIFFTAG_COMPRESSION, &compress);
    return compress == COMPRESSION_CCITTFAX3 ||
        compress == COMPRESSION_CCITTFAX4 ||
        compress == COMPRESSION_CCITTRLE ||
        compress == COMPRESSION_CCITTRLEW;
}

static  tsize_t tf_bytesperrow;
static  tsize_t ps_bytesperrow;
static  tsize_t tf_rowsperstrip;
static  tsize_t tf_numberstrips;
static  const char *hex = "0123456789abcdef";

/*
 * imagewidth & imageheight are 1/72 inches
 * pagewidth & pageheight are inches
 */
int
PlaceImage(FILE *fp, float pagewidth, float pageheight,
       float imagewidth, float imageheight, int splitpage,
       double lm, double bm, int cnt)
{
    const float xtran = 0;
    float ytran = 0;
    float xscale = 1;
    float yscale = 1;
    float left_offset = static_cast<float>(lm * PS_UNIT_SIZE);
    float bottom_offset = static_cast<float>(bm * PS_UNIT_SIZE);
    float splitheight;

    pagewidth *= PS_UNIT_SIZE;
    pageheight *= PS_UNIT_SIZE;

    if (maxPageHeight==0)
        splitheight = 0;
    else
        splitheight = static_cast<float>(maxPageHeight * PS_UNIT_SIZE);
    float overlap = static_cast<float>(splitOverlap * PS_UNIT_SIZE);

    /*
     * WIDTH:
     *      if too wide, scrunch to fit
     *      else leave it alone
     */
    if (imagewidth <= pagewidth) {
        xscale = imagewidth;
    } else {
        xscale = pagewidth;
    }

    /* HEIGHT:
     *      if too long, scrunch to fit
     *      if too short, move to top of page
     */
    if (imageheight <= pageheight) {
        yscale = imageheight;
        ytran = pageheight - imageheight;
    } else if (imageheight > pageheight &&
        (splitheight == 0 || imageheight <= splitheight)) {
        yscale = pageheight;
    } else /* imageheight > splitheight */ {
        float subimageheight = imageheight - (pageheight - overlap) * splitpage;
        if (subimageheight <= pageheight) {
            yscale = imageheight;
            ytran = pageheight - subimageheight;
            splitpage = 0;
        } else if ( subimageheight > pageheight && subimageheight <= splitheight) {
            yscale = imageheight * pageheight / subimageheight;
            ytran = 0;
            splitpage = 0;
        } else /* sumimageheight > splitheight */ {
            yscale = imageheight;
            ytran = pageheight - subimageheight;
            splitpage++;
        }
    }

    bottom_offset += ytran / (cnt?2:1);
    if (cnt)
        left_offset += xtran / 2;
    fprintf(fp, "%f %f translate\n", left_offset, bottom_offset);
    fprintf(fp, "%f %f scale\n", xscale, yscale);
    if (rotate)
        fputs ("1 1 translate 180 rotate\n", fp);

    return splitpage;
}


/* returns the sequence number of the page processed */
int
TIFF2PS(FILE* fd, TIFF* tif, float pw, float ph, double lm, double bm, int cnt, int& npages,
        const char* outfile, const char *szTitle)
{
    uint32 w, h;
    float ox, oy, prw, prh;
    float left_offset = static_cast<float>(lm * PS_UNIT_SIZE);
    float bottom_offset = static_cast<float>(bm * PS_UNIT_SIZE);
    uint32 subfiletype;
    uint16* sampleinfo;

    if (!TIFFGetField(tif, TIFFTAG_XPOSITION, &ox))
        ox = 0;
    if (!TIFFGetField(tif, TIFFTAG_YPOSITION, &oy))
        oy = 0;
    setupPageState(tif, &w, &h, &prw, &prh);

    do {
        tf_numberstrips = TIFFNumberOfStrips(tif);
        TIFFGetFieldDefaulted(tif, TIFFTAG_ROWSPERSTRIP,
            &tf_rowsperstrip);
        setupPageState(tif, &w, &h, &prw, &prh);
        if (!npages)
            PSHead(fd, tif, w, h, prw, prh, ox, oy, outfile, szTitle);
        TIFFGetFieldDefaulted(tif, TIFFTAG_BITSPERSAMPLE,
            &bitspersample);
        TIFFGetFieldDefaulted(tif, TIFFTAG_SAMPLESPERPIXEL,
            &samplesperpixel);
        TIFFGetFieldDefaulted(tif, TIFFTAG_PLANARCONFIG,
            &planarconfiguration);
        TIFFGetField(tif, TIFFTAG_COMPRESSION, &compression);
        TIFFGetFieldDefaulted(tif, TIFFTAG_EXTRASAMPLES,
            &extrasamples, &sampleinfo);
        alpha = extrasamples == 1 &&
            sampleinfo[0] == EXTRASAMPLE_ASSOCALPHA;
        if (!TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric)) {
            switch (samplesperpixel - extrasamples) {
            case 1:
                if (isCCITTCompression(tif))
                    photometric = PHOTOMETRIC_MINISWHITE;
                else
                    photometric = PHOTOMETRIC_MINISBLACK;
                break;
            case 3:
                photometric = PHOTOMETRIC_RGB;
                break;
            case 4:
                photometric = PHOTOMETRIC_SEPARATED;
                break;
            }
        }
        if (checkImage(tif)) {
            tf_bytesperrow = TIFFScanlineSize(tif);
            npages++;
            fprintf(fd, "%%%%Page: %d %d\n", npages, npages);
            fprintf(fd, "gsave\n");
            fprintf(fd, "100 dict begin\n");
            if (pw != 0 && ph != 0) {
                if (maxPageHeight)
                {
                    /* used -H option */
                    int split = PlaceImage(fd, pw, ph, prw, prh, 0, lm, bm, cnt);
                    while( split ) {
                        PSpage(fd, tif, w, h);
                        fprintf(fd, "end\n");
                        fprintf(fd, "grestore\n");
                        fprintf(fd, "showpage\n");
                        npages++;
                        fprintf(fd, "%%%%Page: %d %d\n", npages, npages);
                        fprintf(fd, "gsave\n");
                        fprintf(fd, "100 dict begin\n");
                        split = PlaceImage(fd,pw,ph,prw,prh,split,lm,bm,cnt);
                    }
                } else {
                    /* NB: maintain image aspect ratio */
                    float scale = pw * PS_UNIT_SIZE / prw < ph * PS_UNIT_SIZE / prh
                                      ? static_cast<float>((pw * PS_UNIT_SIZE / prw))
                                      : static_cast<float>((ph * PS_UNIT_SIZE / prh));
                    if (scale > 1.0)
                        scale = 1.0;
                    bottom_offset +=
                        static_cast<float>((ph * PS_UNIT_SIZE - prh * scale)) / (cnt?2:1);
                    if (cnt)
                        left_offset += static_cast<float>((pw * PS_UNIT_SIZE - prw * scale)) / 2;
                    fprintf(fd, "%f %f translate\n",
                        left_offset, bottom_offset);
                    fprintf(fd, "%f %f scale\n", prw * scale, prh * scale);
                    if (rotate)
                        fputs ("1 1 translate 180 rotate\n", fd);
                }
            } else {
                fprintf(fd, "%f %f scale\n", prw, prh);
                if (rotate)
                    fputs ("1 1 translate 180 rotate\n", fd);
            }
            PSpage(fd, tif, w, h);
            fprintf(fd, "end\n");
            fprintf(fd, "grestore\n");
            fprintf(fd, "showpage\n");
        }
        if (generateEPSF)
            break;
        TIFFGetFieldDefaulted(tif, TIFFTAG_SUBFILETYPE, &subfiletype);
    } while ((subfiletype & FILETYPE_PAGE || printAll) &&
        TIFFReadDirectory(tif));

    return npages;
}


static char DuplexPreamble[] = "\
%%BeginFeature: *Duplex True\n\
systemdict begin\n\
  /languagelevel where { pop languagelevel } { 1 } ifelse\n\
  2 ge { 1 dict dup /Duplex true put setpagedevice }\n\
  { statusdict /setduplex known { statusdict begin setduplex true end } if\n\
  } ifelse\n\
end\n\
%%EndFeature\n\
";

static char TumblePreamble[] = "\
%%BeginFeature: *Tumble True\n\
systemdict begin\n\
  /languagelevel where { pop languagelevel } { 1 } ifelse\n\
  2 ge { 1 dict dup /Tumble true put setpagedevice }\n\
  { statusdict /settumble known { statusdict begin true settumble end } if\n\
  } ifelse\n\
end\n\
%%EndFeature\n\
";

static char AvoidDeadZonePreamble[] = "\
gsave newpath clippath pathbbox grestore\n\
  4 2 roll 2 copy translate\n\
  exch 3 1 roll sub 3 1 roll sub exch\n\
  currentpagedevice /PageSize get aload pop\n\
  exch 3 1 roll div 3 1 roll div abs exch abs\n\
  2 copy gt { exch } if pop\n\
  dup 1 lt { dup scale } { pop } ifelse\n\
";

void
PSHead(FILE *fd, TIFF *tif, uint32 w, uint32 h, float pw, float ph,
    float ox, float oy, const char * /*outfile*/, const char *szTitle)
{
    time_t t;

    (void) tif; (void) w; (void) h;
    t = time(nullptr);
    fprintf(fd, "%%!PS-Adobe-3.0%s\n", generateEPSF ? " EPSF-3.0" : "");
    CTL_StringType Buffer = GetVersionString();
    fprintf(fd, "%%%%Creator: %s\n", StringConversion::Convert_Native_To_Ansi(Buffer).c_str());
    fprintf(fd, "%%%%Title: %s\n", szTitle);
    fprintf(fd, "%%%%CreationDate: %s", ctime(&t));
    fprintf(fd, "%%%%DocumentData: Clean7Bit\n");
    fprintf(fd, "%%%%Origin: %ld %ld\n", static_cast<long>(ox), static_cast<long>(oy));
    /* NB: should use PageBoundingBox */
    fprintf(fd, "%%%%BoundingBox: 0 0 %ld %ld\n",
        static_cast<long>(ceil(pw)), static_cast<long>(ceil(ph)));
    fprintf(fd, "%%%%LanguageLevel: %d\n", level3 ? 3 : level2 ? 2 : 1);
    fprintf(fd, "%%%%Pages: (atend)\n");
    fprintf(fd, "%%%%EndComments\n");
    fprintf(fd, "%%%%BeginSetup\n");
    if (PSduplex)
        fprintf(fd, "%s", DuplexPreamble);
    if (PStumble)
        fprintf(fd, "%s", TumblePreamble);
    if (PSavoiddeadzone && (level2 || level3))
        fprintf(fd, "%s", AvoidDeadZonePreamble);
    fprintf(fd, "%%%%EndSetup\n");
}

void
PSTail(FILE *fd, int npages)
{
    fprintf(fd, "%%%%Trailer\n");
    fprintf(fd, "%%%%Pages: %d\n", npages);
    fprintf(fd, "%%%%EOF\n");
}

static int
checkcmap(TIFF* tif, int n, uint16* r, uint16* g, uint16* b)
{
    (void) tif;
    while (n-- > 0)
        if (*r++ >= 256 || *g++ >= 256 || *b++ >= 256)
            return 16;
    TIFFWarning(filename, "Assuming 8-bit colormap");
    return 8;
}

static void
PS_Lvl2colorspace(FILE* fd, TIFF* tif)
{
    uint16 *rmap, *gmap, *bmap;
    int i;
    const char * colorspace_p;

    switch ( photometric )
    {
    case PHOTOMETRIC_SEPARATED:
        colorspace_p = "CMYK";
        break;

    case PHOTOMETRIC_RGB:
        colorspace_p = "RGB";
        break;

    default:
        colorspace_p = "Gray";
    }

    /*
     * Set up PostScript Level 2 colorspace according to
     * section 4.8 in the PostScript refenence manual.
     */
    fputs("% PostScript Level 2 only.\n", fd);
    if (photometric != PHOTOMETRIC_PALETTE) {
        if (photometric == PHOTOMETRIC_YCBCR) {
            /* MORE CODE HERE */
        }
        fprintf(fd, "/Device%s setcolorspace\n", colorspace_p );
        return;
    }

    /*
     * Set up an indexed/palette colorspace
     */
    int num_colors = 1 << bitspersample;
    if (!TIFFGetField(tif, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap)) {
        TIFFError(filename,
            "Palette image w/o \"Colormap\" tag");
        return;
    }
    if (checkcmap(tif, num_colors, rmap, gmap, bmap) == 16) {
        /*
         * Convert colormap to 8-bits values.
         */
#define CVT(x)      (((x) * 255) / ((1L<<16)-1))
        for (i = 0; i < num_colors; i++) {
            rmap[i] = CVT(rmap[i]);
            gmap[i] = CVT(gmap[i]);
            bmap[i] = CVT(bmap[i]);
        }
#undef CVT
    }
    fprintf(fd, "[ /Indexed /DeviceRGB %d", num_colors - 1);
    if (ascii85) {
        Ascii85Init();
        fputs("\n<~", fd);
        ascii85breaklen -= 2;
    } else
        fputs(" <", fd);
    for (i = 0; i < num_colors; i++) {
        if (ascii85) {
            Ascii85Put(static_cast<unsigned char>(rmap[i]), fd);
            Ascii85Put(static_cast<unsigned char>(gmap[i]), fd);
            Ascii85Put(static_cast<unsigned char>(bmap[i]), fd);
        } else {
            fputs(i % 8 ? " " : "\n  ", fd);
            fprintf(fd, "%02x%02x%02x",
                rmap[i], gmap[i], bmap[i]);
        }
    }
    if (ascii85)
        Ascii85Flush(fd);
    else
        fputs(">\n", fd);
    fputs("] setcolorspace\n", fd);
}

static int
PS_Lvl2ImageDict(FILE* fd, TIFF* tif, uint32 w, uint32 h)
{
    uint32 tile_width, tile_height;
    uint16 predictor, minsamplevalue, maxsamplevalue;
    int repeat_count;
    char im_h[64], im_x[64], im_y[64];
    const char * imageOp = "image";

    if ( useImagemask && bitspersample == 1 )
        imageOp = "imagemask";

    (void)strcpy(im_x, "0");
    (void)sprintf(im_y, "%lu", static_cast<long>(h));
    (void)sprintf(im_h, "%lu", static_cast<long>(h));
    tile_width = w;
    tile_height = h;
    if (TIFFIsTiled(tif)) {
        repeat_count = TIFFNumberOfTiles(tif);
        TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tile_width);
        TIFFGetField(tif, TIFFTAG_TILELENGTH, &tile_height);
        if (tile_width > w || tile_height > h ||
            w % tile_width != 0 || h % tile_height != 0) {
            /*
             * The tiles does not fit image width and height.
             * Set up a clip rectangle for the image unit square.
             */
            fputs("0 0 1 1 rectclip\n", fd);
        }
        if (tile_width < w) {
            fputs("/im_x 0 def\n", fd);
            (void)strcpy(im_x, "im_x neg");
        }
        if (tile_height < h) {
            fputs("/im_y 0 def\n", fd);
            (void)sprintf(im_y, "%lu im_y sub", static_cast<unsigned long>(h));
        }
    } else {
        repeat_count = tf_numberstrips;
        tile_height = tf_rowsperstrip;
        if (tile_height > h)
            tile_height = h;
        if (repeat_count > 1) {
            fputs("/im_y 0 def\n", fd);
            fprintf(fd, "/im_h %lu def\n",
                static_cast<unsigned long>(tile_height));
            (void)strcpy(im_h, "im_h");
            (void)sprintf(im_y, "%lu im_y sub", static_cast<unsigned long>(h));
        }
    }

    /*
     * Output start of exec block
     */
    fputs("{ % exec\n", fd);

    if (repeat_count > 1)
        fprintf(fd, "%d { %% repeat\n", repeat_count);

    /*
     * Output filter options and image dictionary.
     */
    if (ascii85)
        fputs(" /im_stream currentfile /ASCII85Decode filter def\n",
            fd);
    fputs(" <<\n", fd);
    fputs("  /ImageType 1\n", fd);
    fprintf(fd, "  /Width %lu\n", static_cast<unsigned long>(tile_width));
    /*
     * Workaround for some software that may crash when last strip
     * of image contains fewer number of scanlines than specified
     * by the `/Height' variable. So for stripped images with multiple
     * strips we will set `/Height' as `im_h', because one is
     * recalculated for each strip - including the (smaller) final strip.
     * For tiled images and images with only one strip `/Height' will
     * contain number of scanlines in tile (or image height in case of
     * one-stripped image).
     */
    if (TIFFIsTiled(tif) || tf_numberstrips == 1)
        fprintf(fd, "  /Height %lu\n", static_cast<unsigned long>(tile_height));
    else
        fprintf(fd, "  /Height im_h\n");

    if (planarconfiguration == PLANARCONFIG_SEPARATE && samplesperpixel > 1)
        fputs("  /MultipleDataSources true\n", fd);
    fprintf(fd, "  /ImageMatrix [ %lu 0 0 %ld %s %s ]\n",
        static_cast<unsigned long>(w), - static_cast<long>(h), im_x, im_y);
    fprintf(fd, "  /BitsPerComponent %d\n", bitspersample);
    fprintf(fd, "  /Interpolate %s\n", interpolate ? "true" : "false");

    switch (samplesperpixel) {
    case 1:
        switch (photometric) {
        case PHOTOMETRIC_MINISBLACK:
            fputs("  /Decode [0 1]\n", fd);
            break;
        case PHOTOMETRIC_MINISWHITE:
            switch (compression) {
            case COMPRESSION_CCITTRLE:
            case COMPRESSION_CCITTRLEW:
            case COMPRESSION_CCITTFAX3:
            case COMPRESSION_CCITTFAX4:
                fputs("  /Decode [1 0]\n", fd);
                break;
            default:
                fputs("  /Decode [0 1]\n", fd);
                break;
            }
            break;
        case PHOTOMETRIC_PALETTE:
            TIFFGetFieldDefaulted(tif, TIFFTAG_MINSAMPLEVALUE,
                &minsamplevalue);
            TIFFGetFieldDefaulted(tif, TIFFTAG_MAXSAMPLEVALUE,
                &maxsamplevalue);
            fprintf(fd, "  /Decode [%u %u]\n",
                    minsamplevalue, maxsamplevalue);
            break;
        default:
            /*
             * ERROR ?
             */
            fputs("  /Decode [0 1]\n", fd);
            break;
        }
        break;
    case 3:
        switch (photometric) {
        case PHOTOMETRIC_RGB:
            fputs("  /Decode [0 1 0 1 0 1]\n", fd);
            break;
        case PHOTOMETRIC_MINISWHITE:
        case PHOTOMETRIC_MINISBLACK:
        default:
            /*
             * ERROR??
             */
            fputs("  /Decode [0 1 0 1 0 1]\n", fd);
            break;
        }
        break;
    case 4:
        /*
         * ERROR??
         */
        fputs("  /Decode [0 1 0 1 0 1 0 1]\n", fd);
        break;
    }
    fputs("  /DataSource", fd);
    if (planarconfiguration == PLANARCONFIG_SEPARATE &&
        samplesperpixel > 1)
        fputs(" [", fd);
    if (ascii85)
        fputs(" im_stream", fd);
    else
        fputs(" currentfile /ASCIIHexDecode filter", fd);

    int use_rawdata = TRUE;
    switch (compression) {
    case COMPRESSION_NONE:      /* 1: uncompressed */
        break;
    case COMPRESSION_CCITTRLE:  /* 2: CCITT modified Huffman RLE */
    case COMPRESSION_CCITTRLEW: /* 32771: #1 w/ word alignment */
    case COMPRESSION_CCITTFAX3: /* 3: CCITT Group 3 fax encoding */
    case COMPRESSION_CCITTFAX4: /* 4: CCITT Group 4 fax encoding */
        fputs("\n\t<<\n", fd);
        if (compression == COMPRESSION_CCITTFAX3) {
            uint32 g3_options;

            fputs("\t /EndOfLine true\n", fd);
            fputs("\t /EndOfBlock false\n", fd);
            if (!TIFFGetField(tif, TIFFTAG_GROUP3OPTIONS,
                        &g3_options))
                g3_options = 0;
            if (g3_options & GROUP3OPT_2DENCODING)
                fprintf(fd, "\t /K %s\n", im_h);
            if (g3_options & GROUP3OPT_UNCOMPRESSED)
                fputs("\t /Uncompressed true\n", fd);
            if (g3_options & GROUP3OPT_FILLBITS)
                fputs("\t /EncodedByteAlign true\n", fd);
        }
        if (compression == COMPRESSION_CCITTFAX4) {
            uint32 g4_options;

            fputs("\t /K -1\n", fd);
            TIFFGetFieldDefaulted(tif, TIFFTAG_GROUP4OPTIONS,
                           &g4_options);
            if (g4_options & GROUP4OPT_UNCOMPRESSED)
                fputs("\t /Uncompressed true\n", fd);
        }
        if (!(tile_width == w && w == 1728U))
            fprintf(fd, "\t /Columns %lu\n",
                static_cast<unsigned long>(tile_width));
        fprintf(fd, "\t /Rows %s\n", im_h);
        if (compression == COMPRESSION_CCITTRLE ||
            compression == COMPRESSION_CCITTRLEW) {
            fputs("\t /EncodedByteAlign true\n", fd);
            fputs("\t /EndOfBlock false\n", fd);
        }
        if (photometric == PHOTOMETRIC_MINISBLACK)
            fputs("\t /BlackIs1 true\n", fd);
        fprintf(fd, "\t>> /CCITTFaxDecode filter");
        break;
    case COMPRESSION_LZW:   /* 5: Lempel-Ziv & Welch */
        TIFFGetFieldDefaulted(tif, TIFFTAG_PREDICTOR, &predictor);
        if (predictor == 2) {
            fputs("\n\t<<\n", fd);
            fprintf(fd, "\t /Predictor %u\n", predictor);
            fprintf(fd, "\t /Columns %lu\n",
                static_cast<unsigned long>(tile_width));
            fprintf(fd, "\t /Colors %u\n", samplesperpixel);
            fprintf(fd, "\t /BitsPerComponent %u\n",
                bitspersample);
            fputs("\t>>", fd);
        }
        fputs(" /LZWDecode filter", fd);
        break;
    case COMPRESSION_DEFLATE:   /* 5: ZIP */
    case COMPRESSION_ADOBE_DEFLATE:
        if ( level3 ) {
             TIFFGetFieldDefaulted(tif, TIFFTAG_PREDICTOR, &predictor);
             if (predictor > 1) {
                fprintf(fd, "\t  PostScript Level 3 only.");
                fputs("\n\t<<\n", fd);
                fprintf(fd, "\t /Predictor %u\n", predictor);
                fprintf(fd, "\t /Columns %lu\n",
                    static_cast<unsigned long>(tile_width));
                fprintf(fd, "\t /Colors %u\n", samplesperpixel);
                    fprintf(fd, "\t /BitsPerComponent %u\n",
                    bitspersample);
                fputs("\t>>", fd);
             }
             fputs(" /FlateDecode filter", fd);
        } else {
            use_rawdata = FALSE ;
        }
        break;
    case COMPRESSION_PACKBITS:  /* 32773: Macintosh RLE */
        fputs(" /RunLengthDecode filter", fd);
        use_rawdata = TRUE;
        break;
    case COMPRESSION_OJPEG:     /* 6: !6.0 JPEG */
    case COMPRESSION_JPEG:      /* 7: %JPEG DCT compression */
#ifdef notdef
        /*
         * Code not tested yet
         */
        fputs(" /DCTDecode filter", fd);
        use_rawdata = TRUE;
#else
        use_rawdata = FALSE;
#endif
        break;
    case COMPRESSION_NEXT:      /* 32766: NeXT 2-bit RLE */
    case COMPRESSION_THUNDERSCAN:   /* 32809: ThunderScan RLE */
    case COMPRESSION_PIXARFILM: /* 32908: Pixar companded 10bit LZW */
    case COMPRESSION_JBIG:      /* 34661: ISO JBIG */
    case COMPRESSION_SGILOG:    /* 34676: SGI LogL or LogLuv */
    case COMPRESSION_SGILOG24:  /* 34677: SGI 24-bit LogLuv */
        use_rawdata = FALSE;
        break;
    default:
        /*
         * ERROR...
         */
        use_rawdata = FALSE;
        break;
    }
    if (planarconfiguration == PLANARCONFIG_SEPARATE &&
        samplesperpixel > 1) {
        /*
         * NOTE: This code does not work yet...
         */
        for (uint16 i = 1; i < samplesperpixel; i++)
            fputs(" dup", fd);
        fputs(" ]", fd);
    }

    fprintf( fd, "\n >> %s\n", imageOp );
    if (ascii85)
        fputs(" im_stream status { im_stream flushfile } if\n", fd);
    if (repeat_count > 1) {
        if (tile_width < w) {
            fprintf(fd, " /im_x im_x %lu add def\n",
                static_cast<unsigned long>(tile_width));
            if (tile_height < h) {
                fprintf(fd, " im_x %lu ge {\n",
                    static_cast<unsigned long>(w));
                fputs("  /im_x 0 def\n", fd);
                fprintf(fd, " /im_y im_y %lu add def\n",
                    static_cast<unsigned long>(tile_height));
                fputs(" } if\n", fd);
            }
        }
        if (tile_height < h) {
            if (tile_width >= w) {
                fprintf(fd, " /im_y im_y %lu add def\n",
                    static_cast<unsigned long>(tile_height));
                if (!TIFFIsTiled(tif)) {
                    fprintf(fd, " /im_h %lu im_y sub",
                        static_cast<unsigned long>(h));
                    fprintf(fd, " dup %lu gt { pop",
                        static_cast<unsigned long>(tile_height));
                    fprintf(fd, " %lu } if def\n",
                        static_cast<unsigned long>(tile_height));
                }
            }
        }
        fputs("} repeat\n", fd);
    }
    /*
     * End of exec function
     */
    fputs("}\n", fd);

    return use_rawdata;
}

int
PS_Lvl2page(FILE* fd, TIFF* tif, uint32 w, uint32 h)
{
    uint16 fillorder;
    int use_rawdata, tiled_image, breaklen = 1;
    uint32 chunk_no, num_chunks, *bc;
    unsigned char *buf_data;
    tsize_t chunk_size, byte_count;

#if defined( EXP_ASCII85ENCODER )
    int         ascii85_l;  /* Length, in bytes, of ascii85_p[] data */
    uint8       *   ascii85_p = nullptr;  /* Holds ASCII85 encoded data */
#endif

    PS_Lvl2colorspace(fd, tif);
    use_rawdata = PS_Lvl2ImageDict(fd, tif, w, h);

/* See http://bugzilla.remotesensing.org/show_bug.cgi?id=80 */
#ifdef ENABLE_BROKEN_BEGINENDDATA
    fputs("%%BeginData:\n", fd);
#endif
    fputs("exec\n", fd);

    tiled_image = TIFFIsTiled(tif);
    if (tiled_image) {
        num_chunks = TIFFNumberOfTiles(tif);
        TIFFGetField(tif, TIFFTAG_TILEBYTECOUNTS, &bc);
    } else {
        num_chunks = TIFFNumberOfStrips(tif);
        TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &bc);
    }

    if (use_rawdata) {
        chunk_size = bc[0];
        for (chunk_no = 1; chunk_no < num_chunks; chunk_no++)
            if (bc[chunk_no] > static_cast<uint32>(chunk_size))
                chunk_size = bc[chunk_no];
    } else {
        if (tiled_image)
            chunk_size = TIFFTileSize(tif);
        else
            chunk_size = TIFFStripSize(tif);
    }
    buf_data = static_cast<unsigned char*>(_TIFFmalloc(chunk_size));
    if (!buf_data) {
        TIFFError(filename, "Can't alloc %u bytes for %s.",
            chunk_size, tiled_image ? "tiles" : "strips");
        return FALSE;
    }

#if defined( EXP_ASCII85ENCODER )
    if ( ascii85 ) {
        /*
         * Allocate a buffer to hold the ASCII85 encoded data.  Note
         * that it is allocated with sufficient room to hold the
         * encoded data (5*chunk_size/4) plus the EOD marker (+8)
         * and formatting line breaks.  The line breaks are more
         * than taken care of by using 6*chunk_size/4 rather than
         * 5*chunk_size/4.
         */

        ascii85_p = static_cast<unsigned char*>(_TIFFmalloc(chunk_size + chunk_size / 2 + 8));

        if ( !ascii85_p ) {
        _TIFFfree( buf_data );

        TIFFError( filename, "Cannot allocate ASCII85 encoding buffer." );
        return FALSE;
        }
    }
#endif

    TIFFGetFieldDefaulted(tif, TIFFTAG_FILLORDER, &fillorder);
    for (chunk_no = 0; chunk_no < num_chunks; chunk_no++) {
        if (ascii85)
            Ascii85Init();
        else
            breaklen = 36;
        if (use_rawdata) {
            if (tiled_image)
                byte_count = TIFFReadRawTile(tif, chunk_no,
                          buf_data, chunk_size);
            else
                byte_count = TIFFReadRawStrip(tif, chunk_no,
                          buf_data, chunk_size);
            if (fillorder == FILLORDER_LSB2MSB)
                TIFFReverseBits(buf_data, byte_count);
        } else {
            if (tiled_image)
                byte_count = TIFFReadEncodedTile(tif,
                        chunk_no, buf_data,
                        chunk_size);
            else
                byte_count = TIFFReadEncodedStrip(tif,
                        chunk_no, buf_data,
                        chunk_size);
        }
        if (byte_count < 0) {
            TIFFError(filename, "Can't read %s %d.",
                tiled_image ? "tile" : "strip", chunk_no);
            if (ascii85)
                Ascii85Put('\0', fd);
        }

        if (ascii85) {
#if defined( EXP_ASCII85ENCODER )
            ascii85_l = Ascii85EncodeBlock(ascii85_p, 1, buf_data, byte_count );

            if ( ascii85_l > 0 )
                fwrite( ascii85_p, ascii85_l, 1, fd );
#else
            for (cp = buf_data; byte_count > 0; byte_count--)
                Ascii85Put(*cp++, fd);
#endif
        }
        else
        {
            for (unsigned char* cp = buf_data; byte_count > 0; byte_count--) {
                putc(hex[*cp>>4&0xf], fd);
                putc(hex[*cp&0xf], fd);
                cp++;

                if (--breaklen <= 0) {
                    putc('\n', fd);
                    breaklen = 36;
                }
            }
        }

        if ( !ascii85 ) {
            if ( level2 || level3 )
                putc( '>', fd );
            putc('\n', fd);
        }
#if !defined( EXP_ASCII85ENCODER )
        else
            Ascii85Flush(fd);
#endif
    }

#if defined( EXP_ASCII85ENCODER )
    if ( ascii85_p )
        _TIFFfree( ascii85_p );
#endif

    _TIFFfree(buf_data);
#ifdef ENABLE_BROKEN_BEGINENDDATA
    fputs("%%EndData\n", fd);
#endif
    return TRUE;
}

void
PSpage(FILE* fd, TIFF* tif, uint32 w, uint32 h)
{
    const char  *   imageOp = "image";

    if ( useImagemask && bitspersample == 1 )
        imageOp = "imagemask";

    if ((level2 || level3) && PS_Lvl2page(fd, tif, w, h))
        return;
    ps_bytesperrow = tf_bytesperrow;
    switch (photometric) {
    case PHOTOMETRIC_RGB:
        if (planarconfiguration == PLANARCONFIG_CONTIG) {
            fprintf(fd, "%s", RGBcolorimage);
            PSColorContigPreamble(fd, w, h, 3);
            PSDataColorContig(fd, tif, w, h, 3);
        } else {
            PSColorSeparatePreamble(fd, w, h, 3);
            PSDataColorSeparate(fd, tif, w, h, 3);
        }
        break;
    case PHOTOMETRIC_SEPARATED:
        /* XXX should emit CMYKcolorimage */
        if (planarconfiguration == PLANARCONFIG_CONTIG) {
            PSColorContigPreamble(fd, w, h, 4);
            PSDataColorContig(fd, tif, w, h, 4);
        } else {
            PSColorSeparatePreamble(fd, w, h, 4);
            PSDataColorSeparate(fd, tif, w, h, 4);
        }
        break;
    case PHOTOMETRIC_PALETTE:
        fprintf(fd, "%s", RGBcolorimage);
        PhotoshopBanner(fd, w, h, 1, 3, "false 3 colorimage");
        fprintf(fd, "/scanLine %ld string def\n",
            static_cast<long>(ps_bytesperrow) * 3L);
        fprintf(fd, "%lu %lu 8\n",
            static_cast<unsigned long>(w), static_cast<unsigned long>(h));
        fprintf(fd, "[%lu 0 0 -%lu 0 %lu]\n",
            static_cast<unsigned long>(w), static_cast<unsigned long>(h), static_cast<unsigned long>(h));
        fprintf(fd, "{currentfile scanLine readhexstring pop} bind\n");
        fprintf(fd, "false 3 colorimage\n");
        PSDataPalette(fd, tif, w, h);
        break;
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_MINISWHITE:
        PhotoshopBanner(fd, w, h, 1, 1, imageOp);
        fprintf(fd, "/scanLine %ld string def\n",
            static_cast<long>(ps_bytesperrow));
        fprintf(fd, "%lu %lu %d\n",
            static_cast<unsigned long>(w), static_cast<unsigned long>(h), bitspersample);
        fprintf(fd, "[%lu 0 0 -%lu 0 %lu]\n",
            static_cast<unsigned long>(w), static_cast<unsigned long>(h), static_cast<unsigned long>(h));
        fprintf(fd,
            "{currentfile scanLine readhexstring pop} bind\n");
        fprintf(fd, "%s\n", imageOp);
        PSDataBW(fd, tif, w, h);
        break;
    }
    putc('\n', fd);
}

void
PSColorContigPreamble(FILE* fd, uint32 w, uint32 h, int nc)
{
    ps_bytesperrow = nc * (tf_bytesperrow / samplesperpixel);
    PhotoshopBanner(fd, w, h, 1, nc, "false %d colorimage");
    fprintf(fd, "/line %ld string def\n", static_cast<long>(ps_bytesperrow));
    fprintf(fd, "%lu %lu %d\n",
        static_cast<unsigned long>(w), static_cast<unsigned long>(h), bitspersample);
    fprintf(fd, "[%lu 0 0 -%lu 0 %lu]\n",
        static_cast<unsigned long>(w), static_cast<unsigned long>(h), static_cast<unsigned long>(h));
    fprintf(fd, "{currentfile line readhexstring pop} bind\n");
    fprintf(fd, "false %d colorimage\n", nc);
}

void
PSColorSeparatePreamble(FILE* fd, uint32 w, uint32 h, int nc)
{
    int i;

    PhotoshopBanner(fd, w, h, ps_bytesperrow, nc, "true %d colorimage");
    for (i = 0; i < nc; i++)
        fprintf(fd, "/line%d %ld string def\n",
            i, static_cast<long>(ps_bytesperrow));
    fprintf(fd, "%lu %lu %d\n",
        static_cast<unsigned long>(w), static_cast<unsigned long>(h), bitspersample);
    fprintf(fd, "[%lu 0 0 -%lu 0 %lu] \n",
        static_cast<unsigned long>(w), static_cast<unsigned long>(h), static_cast<unsigned long>(h));
    for (i = 0; i < nc; i++)
        fprintf(fd, "{currentfile line%d readhexstring pop}bind\n", i);
    fprintf(fd, "true %d colorimage\n", nc);
}

#define MAXLINE     36
#define DOBREAK(len, howmany, fd) \
    if (((len) -= (howmany)) <= 0) {    \
        putc('\n', fd);         \
        (len) = MAXLINE-(howmany);  \
    }
#define PUTHEX(c,fd)    putc(hex[((c)>>4)&0xf],fd); putc(hex[(c)&0xf],fd)

void
PSDataColorContig(FILE* fd, TIFF* tif, uint32 w, uint32 h, int nc)
{
    int breaklen = MAXLINE, cc, es = samplesperpixel - nc;
    unsigned char c;

    (void) w;
    unsigned char* tf_buf = static_cast<unsigned char*>(_TIFFmalloc(tf_bytesperrow));
    if (tf_buf == nullptr) {
        TIFFError(filename, "No space for scanline buffer");
        return;
    }
    for (uint32 row = 0; row < h; row++) {
        if (TIFFReadScanline(tif, tf_buf, row, 0) < 0)
            break;
        unsigned char* cp = tf_buf;
        if (alpha) {
            cc = 0;
            for (; cc < tf_bytesperrow; cc += samplesperpixel) {
                DOBREAK(breaklen, nc, fd)
                /*
                 * For images with alpha, matte against
                 * a white background; i.e.
                 *    Cback * (1 - Aimage)
                 * where Cback = 1.
                 */
                int adjust = 255 - cp[nc];
                switch (nc) {
                case 4: c = *cp++ + adjust; PUTHEX(c,fd);
                case 3: c = *cp++ + adjust; PUTHEX(c,fd);
                case 2: c = *cp++ + adjust; PUTHEX(c,fd);
                case 1: c = *cp++ + adjust; PUTHEX(c,fd);
                }
                cp += es;
            }
        } else {
            cc = 0;
            for (; cc < tf_bytesperrow; cc += samplesperpixel) {
                DOBREAK(breaklen, nc, fd)
                switch (nc) {
                case 4: c = *cp++; PUTHEX(c,fd);
                case 3: c = *cp++; PUTHEX(c,fd);
                case 2: c = *cp++; PUTHEX(c,fd);
                case 1: c = *cp++; PUTHEX(c,fd);
                }
                cp += es;
            }
        }
    }
    _TIFFfree((char *) tf_buf);
}

void
PSDataColorSeparate(FILE* fd, TIFF* tif, uint32 w, uint32 h, int nc)
{
    int breaklen = MAXLINE, cc;
    unsigned char *cp;

    (void) w;
    unsigned char* tf_buf = static_cast<unsigned char*>(_TIFFmalloc(tf_bytesperrow));
    if (tf_buf == nullptr) {
        TIFFError(filename, "No space for scanline buffer");
        return;
    }
    int maxs = samplesperpixel > nc ? nc : samplesperpixel;
    for (uint32 row = 0; row < h; row++) {
        for (int s = 0; s < maxs; s++) {
            if (TIFFReadScanline(tif, tf_buf, row, s) < 0)
                break;
            for (cp = tf_buf, cc = 0; cc < tf_bytesperrow; cc++) {
                DOBREAK(breaklen, 1, fd)
                unsigned char c = *cp++;
                PUTHEX(c,fd);
            }
        }
    }
    _TIFFfree((char *) tf_buf);
}

#define PUTRGBHEX(c,fd) \
    PUTHEX(rmap[c],fd); PUTHEX(gmap[c],fd); PUTHEX(bmap[c],fd)

void
PSDataPalette(FILE* fd, TIFF* tif, uint32 w, uint32 h)
{
    uint16 *rmap, *gmap, *bmap;
    int breaklen = MAXLINE, cc;
    unsigned char *cp, c;

    (void) w;
    if (!TIFFGetField(tif, TIFFTAG_COLORMAP, &rmap, &gmap, &bmap)) {
        TIFFError(filename, "Palette image w/o \"Colormap\" tag");
        return;
    }
    switch (bitspersample) {
    case 8: case 4: case 2: case 1:
        break;
    default:
        TIFFError(filename, "Depth %d not supported", bitspersample);
        return;
    }
    int nc = 3 * (8 / bitspersample);
    unsigned char* tf_buf = static_cast<unsigned char*>(_TIFFmalloc(tf_bytesperrow));
    if (tf_buf == nullptr) {
        TIFFError(filename, "No space for scanline buffer");
        return;
    }
    if (checkcmap(tif, 1<<bitspersample, rmap, gmap, bmap) == 16) {
#define CVT(x)      (((x) * 255) / ((1U<<16)-1))
        for (int i = (1 << bitspersample) - 1; i >= 0; i--) {
            rmap[i] = CVT(rmap[i]);
            gmap[i] = CVT(gmap[i]);
            bmap[i] = CVT(bmap[i]);
        }
#undef CVT
    }
    for (uint32 row = 0; row < h; row++) {
        if (TIFFReadScanline(tif, tf_buf, row, 0) < 0)
            break;
        for (cp = tf_buf, cc = 0; cc < tf_bytesperrow; cc++) {
            DOBREAK(breaklen, nc, fd)
            switch (bitspersample) {
            case 8:
                c = *cp++; PUTRGBHEX(c, fd);
                break;
            case 4:
                c = *cp++; PUTRGBHEX(c&0xf, fd);
                c >>= 4;   PUTRGBHEX(c, fd);
                break;
            case 2:
                c = *cp++; PUTRGBHEX(c&0x3, fd);
                c >>= 2;   PUTRGBHEX(c&0x3, fd);
                c >>= 2;   PUTRGBHEX(c&0x3, fd);
                c >>= 2;   PUTRGBHEX(c, fd);
                break;
            case 1:
                c = *cp++; PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c&0x1, fd);
                c >>= 1;   PUTRGBHEX(c, fd);
                break;
            }
        }
    }
    _TIFFfree((char *) tf_buf);
}

void
PSDataBW(FILE* fd, TIFF* tif, uint32 w, uint32 h)
{
    int breaklen = MAXLINE;
    unsigned char* tf_buf;
    unsigned char* cp;
    const tsize_t stripsize = TIFFStripSize(tif);

#if defined( EXP_ASCII85ENCODER )
    int ascii85_l;      /* Length, in bytes, of ascii85_p[] data */
    uint8   *ascii85_p = nullptr;     /* Holds ASCII85 encoded data */
#endif

    (void) w; (void) h;
    tf_buf = static_cast<unsigned char*>(_TIFFmalloc(stripsize));
    if (tf_buf == nullptr) {
        TIFFError(filename, "No space for scanline buffer");
        return;
    }

#if defined( EXP_ASCII85ENCODER )
    if ( ascii85 ) {
        /*
         * Allocate a buffer to hold the ASCII85 encoded data.  Note
         * that it is allocated with sufficient room to hold the
         * encoded data (5*stripsize/4) plus the EOD marker (+8)
         * and formatting line breaks.  The line breaks are more
         * than taken care of by using 6*stripsize/4 rather than
         * 5*stripsize/4.
         */

        ascii85_p = static_cast<unsigned char*>(_TIFFmalloc(stripsize + stripsize / 2 + 8));

        if ( !ascii85_p ) {
        _TIFFfree( tf_buf );

        TIFFError( filename, "Cannot allocate ASCII85 encoding buffer." );
        return;
        }
    }
#endif

    if (ascii85)
        Ascii85Init();

    for (tstrip_t s = 0; s < TIFFNumberOfStrips(tif); s++) {
        int cc = TIFFReadEncodedStrip(tif, s, tf_buf, stripsize);
        if (cc < 0) {
            TIFFError(filename, "Can't read strip");
            break;
        }
        cp = tf_buf;
        if (photometric == PHOTOMETRIC_MINISWHITE) {
            for (cp += cc; --cp >= tf_buf;)
                *cp = ~*cp;
            cp++;
        }
        if (ascii85 && ascii85_p) {
#if defined( EXP_ASCII85ENCODER )
            ascii85_l = Ascii85EncodeBlock( ascii85_p, 1, cp, cc );

            if ( ascii85_l > 0 )
                fwrite( ascii85_p, ascii85_l, 1, fd );
#else
            while (cc-- > 0)
                Ascii85Put(*cp++, fd);
#endif /* EXP_ASCII85_ENCODER */
        } else {
            while (cc-- > 0) {
                unsigned char c = *cp++;
                DOBREAK(breaklen, 1, fd)
                PUTHEX(c, fd);
            }
        }
    }

    if ( !ascii85 )
    {
        if ( level2 || level3)
        fputs(">\n", fd);
    }
#if !defined( EXP_ASCII85ENCODER )
    else
        Ascii85Flush(fd);
#else
    if ( ascii85_p )
        _TIFFfree( ascii85_p );
#endif

    _TIFFfree(tf_buf);
}

void
PSRawDataBW(FILE* fd, TIFF* tif, uint32 w, uint32 h)
{
    uint32 *bc;
    uint32 bufsize;
    int breaklen = MAXLINE, cc;
    uint16 fillorder;
    unsigned char *tf_buf;
    unsigned char c;
    tstrip_t s;

#if defined( EXP_ASCII85ENCODER )
    int         ascii85_l;      /* Length, in bytes, of ascii85_p[] data */
    uint8       *   ascii85_p = nullptr;      /* Holds ASCII85 encoded data */
#endif

    (void) w; (void) h;
    TIFFGetFieldDefaulted(tif, TIFFTAG_FILLORDER, &fillorder);
    TIFFGetField(tif, TIFFTAG_STRIPBYTECOUNTS, &bc);

    /*
     * Find largest strip:
     */

    bufsize = bc[0];

    for ( s = 0; ++s < static_cast<tstrip_t>(tf_numberstrips); ) {
        if ( bc[s] > bufsize )
            bufsize = bc[s];
    }

    tf_buf = static_cast<unsigned char*>(_TIFFmalloc(bufsize));
    if (tf_buf == nullptr) {
        TIFFError(filename, "No space for strip buffer");
        return;
    }

#if defined( EXP_ASCII85ENCODER )
    if ( ascii85 ) {
        /*
         * Allocate a buffer to hold the ASCII85 encoded data.  Note
         * that it is allocated with sufficient room to hold the
         * encoded data (5*bufsize/4) plus the EOD marker (+8)
         * and formatting line breaks.  The line breaks are more
         * than taken care of by using 6*bufsize/4 rather than
         * 5*bufsize/4.
         */

        ascii85_p = static_cast<unsigned char*>(_TIFFmalloc(bufsize + bufsize / 2 + 8));

        if ( !ascii85_p ) {
        _TIFFfree( tf_buf );

        TIFFError( filename, "Cannot allocate ASCII85 encoding buffer." );
        return;
        }
    }
#endif

    for (s = 0; s < static_cast<tstrip_t>(tf_numberstrips); s++) {
        cc = TIFFReadRawStrip(tif, s, tf_buf, bc[s]);
        if (cc < 0) {
            TIFFError(filename, "Can't read strip");
            break;
        }
        if (fillorder == FILLORDER_LSB2MSB)
            TIFFReverseBits(tf_buf, cc);
        if (!ascii85) {
            for (unsigned char* cp = tf_buf; cc > 0; cc--) {
                DOBREAK(breaklen, 1, fd)
                c = *cp++;
                PUTHEX(c, fd);
            }
            fputs(">\n", fd);
            breaklen = MAXLINE;
        } else {
            Ascii85Init();
#if defined( EXP_ASCII85ENCODER )
            ascii85_l = Ascii85EncodeBlock( ascii85_p, 1, tf_buf, cc );

            if ( ascii85_l > 0 && ascii85_p )
                fwrite( ascii85_p, ascii85_l, 1, fd );
#else
            for (cp = tf_buf; cc > 0; cc--)
                Ascii85Put(*cp++, fd);
            Ascii85Flush(fd);
#endif  /* EXP_ASCII85ENCODER */
        }
    }
    _TIFFfree((char *) tf_buf);

#if defined( EXP_ASCII85ENCODER )
    if ( ascii85_p )
        _TIFFfree( ascii85_p );
#endif
}

void
Ascii85Init(void)
{
    ascii85breaklen = 2*MAXLINE;
    ascii85count = 0;
}

static char*
Ascii85Encode(unsigned char* raw)
{
    static char encoded[6];

    uint32 word = ((raw[0] << 8) + raw[1] << 16) + (raw[2] << 8) + raw[3];
    if (word != 0L) {
        uint32 q = word / (85L * 85 * 85 * 85);  /* actually only a byte */
        encoded[0] = q + '!';

        word -= q * (85L*85*85*85); q = word / (85L*85*85);
        encoded[1] = q + '!';

        word -= q * (85L*85*85); q = word / (85*85);
        encoded[2] = q + '!';

        uint16 w1 = static_cast<uint16>(word - q * (85L * 85));
        encoded[3] = w1 / 85 + '!';
        encoded[4] = w1 % 85 + '!';
        encoded[5] = '\0';
    } else
        encoded[0] = 'z', encoded[1] = '\0';
    return encoded;
}

void
Ascii85Put(unsigned char code, FILE* fd)
{
    ascii85buf[ascii85count++] = code;
    if (ascii85count >= 4) {
        unsigned char* p;
        int n;

        for (n = ascii85count, p = ascii85buf; n >= 4; n -= 4, p += 4) {
            for (char* cp = Ascii85Encode(p); *cp; cp++) {
                putc(*cp, fd);
                if (--ascii85breaklen == 0) {
                    putc('\n', fd);
                    ascii85breaklen = 2*MAXLINE;
                }
            }
        }
        _TIFFmemcpy(ascii85buf, p, n);
        ascii85count = n;
    }
}

void
Ascii85Flush(FILE* fd)
{
    if (ascii85count > 0) {
        _TIFFmemset(&ascii85buf[ascii85count], 0, 3);
        char* res = Ascii85Encode(ascii85buf);
        fwrite(res[0] == 'z' ? "!!!!" : res, ascii85count + 1, 1, fd);
    }
    fputs("~>\n", fd);
}
#if defined( EXP_ASCII85ENCODER)

#define A85BREAKCNTR    ascii85breaklen
#define A85BREAKLEN     (2*MAXLINE)

/*****************************************************************************
*
* Name:         Ascii85EncodeBlock( ascii85_p, f_eod, raw_p, raw_l )
*
* Description:  This routine will encode the raw data in the buffer described
*               by raw_p and raw_l into ASCII85 format and store the encoding
*               in the buffer given by ascii85_p.
*
* Parameters:   ascii85_p   -   A buffer supplied by the caller which will
*                               contain the encoded ASCII85 data.
*               f_eod       -   Flag: Nz means to end the encoded buffer with
*                               an End-Of-Data marker.
*               raw_p       -   Pointer to the buffer of data to be encoded
*               raw_l       -   Number of bytes in raw_p[] to be encoded
*
* Returns:      (int)   <   0   Error, see errno
*                       >=  0   Number of bytes written to ascii85_p[].
*
* Notes:        An external variable given by A85BREAKCNTR is used to
*               determine when to insert newline characters into the
*               encoded data.  As each byte is placed into ascii85_p this
*               external is decremented.  If the variable is decrement to
*               or past zero then a newline is inserted into ascii85_p
*               and the A85BREAKCNTR is then reset to A85BREAKLEN.
*                   Note:  for efficiency reasons the A85BREAKCNTR variable
*                          is not actually checked on *every* character
*                          placed into ascii85_p but often only for every
*                          5 characters.
*
*               THE CALLER IS RESPONSIBLE FOR ENSURING THAT ASCII85_P[] IS
*               SUFFICIENTLY LARGE TO THE ENCODED DATA!
*                   You will need at least 5 * (raw_l/4) bytes plus space for
*                   newline characters and space for an EOD marker (if
*                   requested).  A safe calculation is to use 6*(raw_l/4) + 8
*                   to size ascii85_p.
*
*****************************************************************************/

int Ascii85EncodeBlock( uint8 * ascii85_p, unsigned f_eod, const uint8 * raw_p, int raw_l )

{
    char                        ascii85_block[5];     /* Encoded 5 tuple */
    int                         rc;             /* Return code */
    uint32                      val32;          /* Unencoded 4 tuple */

    int ascii85_l = 0;                              /* Nothing written yet */

    if ( raw_p )
    {
        --raw_p;                                /* Prepare for pre-increment fetches */

        for ( ; raw_l > 3; raw_l -= 4 )
        {
            val32  = *++raw_p << 24;
            val32 += *++raw_p << 16;
            val32 += *++raw_p <<  8;
            val32 += *++raw_p;

            if ( val32 == 0 )                   /* Special case */
            {
                ascii85_p[ascii85_l] = 'z';
                rc = 1;
            }

            else
            {
                ascii85_block[4] = val32 % 85 + 33;
                val32 /= 85;

                ascii85_block[3] = val32 % 85 + 33;
                val32 /= 85;

                ascii85_block[2] = val32 % 85 + 33;
                val32 /= 85;

                ascii85_block[1] = val32 % 85 + 33;
                ascii85_block[0] = val32 / 85 + 33;

                _TIFFmemcpy( &ascii85_p[ascii85_l], ascii85_block, sizeof ascii85_block );
                rc = sizeof ascii85_block;
            }

            ascii85_l += rc;

            if ( (A85BREAKCNTR -= rc) <= 0 )
            {
                ascii85_p[ascii85_l] = '\n';
                ++ascii85_l;
                A85BREAKCNTR = A85BREAKLEN;
            }
        }

        /*
         * Output any straggler bytes:
         */

        if ( raw_l > 0 )
        {
            int len = raw_l + 1;
            val32 = *++raw_p << 24;             /* Prime the pump */

            if ( --raw_l > 0 )  val32 += *++raw_p << 16;
            if ( --raw_l > 0 )  val32 += *++raw_p <<  8;

            val32 /= 85;

            ascii85_block[3] = val32 % 85 + 33;
            val32 /= 85;

            ascii85_block[2] = val32 % 85 + 33;
            val32 /= 85;

            ascii85_block[1] = val32 % 85 + 33;
            ascii85_block[0] = val32 / 85 + 33;

            _TIFFmemcpy( &ascii85_p[ascii85_l], ascii85_block, len );
            ascii85_l += len;
        }
    }

    /*
     * If requested add an ASCII85 End Of Data marker:
     */

    if ( f_eod )
    {
        ascii85_p[ascii85_l++] = '~';
        ascii85_p[ascii85_l++] = '>';
        ascii85_p[ascii85_l++] = '\n';
    }

    return ascii85_l;

}   /* Ascii85EncodeBlock() */

#endif  /* EXP_ASCII85ENCODER */


char* stuff[] = {
/*"usage: tiff2ps [options] input.tif ...",
"where options are:",
" -1            generate PostScript Level 1 (default)",
" -2            generate PostScript Level 2",
" -3            generate PostScript Level 3",
" -8            disable use of ASCII85 encoding with PostScript Level 2/3",
" -a            convert all directories in file (default is first)",
" -b #          set the bottom margin to # inches",
" -c            center image (-b and -l still add to this)",
" -d #          convert directory number #",
" -D            enable duplex printing (two pages per sheet of paper)",
" -e            generate Encapsulated PostScript (EPS) (implies -z)",
" -h #          assume printed page height is # inches (default 11)",
" -w #          assume printed page width is # inches (default 8.5)",
" -H #          split image if height is more than # inches",
" -L #          overLap split images by # inches",
" -i #          enable/disable (Nz/0) pixel interpolation (default: enable)",
" -l #          set the left margin to # inches",
" -m            use \"imagemask\" operator instead of \"image\"",
" -o #          convert directory at file offset #",
" -O file       write PostScript to file instead of standard output",
" -p            generate regular PostScript",
" -r            rotate by 180 degrees",
" -s            generate PostScript for a single image",
" -T            print pages for top edge binding",
" -x            override resolution units as centimeters",
" -y            override resolution units as inches",
" -z            enable printing in the deadzone (only for PostScript Level 2/3)",*/
nullptr
};

static void
usage(int /*code*/)
{
/*  char buf[BUFSIZ];
    int i;

    setbuf(stderr, buf);
        fprintf(stderr, "%s\n\n", TIFFGetVersion());
    for (i = 0; stuff[i] != NULL; i++)
        fprintf(stderr, "%s\n", stuff[i]);
    exit(code);*/
}
