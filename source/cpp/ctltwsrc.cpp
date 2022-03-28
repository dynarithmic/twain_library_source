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
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <string>
#include <boost/lexical_cast.hpp>
#include <utility>
#include "ctltwsrc.h"
#include "ctltr009.h"
#include "ctltwmgr.h"
#include "ctldib.h"
#include "dtwain.h"
#include "ctltmpl3.h"
#include "imagexferfilewriter.h"
#include "ctltr038.h"
#include "enumeratorfuncs.h"
#include "ctlfileutils.h"
#include "tiff.h"

using namespace dynarithmic;

//////////////////////////////////////////////////////////////////////////////
CTL_ITwainSource* CTL_ITwainSource::Create( CTL_ITwainSession* pSession,
                                            LPCTSTR lpszProduct/*=nullptr*/ )
{
    return new CTL_ITwainSource( pSession, lpszProduct );
}

void CTL_ITwainSource::Destroy(const CTL_ITwainSource* pSource)
{ delete pSource; }

void CTL_ITwainSource::SetUIOpen(bool bSet)
{ m_bUIOpened = bSet; }

void CTL_ITwainSource::SetActive(bool bSet)
{ m_bActive = bSet; }

bool CTL_ITwainSource::IsSourceCompliant( CTL_EnumTwainVersion TVersion, CTL_TwainCapArray &rArray ) const
{
    CTL_TwainCapArray Array;
    rArray.clear();
    switch ( TVersion )
    {
        case CTL_TwainVersion15:
            Array.push_back( DTWAIN_CV_CAPXFERCOUNT );
            Array.push_back( DTWAIN_CV_ICAPCOMPRESSION );
            Array.push_back( DTWAIN_CV_ICAPPIXELTYPE );
            Array.push_back( DTWAIN_CV_ICAPUNITS );
            Array.push_back( DTWAIN_CV_ICAPXFERMECH );
        break;

        case CTL_TwainVersion16:
        case CTL_TwainVersion17:
        case CTL_TwainVersion18:
            Array.push_back( DTWAIN_CV_CAPXFERCOUNT );
            Array.push_back( DTWAIN_CV_ICAPCOMPRESSION );
            Array.push_back( DTWAIN_CV_ICAPPIXELTYPE );
            Array.push_back( DTWAIN_CV_ICAPUNITS );
            Array.push_back( DTWAIN_CV_ICAPXFERMECH );
            Array.push_back( DTWAIN_CV_CAPSUPPORTEDCAPS );
            Array.push_back( DTWAIN_CV_CAPUICONTROLLABLE );
            Array.push_back( DTWAIN_CV_ICAPPLANARCHUNKY );
            Array.push_back( DTWAIN_CV_ICAPPHYSICALHEIGHT );
            Array.push_back( DTWAIN_CV_ICAPPHYSICALWIDTH );
            Array.push_back( DTWAIN_CV_ICAPBITDEPTH );
            Array.push_back( DTWAIN_CV_ICAPBITORDER );
            Array.push_back( DTWAIN_CV_ICAPXRESOLUTION );
            Array.push_back( DTWAIN_CV_ICAPYRESOLUTION );
            Array.push_back(DTWAIN_CV_ICAPPIXELFLAVOR);
        break;
        default:
            break;
    }

    bool bIsCompliant = true;
    for (unsigned short Cap : Array)
    {
        int nValue = 0;
        const int nMask = CTL_TwainAppMgr::GetCapMaskFromCap(Cap);

        if ( CTL_TwainAppMgr::IsCapMaskOn( Cap, static_cast<CTL_EnumGetType>(CTL_CapMaskGET) ) )
        {
            bIsCompliant = CTL_TwainAppMgr::IsCapabilitySupported( this, Cap, CTL_GetTypeGET );
            if ( bIsCompliant )
                nValue |= CTL_CapMaskGET;
        }

        if ( CTL_TwainAppMgr::IsCapMaskOn( Cap, static_cast<CTL_EnumGetType>(CTL_CapMaskGETCURRENT) ) )
        {
            bIsCompliant = CTL_TwainAppMgr::IsCapabilitySupported( this, Cap, CTL_GetTypeGETCURRENT );
            if ( bIsCompliant )
                nValue |= CTL_CapMaskGETCURRENT;
        }

        if ( CTL_TwainAppMgr::IsCapMaskOn( Cap, static_cast<CTL_EnumGetType>(CTL_CapMaskGETDEFAULT) ) )
        {
            bIsCompliant = CTL_TwainAppMgr::IsCapabilitySupported( this, Cap, CTL_GetTypeGETDEFAULT );
            if ( bIsCompliant )
                nValue |= CTL_CapMaskGETDEFAULT;
        }

        if ( CTL_TwainAppMgr::IsCapMaskOn( Cap, static_cast<CTL_EnumSetType>(CTL_CapMaskSET) ) )
        {
            nValue |= CTL_CapMaskSET;
        }

        if ( CTL_TwainAppMgr::IsCapMaskOn( Cap, static_cast<CTL_EnumSetType>(CTL_CapMaskRESET) ) )
        {
            nValue |= CTL_CapMaskRESET;
        }

        if ( nValue != nMask )
            rArray.push_back( Cap );
    }

    if ( !rArray.empty() )
        return false;
    return true;
}


bool CTL_ITwainSource::IsActive() const
{
    return m_bActive;
}

CTL_ITwainSource::CTL_ITwainSource(CTL_ITwainSession* pSession, LPCTSTR lpszProduct)
    :
    m_pUserPtr(nullptr),
    CapCacheInfo(),
    m_bDSMVersion2(false),
    m_bIsOpened(false),
    m_bIsSelected(false),
    m_SourceId{},
    m_pSession(pSession),
    m_bUIOpened(false),
    m_bPromptPending(false),
    m_bActive(true),
    m_hOutWnd(nullptr),
    m_DibArray(std::make_shared<CTL_TwainDibArray>(CTL_TwainDibArray())),
    m_bUseFeeder(true),
    m_bUseAutomaticSenseMediumEnabledMode(true),
    m_bDibAutoDelete(false),
    m_AcquireType(TWAINAcquireType_Native),
    m_nImageNum(0),
    m_nCurDibPage(0),
    m_bDeleteOnScan(false),
    m_bUIOnAcquire(true),
    m_nFileAcquireType(TWAINFileFormat_Invalid),
    m_lFileFlags(0L),
    m_bAcquireAttempt(false),
    m_nAcquireCount(-1),
    m_lAcquireNum(-1L),
    m_pFileEnumerator(nullptr),
    m_bTransferDone ( false ),
    m_bAcquireStarted(false),
    m_bDialogModal(true),
    m_bOpenAfterAcquire(false),
    m_bAcquireAutoClose(false),
    m_nMaxAcquisitions(DTWAIN_MAXACQUIRE),
    m_nUIMaxAcquisitions(DTWAIN_MAXACQUIRE),
    m_nNumAcquires(0),
    m_nSpecialMode(0),
    m_UserInterface{},
    m_aAcqAttempts(nullptr),
    m_bShowUIOnly(false),
    m_nCompression(DTWAIN_CP_NONE),
    m_nState(SOURCE_STATE_CLOSED),
    m_nCompressBytes(0),
    m_bCapCached(false),
    m_bRetrievedAllCaps(false),
    m_bFastCapRetrieval(true),
    m_nJpegQuality ( 0 ),
    m_bJpegProgressive ( false ),
    m_bAutoFeed(true),
    m_nJobControl ( TWJC_NONE ),
    m_nFailAction ( DTWAIN_PAGEFAIL_RETRY ),
    m_nMaxRetryAttempts ( DTWAIN_MAXRETRY_ATTEMPTS ),
    m_nCurRetryCount ( 0 ),
    m_pImageHandler(nullptr),
    m_hAcquireStrip(nullptr),
    m_bUserStripUsed ( false ),
    m_nUserStripSize ( 0 ),
    m_bImagesStored ( false ),
    m_bAutoIncrementFile ( false ),
    m_nCurFileNum ( 0 ),
    m_nFileNameBaseNum ( DTWAIN_INCREMENT_DEFAULT ),
    m_nFileIncrement ( 1 ),
    m_nFileDigits ( 0 ),
    m_nAutoIncrementFlags ( DTWAIN_INCREMENT_DYNAMIC ),
    m_nStartFileNum ( 0 ),
    m_bManualDuplexModeOn ( false ),
    m_nManualDuplexModeFlags ( DTWAIN_MANDUP_FACEDOWNBOTTOMPAGE ),
    m_nMultiPageScanMode ( DTWAIN_FILESAVE_DEFAULT ),
    m_nCurrentSideAcquired ( 0 ),
    m_bDuplexSideDone{ false,false },
    m_EOJDetectedValue ( 1 ),
    m_bIsFileSaveIncomplete ( false ),
    m_nJobNum ( 0 ),
    m_bJobStarted ( false ),
    m_bJobFileHandling ( false ),
    m_bImageLayoutValid ( false ),
    m_bIsBlankPageDetectionOn ( false ),
    m_bIsBlankPageDetectionNoSampleOn(false),
    m_bIsBlankPageDetectionSampleOn(false),
    m_lBlankPageAutoDetect ( DTWAIN_BP_AUTODISCARD_NONE ),
    m_dBlankPageThreshold ( 0.99 ),
    m_nBlankPageCount ( 0 ),
    m_bForceScanOnNoUI ( false ),
    m_bImageNegative ( false ),
    m_bProcessingPixelInfo ( false ),
    m_bSkipImageInfoErrors ( false ),
    m_nForcedBpp ( 0 ),
    m_ImageInfo (),
    m_FileSystem(),
    m_pImageMemXfer(nullptr),
    m_PersistentArray(nullptr),
    m_bImageInfoRetrieved(false)
 {
    if ( lpszProduct )
        StringWrapperA::SafeStrcpy( m_SourceId.ProductName,
                                    StringConversion::Convert_Native_To_Ansi(lpszProduct).c_str(), sizeof m_SourceId.ProductName - 1 );

    // Image information default values
    m_ImageInfoEx.nJpegQuality = 75;
    m_ImageInfoEx.bProgressiveJpeg = false;
    m_ImageInfoEx.PDFAuthor = _T("None");
    m_ImageInfoEx.PDFProducer = _T("None");
    m_ImageInfoEx.PDFTitle = _T("None");
    m_ImageInfoEx.PDFKeywords = _T("None");
    m_ImageInfoEx.PDFSubject = _T("None");
    m_ImageInfoEx.PDFOrientation = DTWAIN_PDF_PORTRAIT;
    m_ImageInfoEx.PDFPageSize = DTWAIN_FS_USLETTER;
    m_ImageInfoEx.PDFCustomSize[0] = 8.5f;
    m_ImageInfoEx.PDFCustomSize[1] = 11.0f;
    m_ImageInfoEx.PDFUseCompression = true;
    m_ImageInfoEx.PDFUseNoCompression = false;
    m_ImageInfoEx.PDFCustomScale[0] = 100.0;
    m_ImageInfoEx.PDFCustomScale[1] = 100.0;
    m_ImageInfoEx.PDFUseThumbnail = false;
    m_ImageInfoEx.PDFThumbnailScale[0] = m_ImageInfoEx.PDFThumbnailScale[1] = 0.1;
    m_ImageInfoEx.PhotoMetric = PHOTOMETRIC_MINISBLACK;
    m_ImageInfoEx.theSource = this;
    m_AltAcquireArea.UnitOfMeasure = DTWAIN_INCHES;
    char commentStr[256] = {};
    GetResourceStringA(IDS_DTWAIN_APPTITLE, commentStr, 255);
    SetPDFValue(PDFPRODUCERKEY, StringConversion::Convert_Ansi_To_Native(commentStr));
}

void CTL_ITwainSource::Reset() const
{
    RemoveAllDibs();
}

void CTL_ITwainSource::SetAlternateAcquireArea(double left, double top, double right, double bottom,
                                               LONG /*flags*/, LONG Unit, bool bSet)
{
    if ( bSet )
    {
        m_AltAcquireArea.flags |= CROP_FLAG;
        m_AltAcquireArea.m_rect.left = left;
        m_AltAcquireArea.m_rect.top = top;
        m_AltAcquireArea.m_rect.right = right;
        m_AltAcquireArea.m_rect.bottom = bottom;
        m_AltAcquireArea.UnitOfMeasure = Unit;
    }
    else
        m_AltAcquireArea.flags =
            m_AltAcquireArea.flags &~ CROP_FLAG;
}

void CTL_ITwainSource::SetImageScale(double xscale, double yscale, bool bSet)
{
    if ( bSet )
    {
        m_AltAcquireArea.m_rectScaling.left = xscale;
        m_AltAcquireArea.m_rectScaling.top  = yscale;
        m_AltAcquireArea.flags |= SCALE_FLAG;
    }
    else
        m_AltAcquireArea.flags =
            m_AltAcquireArea.flags &~ SCALE_FLAG;
}

bool CTL_ITwainSource::SetManualDuplexMode(LONG nFlags, bool bSet)
{
    m_bManualDuplexModeOn = bSet;
    m_nManualDuplexModeFlags = nFlags;
    return true;
}

void CTL_ITwainSource::GetAlternateAcquireArea(FloatRect& r, LONG& UnitOfMeasure, LONG& flags) const
{
    r.left = m_AltAcquireArea.m_rect.left;
    r.top = m_AltAcquireArea.m_rect.top;
    r.right = m_AltAcquireArea.m_rect.right;
    r.bottom = m_AltAcquireArea.m_rect.bottom;
    UnitOfMeasure = m_AltAcquireArea.UnitOfMeasure;
    flags = static_cast<LONG>(m_AltAcquireArea.flags);
}

void CTL_ITwainSource::GetImageScale(double& xscale, double& yscale, LONG& flags) const
{
    xscale = m_AltAcquireArea.m_rectScaling.left;
    yscale = m_AltAcquireArea.m_rectScaling.top;
    flags = static_cast<LONG>(m_AltAcquireArea.flags);
}

void CTL_ITwainSource::AddCapToStateInfo(TW_UINT16 nCap, short int cStateInfo)
{
    m_mapCapToState[nCap] = cStateInfo;
}

bool CTL_ITwainSource::IsCapNegotiableInState(TW_UINT16 nCap, int nState) const
{
    const auto it = m_mapCapToState.find(nCap);
    //...
    if (it != m_mapCapToState.end())
    {
        if ((*it).second & (1 << (nState - 1)))
            return true;
        return false;
    }
    return true;
}

bool CTL_ITwainSource::IsCapabilityCached(TW_UINT16 nCap) const
{
    return m_aCapCache.find(nCap) != m_aCapCache.end();
}

void CTL_ITwainSource::SetCapCached(TW_UINT16 nCapability, bool bSet)
{
    const CachedCapMap::iterator found = m_aCapCache.find(static_cast<TW_UINT16>(nCapability));
    const TW_UINT16 nVal = nCapability;
    bool bCached = false;
    if ( found != m_aCapCache.end())
        bCached = true;
    // Check if setting and value is not cached
    if (bSet && !bCached)
        m_aCapCache[nVal] = true;   // Add to cache
    else
    if (!bSet && bCached)
        m_aCapCache.erase(found); // Delete from cache
}

int CTL_ITwainSource::IsCapSupportedFromCache(TW_UINT16 nCap)
{
    const auto found = m_aCapCache.find(static_cast<TW_UINT16>(nCap));
    if ( found == m_aCapCache.end())
        return -1;
    return (*found).second;
}

void CTL_ITwainSource::AddCapToUnsupportedList(TW_UINT16 nCap)
{
    AddCapToList(m_aUnsupportedCapCache, nCap);
}

void CTL_ITwainSource::AddCapToSupportedList(TW_UINT16 nCap)
{
    AddCapToList(m_aSupportedCapCache, nCap);
}

void CTL_ITwainSource::AddCapToList(CapList& vList, TW_UINT16 nCap)
{
    vList.insert(nCap);
}

bool CTL_ITwainSource::IsCapInUnsupportedList(TW_UINT16 nCap) const
{
    return IsCapInList(m_aUnsupportedCapCache, nCap);
}

bool CTL_ITwainSource::IsCapInSupportedList(TW_UINT16 nCap) const
{
    return IsCapInList(m_aSupportedCapCache, nCap);
}

bool CTL_ITwainSource::IsCapInList(const CapList& vList, TW_UINT16 nCap)
{
    return vList.count(nCap)?true:false;
}

CapList& CTL_ITwainSource::GetCapSupportedList()
{
    return m_aSupportedCapCache;
}

void CTL_ITwainSource::SetCapSupportedList(CTL_TwainCapArray& rArray)
{
    const auto it = m_aSupportedCapCache.begin();
    std::copy(rArray.begin(), rArray.end(), std::inserter(m_aSupportedCapCache, it));
}

void CTL_ITwainSource::SetFeederEnableMode( bool bMode )
{
    m_bUseFeeder = bMode;
}

bool CTL_ITwainSource::IsFeederEnabledMode() const
{
    return m_bUseFeeder;
}

void CTL_ITwainSource::SetAutomaticSenseMediumEnableMode(bool bMode)
{
    m_bUseAutomaticSenseMediumEnabledMode = bMode;
}

bool CTL_ITwainSource::IsAutomaticSenseMediumEnabledMode() const
{
    return m_bUseAutomaticSenseMediumEnabledMode;
}

bool CTL_ITwainSource::CloseSource(bool bForce)
{
    RemoveAllDibs();
    if ( m_bIsOpened )
    {
        if ( bForce && m_bActive)
        {
            ProcessMultipageFile();
            CTL_CloseSourceTriplet CS( m_pSession, this );
            const TW_UINT16 rc = CS.Execute();
            if ( rc != TWRC_SUCCESS )
            {
                CTL_TwainAppMgr::ProcessConditionCodeError(
                    CTL_TwainAppMgr::GetConditionCode( m_pSession, nullptr, rc ));
                m_bIsOpened = false;
                return false;
            }
            m_nState = SOURCE_STATE_CLOSED;
        }
    }
    m_bIsOpened = false;
    return true;
}

void CTL_ITwainSource::RemoveAllDibs() const
{
    m_DibArray->RemoveAllDibs();
}

void CTL_ITwainSource::Clone(const CTL_ITwainSource* pSource)
{
    m_bIsOpened = pSource->m_bIsOpened;
    m_bUIOpened = pSource->m_bUIOpened;
}

void CTL_ITwainSource::SetDibHandle(HANDLE hDib, size_t nWhich/*=0*/) const
{
    SetDibHandleProc(hDib, nWhich, false);
}


void CTL_ITwainSource::SetDibHandleProc(HANDLE hDib, size_t nWhich, bool bCreatePalette) const
{
    const size_t nSize = m_DibArray->GetSize();

    if ( nWhich < nSize )
    {
        // replace DIB with this DIB
        m_DibArray->GetAt(nWhich)->SetHandle( hDib, bCreatePalette );
    }
    else
    {
        // Create a dib
        const CTL_TwainDibPtr pDib = m_DibArray->CreateDib();
        pDib->SetHandle( hDib, bCreatePalette );
        pDib->SetAutoDelete( m_bDibAutoDelete );
    }
}

void CTL_ITwainSource::SetDibHandleNoPalette(HANDLE hDib, int nWhich/*=0*/ ) const
{
    SetDibHandleProc(hDib, nWhich, false);
}


HANDLE CTL_ITwainSource::GetDibHandle(int nWhich /*=0*/) const
{
    const CTL_TwainDibPtr pDib = GetDibObject(nWhich);
    if ( pDib )
        return *pDib;
    return nullptr;
}


CTL_TwainDibPtr CTL_ITwainSource::GetDibObject(int nWhich /*=0*/) const
{
    const auto nSize = m_DibArray->GetSize();
    if ( static_cast<size_t>(nWhich) < nSize )
    {
        // replace DIB with this DIB (returns reference to existing object)
        CTL_TwainDibPtr pDib = m_DibArray->GetAt(nWhich);
        return pDib;
    }
    else
        return CTL_TwainDibPtr();
}


bool CTL_ITwainSource::SetCurrentDibPage(int nPage)
{
    const auto nDibPages = m_DibArray->GetSize();
    if ( static_cast<size_t>(nPage) >= nDibPages )
        return false;
    m_nCurDibPage = nPage;
    return true;
}


int  CTL_ITwainSource::GetCurrentDibPage() const
{
    return m_nCurDibPage;
}

// Get the current page file name for file transfers
CTL_StringType CTL_ITwainSource::GetImageFileName(int curFile) const
{
    CTL_StringType strTemp;
    const DTWAIN_ARRAY pDTWAINArray = m_pFileEnumerator;
    if ( !pDTWAINArray )
        return {};

    const int nCount = static_cast<int>(EnumeratorFunctionImpl::EnumeratorGetCount(pDTWAINArray));
    if ( nCount > 0 && curFile < nCount )
    {
        EnumeratorFunctionImpl::EnumeratorGetAt(pDTWAINArray, curFile, &strTemp);
        return strTemp;
    }
    return {};
}

bool CTL_ITwainSource::IsNewJob() const
{
    if ( GetCurrentJobControl() == TWJC_NONE )
        return false;
    if ( !m_bJobStarted )
        return true;
    return false;
}

void CTL_ITwainSource::AddPixelTypeAndBitDepth(int PixelType, int BitDepth)
{
    const auto it = FindPixelType(PixelType);
    if ( it == m_aPixelTypeMap.end())
    {
        // pixel type not found, so add it
        std::vector<int> BitDepths;
        BitDepths.push_back( BitDepth );
        m_aPixelTypeMap[PixelType] = BitDepths;
    }
    else
    {
        // pixel type found, so see if bit depth exists
        if ( !IsBitDepthSupported( PixelType, BitDepth ))
        {
            // add the bit depth
            (*it).second.push_back(BitDepth);
        }
    }
}

CTL_ITwainSource::CachedPixelTypeMap::iterator CTL_ITwainSource::FindPixelType(int PixelType)
{
    return m_aPixelTypeMap.find(PixelType);
}

bool CTL_ITwainSource::IsPixelTypeSupported(int PixelType)
{
    return FindPixelType(PixelType) != m_aPixelTypeMap.end();
}

bool CTL_ITwainSource::IsBitDepthSupported(int PixelType, int BitDepth)
{
    const auto it = FindPixelType(PixelType);
    if ( it != m_aPixelTypeMap.end())
        // search for bit depth
        return std::find((*it).second.begin(), (*it).second.end(), BitDepth) != (*it).second.end();
    return false;
}

bool CTL_ITwainSource::PixelTypesRetrieved() const
{
    return !m_aPixelTypeMap.empty();
}

CTL_StringType CTL_ITwainSource::GetCurrentImageFileName()// const
{
    // Get the current page number
    int nCurImage = GetPendingImageNum() - GetBlankPageCount();
    if ( nCurImage < 0 )
        nCurImage = 0;

    if ( GetCurrentJobControl() != TWJC_NONE &&
        IsFileTypeMultiPage(GetAcquireFileType()) &&
        IsJobFileHandlingOn())
    {
        // Get the next job number if multipage and job control
        // is being used.
        nCurImage = GetPendingJobNum();
    }

    long lFlags   = GetAcquireFileFlags();

    if ( m_bAutoIncrementFile )
    {
        CTL_StringType strTemp;
        const DTWAIN_ARRAY pDTWAINArray = m_pFileEnumerator;
        if ( !pDTWAINArray )
            return m_strAcquireFile;

        const int nCount = static_cast<int>(EnumeratorFunctionImpl::EnumeratorGetCount(pDTWAINArray));
        if ( nCount > 0 )
        {
            EnumeratorFunctionImpl::EnumeratorGetAt(pDTWAINArray, 0, &strTemp);
            m_strAcquireFile = StringWrapper::CreateFileNameFromNumber(strTemp, m_nCurFileNum, static_cast<int>(m_nFileDigits));
        }
        else
            m_strAcquireFile.clear();
        m_nCurFileNum += m_nFileIncrement;
        return m_strAcquireFile;
    }

    if ( lFlags & (DTWAIN_USENAME | DTWAIN_USELONGNAME)) //TWAINFileFlag_USELIST )
    {
        // Get array
        CTL_StringType strTemp;
        const DTWAIN_ARRAY pDTWAINArray = m_pFileEnumerator;
        if ( !pDTWAINArray )
            return m_strAcquireFile;
        bool bRet = EnumeratorFunctionImpl::EnumeratorGetAt(pDTWAINArray, nCurImage, &strTemp); //pDTWAINArray->Value(&strTemp, nCurImage, nStatus);
        if ( !bRet ) // No more names
        {
            const int nCount = EnumeratorFunctionImpl::EnumeratorGetCount(pDTWAINArray);
            bRet = EnumeratorFunctionImpl::EnumeratorGetAt(pDTWAINArray, nCount-1, &strTemp);
            if ( !bRet )
                return m_strAcquireFile;
            return StringWrapper::GetPageFileName( strTemp, nCurImage, lFlags & DTWAIN_USELONGNAME?true:false );
        }
        else
            return strTemp;
    }
    return m_strAcquireFile;
}

bool CTL_ITwainSource::IsFileTypeMultiPage(CTL_TwainFileFormatEnum FileType) // static function
{
    return FileType == TWAINFileFormat_TIFFGROUP3MULTI ||
        FileType == TWAINFileFormat_TIFFGROUP4MULTI ||
        FileType == TWAINFileFormat_TIFFNONEMULTI   ||
        FileType == TWAINFileFormat_TIFFJPEGMULTI   ||
        FileType == TWAINFileFormat_TIFFPACKBITSMULTI ||
        FileType == TWAINFileFormat_TIFFDEFLATEMULTI ||
        FileType == TWAINFileFormat_PDFMULTI        ||
        FileType == TWAINFileFormat_POSTSCRIPT1MULTI ||
        FileType == TWAINFileFormat_POSTSCRIPT2MULTI ||
        FileType == TWAINFileFormat_POSTSCRIPT3MULTI ||
        FileType == TWAINFileFormat_TIFFLZWMULTI ||
        FileType == TWAINFileFormat_TIFFPIXARLOGMULTI ||
        FileType == TWAINFileFormat_DCX           ||
        FileType == TWAINFileFormat_TEXTMULTI ||
        FileType == DTWAIN_FF_TIFFMULTI;
}

CTL_TwainFileFormatEnum CTL_ITwainSource::GetMultiPageType(CTL_TwainFileFormatEnum FileType)
{
    static const std::unordered_map<CTL_TwainFileFormatEnum, CTL_TwainFileFormatEnum> pageMap =
    {
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
        { TWAINFileFormat_TIFFPIXARLOG,    TWAINFileFormat_TIFFPIXARLOGMULTI },
        { TWAINFileFormat_PCX,             TWAINFileFormat_DCX },
        { TWAINFileFormat_TEXT,            TWAINFileFormat_TEXTMULTI }
    };

    const auto iter = pageMap.find(FileType);
    if (iter != pageMap.end())
        return iter->second;
    return FileType;
}

bool CTL_ITwainSource::IsFileTypeTIFF(CTL_TwainFileFormatEnum FileType)
{
    static const std::unordered_set<CTL_TwainFileFormatEnum> setInfo = {
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
                            TWAINFileFormat_TIFFLZW
                    };
    return setInfo.count(FileType) == 1;
}

void CTL_ITwainSource::initFileSaveMap() const
{
    #define MAKE_FILE_FORMAT_INFO(T, E) FileFormatInfo((_T(T)), sizeof(T)-1, (_T(E)))

    if ( m_saveFileMap.empty() )
    {
        m_saveFileMap[TWAINFileFormat_TIFFLZW] =
            m_saveFileMap[TWAINFileFormat_TIFFLZWMULTI] = MAKE_FILE_FORMAT_INFO("TIFF Format (LZW) (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_TIFFNONE] =
            m_saveFileMap[TWAINFileFormat_TIFFNONEMULTI] =
            m_saveFileMap[DTWAIN_FF_TIFF] = MAKE_FILE_FORMAT_INFO("TIFF Uncompressed Format (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_TIFFGROUP3] =
            m_saveFileMap[TWAINFileFormat_TIFFGROUP3MULTI] = MAKE_FILE_FORMAT_INFO("TIFF Fax Group 3 Format (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_TIFFGROUP4] =
            m_saveFileMap[TWAINFileFormat_TIFFGROUP4MULTI] = MAKE_FILE_FORMAT_INFO("TIFF Fax Group 4 Format (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_TIFFPIXARLOG] =
            m_saveFileMap[TWAINFileFormat_TIFFPIXARLOGMULTI] = MAKE_FILE_FORMAT_INFO("TIFF (Pixar-Log Compression) (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_TIFFJPEG] =
            m_saveFileMap[TWAINFileFormat_TIFFJPEGMULTI] = MAKE_FILE_FORMAT_INFO("TIFF (JPEG Compression) (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[DTWAIN_TIFFPACKBITS] =
            m_saveFileMap[DTWAIN_TIFFPACKBITSMULTI] = MAKE_FILE_FORMAT_INFO("TIFF (Macintosh RLE Compression) (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[DTWAIN_TIFFDEFLATE] =
            m_saveFileMap[DTWAIN_TIFFDEFLATEMULTI] = MAKE_FILE_FORMAT_INFO("TIFF (ZLib Deflate Compression) (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_JBIG] = MAKE_FILE_FORMAT_INFO("JBIG Format (*.jbg)\0*.jbg\0\0", ".jbg");

        m_saveFileMap[TWAINFileFormat_JPEG2000] = MAKE_FILE_FORMAT_INFO("JPEG-2000 Format (*.jp2)\0*.jp2\0\0",".jp2");

        m_saveFileMap[TWAINFileFormat_WMF] = MAKE_FILE_FORMAT_INFO("Windows MetaFile (*.wmf)\0*.wmf\0\0",".wmf");

        m_saveFileMap[TWAINFileFormat_EMF] = MAKE_FILE_FORMAT_INFO("Windows Enhanced MetaFile (*.emf)\0*.emf\0\0",".emf");

        m_saveFileMap[TWAINFileFormat_PSD] = MAKE_FILE_FORMAT_INFO("Adobe Photoshop Format (*.psd)\0*.psd\0\0",".psd");

        m_saveFileMap[DTWAIN_FF_TIFFMULTI] = MAKE_FILE_FORMAT_INFO("Multipage TIFF Format (*.tif)\0*.tif\0\0", ".tif");

        m_saveFileMap[TWAINFileFormat_BMP] =
            m_saveFileMap[DTWAIN_FF_BMP] = MAKE_FILE_FORMAT_INFO("Windows Bitmap Format (*.bmp)\0*.bmp\0\0", ".bmp");

        m_saveFileMap[TWAINFileFormat_JPEG] =
            m_saveFileMap[DTWAIN_FF_JFIF] = MAKE_FILE_FORMAT_INFO("JFIF (JPEG) Format (*.jpg)\0*.jpg\0\0",".jpg");

        m_saveFileMap[TWAINFileFormat_PDF] =
            m_saveFileMap[TWAINFileFormat_PDFMULTI] = MAKE_FILE_FORMAT_INFO("Adobe Acrobat Format (*.pdf)\0*.pdf\0\0",".pdf");

        m_saveFileMap[TWAINFileFormat_TEXT] =
            m_saveFileMap[TWAINFileFormat_TEXTMULTI] = MAKE_FILE_FORMAT_INFO("Text file (*.txt)\0*.txt\0\0",".txt");

        m_saveFileMap[TWAINFileFormat_ICO] =
            m_saveFileMap[TWAINFileFormat_ICO_VISTA] = MAKE_FILE_FORMAT_INFO("Icon file (*.ico)\0*.ico\0\0",".ico");

        m_saveFileMap[DTWAIN_FF_SPIFF] = MAKE_FILE_FORMAT_INFO("SPIFF Format (*.spf)\0*.spf\0\0",".spf");

        m_saveFileMap[DTWAIN_FF_EXIF] = MAKE_FILE_FORMAT_INFO("EXIF Format (*.exf)\0*.exf\0\0",".exf");

        m_saveFileMap[TWAINFileFormat_PCX] = MAKE_FILE_FORMAT_INFO("PCX Format (*.pcx)\0*.pcx\0\0",".pcx");

        m_saveFileMap[TWAINFileFormat_DCX] = MAKE_FILE_FORMAT_INFO("DCX Format (*.dcx)\0*.dcx\0\0", ".dcx");

        m_saveFileMap[TWAINFileFormat_WBMP] = MAKE_FILE_FORMAT_INFO("WBMP (Wireless Bitmap Format) (*.wbmp)\0*.wbmp\0\0", ".wbmp");

        m_saveFileMap[TWAINFileFormat_PNG] =
            m_saveFileMap[DTWAIN_FF_PNG] = MAKE_FILE_FORMAT_INFO("PNG Format (*.png)\0*.png\0\0",".png");

        m_saveFileMap[TWAINFileFormat_TGA] = MAKE_FILE_FORMAT_INFO("Targa (TGA) Format (*.tga)\0*.tga\0\0",".tga");

        m_saveFileMap[DTWAIN_POSTSCRIPT1] =
            m_saveFileMap[DTWAIN_POSTSCRIPT1MULTI] = MAKE_FILE_FORMAT_INFO("Postscript Level 1 Format (*.ps)\0*.ps\0\0",".ps");

        m_saveFileMap[DTWAIN_POSTSCRIPT2] =
            m_saveFileMap[DTWAIN_POSTSCRIPT2MULTI] = MAKE_FILE_FORMAT_INFO("Postscript Level 2 Format (*.ps)\0*.ps\0\0",".ps");

        m_saveFileMap[DTWAIN_POSTSCRIPT3] =
            m_saveFileMap[DTWAIN_POSTSCRIPT3MULTI] = MAKE_FILE_FORMAT_INFO("Postscript Level 3 Format (*.ps)\0*.ps\0\0",".ps");

        m_saveFileMap[TWAINFileFormat_GIF] = MAKE_FILE_FORMAT_INFO("GIF Format (*.gif)\0*.gif\0\0",".gif");

        m_saveFileMap[DTWAIN_FF_FPX] = MAKE_FILE_FORMAT_INFO("Flash Picture (*.fpx)\0*.fpx\0\0",".fpx");

        m_saveFileMap[DTWAIN_FF_PICT] = MAKE_FILE_FORMAT_INFO("Macintosh PICT format (*.pic)\0*.pic\0\0",".pic");

        m_saveFileMap[DTWAIN_FF_XBM] = MAKE_FILE_FORMAT_INFO("XBM format (*.xbm)\0*.xbm\0\0",".xbm");

        m_saveFileMap[DTWAIN_WEBP] = MAKE_FILE_FORMAT_INFO("webp format (*.webp)\0*.webp\0\0", ".webp");
        m_saveFileMap[DTWAIN_PBM] = MAKE_FILE_FORMAT_INFO("pbm format (*.pbm)\0*.pbm\0\0", ".pbm");
    }
}

bool CTL_ITwainSource::IsFileTypePostscript(CTL_TwainFileFormatEnum FileType)
{
    return FileType == TWAINFileFormat_POSTSCRIPT1 ||
        FileType == TWAINFileFormat_POSTSCRIPT1MULTI ||
        FileType == TWAINFileFormat_POSTSCRIPT2 ||
        FileType == TWAINFileFormat_POSTSCRIPT2MULTI ||
        FileType == TWAINFileFormat_POSTSCRIPT3 ||
        FileType == TWAINFileFormat_POSTSCRIPT3MULTI;
}

CTL_StringType CTL_ITwainSource::PromptForFileName() const
{
    OPENFILENAME ofn;
    OPENFILENAME *pOfn = &ofn;
    CTL_StringType szFilter;
    LPCTSTR szExt;

    initFileSaveMap();
    const auto it = m_saveFileMap.find(m_nFileAcquireType);
    if ( it != m_saveFileMap.end() )
    {
        szFilter.append(it->second.filter, it->second.len);
        szExt = it->second.extension;
    }
    else
    {
        CTL_StringStreamType strm;
        strm << m_nFileAcquireType <<_T(" format");
        szFilter = strm.str();
        szFilter.append(_T("*\0\0"), 3);
            szExt = _T(".");
    }

    #ifdef _WIN32
    TCHAR szFile[1024];
    // prompt for filename
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    if (pHandle->m_pofn)
        pOfn = pHandle->m_pofn.get();
    else
        memset(pOfn, 0, sizeof(OPENFILENAME));
    szFile[0] = _T('\0');
    pOfn->lStructSize = sizeof(OPENFILENAME);
    const auto sTitle = pHandle->m_CustomPlacement.sTitle;
    if ( pOfn == &ofn )
    {
        pOfn->hwndOwner = nullptr;
        pOfn->lpstrFilter = szFilter.data();
        pOfn->lpstrFile= szFile;
        pOfn->nMaxFile = sizeof szFile - 5;
        pOfn->Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
                    OFN_NOREADONLYRETURN | OFN_EXPLORER;
        if ( pHandle->m_CustomPlacement.nOptions != 0 )
        {
            pOfn->lpfnHook = pHandle->m_pSaveAsDlgProc;
            pOfn->Flags |= OFN_ENABLEHOOK;
            pOfn->lCustData = (LPARAM)&pHandle->m_CustomPlacement;
            if ( !StringWrapper::IsEmpty(sTitle))
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

bool CTL_ITwainSource::SetDibAutoDelete(bool bSet)
{
    m_bDibAutoDelete = bSet;
    return true;
}

void CTL_ITwainSource:: SetAcquireType(CTL_TwainAcquireEnum AcquireType, LPCTSTR lpszFile)
{
    m_AcquireType = AcquireType;
    if ( lpszFile )
        m_strAcquireFile = lpszFile;
    else
        m_strAcquireFile.clear();
}

int CTL_ITwainSource::GetNumDibs() const
{
    return static_cast<int>(m_DibArray->GetSize());
}

CTL_ITwainSource::~CTL_ITwainSource()
{
    try
    {
        ResetManualDuplexMode();
        CloseSource(true);
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

        // Remove all of the PDF text elements for this source
        if ( pHandle )
            pHandle->m_mapPDFTextElement.erase(this);

        EnumeratorFunctionImpl::EnumeratorDestroy(m_pFileEnumerator);
    }
    catch(...)
    {
        // No exceptions can escape here
    }
}

#undef GetWindow
HWND CTL_ITwainSource::GetOutputWindow() const
{
    return m_hOutWnd;
}


///////////// Specialized cap values that DTWAIN needs to keep in a cache ///////////////////
void CTL_ITwainSource::SetCapCacheValue( LONG lCap, double dValue, bool bTurnOn )
{
    switch (lCap)
    {
        case DTWAIN_CV_ICAPBRIGHTNESS:
            CapCacheInfo.Brightness     = dValue;
            CapCacheInfo.UseBrightness  = bTurnOn;
            return;

        case DTWAIN_CV_ICAPCONTRAST:
            CapCacheInfo.Contrast     = dValue;
            CapCacheInfo.UseContrast  = bTurnOn;
            return;

        case DTWAIN_CV_ICAPXRESOLUTION:
            CapCacheInfo.XResolution   = dValue;
            CapCacheInfo.UseXResolution= bTurnOn;
            return;

        case DTWAIN_CV_ICAPYRESOLUTION:
            CapCacheInfo.YResolution   = dValue;
            CapCacheInfo.UseYResolution= bTurnOn;
            return;

        case DTWAIN_CV_ICAPPIXELFLAVOR:
            CapCacheInfo.PixelFlavor   = static_cast<int>(dValue);
            CapCacheInfo.UsePixelFlavor= bTurnOn;
            return;

        case DTWAIN_CV_ICAPXNATIVERESOLUTION:
            CapCacheInfo.XNativeResolution   = dValue;
            CapCacheInfo.UseXNativeResolution= bTurnOn;
            return;

        case DTWAIN_CV_ICAPPIXELTYPE:
            CapCacheInfo.PixelType = static_cast<int>(dValue);
            CapCacheInfo.UsePixelType = bTurnOn;
            return;

        case DTWAIN_CV_ICAPBITDEPTH:
            CapCacheInfo.BitDepth = static_cast<int>(dValue);
            CapCacheInfo.UseBitDepth = bTurnOn;
            return;
    }
}

double CTL_ITwainSource::GetCapCacheValue( LONG lCap, LONG *pTurnOn ) const
{
    double dValue = 0;
    switch (lCap)
    {
        case DTWAIN_CV_ICAPBRIGHTNESS:
            dValue = CapCacheInfo.Brightness;
            *pTurnOn = CapCacheInfo.UseBrightness;
            return dValue;

        case DTWAIN_CV_ICAPCONTRAST:
            dValue = CapCacheInfo.Contrast;
            *pTurnOn = CapCacheInfo.UseContrast;
            return dValue;

        case DTWAIN_CV_ICAPXRESOLUTION:
            dValue = CapCacheInfo.XResolution;
            *pTurnOn = CapCacheInfo.UseXResolution;
            return dValue;

        case DTWAIN_CV_ICAPYRESOLUTION:
            dValue = CapCacheInfo.YResolution;
            *pTurnOn = CapCacheInfo.UseYResolution;
            return dValue;

       case DTWAIN_CV_ICAPPIXELFLAVOR:
            dValue = static_cast<double>(CapCacheInfo.PixelFlavor);
            *pTurnOn = CapCacheInfo.UsePixelFlavor;
            return dValue;

        case DTWAIN_CV_ICAPXNATIVERESOLUTION:
            dValue = CapCacheInfo.XNativeResolution;
            *pTurnOn = CapCacheInfo.UseXNativeResolution;
            return dValue;

        case DTWAIN_CV_ICAPPIXELTYPE:
            dValue = static_cast<double>(CapCacheInfo.PixelType);
            *pTurnOn = CapCacheInfo.UsePixelType;
            return dValue;

        case DTWAIN_CV_ICAPBITDEPTH:
            dValue = static_cast<double>(CapCacheInfo.BitDepth);
            *pTurnOn = CapCacheInfo.UseBitDepth;
            return dValue;
    }
    *pTurnOn = -1;
    return 0.0;
}

void CTL_ITwainSource::AddDibsToAcquisition(DTWAIN_ARRAY aDibs) const
{
   EnumeratorFunctionImpl::EnumeratorAddValue( m_aAcqAttempts, &aDibs );
   EnumeratorFunctionImpl::EnumeratorAddValue(m_PersistentArray, &aDibs);
}

void CTL_ITwainSource::ResetAcquisitionAttempts(DTWAIN_ARRAY aNewAttempts)
{
    // Remove any old acquisitions
    if ( aNewAttempts != m_aAcqAttempts)
    {
        EnumeratorFunctionImpl::EnumeratorDestroy(m_aAcqAttempts);
        m_aAcqAttempts = aNewAttempts;
    }
}

DTWAIN_ARRAY CTL_ITwainSource::GetAcquisitionArray() const
{
    return m_aAcqAttempts;
}


void CTL_ITwainSource::SetPDFValue(const CTL_StringType& nWhich, const CTL_StringType& sz)
{
    if ( nWhich == PDFAUTHORKEY)
        m_ImageInfoEx.PDFAuthor = sz;
    else
    if ( nWhich == PDFPRODUCERKEY)
        m_ImageInfoEx.PDFProducer = sz;
    else
    if ( nWhich == PDFKEYWORDSKEY)
        m_ImageInfoEx.PDFKeywords = sz;
    else
    if ( nWhich == PDFTITLEKEY)
        m_ImageInfoEx.PDFTitle = sz;
    else
    if ( nWhich == PDFSUBJECTKEY )
        m_ImageInfoEx.PDFSubject = sz;
    else
    if ( nWhich == PSTITLEKEY)
        m_ImageInfoEx.PSTitle = sz;
    else
    if ( nWhich == PDFOWNERPASSKEY)
        m_ImageInfoEx.PDFOwnerPassword = sz;
    else
    if ( nWhich == PDFUSERPASSKEY)
        m_ImageInfoEx.PDFUserPassword = sz;
    else
    if ( nWhich == PDFCREATORKEY)
        m_ImageInfoEx.PDFCreator = sz;
}


void CTL_ITwainSource::SetPDFValue(const CTL_StringType& nWhich, LONG nValue)
{
    if ( nWhich == PDFORIENTATIONKEY)
        m_ImageInfoEx.PDFOrientation = nValue;
    else
    if ( nWhich == PDFSCALINGKEY)
        m_ImageInfoEx.PDFScaleType = nValue;
    else
    if ( nWhich == PDFCOMPRESSIONKEY)
        m_ImageInfoEx.PDFUseCompression = nValue?true:false;
    else
    if ( nWhich == PDFASCIICOMPRESSKEY)
        m_ImageInfoEx.PDFUseASCIICompression = nValue?true:false;
    else
    if ( nWhich == PSTYPEKEY)
        m_ImageInfoEx.PSType = nValue;
    else
    if ( nWhich == PDFPERMISSIONSKEY)
        m_ImageInfoEx.PDFPermissions = nValue;
    else
    if ( nWhich == PDFJPEGQUALITYKEY)
        m_ImageInfoEx.nPDFJpegQuality = nValue;
    else
    if ( nWhich == PDFOCRMODE)
        m_ImageInfoEx.IsOCRUsedForPDF = nValue?true:false;
    else
    if ( nWhich == PDFPOLARITYKEY)
        m_ImageInfoEx.nPDFPolarity = nValue;
    else
    if (nWhich == PDFAESKEY )
        m_ImageInfoEx.bIsAESEncrypted = nValue?true:false;
}

void CTL_ITwainSource::SetPDFValue(const CTL_StringType& nWhich, DTWAIN_FLOAT f1, DTWAIN_FLOAT f2)
{
    if ( nWhich == PDFSCALINGKEY )
    {
        m_ImageInfoEx.PDFCustomScale[0] = f1;
        m_ImageInfoEx.PDFCustomScale[1] = f2;
    }
}

void CTL_ITwainSource::SetPDFValue(const CTL_StringType& nWhich, const PDFTextElementPtr& element)
{
    if ( nWhich == PDFTEXTELEMENTKEY )
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
        pHandle->m_mapPDFTextElement[this].push_back(element);
    }
}
void CTL_ITwainSource::SetPDFPageSize(LONG nPageSize, DTWAIN_FLOAT cWidth, DTWAIN_FLOAT cHeight)
{
    m_ImageInfoEx.PDFPageSize = nPageSize;
    m_ImageInfoEx.PDFCustomSize[0] = cWidth;
    m_ImageInfoEx.PDFCustomSize[1] = cHeight;
}

void CTL_ITwainSource::SetPDFEncryption(bool bIsEncrypted,
                                        const CTL_StringType& strUserPassword, const CTL_StringType& strOwnerPassword,
                                        LONG Permissions, bool bUseStrongEncryption)
{
    if ( bIsEncrypted )
    {
        SetPDFValue(PDFUSERPASSKEY, strUserPassword);
        SetPDFValue(PDFOWNERPASSKEY, strOwnerPassword);
        SetPDFValue(PDFPERMISSIONSKEY, Permissions);
        m_ImageInfoEx.bUseStrongEncryption = bUseStrongEncryption?true:false;
        m_ImageInfoEx.bIsPDFEncrypted = true;
    }
    else
        m_ImageInfoEx.bIsPDFEncrypted = false;
}

void CTL_ITwainSource::ClearPDFText()
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const auto it = pHandle->m_mapPDFTextElement.find(this);
    if ( it != pHandle->m_mapPDFTextElement.end())
        it->second.clear();
}

void CTL_ITwainSource::SetPhotometric(LONG Setting)
{
    if ( Setting == 0 )
        m_ImageInfoEx.PhotoMetric = PHOTOMETRIC_MINISWHITE;
    else
    if ( Setting == 1 )
        m_ImageInfoEx.PhotoMetric = PHOTOMETRIC_MINISBLACK;
}



bool CTL_ITwainSource::InitExtImageInfo(int nNum)
{
    if ( !CTL_TwainAppMgr::IsCapabilitySupported(this, DTWAIN_CV_ICAPEXTIMAGEINFO) )
        return false;

    TW_UINT16 nValue;

    if ( !CTL_TwainAppMgr::GetOneCapValue( this, &nValue, TwainCap_EXTIMAGEINFO, TWTY_BOOL ) )
        return false;

    if ( !nValue )
        return false;

    m_pExtImageTriplet.reset(new CTL_ExtImageInfoTriplet(m_pSession, this, nNum));
        return GetExtImageInfo(true);
    }


bool CTL_ITwainSource::AddExtImageInfo(const TW_INFO &Info) const
{
    if ( m_pExtImageTriplet )
    {
        m_pExtImageTriplet->AddInfo(Info);
        return true;
    }
    return false;
}

bool CTL_ITwainSource::GetExtImageInfo(bool bExecute)
{
    if ( !m_pExtImageTriplet )
       return false;

    if ( bExecute )
    {
        const TW_UINT16 rc = m_pExtImageTriplet->Execute();
        switch (rc)
        {
            case TWRC_SUCCESS:
                m_pExtImageTriplet->RetrieveInfo(m_ExtImageVector);
                return true;
        }
    }
    return false;
}

TW_INFO CTL_ITwainSource::GetExtImageInfoItem(int nItem, int nSearchType ) const
{
    TW_INFO Info {};
    Info.NumItems = static_cast<TW_UINT16>(-1);

    if ( !m_pExtImageTriplet )
        return Info;

    return m_pExtImageTriplet->GetInfo(nItem, nSearchType );
}

bool CTL_ITwainSource::DestroyExtImageInfo()
{
    m_pExtImageTriplet.reset();
    return true;
}

bool CTL_ITwainSource::GetExtImageInfoData(int nWhichItem, int /*nSearch*/, int nWhichValue, LPVOID Data, size_t* pNumChars) const
{
    if ( !m_pExtImageTriplet )
        return false;
    return m_pExtImageTriplet->GetItemData(nWhichItem, DTWAIN_BYID, nWhichValue, Data, pNumChars)?true:false;
}

bool CTL_ITwainSource::EnumExtImageInfo(CTL_IntArray& r)
{
    // Function assumes that DAT_EXTIMAGEINFO exists for the Source
    if ( CTL_ExtImageInfoTriplet::EnumSupported(this, m_pSession, r) )
        return true;
    return false;
}

///////////////////////////////////////////////
bool CTL_ITwainSource::IsExtendedCapNegotiable(LONG nCap)
{
    if (find(m_aExtendedCaps.begin(), m_aExtendedCaps.end(), nCap) !=
                m_aExtendedCaps.end())
        return true;
    return false;
}

bool CTL_ITwainSource::AddCapToExtendedCapList(LONG nCap)
{
    m_aExtendedCaps.insert(nCap);
    return  true;
}

bool CTL_ITwainSource::InitFileAutoIncrementData(CTL_StringType sName)
{
    m_nCurFileNum = StringWrapper::GetInitialFileNumber(std::move(sName), m_nFileDigits);
    m_nStartFileNum = m_nCurFileNum;
    return true;
}

bool CTL_ITwainSource::ResetFileAutoIncrementData()
{
    if ( m_nAutoIncrementFlags == DTWAIN_INCREMENT_DYNAMIC )
        m_nCurFileNum = m_nStartFileNum;
    return true;
}


void CTL_ITwainSource::AddDuplexFileData(CTL_StringType fName,
                                         unsigned long nBytes,
                                         int nWhich,
                                         CTL_StringType fRealName,
                                         bool bIsJobControl/*=false*/)
{
    sDuplexFileData filedata;
    filedata.sFileName = std::move(fName);
    filedata.nBytes = nBytes;
    filedata.sRealFileName = std::move(fRealName);
    filedata.bIsJobControlPage = bIsJobControl;

    if ( nWhich == 0 )  // add to front side
        m_DuplexFileData.first.push_back(filedata);
    else
        m_DuplexFileData.second.push_back(filedata);  // add to back side
}

sDuplexFileData CTL_ITwainSource::GetDuplexFileData( int nPage, int nWhich ) const
{
    sDuplexFileData junk;
    const std::vector<sDuplexFileData> *pData;
    if ( nWhich == 0 )
        pData = &m_DuplexFileData.first;
    else
        pData = &m_DuplexFileData.second;

    if ( nPage < static_cast<int>(pData->size()) )
        return pData->at(nPage);
    return junk;
}

void CTL_ITwainSource::RemoveDuplexFileData()
{
    m_bDuplexSideDone[0] = m_bDuplexSideDone[1] = false;
    m_DuplexFileData.first.clear();
    m_DuplexFileData.second.clear();
}

void CTL_ITwainSource::ResetManualDuplexMode(int nWhich/*=-1*/)
{
    if ( nWhich == -1 )
    {
        m_bDuplexSideDone[0] = m_bDuplexSideDone[1] = false;
        DeleteDuplexFiles(0);
        DeleteDuplexFiles(1);
        m_DuplexFileData.first.clear();
        m_DuplexFileData.second.clear();
    }
    else
    {
        m_bDuplexSideDone[nWhich] = false;
        if ( nWhich == 0 )
        {
            DeleteDuplexFiles(0);
            m_DuplexFileData.first.clear();
        }
        else
        {
            DeleteDuplexFiles(1);
            m_DuplexFileData.second.clear();
        }
    }
}

void CTL_ITwainSource::DeleteDuplexFiles(int nWhich)
{
    std::vector<sDuplexFileData> *pData;
    if ( nWhich == 0 )
        pData = &m_DuplexFileData.first;
    else
        pData = &m_DuplexFileData.second;
    for_each(pData->begin(), pData->end(), [](const sDuplexFileData& Data) {delete_file(Data.sFileName.c_str()); });
}

unsigned long CTL_ITwainSource::GetNumDuplexFiles(int nWhich) const
{
    if ( nWhich == 0 )
        return static_cast<unsigned long>(m_DuplexFileData.first.size());
    return static_cast<unsigned long>(m_DuplexFileData.second.size());
}

void CTL_ITwainSource::GetImageInfoEx(DTWAINImageInfoEx &ImageInfoEx) const
{
    ImageInfoEx = m_ImageInfoEx;
}

void CTL_ITwainSource::ProcessMultipageFile()
{
    if ( !m_DuplexFileData.first.empty() ||
         !m_DuplexFileData.second.empty() )
    {
        const ImageXferFileWriter FileWriter(nullptr, m_pSession ,this);
        FileWriter.CloseMultiPageDibFile(GetMutiPageScanMode() != DTWAIN_FILESAVE_MANUALSAVE);
    }
    ClearPDFText(); // clear the text elements
}

static bool isIntCap(LONG capType);

bool isIntCap(DTWAIN_SOURCE Source, LONG nCap)
{
    return isIntCap(DTWAIN_GetCapDataType(Source, nCap));
}

bool isIntCap(LONG capType)
{
    return  capType == TWTY_INT16 ||
        capType == TWTY_INT32 ||
        capType == TWTY_BOOL ||
        capType == TWTY_INT8 ||
        capType == TWTY_UINT8 ||
        capType == TWTY_UINT16 ||
        capType == TWTY_UINT32;
}

bool isFloatCap(LONG capType)
{
    return  capType == TWTY_FIX32;
}

template <typename T>
static DTWAIN_ARRAY PopulateArray(const std::vector<boost::any>& dataArray, CTL_ITwainSource* pSource, TW_UINT16 nCap)
{
    const DTWAIN_ARRAY theArray = DTWAIN_ArrayCreateFromCap(pSource, static_cast<LONG>(nCap), static_cast<LONG>(dataArray.size()));
    auto& vVector = EnumeratorVector<T>(theArray);
    std::transform(dataArray.begin(), dataArray.end(), vVector.begin(), [](boost::any theAny) { return boost::any_cast<T>(theAny);});
    return theArray;
}

template <typename T>
static bool PopulateCache(DTWAIN_ARRAY theArray, std::vector<boost::any>& dataArray)
{
    auto& vVector = EnumeratorVector<T>(theArray);
    std::transform(vVector.begin(), vVector.end(), std::back_inserter(dataArray), [](T value){ return value;});
    return true;
}

CTL_ITwainSession* CTL_ITwainSource::GetTwainSession() const
{ return m_pSession; }

DTWAIN_ARRAY CTL_ITwainSource::getCapCachedValues(TW_UINT16 lCap, LONG getType)
{
    // get the data type for this cap.
    const CapToValuesMap* mapToUse = &m_capToValuesMap_G;
    if (getType == DTWAIN_CAPGETDEFAULT)
        mapToUse = &m_capToValuesMap_GD;
    const auto iter = mapToUse->find(lCap);
    if (iter == mapToUse->end() )
        return nullptr;
    const container_values& cValues = (*iter).second;
    if (isIntCap(cValues.m_dataType))
        return PopulateArray<LONG>(cValues.m_data, this, lCap);
    else
    if ( isFloatCap(cValues.m_dataType))
        return PopulateArray<double>(cValues.m_data, this, lCap);
    return nullptr;
}

bool CTL_ITwainSource::setCapCachedValues(DTWAIN_ARRAY array, TW_UINT16 lCap, LONG getType)
{
    // get the data type for this cap.
    CapToValuesMap* mapToUse = &m_capToValuesMap_G;
    if (getType == DTWAIN_CAPGETDEFAULT)
        mapToUse = &m_capToValuesMap_GD;
    const auto iter = mapToUse->find(lCap);
    if (iter != mapToUse->end())
        return true;
    container_values cValues;
    cValues.m_dataType = DTWAIN_GetCapDataType(this, lCap);
    if (isIntCap(cValues.m_dataType))
    {
        const bool retVal = PopulateCache<LONG>(array, cValues.m_data);
        if (retVal)
            return mapToUse->insert({lCap, cValues}).second;
    }
    return false;
}

/*std::string CreateFileNameFromNumber(const std::string& sFileName, int num, int nDigits)
{
    StringArray rArray;
    StringWrapperA::SplitPath(sFileName, rArray);

    // Adjust the file name
    char szBuf[25];
    sprintf(szBuf, "%0*d", nDigits, num);
    std::string& sTemp = rArray[StringWrapperA::NAME_POS];
    sTemp = sTemp.substr(0, sTemp.length() - nDigits) + szBuf;
    return StringWrapperA::MakePath(rArray);
}

int GetInitialFileNumber(const std::string& sFileName, size_t &nDigits)
{
    StringArray rArray;
    StringWrapperA::SplitPath(sFileName, rArray);
    nDigits = 0;
    std::string sTemp;

    size_t nLen = rArray[StringWrapperA::NAME_POS].length();
    for ( size_t i = nLen - 1; ; --i)
    {
        if ( StringTraitsA::IsDigit(rArray[StringWrapperA::NAME_POS][i]) )
        {
            sTemp = rArray[StringWrapperA::NAME_POS][i] + sTemp;
            nDigits++;
        }
        else
            break;
        if ( i == 0 )
            break;
    }

    // now loop until we get a good cast from the string we have
    while (!sTemp.empty())
    {
        try
        {
            return boost::lexical_cast<int>(sTemp);
        }
        catch (boost::bad_lexical_cast&)
        {
            sTemp.erase(sTemp.begin());
        }
    }
    return 0;
}
*/
///////////////////////////////////////////////////////////////////////////////////////////
