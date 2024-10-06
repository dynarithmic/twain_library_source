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
#ifdef _MSC_VER
#pragma warning (disable : 4786)
#endif
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <functional>
#include <memory>
#include <boost/optional.hpp>
#include "winconst.h"
#include "winbit32.h"
#include "ctltwainmanager.h"
#include "ctldib.h"
#include "arrayfactory.h"
#include "ctlfileutils.h"
/* Header signatures for various resources */
#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */
#define MAXREAD     65535
#define BOUND(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

#define LPBimage(lpbi)  ((HPSTR)lpbi+lpbi->biSize+(long)(lpbi->biClrUsed*sizeof(RGBQUAD)))
#define LPBlinewidth(lpbi) (WIDTHBYTES((WORD)lpbi->biWidth*lpbi->biBitCount))

#ifdef USE_NAMESPACES

#endif
using namespace dynarithmic;

CTL_TwainDibInfo::CTL_TwainDibInfo() : m_hDib(nullptr), m_hPal(nullptr)
{}

bool CTL_TwainDibInfo::operator == (const CTL_TwainDibInfo& rInfo) const
{
    if ( this == &rInfo )
        return true;
    return m_hPal == rInfo.m_hPal &&
        m_hDib == rInfo.m_hDib;
}

void CTL_TwainDibInfo::SetDib(HANDLE hDib)
{
    m_hDib = hDib;
}

void CTL_TwainDibInfo::SetPalette( HPALETTE hPal )
{ m_hPal = hPal; }

void CTL_TwainDibInfo::DeleteAllDibInfo()
{
    DeleteDibPalette();
    DeleteDib();
}

void CTL_TwainDibInfo::DeleteDibPalette()
{
    if (m_hPal)
    {
        ImageMemoryHandler::DeleteObject(static_cast<HGDIOBJ>(m_hPal));
        m_hPal = nullptr;
    }
}

void CTL_TwainDibInfo::DeleteDib()
{
    if (m_hDib)
    {
        ImageMemoryHandler::GlobalFree(m_hDib);
        m_hDib = nullptr;
    }
}

HANDLE CTL_TwainDibInfo::GetDib() const
{
    return m_hDib;
}

HPALETTE CTL_TwainDibInfo::GetPalette() const
{
    return m_hPal;
}

/////////////////////////////////////////////////////////////////////////
// DIB utilities (Dib utilities classed from the EZ_TWain application  //
/////////////////////////////////////////////////////////////////////////
// Construction
CTL_TwainDib::CTL_TwainDib() : m_bAutoDelete(false), m_bAutoDeletePalette(false),
                                m_bIsValid(false), m_bJpegProgressive(false), m_nJpegQuality(75)
{ }

CTL_TwainDib::CTL_TwainDib(HANDLE hDib, HWND hWnd) : m_bAutoDelete(true), m_bAutoDeletePalette(false),
m_bIsValid(true),m_bJpegProgressive(false), m_nJpegQuality(75)
{
    m_TwainDibInfo.SetDib(hDib);
}

// Read a Dib from a file
CTL_TwainDib::CTL_TwainDib(LPCSTR lpszFileName, HWND hWnd) :  m_bAutoDelete(false),
                                                              m_bAutoDeletePalette(false),
                                                              m_bIsValid(true),
                                                              m_bJpegProgressive(false),
                                                              m_nJpegQuality(75)
{
    Init();
    m_TwainDibInfo.SetDib( ReadDibBitmap( lpszFileName ));
}

CTL_TwainDib::CTL_TwainDib(const CTL_TwainDib &rDib) : m_bAutoDelete(false),
                                                        m_bAutoDeletePalette(false),
                                                        m_bIsValid(true),
                                                        m_bJpegProgressive(false),
                                                        m_nJpegQuality(75)
{ SetEqual( rDib ); }

void CTL_TwainDib::swap(CTL_TwainDib& left, CTL_TwainDib& rt) noexcept
{
    std::swap(left.m_TwainDibInfo, rt.m_TwainDibInfo);
    std::swap(left.m_bIsValid, rt.m_bIsValid);
    std::swap(left.m_bJpegProgressive, rt.m_bJpegProgressive);
    std::swap(left.m_nJpegQuality, rt.m_nJpegQuality);
    std::swap(left.m_bAutoDelete, rt.m_bAutoDelete);
    std::swap(left.m_bAutoDeletePalette, rt.m_bAutoDeletePalette);
}

CTL_TwainDib& CTL_TwainDib::operator=(const CTL_TwainDib& rDib)
{
    if ( this != &rDib )
    {
        CTL_TwainDib temp(*this);
        swap(*this, temp);
    }
    return *this;
}

void CTL_TwainDib::SetEqual( const CTL_TwainDib &rDib )
{
    m_bAutoDelete = rDib.m_bAutoDelete;
    CTL_TwainDib *pDib = (CTL_TwainDib *)&rDib;
    pDib->m_bAutoDelete = false;
    m_bIsValid = rDib.m_bIsValid;
    m_TwainDibInfo = rDib.m_TwainDibInfo;
    m_nJpegQuality = rDib.m_nJpegQuality;
    m_bAutoDeletePalette = rDib.m_bAutoDeletePalette;
}

/***************************************************************************
*  PURPOSE    : Will read a file in DIB format and return a global HANDLE  *
*               to it's BITMAPINFO.  This function will work with both     *
*               "old" (BITMAPCOREHEADER) and "new" (BITMAPINFOHEADER)      *
*               bitmap formats, but will always return a "new" BITMAPINFO  *
*                                                                          *
*  RETURNS    : A handle to the BITMAPINFO of the DIB in the file.         *
*                                                                          *
****************************************************************************/
HANDLE CTL_TwainDib::ReadDibBitmap(LPCSTR)
{
    return nullptr;
}


void CTL_TwainDib::SetJpegValues(int nQuality, bool bProgressive)
{
    m_nJpegQuality = nQuality;
    m_bJpegProgressive = bProgressive;
}


int CTL_TwainDib::WriteDibBitmap (DTWAINImageInfoEx& ImageInfo,
                                  LPCTSTR szFile, int nFormat/*=BmpFormat*/,
                                  bool bOpenFile/*=TRUE*/, int fhFile/*=0*/)
{
    std::unique_ptr<CTL_ImageIOHandler> pHandler;
    ImageInfo.IsPDF = false;
    ResolvePostscriptOptions(ImageInfo, nFormat);
    ImageInfo.IsBigTiff = ConstexprUtils::IsFileTypeBigTiff(static_cast<dynarithmic::CTL_TwainFileFormatEnum>(nFormat));
    switch (nFormat )
    {
        case BmpFormat:
        case BmpFormatRLE:
            ImageInfo.IsRLE = (nFormat == BmpFormatRLE);
            pHandler = std::make_unique<CTL_BmpIOHandler>( this, ImageInfo );
        break;

        case JpegFormat:
            pHandler = std::make_unique<CTL_JpegIOHandler>( this, ImageInfo);
        break;
        case Jpeg2000Format:
            ImageInfo.nJpegQuality = 100;
            ImageInfo.bProgressiveJpeg = 0;
            pHandler = std::make_unique<CTL_Jpeg2KIOHandler>( this, ImageInfo );
        break;

        case PDFFormat:
        case PDFFormatMULTI:
        {
            ImageInfo.nJpegQuality = 100;
            ImageInfo.bProgressiveJpeg = 0;
            ImageInfo.IsPDF = true;
            pHandler = std::make_unique<CTL_PDFIOHandler>(this, nFormat, ImageInfo);
        }
        break;

        case PngFormat:
            pHandler = std::make_unique<CTL_PngIOHandler>( this, ImageInfo );
        break;

        case PcxFormat:
            pHandler = std::make_unique<CTL_PcxIOHandler>( this, nFormat, ImageInfo );
        break;
        case TiffFormatNONE:
        case TiffFormatGROUP4:
        case TiffFormatGROUP3:
        case TiffFormatLZW:
        case TiffFormatPACKBITS:
        case TiffFormatDEFLATE:
        case TiffFormatJPEG:
        case TiffFormatPIXARLOG:
        case BigTiffFormatLZW:
        case BigTiffFormatNONE:
        case BigTiffFormatPACKBITS:
        case BigTiffFormatDEFLATE:
        case BigTiffFormatGROUP3:
        case BigTiffFormatGROUP4:
        case BigTiffFormatJPEG:
        case PSFormatLevel1:
        case PSFormatLevel2:
        case PSFormatLevel3:
            if ( nFormat == PSFormatLevel1 || nFormat == PSFormatLevel2 || nFormat == PSFormatLevel3 )
                ImageInfo.IsPostscript = true;
            pHandler = std::make_unique<CTL_TiffIOHandler>( this, nFormat, ImageInfo );
        break;
        case TgaFormat:
        case TgaFormatRLE:
            ImageInfo.IsRLE = (nFormat == TWAINFileFormat_TGARLE);
            pHandler = std::make_unique<CTL_TgaIOHandler>( this, ImageInfo );
        break;

        case WmfFormat:
        case EmfFormat:
            pHandler = std::make_unique<CTL_WmfIOHandler>( this, nFormat );
        break;

        case PsdFormat:
            pHandler = std::make_unique<CTL_PsdIOHandler>( this );
        break;

        case GifFormat:
            pHandler = std::make_unique<CTL_GifIOHandler>( this );
        break;

        case IcoFormat:
        case IcoVistaFormat:
        case IcoFormatResized:
            ImageInfo.IsVistaIcon = nFormat == IcoVistaFormat;
            ImageInfo.IsIcoResized = (nFormat == IcoFormatResized);
            pHandler = std::make_unique<CTL_IcoIOHandler>( this, ImageInfo );
        break;

        case WBMPFormat:
        case WBMPFormatResized:
            ImageInfo.IsWBMPResized = (nFormat == WBMPFormatResized);
            pHandler = std::make_unique<CTL_WBMPIOHandler>(this, ImageInfo);
        break;

        case WEBPFormat:
            pHandler = std::make_unique<CTL_WebpIOHandler>(this);
        break;

        case PBMFormat:
            pHandler = std::make_unique<CTL_PBMIOHandler>(this);
        break;

        case TextFormat:
        case TextFormatMulti:
        {
            // Get the current OCR engine's input format
            DTWAIN_ARRAY a = nullptr;
            const auto pHandle = ImageInfo.theSource->GetDTWAINHandle();
            auto& factory = pHandle->m_ArrayFactory;
            DTWAIN_GetOCRCapValues(static_cast<DTWAIN_OCRENGINE>(pHandle->m_pOCRDefaultEngine.get()), DTWAIN_OCRCV_IMAGEFILEFORMAT,
                                    DTWAIN_CAPGETCURRENT, &a);
            DTWAINArrayLowLevel_RAII raii(pHandle, a);
            if ( a )
            {
                const auto& vValues = factory->underlying_container_t<LONG>(a);
                if ( !vValues.empty() )
                {
                    LONG InputFormat = vValues[0];
                    pHandler = std::make_unique<CTL_TextIOHandler>(this, InputFormat, ImageInfo, pHandle->m_pOCRDefaultEngine.get());
                }
                else
                    return DTWAIN_ERR_BADPARAM;
            }
            else
                return pHandle->m_lLastError;
        }
        break;

        default:
            return DTWAIN_ERR_BADPARAM;
    }
    int bRet;
    try
    {
        pHandler->SetBaseImageInfo(ImageInfo);
        bRet = pHandler->WriteBitmap( szFile, bOpenFile, fhFile );
    }
    catch(...)
    {
        //Exception error
        bRet = DTWAIN_ERR_EXCEPTION_ERROR;
    }
    if ( bRet != 0)
    {
        // If this error is > 0, this is an ISource error, else this
        // is a DTWAIN error
        if ( bRet > 0 )
            bRet = DTWAIN_ERR_FILEXFERSTART - bRet;

    }
    return bRet;
}

CTL_ImageIOHandlerPtr CTL_TwainDib::WriteFirstPageDibMulti(DTWAINImageInfoEx& ImageInfo, LPCTSTR szFile, int nFormat,
                                                          bool bOpenFile, int fhFile, int &nStatus)
{
    CTL_ImageIOHandlerPtr pHandler;
    ImageInfo.IsPDF = false;
    ResolvePostscriptOptions(ImageInfo, nFormat);
    ImageInfo.IsBigTiff = ConstexprUtils::IsFileTypeBigTiff(static_cast<dynarithmic::CTL_TwainFileFormatEnum>(nFormat));
    nStatus = DTWAIN_NO_ERROR;
    switch (nFormat)
    {
        case TiffFormatNONEMULTI:
        case TiffFormatGROUP3MULTI:
        case TiffFormatGROUP4MULTI:
        case TiffFormatPACKBITSMULTI:
        case TiffFormatDEFLATEMULTI:
        case TiffFormatJPEGMULTI:
        case TiffFormatLZWMULTI:
        case BigTiffFormatLZWMULTI:
        case BigTiffFormatNONEMULTI:
        case BigTiffFormatPACKBITSMULTI:
        case BigTiffFormatDEFLATEMULTI:
        case BigTiffFormatGROUP3MULTI:
        case BigTiffFormatGROUP4MULTI:
        case BigTiffFormatJPEGMULTI:
        case PSFormatLevel1Multi:
        case PSFormatLevel2Multi:
        case PSFormatLevel3Multi:
        case TiffFormatPIXARLOGMULTI:
            pHandler = std::make_shared<CTL_TiffIOHandler>( this, nFormat, ImageInfo );
        break;

        case DcxFormat:
            pHandler = std::make_shared<CTL_PcxIOHandler>( this, nFormat, ImageInfo );
        break;
        case PDFFormatMULTI:
            ImageInfo.nJpegQuality = 100;
            ImageInfo.bProgressiveJpeg = 0;
            ImageInfo.IsPDF = true;
            pHandler = std::make_shared<CTL_PDFIOHandler>( this, nFormat, ImageInfo);
        break;
        case TextFormatMulti:
        {
            // Get the current OCR engine's input format
            DTWAIN_ARRAY a = nullptr;
            const auto pHandle = ImageInfo.theSource->GetDTWAINHandle();
            DTWAIN_GetOCRCapValues(static_cast<DTWAIN_OCRENGINE>(pHandle->m_pOCRDefaultEngine.get()), DTWAIN_OCRCV_IMAGEFILEFORMAT,
                                    DTWAIN_CAPGETCURRENT, &a);
            DTWAINArrayLowLevel_RAII raii(pHandle, a);
            if ( a )
            {
                const auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(a);
                if ( !vValues.empty() )
                {
                    LONG InputFormat = vValues[0];
                    pHandler = std::make_shared<CTL_TextIOHandler>(this, InputFormat, ImageInfo, pHandle->m_pOCRDefaultEngine.get());
                }
            }
            else
                nStatus = pHandle->m_lLastError;
        break;
        }
    }
    if ( !pHandler )
    {
        if ( nStatus == DTWAIN_NO_ERROR)
            nStatus = DTWAIN_ERR_BADPARAM;
        return nullptr;
    }

    DibMultiPageStruct s;
    s.Stage = DIB_MULTI_FIRST;
    s.strName = szFile;
    s.pUserData = pHandler->GetMultiDibData();
    pHandler->SetMultiDibInfo(s);

    try
    {
        nStatus = pHandler->WriteBitmap( szFile, bOpenFile, fhFile, &s );
    }
    catch(...)
    {
        // An exception error occurred.
        nStatus = DTWAIN_ERR_EXCEPTION_ERROR;
    }
    if ( nStatus != 0)
    {
        if ( nStatus > 0 )
            nStatus = DTWAIN_ERR_FILEXFERSTART - nStatus;

    }
    pHandler->SetMultiDibData(s.pUserData);
    return pHandler;
}


int CTL_TwainDib::WriteNextPageDibMulti(CTL_ImageIOHandlerPtr& pImgHandler, int &nStatus,
                                        const DTWAINImageInfoEx& ImageInfo)
{
    nStatus = DTWAIN_ERR_BADPARAM;
    if (pImgHandler)
    {
        DibMultiPageStruct s2 = pImgHandler->GetMultiDibInfo();

        s2.Stage = DIB_MULTI_NEXT;
        s2.pUserData = pImgHandler->GetMultiDibData();
        pImgHandler->SetDib(this);
        pImgHandler->SetImageInfo(ImageInfo);
        try
        {
            nStatus = pImgHandler->WriteBitmap(s2.strName.c_str(), false, 0, &s2);
        }
        catch(...)
        {
            // An exception occurred
            nStatus = DTWAIN_ERR_EXCEPTION_ERROR;
        }
        if ( nStatus != 0)
        {
            // If this error is > 0, this is an ISource error, else this
            // is a DTWAIN error
            if ( nStatus > 0 )
                nStatus = DTWAIN_ERR_FILEXFERSTART - nStatus;

        }
        pImgHandler->SetMultiDibData(s2.pUserData);
    }
    return nStatus;
}


int CTL_TwainDib::WriteLastPageDibMulti(CTL_ImageIOHandlerPtr& pImgHandler, int &nStatus, bool bSaveFile/*=true*/)
{
    nStatus = DTWAIN_ERR_BADPARAM;
    if (pImgHandler)
    {
        DibMultiPageStruct s = pImgHandler->GetMultiDibInfo();
        s.Stage = DIB_MULTI_LAST;
        s.pUserData = pImgHandler->GetMultiDibData();

        try
        {
            nStatus = pImgHandler->WriteBitmap(s.strName.c_str(), false, 0, &s);
        }
        catch(...)
        {
            // An exception occurred
            nStatus = DTWAIN_ERR_EXCEPTION_ERROR;
        }
        if ( nStatus != 0)
        {
            // If this error is > 0, this is an ISource error, else this
            // is a DTWAIN error
            if ( nStatus > 0 )
                nStatus = DTWAIN_ERR_FILEXFERSTART - nStatus;

        }

        if ( !bSaveFile)
        {
            // remove the file
            delete_file(s.strName.c_str());
        }
    }
    else
    // null was passed
        nStatus = 0;
    return nStatus;
}

void CTL_TwainDib::ResolvePostscriptOptions(const DTWAINImageInfoEx& Info, int &nFormat )
{
    if ( !Info.IsPostscript )
        return;

    if ( Info.IsPostscriptMultipage )
        nFormat = TiffFormatPACKBITSMULTI;
    else
        nFormat = TiffFormatPACKBITS;
}



/****************************************************************************
 *                                                                          *
 *  FUNCTION   :  PaletteSize(void * pv)                                *
 *                                                                          *
 *  PURPOSE    :  Calculates the palette size in bytes. If the info. block  *
 *                is of the BITMAPCOREHEADER type, the number of colors is  *
 *                multiplied by 3 to give the palette size, otherwise the   *
 *                number of colors is multiplied by 4.                                                          *
 *                                                                          *
 *  RETURNS    :  Palette size in number of bytes.                          *
 *                                                                          *
 ****************************************************************************/
WORD CTL_TwainDib::PaletteSize (void  *pv)
{
    const auto lpbi = static_cast<LPBITMAPINFOHEADER>(pv);
    const WORD NumColors = static_cast<WORD>(DibNumColors(lpbi));

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
        return NumColors * sizeof(RGBTRIPLE);
    return NumColors * sizeof(RGBQUAD);
}

void CTL_TwainDib::Init()
{
    m_bAutoDelete = true;
}

CTL_TwainDib::~CTL_TwainDib()
{
    if ( m_bAutoDelete )
        m_TwainDibInfo.DeleteDib();
}

void CTL_TwainDib::Delete()
{
    m_TwainDibInfo.DeleteAllDibInfo();
}


void CTL_TwainDib::SetHandle(HANDLE hDib, bool bSetPalette/*=TRUE*/)
{
    m_TwainDibInfo.SetDib( hDib );
    m_bIsValid = true;
}

HANDLE CTL_TwainDib::GetHandle() const
{
    return m_TwainDibInfo.GetDib();
}

int CTL_TwainDib::GetDepth() const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if ( !hDib )
        return -1;
    DTWAINGlobalHandle_RAII handler(hDib);
    const auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hDib));
    const int nDepth = pbi->biBitCount;
    return nDepth;
}

int CTL_TwainDib::GetBitsPerPixel() const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (!hDib)
        return 0;
    DTWAINGlobalHandle_RAII handler(hDib);
    const auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hDib));

    if (pbi->biSize != sizeof(BITMAPINFOHEADER))
        return 0;

    return pbi->biBitCount;
}

int CTL_TwainDib::GetWidth() const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if ( !hDib )
        return -1;
    DTWAINGlobalHandle_RAII handler(hDib);
    const auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hDib));
    const int nWid = static_cast<int>(pbi->biWidth);
   return nWid;
}


int CTL_TwainDib::GetHeight() const
{
   const HANDLE hDib = m_TwainDibInfo.GetDib();
   DTWAINGlobalHandle_RAII handler(hDib);
   const auto pbi = static_cast<LPBITMAPINFOHEADER>(ImageMemoryHandler::GlobalLock(hDib));
   const int nHeight = static_cast<int>(pbi->biHeight);
   return nHeight;
}

int CTL_TwainDib::GetResolution() const
{
    return (GetWidth() * GetDepth() + 7) / 8;
}


int CTL_TwainDib::GetNumColors()  const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if ( !hDib )
        return -1;
    DTWAINGlobalHandle_RAII handler(hDib);
    void  *pv = ImageMemoryHandler::GlobalLock(hDib);
    const int nColors = DibNumColors(pv);
    return nColors;
}


int CTL_TwainDib::DibNumColors(void *pv)
{
    const auto lpbi = static_cast<LPBITMAPINFOHEADER>(pv);
    const auto lpbc = static_cast<LPBITMAPCOREHEADER>(pv);

    int nColors;

    if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
    {
        nColors = 1 << lpbc->bcBitCount;
    }
    else
    if (lpbi->biClrUsed == 0)
    {
        nColors = 1 << lpbi->biBitCount;
    }
    else
    {
        nColors = static_cast<int>(lpbi->biClrUsed);
    }
    if (nColors > 256)
        nColors = 0;
    return nColors;
}

boost::optional<DWORD> CTL_TwainDib::GetBitsOffset() const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if ( hDib )
    {
        BYTE* pDib = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hDib));
        DTWAINGlobalHandle_RAII hDibHandler(hDib);
        const auto pdib = reinterpret_cast<LPBITMAPINFO>(pDib);
        DWORD offset = sizeof(BITMAPINFOHEADER);
        offset += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);
        return offset;
    }
    return boost::none;
}

int CTL_TwainDib::CropDib(const FloatRect& ActualRect, const FloatRect& RequestedRect,
                          LONG SourceUnit, LONG DestUnit, int dpi, int flags)
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        int retval;
        const HANDLE hNewDib = CDibInterface::CropDIB(hDib,
                                                      ActualRect,
                                                      RequestedRect,
                                                      SourceUnit,
                                                      DestUnit,
                                                      dpi,
                                                      flags?true:false,
                                                      retval);
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return 1;
        }
    }
    return 0;
}

bool CTL_TwainDib::IsGrayScale() const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
        return CDibInterface::IsGrayScale(hDib, GetDepth())?true:false;
    return false;
}

bool CTL_TwainDib::IncreaseBpp(unsigned long bpp)
{
    if ( bpp == static_cast<unsigned long>(GetDepth()))
        return true;

    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        HANDLE hNewDib = CDibInterface::IncreaseBpp(hDib, bpp);
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return true;
        }
    }
    return false;
}

bool CTL_TwainDib::DecreaseBpp(unsigned long bpp)
{
    if ( bpp == static_cast<unsigned long>(GetDepth()))
        return true;

    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        HANDLE hNewDib = CDibInterface::DecreaseBpp(hDib, bpp);
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return true;
        }
    }
    return false;
}

int CTL_TwainDib::ResampleDib(const FloatRect& ResampleRect, int flags)
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        HANDLE hNewDib= nullptr;
        if ( flags & CTL_ITwainSource::RESIZE_FLAG)
            hNewDib = CDibInterface::ResampleDIB(hDib, static_cast<long>(ResampleRect.left), static_cast<long>(ResampleRect.top));
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return 1;
        }
    }
    return 0;
}


int CTL_TwainDib::ResampleDib(double xscale, double yscale)
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        HANDLE hNewDib = CDibInterface::ResampleDIB(hDib, xscale, yscale);
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return 1;
        }
    }
    return 0;
}

int CTL_TwainDib::NegateDib()
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        HANDLE hNewDib = CDibInterface::NegateDIB(hDib);
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return 1;
        }
    }
    return 0;
}

int CTL_TwainDib::NormalizeDib()
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
    {
        const HANDLE hNewDib = CDibInterface::NormalizeDib(hDib);
        if ( hNewDib )
        {
            m_TwainDibInfo.DeleteDib();
            m_TwainDibInfo.SetDib(hNewDib);
            return 1;
        }
    }
    return 0;
}

bool CTL_TwainDib::IsBlankDIB(double threshold) const
{
    const HANDLE hDib = m_TwainDibInfo.GetDib();
    if (hDib)
        return CDibInterface::IsBlankDIB(hDib, threshold) ? true : false;
    return false;
}

HANDLE CTL_TwainDib::CreateBMPBitmapFromDIB(HANDLE hDib)
{
    // if hDIB is NULL, do nothing
    if (!hDib)
        return {};

    HandleRAII raii(hDib);
    const LPBYTE pDibData = raii.getData();

    HANDLE returnHandle = nullptr;

    // attach file header if this is a DIB
    BITMAPFILEHEADER fileheader;
    memset(&fileheader, 0, sizeof(BITMAPFILEHEADER));
    fileheader.bfType = 'MB';
    const auto lpbi = reinterpret_cast<LPBITMAPINFOHEADER>(pDibData);
    const unsigned int bpp = lpbi->biBitCount;
    fileheader.bfSize = GlobalSize(hDib) + sizeof(BITMAPFILEHEADER);
    fileheader.bfReserved1 = 0;
    fileheader.bfReserved2 = 0;
    fileheader.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER)) +
        lpbi->biSize + CDibInterface::CalculateUsedPaletteEntries(bpp) * sizeof(RGBQUAD);

    // we need to attach the bitmap header info onto the data
    const size_t totalSize = ImageMemoryHandler::GlobalSize(hDib) + sizeof(BITMAPFILEHEADER);

    // Allocate for returned handle
    returnHandle = static_cast<HANDLE>(ImageMemoryHandler::GlobalAlloc(GMEM_FIXED, totalSize));
    const HandleRAII raii2(returnHandle);
    if (const LPBYTE bFullImage = raii2.getData())
    {
        char* pFileHeader = reinterpret_cast<char*>(&fileheader);
        std::copy_n(pFileHeader, sizeof(BITMAPFILEHEADER), &bFullImage[0]);
        std::copy_n(pDibData, ImageMemoryHandler::GlobalSize(hDib), &bFullImage[sizeof(BITMAPFILEHEADER)]);
    }
    return returnHandle;
}

bool CTL_TwainDib::FlipBitMap(bool /*bRGB*/)
{
    int                     pixels;
    long                    items;
    long                    i;

    const HANDLE hDib = m_TwainDibInfo.GetDib();

    if (hDib)
    {
        BYTE* pDib = static_cast<BYTE*>(ImageMemoryHandler::GlobalLock(hDib));

        DTWAINGlobalHandle_RAII hDibHandler(hDib);

        LPBITMAPINFO pdib = (LPBITMAPINFO)pDib;
        LONG Width = pdib->bmiHeader.biWidth;
        LONG Height = pdib->bmiHeader.biHeight;
        DWORD SizeImage = pdib->bmiHeader.biSizeImage;
        WORD BitCount = pdib->bmiHeader.biBitCount;

        HANDLE temp = ImageMemoryHandler::GlobalAlloc(GHND, SizeImage);
        if (temp)
        {
            HUGEPTR_CHAR tempptr = static_cast<unsigned char*>(ImageMemoryHandler::GlobalLock(temp));

            // make sure we unlock and free
            DTWAINGlobalHandleUnlockFree_RAII memHandler(temp);

            HUGEPTR_CHAR tempptrsave = tempptr;

            // calculate offset to start of the bitmap data
            DWORD offset = sizeof(BITMAPINFOHEADER);
            offset += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

            LONG Linelength = (Width * BitCount + 31) / 32 * 4;

            //Goto Last line in bitmap
            offset += Linelength * (Height-1);
            pDib = pDib + offset - Linelength;

            //For each line
            for (LONG indexH = 1; indexH < Height; indexH++)
            {
                memcpy(tempptr, pDib, Linelength);
                pDib -= Linelength;
                tempptr += Linelength;
            }

            // Copy temp over hBM
            HUGEPTR_CHAR pbuffer = reinterpret_cast<unsigned char*>(pdib);
             pbuffer += sizeof(BITMAPINFOHEADER);
            pbuffer += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

            memcpy(pbuffer, tempptrsave, SizeImage);

            //Flip RGB color table
            if ( BitCount == 4 )
            {
                pbuffer = reinterpret_cast<unsigned char*>(pdib);
                pbuffer += sizeof(BITMAPINFOHEADER);
                pbuffer += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

                pixels = static_cast<int>(pdib->bmiHeader.biWidth);
                for (items = 0; items < Height; items++)
                {
                    tempptr = pbuffer;
                    for (i=0; i<pixels; i++)
                    {
                        //Switch Red byte and Blue nibble
                        *tempptr = *tempptr << 4 | *tempptr >> 4;
                        tempptr++;
                    }
                    pbuffer += Linelength;
                }
            }
            else
            if ( BitCount > 1 && BitCount != 8) //bRGB )
            {
                pbuffer = reinterpret_cast<unsigned char*>(pdib);
                pbuffer += sizeof(BITMAPINFOHEADER);
                pbuffer += pdib->bmiHeader.biClrUsed * sizeof(RGBQUAD);

                pixels = static_cast<int>(pdib->bmiHeader.biWidth);
                for (items = 0; items < Height; items++)
                {
                    tempptr = pbuffer;
                    for (i=0; i<pixels; i++)
                    {
                        //Switch Red byte and Blue byte
                        const BYTE SaveRed = *tempptr;
                        const BYTE SaveBlue = *(tempptr + 2);

                        // Test
                        *tempptr = SaveBlue;
                        *(tempptr+2) = SaveRed;
                        //increment to next triplet
                        tempptr += 3;
                    }
                    pbuffer += Linelength;
                }
            }
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
CTL_TwainDibPtr CTL_TwainDibArray::CreateDib()
{
    return InitializeDibInfo(std::make_shared<CTL_TwainDib>());
}


CTL_TwainDibPtr CTL_TwainDibArray::CreateDib(HANDLE hDib, HWND hWnd/*=NULL*/)
{
    return InitializeDibInfo(std::make_shared<CTL_TwainDib>(hDib, hWnd));
}


CTL_TwainDibPtr CTL_TwainDibArray::CreateDib(LPCSTR lpszFileName, HWND hWnd/*=NULL*/)
{
    return InitializeDibInfo(std::make_shared<CTL_TwainDib>(lpszFileName, hWnd));
}

CTL_TwainDibPtr CTL_TwainDibArray::CreateDib( const CTL_TwainDib& rDib )
{
    return InitializeDibInfo(std::make_shared<CTL_TwainDib>(rDib));
}

CTL_TwainDibPtr CTL_TwainDibArray::InitializeDibInfo(CTL_TwainDibPtr Dib)
{
    Dib->SetAutoDelete( m_bAutoDelete );
    m_TwainDibArray.push_back(Dib);
    return m_TwainDibArray[m_TwainDibArray.size() - 1];
}

CTL_TwainDibArray::CTL_TwainDibArray(bool bAutoDelete) : m_bAutoDelete(bAutoDelete)
{}

CTL_TwainDibArray::~CTL_TwainDibArray()
{
    RemoveAllDibs();
}


bool CTL_TwainDibArray::RemoveDib( CTL_TwainDibPtr pDib )
{
    const auto it = find(m_TwainDibArray.begin(),
                         m_TwainDibArray.end(),
                         pDib);
    if ( it != m_TwainDibArray.end() )
    {
        m_TwainDibArray.erase(it);
        return true;
    }
    return false;
}

bool CTL_TwainDibArray::RemoveDib( size_t nWhere )
{
    const size_t nSize = m_TwainDibArray.size();
    if ( nWhere >= nSize )
        return false;
    m_TwainDibArray.erase(m_TwainDibArray.begin() + nWhere);
    return true;
}


bool CTL_TwainDibArray::RemoveDib( HANDLE hDib )
{
    const auto it = std::find_if(m_TwainDibArray.begin(),
        m_TwainDibArray.end(), [&](const CTL_TwainDibPtr& ptr) {return ptr->GetHandle() == hDib; });

    if (it != m_TwainDibArray.end())
    {
        m_TwainDibArray.erase(it);
        return true;
    }
    return false;
}

CTL_TwainDibPtr CTL_TwainDibArray::GetAt(size_t nPos)
{
    #ifdef NO_STL_AT_DEFINED
    return m_TwainDibArray[nPos];
    #else
    return m_TwainDibArray.at(nPos);
    #endif
}

CTL_TwainDibPtr CTL_TwainDibArray::operator[](size_t nPos)
{
    return GetAt(nPos);
}

bool CTL_TwainDibArray::IsAutoDelete() const
{
    return m_bAutoDelete;
}

void CTL_TwainDibArray::RemoveAllDibs()
{
    m_TwainDibArray.clear();
}

bool CTL_TwainDibArray::DeleteDibMemory(CTL_TwainDibPtr Dib)
{
    auto it = find(m_TwainDibArray.begin(),
                                                  m_TwainDibArray.end(),
                                                  Dib);
    if ( it != m_TwainDibArray.end() )
    {
        (*it)->Delete();
        return true;
    }
    return false;
}

bool CTL_TwainDibArray::DeleteDibMemory(size_t nWhere )
{
    m_TwainDibArray[nWhere]->Delete();
    return true;
}


bool CTL_TwainDibArray::DeleteDibMemory(HANDLE hDib )
{
    const auto it = std::find_if(m_TwainDibArray.begin(),
                            m_TwainDibArray.end(),
                                [&](const CTL_TwainDibPtr& ptr) {return ptr->GetHandle() == hDib; });

    if ( it != m_TwainDibArray.end() )
    {
        (*it)->Delete();
        return true;
    }
    return false;
}
