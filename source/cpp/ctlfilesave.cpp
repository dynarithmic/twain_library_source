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
#include "ctlfilesave.h"

using namespace dynarithmic;

#define CREATEFILEFORMATINFO(T,E) _T(T), _T(E), sizeof(T)-1

struct FileNode
{
    int fileType;
    LPCTSTR filter;
    LPCTSTR extension;
    int filtersize;
    constexpr FileNode(int fType, LPCTSTR fil, LPCTSTR ext, int filSize) :
        fileType(fType), filter(fil), extension(ext), filtersize(filSize - 1) {}
};

CTL_StringType dynarithmic::PromptForFileName(CTL_TwainDLLHandle* pHandle, CTL_TwainFileFormatEnum nFileAcquireType)
{
    constexpr std::array<FileNode, 71> saveFileMap = { {
        {TWAINFileFormat_TIFFLZW, CREATEFILEFORMATINFO("TIFF Format (LZW) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFLZWMULTI, CREATEFILEFORMATINFO("TIFF Format (LZW) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFLZW, CREATEFILEFORMATINFO("Big TIFF Format (LZW) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFLZWMULTI, CREATEFILEFORMATINFO("Big TIFF Format (LZW) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFNONE, CREATEFILEFORMATINFO("Big TIFF Format (Uncompressed) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFNONEMULTI, CREATEFILEFORMATINFO("Big TIFF Format (Uncompressed) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFNONE, CREATEFILEFORMATINFO("TIFF Uncompressed Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFNONEMULTI, CREATEFILEFORMATINFO("TIFF Uncompressed Format (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_FF_TIFF, CREATEFILEFORMATINFO("TIFF Uncompressed Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFGROUP3, CREATEFILEFORMATINFO("TIFF Fax Group 3 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFGROUP3MULTI, CREATEFILEFORMATINFO("TIFF Fax Group 3 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFGROUP4, CREATEFILEFORMATINFO("TIFF Fax Group 4 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFGROUP4MULTI, CREATEFILEFORMATINFO("TIFF Fax Group 4 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFGROUP3, CREATEFILEFORMATINFO("Big TIFF Fax Group 3 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFGROUP3MULTI, CREATEFILEFORMATINFO("Big TIFF Fax Group 3 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFGROUP4, CREATEFILEFORMATINFO("Big TIFF Fax Group 4 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFGROUP4MULTI, CREATEFILEFORMATINFO("Big TIFF Fax Group 4 Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFPIXARLOG, CREATEFILEFORMATINFO("TIFF (Pixar-Log Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFPIXARLOGMULTI, CREATEFILEFORMATINFO("TIFF (Pixar-Log Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFJPEG, CREATEFILEFORMATINFO("TIFF (JPEG Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_TIFFJPEGMULTI, CREATEFILEFORMATINFO("TIFF (JPEG Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFJPEG, CREATEFILEFORMATINFO("Big TIFF (JPEG Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BIGTIFFJPEGMULTI, CREATEFILEFORMATINFO("Big TIFF (JPEG Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_TIFFPACKBITS, CREATEFILEFORMATINFO("TIFF (Macintosh RLE Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_TIFFPACKBITSMULTI, CREATEFILEFORMATINFO("TIFF (Macintosh RLE Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_TIFFDEFLATE, CREATEFILEFORMATINFO("TIFF (ZLib Deflate Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_TIFFDEFLATEMULTI, CREATEFILEFORMATINFO("TIFF (ZLib Deflate Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_BIGTIFFDEFLATE, CREATEFILEFORMATINFO("Big TIFF (ZLib Deflate Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_BIGTIFFDEFLATEMULTI, CREATEFILEFORMATINFO("Big TIFF (ZLib Deflate Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_BIGTIFFPACKBITS, CREATEFILEFORMATINFO("Big TIFF (Macintosh RLE Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {DTWAIN_BIGTIFFPACKBITSMULTI, CREATEFILEFORMATINFO("Big TIFF (Macintosh RLE Compression) (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_JBIG, CREATEFILEFORMATINFO("JBIG Format (*.jbg)\0*.jbg\0\0", ".jbg")},
        {TWAINFileFormat_JPEG2000, CREATEFILEFORMATINFO("JPEG-2000 Format (*.jp2)\0*.jp2\0\0",".jp2")},
        {TWAINFileFormat_WMF, CREATEFILEFORMATINFO("Windows MetaFile (*.wmf)\0*.wmf\0\0",".wmf")},
        {TWAINFileFormat_EMF, CREATEFILEFORMATINFO("Windows Enhanced MetaFile (*.emf)\0*.emf\0\0",".emf")},
        {TWAINFileFormat_PSD, CREATEFILEFORMATINFO("Adobe Photoshop Format (*.psd)\0*.psd\0\0",".psd")},
        {DTWAIN_FF_TIFFMULTI, CREATEFILEFORMATINFO("Multipage TIFF Format (*.tif)\0*.tif\0\0", ".tif")},
        {TWAINFileFormat_BMP, CREATEFILEFORMATINFO("Windows Bitmap Format (*.bmp)\0*.bmp\0\0", ".bmp")},
        {DTWAIN_FF_BMP, CREATEFILEFORMATINFO("Windows Bitmap Format (*.bmp)\0*.bmp\0\0", ".bmp")},
        {TWAINFileFormat_BMPRLE, CREATEFILEFORMATINFO("Windows Bitmap Format (RLE) (*.bmp)\0*.bmp\0\0", ".bmp")},
        {TWAINFileFormat_JPEG, CREATEFILEFORMATINFO("JFIF (JPEG) Format (*.jpg)\0*.jpg\0\0",".jpg")},
        {DTWAIN_FF_JFIF, CREATEFILEFORMATINFO("JFIF (JPEG) Format (*.jpg)\0*.jpg\0\0",".jpg")},
        {TWAINFileFormat_PDF, CREATEFILEFORMATINFO("Adobe Acrobat Format (*.pdf)\0*.pdf\0\0",".pdf")},
        {TWAINFileFormat_PDFMULTI, CREATEFILEFORMATINFO("Adobe Acrobat Format (*.pdf)\0*.pdf\0\0",".pdf")},
        {TWAINFileFormat_TEXT, CREATEFILEFORMATINFO("Text file (*.txt)\0*.txt\0\0",".txt")},
        {TWAINFileFormat_TEXTMULTI, CREATEFILEFORMATINFO("Text file (*.txt)\0*.txt\0\0",".txt")},
        {TWAINFileFormat_ICO, CREATEFILEFORMATINFO("Icon file (*.ico)\0*.ico\0\0",".ico")},
        {TwainFileFormat_ICO_RESIZED, CREATEFILEFORMATINFO("Icon file (*.ico)\0*.ico\0\0",".ico")},
        {TWAINFileFormat_ICO_VISTA, CREATEFILEFORMATINFO("Icon file (*.ico)\0*.ico\0\0",".ico")},
        {DTWAIN_FF_SPIFF, CREATEFILEFORMATINFO("SPIFF Format (*.spf)\0*.spf\0\0",".spf")},
        {DTWAIN_FF_EXIF, CREATEFILEFORMATINFO("EXIF Format (*.exf)\0*.exf\0\0",".exf")},
        {TWAINFileFormat_PCX, CREATEFILEFORMATINFO("PCX Format (*.pcx)\0*.pcx\0\0",".pcx")},
        {TWAINFileFormat_DCX, CREATEFILEFORMATINFO("DCX Format (*.dcx)\0*.dcx\0\0", ".dcx")},
        {TWAINFileFormat_WBMP, CREATEFILEFORMATINFO("WBMP (Wireless Bitmap Format) (*.wbmp)\0*.wbmp\0\0", ".wbmp")},
        {TwainFileFormat_WBMP_RESIZED, CREATEFILEFORMATINFO("WBMP (Wireless Bitmap Format) (*.wbmp)\0*.wbmp\0\0", ".wbmp")},
        {TWAINFileFormat_PNG, CREATEFILEFORMATINFO("PNG Format (*.png)\0*.png\0\0",".png")},
        {DTWAIN_FF_PNG, CREATEFILEFORMATINFO("PNG Format (*.png)\0*.png\0\0",".png")},
        {TWAINFileFormat_TGA, CREATEFILEFORMATINFO("Targa (TGA) Format (*.tga)\0*.tga\0\0",".tga")},
        {TWAINFileFormat_TGARLE, CREATEFILEFORMATINFO("Targa (TGA) Format (Run Length Encoded) (*.tga)\0*.tga\0\0", ".tga")},
        {DTWAIN_POSTSCRIPT1, CREATEFILEFORMATINFO("Postscript Level 1 Format (*.ps)\0*.ps\0\0",".ps")},
        {DTWAIN_POSTSCRIPT1MULTI, CREATEFILEFORMATINFO("Postscript Level 1 Format (*.ps)\0*.ps\0\0",".ps")},
        {DTWAIN_POSTSCRIPT2, CREATEFILEFORMATINFO("Postscript Level 2 Format (*.ps)\0*.ps\0\0",".ps")},
        {DTWAIN_POSTSCRIPT2MULTI, CREATEFILEFORMATINFO("Postscript Level 2 Format (*.ps)\0*.ps\0\0",".ps")},
        {DTWAIN_POSTSCRIPT3, CREATEFILEFORMATINFO("Postscript Level 3 Format (*.ps)\0*.ps\0\0",".ps")},
        {DTWAIN_POSTSCRIPT3MULTI, CREATEFILEFORMATINFO("Postscript Level 3 Format (*.ps)\0*.ps\0\0",".ps")},
        {TWAINFileFormat_GIF, CREATEFILEFORMATINFO("GIF Format (*.gif)\0*.gif\0\0",".gif")},
        {DTWAIN_FF_FPX, CREATEFILEFORMATINFO("Flash Picture (*.fpx)\0*.fpx\0\0",".fpx")},
        {DTWAIN_FF_PICT, CREATEFILEFORMATINFO("Macintosh PICT format (*.pic)\0*.pic\0\0",".pic")},
        {DTWAIN_FF_XBM, CREATEFILEFORMATINFO("XBM format (*.xbm)\0*.xbm\0\0",".xbm")},
        {DTWAIN_WEBP, CREATEFILEFORMATINFO("webp format (*.webp)\0*.webp\0\0", ".webp")},
        {DTWAIN_PBM, CREATEFILEFORMATINFO("pbm format (*.pbm)\0*.pbm\0\0", ".pbm")},
    } };

    CTL_StringType szFilter;
    LPCTSTR szExt = nullptr;

    const auto it =
        dynarithmic::generic_array_finder_if(saveFileMap, [&](const FileNode& fNode)
            { return fNode.fileType == nFileAcquireType; });

    if (it.first)
    {
        const FileNode fNode = saveFileMap[it.second];
        szFilter.append(fNode.filter, fNode.filtersize);
        szExt = fNode.extension;
    }
    else
    {
        CTL_StringStreamType strm;
        strm << nFileAcquireType << _T(" format");
        szFilter = strm.str();
        szFilter.append(_T("*\0\0"), 3);
        szExt = _T(".");
    }

#ifdef _WIN32
    TCHAR szFile[1024] = {};
    // prompt for filename

    OPENFILENAME ofn = {};
    OPENFILENAME* pOfn = &ofn;

    if (pHandle->m_pofn)
        pOfn = pHandle->m_pofn.get();
    szFile[0] = _T('\0');
    pOfn->lStructSize = sizeof(OPENFILENAME);
    const auto sTitle = pHandle->m_CustomPlacement.sTitle;
    if (pOfn == &ofn)
    {
        pOfn->hwndOwner = nullptr;
        pOfn->lpstrFilter = szFilter.data();
        pOfn->lpstrFile = szFile;
        pOfn->nMaxFile = sizeof szFile - 5;
        pOfn->Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
            OFN_NOREADONLYRETURN | OFN_EXPLORER;
        if (pHandle->m_CustomPlacement.nOptions != 0)
        {
            pOfn->lpfnHook = pHandle->m_pSaveAsDlgProc;
            pOfn->Flags |= OFN_ENABLEHOOK;
            pOfn->lCustData = (LPARAM)&pHandle->m_CustomPlacement;
            if (!StringWrapper::IsEmpty(sTitle))
                pOfn->lpstrTitle = sTitle.c_str();
        }
    }

    if (!GetSaveFileName(pOfn))
    {
        return {};                    // user canceled dialog
    }

    // supply default extension - GetOpenFileName doesn't seem to do it!
    int nExt = pOfn->nFileExtension;
    if (nExt && !szFile[nExt])
    {
        // no extension
        lstrcat(szFile, szExt);
    }
    return szFile;
#else
    return {};
#endif
}
