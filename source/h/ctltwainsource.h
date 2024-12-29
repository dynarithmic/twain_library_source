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
#ifndef CTLTWAINSOURCE_H
#define CTLTWAINSOURCE_H

#include <vector>
#include <boost/logic/tribool.hpp>
#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <array>

#include "ctlobstr.h"
#include "ctlarray.h"
#include "ctldib.h"
#include "ctlenum.h"
#include "dtwtype.h"
#include "ctldevnt.h"
#include "ctltwainsession.h"
#include "ctltwainidentity.h"
#include "dtwain_anyutils.h"

namespace dynarithmic
{
    typedef boost::container::flat_map<TW_UINT16, short int> CapToStateMap;
    typedef boost::container::flat_set<TW_UINT16> CapList;
    typedef std::vector<TW_UINT16> JobControlList;
    typedef std::vector<TW_INFO> TWINFOVector;

    class CTL_TwainDLLHandle;
    class CTL_ITwainSource;
    class CTL_TwainAppMgr;

    class CTL_ITwainSession;
    class CTL_ITwainSource;
    class CTL_TwainDibArray;
    class CTL_TwainDib;
    class CTL_TwainCapMap;
    class CTL_ImageIOHandler;
    class CTL_ExtImageInfoTriplet;
    using CTL_ITwainSessionPtr = std::unique_ptr<CTL_ITwainSession>;

    enum SourceState {SOURCE_STATE_CLOSED=3,
                      SOURCE_STATE_OPENED=4,
                      SOURCE_STATE_UIENABLED=5,
                      SOURCE_STATE_XFERREADY=6,
                      SOURCE_STATE_TRANSFERRING=7
                        };

    struct sDuplexFileData
    {
        CTL_StringType sFileName;
        CTL_StringType sRealFileName;
        uint64_t nBytes;
        bool bIsJobControlPage;
        CTL_ITwainSource* m_pSource;
        sDuplexFileData() : nBytes(0), bIsJobControlPage(false), m_pSource{} {}
    };

    typedef  std::pair<
             std::vector<sDuplexFileData>,
             std::vector<sDuplexFileData> > DuplexData;

    class CTL_ITwainSource
    {
        struct container_values
        {
            int m_dataType = 0;
            std::vector<anytype_> m_data;
        };

        typedef boost::container::flat_map<TW_UINT16, container_values> CapToValuesMap;
        CapToValuesMap m_capToValuesMap_G;
        CapToValuesMap m_capToValuesMap_GD;

    public:
        CTL_ITwainSource(const CTL_ITwainSource&) = delete;
        CTL_ITwainSource& operator = (const CTL_ITwainSource&) = delete;
        ~CTL_ITwainSource();
        enum { CROP_FLAG = 1, SCALE_FLAG = 2, RESIZE_FLAG = 4};
        static CTL_ITwainSource *Create( CTL_ITwainSession* pSession,
                                         LPCTSTR lpszProductName = nullptr);

        static void Destroy(const CTL_ITwainSource* pSource);
        operator TW_IDENTITY* () { return GetSourceIDPtr(); }
        CTL_TwainIdentity& GetTwainIdentity() { return m_SourceId;  }

        CTL_ITwainSession* GetTwainSession() const;

        bool isCapValuesCached(TW_UINT16 lCap, LONG getType) const
        {
           const CapToValuesMap* mapToUse = &m_capToValuesMap_G;
           if ( getType == DTWAIN_CAPGETDEFAULT)
                mapToUse = &m_capToValuesMap_GD;
            return mapToUse->find(lCap) != mapToUse->end();
        }
        CTL_TwainDLLHandle* GetDTWAINHandle() { return m_pDLLHandle; }
        void SetDTWAINHandle(CTL_TwainDLLHandle* pHandle) { m_pDLLHandle = pHandle; }

        DTWAIN_ARRAY getCapCachedValues(TW_UINT16 lCap, LONG getType);
        bool setCapCachedValues(DTWAIN_ARRAY array, TW_UINT16 lCap, LONG getType);

        TW_IDENTITY *GetSourceIDPtr() { return &m_SourceId.get_identity(); }

        TW_UINT32    GetId() const          { return m_SourceId.get_id(); }
        const TW_VERSION*  GetVersion() const { return &m_SourceId.get_version(); }
        TW_UINT16    GetProtocolMajor() const { return m_SourceId.get_protocol_major(); }
        TW_UINT16    GetProtocolMinor() const { return m_SourceId.get_protocol_minor(); }
        TW_UINT32    GetSupportedGroups() const { return m_SourceId.get_supported_groups(); }
        CTL_StringType GetManufacturer() const { return StringConversion::Convert_Ansi_To_Native(m_SourceId.get_manufacturer()); }
        CTL_StringType GetProductFamily() const { return StringConversion::Convert_Ansi_To_Native(m_SourceId.get_product_family()); }
        CTL_StringType GetProductName() const { return StringConversion::Convert_Ansi_To_Native(m_SourceId.get_product_name()); }
        std::string GetManufacturerA() const { return m_SourceId.get_manufacturer(); }
        std::string GetProductFamilyA() const { return m_SourceId.get_product_family(); }
        std::string GetProductNameA() const { return m_SourceId.get_product_name(); }
        std::string GetSourceInfo() const { return m_SourceId.to_json(); }

        std::wstring GetManufacturerW() const 
        {
            auto str = GetManufacturerA();
            return std::wstring(str.begin(), str.end());
        }

        std::wstring GetProductFamilyW() const 
        { 
            auto str = GetProductFamilyA();
            return std::wstring(str.begin(), str.end());
        }

        std::wstring GetProductNameW() const 
        {
            auto str = GetProductNameA();
            return std::wstring(str.begin(), str.end());
        }

        bool         IsOpened() const { return m_bIsOpened; }
        bool         IsSelected() const { return m_bIsSelected; }
        void         SetSelected(bool bSet) { m_bIsSelected = bSet; }
        void         SetUIOpen(bool bSet);
        bool         IsUIOpen() const { return m_bUIOpened; }
        void         SetUIOpenOnAcquire(bool bSet) { m_bUIOnAcquire = bSet; }
        bool         IsUIOpenOnAcquire() const { return m_bUIOnAcquire; }
        void         SetAcquireFileType(CTL_TwainFileFormatEnum FileType)
                                        { m_nFileAcquireType = FileType; }
        CTL_TwainFileFormatEnum GetAcquireFileType() const { return m_nFileAcquireType; }
        void         Clone(const CTL_ITwainSource* pSource);
        void         SetActive(bool bSet);
        bool         IsActive() const;
        HWND         GetOutputWindow() const;
        void         SetDibHandle(HANDLE hDib, size_t nWhich=0) const;
        void         SetDibHandleNoPalette(HANDLE hDib, int nWhich=0) const;

        HANDLE       GetDibHandle(int nWhich=0) const;
        CTL_TwainDibPtr GetDibObject(int nWhich=0) const;
        void         SetFeederEnableMode( bool bMode=true);
        bool         IsFeederEnabledMode() const;
        void         SetAutomaticSenseMediumEnableMode(bool bMode=true);
        bool         IsAutomaticSenseMediumEnabledMode() const;
        void         SetAutoFeedMode(bool bMode=true) { m_bAutoFeed = bMode; }
        bool         GetAutoFeedMode() const { return m_bAutoFeed; }
        bool         IsDeleteDibOnScan() const { return m_bDeleteOnScan; }
        void         SetDeleteDibOnScan(bool bSet=true) { m_bDeleteOnScan = bSet; }
        bool         SetDibAutoDelete(bool bSet=true);
        void         SetAcquireType(CTL_TwainAcquireEnum AcquireType, LPCTSTR lpszFile=nullptr);
        void         SetAcquireNum(LONG_PTR lNum) { m_lAcquireNum = lNum; }
        LONG_PTR     GetAcquireNum() const { return m_lAcquireNum; }
        long         GetMaxAcquireCount() const { return m_nAcquireCount; }
        void         SetMaxAcquireCount(int nAcquire) { m_nAcquireCount = nAcquire; }
        CTL_TwainAcquireEnum  GetAcquireType() const { return m_AcquireType; }
        CTL_StringType GetAcquireFile() const { return m_strAcquireFile; }
        void         SetAcquireFile(CTL_StringType szFile) { m_strAcquireFile = std::move(szFile); }
        long         GetAcquireFileFlags() const { return m_lFileFlags; }
        void         SetAcquireFileFlags(long lFileFlags) { m_lFileFlags = lFileFlags; }
        void         SetFileSavePageCount(long pageCount) { m_FileSavePageCount = pageCount;} 
        long         GetFileSavePageCount() const { return m_FileSavePageCount; } 

        void         SetPendingImageNum( long nImageNum )
                     { m_nImageNum = nImageNum; }
        long         GetPendingImageNum() const { return m_nImageNum; }
        void         SetPendingJobNum( int nJobNum )
                     { m_nJobNum = nJobNum; }
        int          GetPendingJobNum() const { return m_nJobNum; }
        bool         IsNewJob() const;
        void         ResetJob() { m_bJobStarted = false; }
        void         StartJob() { m_bJobStarted = true; }
        void         SetJobFileHandling(bool bSet=true) { m_bJobFileHandling = bSet; }
        bool         IsJobFileHandlingOn() const { return m_bJobFileHandling; }
        bool         CurrentJobIncludesPage() const { return
                        m_nJobControl == DTWAIN_JC_JSIS || m_nJobControl == DTWAIN_JC_JSIC ||
                        m_nJobControl == DTWAIN_JCBP_JSIS || m_nJobControl == DTWAIN_JCBP_JSIC; }

        int          GetNumDibs() const;
        bool         SetCurrentDibPage(int nPage);
        int          GetCurrentDibPage() const;
        void         Reset() const;
        CTL_TwainDibArray* GetDibArray() const { return m_DibArray.get(); }
        void         SetAcquireAttempt(bool bSet) { m_bAcquireAttempt = bSet; }
        bool         IsAcquireAttempt() const { return m_bAcquireAttempt; }
        bool         IsSourceCompliant( CTL_EnumTwainVersion TVersion, CTL_TwainCapArray& rArray ) const;
        void         RemoveAllDibs() const;
        CTL_StringType GetCurrentImageFileName(); // const;
        CTL_StringType GetImageFileName(int curFile=0) const;
        void         SetFileEnumerator(DTWAIN_ARRAY pDTWAINArray) { m_pFileEnumerator = pDTWAINArray; }
        DTWAIN_ARRAY GetFileEnumerator() const { return m_pFileEnumerator; }
        void         SetTransferDone(bool bDone) { m_bTransferDone = bDone; }
        bool         GetTransferDone() const { return m_bTransferDone; }
        void         SetCapCacheValue( LONG lCap, double dValue, bool bTurnOn );
        double       GetCapCacheValue( LONG lCap, LONG *pTurnOn ) const;
        bool         IsAcquireStarted() const { return m_bAcquireStarted; }
        void         SetAcquireStarted(bool bSet) { m_bAcquireStarted = bSet; }
        void         SetModal(bool bSet) { m_bDialogModal = bSet; }
        bool         IsModal() const    { return m_bDialogModal; }
        bool         IsAcquireAutoClose() const { return m_bAcquireAutoClose; }
        void         SetOpenAfterAcquire(bool bSet) { m_bOpenAfterAcquire = bSet; }
        bool         IsOpenAfterAcquire() const { return m_bOpenAfterAcquire; }

        // Controls whether to reopen the source if it has been closed
        bool         IsReopenAfterAcquire() const { return !m_bOpenAfterAcquire?true:false; }
        int          GetMaxAcquisitions() const { return m_nMaxAcquisitions; }
        void         SetMaxAcquisitions(int nMax) { m_nMaxAcquisitions = nMax; }
        int          GetUIMaxAcquisitions() const { return m_nUIMaxAcquisitions; }
        void         SetUIMaxAcquisitions(int nMax) { m_nUIMaxAcquisitions = nMax; }
        int          GetAcquireCount() const { return m_nNumAcquires; }
        void         SetAcquireCount(int nCount) { m_nNumAcquires = nCount; }
        void         SetSpecialTransferMode(LONG lMode) { m_nSpecialMode = lMode; }
        LONG         GetSpecialTransferMode() const { return m_nSpecialMode; }
        TW_USERINTERFACE *GetTWUserInterface() { return &m_UserInterface; }
        void         AddDibsToAcquisition(DTWAIN_ARRAY aDibs) const;
        void         ResetAcquisitionAttempts(DTWAIN_ARRAY aNewAttempts);
        DTWAIN_ARRAY   GetAcquisitionArray() const;
        CTL_DeviceEvent GetDeviceEvent() const { return m_DeviceEvent; }
        void         SetDeviceEvent( const CTL_DeviceEvent& DevEvent )
                    {  m_DeviceEvent = DevEvent; }
        void         SetUIOnly(bool bSet) { m_bShowUIOnly = bSet; }
        bool         IsUIOnly() const { return m_bShowUIOnly; }
        void         SetCompressionType(LONG nCompression) { m_nCompression = nCompression; }
        LONG         GetCompressionType() const { return m_nCompression; }
        void         SetNumCompressBytes(TW_UINT32 nCompressBytes) { m_nCompressBytes = nCompressBytes; }
        TW_UINT32    GetNumCompressBytes() const { return m_nCompressBytes; }
        void         SetState(SourceState nState) {m_nState = nState;}
        SourceState  GetState() const { return m_nState; }
        TW_UINT32    GetEOJDetectedValue() const { return m_EOJDetectedValue; }
        void         SetEOJDetectedValue(TW_UINT32 nValue) { m_EOJDetectedValue = nValue; }
        void         SetForceScanOnNoUI(bool bSet) { m_bForceScanOnNoUI = bSet; }
        bool         IsForceScanOnNoUI() const { return m_bForceScanOnNoUI; }
        void         AddCapToStateInfo(TW_UINT16 nCap, short int cStateInfo);
        bool         IsCapNegotiableInState(TW_UINT16 nCap, int nState) const;
        bool         IsCapabilityCached() const { return m_bCapCached; }
        void         SetCapCached(bool bSet) { m_bCapCached = bSet; }
        bool         IsCapabilityCached(TW_UINT16 nCap) const;
        void         SetCapCached(TW_UINT16 nCapability, bool bSet);
        int          IsCapSupportedFromCache(TW_UINT16 nCap);
        bool         RetrievedAllCaps() const { return m_bRetrievedAllCaps; }
        void         SetRetrievedAllCaps(bool bSet = true) { m_bRetrievedAllCaps = bSet; }
        bool         IsPromptPending() const { return m_bPromptPending; }
        void         SetPromptPending(bool bSet) { m_bPromptPending = bSet; }

        void         AddCapToUnsupportedList(TW_UINT16 nCap);
        void         AddCapToSupportedList(TW_UINT16 nCap);
        bool         IsCapInUnsupportedList(TW_UINT16 nCap) const;
        bool         IsCapInSupportedList(TW_UINT16 nCap) const;
        CapList&     GetCapSupportedList();
        void         SetCapSupportedList(CTL_TwainCapArray& rArray);

        template <typename Iter>
        void         SetCapSupportedList(Iter iter1, Iter iter2)
        {
            m_aSupportedCapCache.clear();
            while (iter1 != iter2)
            {
                m_aSupportedCapCache.push_back(*iter1);
                ++iter1;
            }
        }

        void         SetFastCapRetrieval(bool bSet=true) { m_bFastCapRetrieval = bSet; }
        bool         IsFastCapRetrieval() const { return m_bFastCapRetrieval; }

        void         SetJpegValues(LONG nQuality, bool bProgressive) { m_ImageInfoEx.nJpegQuality = nQuality;
                                                                       m_ImageInfoEx.bProgressiveJpeg = bProgressive; }

        void         GetJpegValues(LONG &Quality, bool &bProgressive) const { Quality = m_ImageInfoEx.nJpegQuality;
                                                                              bProgressive = m_ImageInfoEx.bProgressiveJpeg; }

        void         SetPDFValue(const CTL_StringType& nWhich, const CTL_StringType& sz);
        void         SetPDFValue(const CTL_StringType& nWhich, LONG nValue);
        void         SetPDFValue(const CTL_StringType& s, DTWAIN_FLOAT f1, DTWAIN_FLOAT f2);
        void         SetPDFValue(const CTL_StringType& nWhich, const PDFTextElementPtr& element);
        void         SetPDFPageSize(LONG nPageSize, DTWAIN_FLOAT cWidth, DTWAIN_FLOAT cHeight);
        void         SetPDFEncryption(bool bIsEncrypted,
                                      const CTL_StringType& strOwnerPassword, const CTL_StringType& strUserPassword,
                                      LONG Permissions, bool bUseStrongEncryption);

        void         SetPhotometric(LONG Setting);

        void         GetImageInfoEx(DTWAINImageInfoEx &ImageInfoEx) const;
        DTWAINImageInfoEx& GetImageInfoExRef() { return m_ImageInfoEx; }

        void         SetCurrentJobControl(TW_UINT16 JobControl=TWJC_NONE) { m_nJobControl = JobControl; }
        TW_UINT16    GetCurrentJobControl() const { return m_nJobControl; }
        bool         IsTwainJobControl() const { return m_nJobControl > TWJC_NONE && m_nJobControl <= DTWAIN_JC_JSXS; }
        CTL_ImageIOHandlerPtr& GetImageHandlerPtr() { return m_pImageHandler; }
        int          GetAcquireFailureAction() const { return m_nFailAction; }
        void         SetAcquireFailureAction(int nAction) { m_nFailAction = nAction; }
        void         SetMaxRetryAttempts(int nMax) {m_nMaxRetryAttempts = nMax; }
        int          GetMaxRetryAttempts() const { return m_nMaxRetryAttempts;  }
        void         SetCurrentRetryCount(int nCount) {m_nCurRetryCount = nCount; }
        int          GetCurrentRetryCount() const { return m_nCurRetryCount;    }
        bool         SkipImageInfoErrors() const { return m_bSkipImageInfoErrors; }
        void         SetImageInfoErrors(bool bSet=true) { m_bSkipImageInfoErrors = bSet; }
        void         SetImageInfo(const TW_IMAGEINFO* pInfo) { memcpy(&m_ImageInfo, pInfo, sizeof(TW_IMAGEINFO)); }
        void         GetImageInfo(TW_IMAGEINFO* pInfo) const { memcpy(pInfo, &m_ImageInfo, sizeof(TW_IMAGEINFO)); }
        void         SetImageLayout(const FloatRect* pInfo) { memcpy(&m_ImageLayout, pInfo, sizeof(FloatRect)); }
        void         GetImageLayout(FloatRect* pInfo) const { memcpy(pInfo, &m_ImageLayout, sizeof(FloatRect)); }
        void         SetImageLayoutValid(bool bSet=true) { m_bImageLayoutValid = bSet; }
        bool         IsImageLayoutValid() const { return m_bImageLayoutValid; }
        void         SetAlternateAcquireArea(double left, double top, double right, double bottom, LONG UnitOfMeasure, LONG flags, bool bSet=true);
        void         GetAlternateAcquireArea(FloatRect& r, LONG& UnitOfMeasure, LONG& flags) const;
        void         SetImageScale(double xscale, double yscale, bool bSet=true);
        void         GetImageScale(double& xscale, double& yscale, LONG& flags) const;
        HANDLE       GetUserStripBuffer() const { return m_hAcquireStrip; }
        void         SetUserStripBuffer(HANDLE h) { m_hAcquireStrip = h; }
        SIZE_T       GetUserStripBufSize() const { return m_nUserStripSize; }
        void         SetUserStripBufSize(SIZE_T nSize) { m_nUserStripSize = nSize; }
        bool         IsUserBufferUsed() const { return m_bUserStripUsed; }
        void         SetBufferStripData(TW_IMAGEMEMXFER* pMemXferBuffer) { m_pImageMemXfer = pMemXferBuffer; }
        TW_IMAGEMEMXFER* GetBufferStripData() const { return m_pImageMemXfer; }

        void         SetImagesStored(bool bSet=true) { m_bImagesStored = bSet; }
        bool         ImagesStored() const { return m_bImagesStored; }
        CTL_StringType GetLastAcquiredFileName() const { return m_strLastAcquiredFile; }
        void         SetLastAcquiredFileName(CTL_StringType sName) { m_strLastAcquiredFile = std::move(sName); }
        TW_FILESYSTEM*  GetFileSystem() { return &m_FileSystem; }

        // Extended image info functions
        bool         IsExtendedImageInfoSupported() const { return m_bExtendedImageInfoSupported; }
        void         SetExtendedImageInfoSupported(bool bSet) { m_bExtendedImageInfoSupported = bSet; }
        bool         InitExtImageInfo(int nNum);
        bool         GetExtImageInfo(bool bExecute);
        bool         AddExtImageInfo(TW_INFO Info) const;
        bool         EnumExtImageInfo(CTL_IntArray& r);
        TW_INFO      GetExtImageInfoItem(int nItem, int nSearch) const;
        std::pair<bool, int32_t> GetExtImageInfoData(int nWhichItem, int nSearch, int nWhichValue, LPVOID Data, LPVOID* pHandleData, size_t* pNumChars=nullptr) const;
        void         SetUserAcquisitionArray(DTWAIN_ARRAY UserArray) { m_PersistentArray = UserArray; }
        DTWAIN_ARRAY GetUserAcquisitionArray() const { return m_PersistentArray; }

        bool         DestroyExtImageInfo();
        bool         IsExtendedCapNegotiable(LONG nCap);
        bool         AddCapToExtendedCapList(LONG nCap);
        bool         ExtendedCapsRetrieved() const { return m_bExtendedCapsRetrieved; }
        void         SetExtendedCapsRetrieved(bool bSet) { m_bExtendedCapsRetrieved = bSet; }
        CapList&     GetExtendedCapCache() { return m_aExtendedCaps;  }
        void         RetrieveExtendedCaps();

        void         SetFileAutoIncrement(bool bSet, LONG nIncrement)
        {
            m_bAutoIncrementFile = bSet;
            m_nFileIncrement = nIncrement;
        }

        bool         IsFileAutoIncrement() const { return m_bAutoIncrementFile; }
        bool         InitFileAutoIncrementData(CTL_StringType sName);
        void         SetFileAutoIncrementFlags(LONG nFlags){ m_nAutoIncrementFlags = nFlags; }
        LONG         GetFileAutoIncrementFlags() const { return m_nAutoIncrementFlags; }
        bool         ResetFileAutoIncrementData();
        void         SetFileAutoIncrementBase( LONG nInitial ) {m_nFileNameBaseNum = nInitial;}
        bool         IsFileAutoCreateDirectory() const { return m_ImageInfoEx.IsCreateDirectory; }
        void         SetFileAutoCreateDirectory(bool bAutoCreate) { m_ImageInfoEx.IsCreateDirectory = bAutoCreate; }

        // Added for manual duplex mode processing
        bool         SetManualDuplexMode(LONG nFlags, bool bSet);
        bool         IsManualDuplexModeOn() const { return m_bManualDuplexModeOn; }
        LONG         GetManualDuplexModeFlags() const { return m_nManualDuplexModeFlags; }
        LONG         GetCurrentSideAcquired() const { return m_nCurrentSideAcquired; }
        void         SetCurrentSideAcquired(LONG nSide) { m_nCurrentSideAcquired = nSide; }
        void         ResetManualDuplexMode(int nWhich = -1);
        void         SetManualDuplexSideDone(int nSide) { m_bDuplexSideDone[nSide] = true; }
        bool         IsManualDuplexSideDone(int nSide) const { return m_bDuplexSideDone[nSide]; }
        bool         IsManualDuplexDone() const { return m_bDuplexSideDone[0] && m_bDuplexSideDone[1];}

        void         SetMultiPageScanMode(LONG ScanMode) { m_nMultiPageScanMode = ScanMode; }
        LONG         GetMutiPageScanMode() const { return m_nMultiPageScanMode; }
        bool         IsMultiPageModeSourceMode() const { return m_nMultiPageScanMode == DTWAIN_FILESAVE_SOURCECLOSE; }
        bool         IsMultiPageModeUIMode() const { return m_nMultiPageScanMode == DTWAIN_FILESAVE_UICLOSE; }
        bool         IsMultiPageModeDefaultMode() const { return m_nMultiPageScanMode == DTWAIN_FILESAVE_DEFAULT; }
        bool         IsMultiPageModeContinuous() const { return !IsMultiPageModeDefaultMode() &&
                                                                !IsMultiPageModeSaveAtEnd(); }
        bool         IsMultiPageModeSaveAtEnd() const { return m_nMultiPageScanMode == DTWAIN_FILESAVE_ENDACQUIRE; }
        void         SetTileMode(bool bSet) { m_bTileMode = bSet; }
        bool         IsTileModeOn() const { return m_bTileMode; }

        void         AddDuplexFileData(CTL_StringType fName, uint64_t nBytes, int nWhich,
                                       CTL_StringType RealName = {}, bool bIsJobControl=false);
        sDuplexFileData GetDuplexFileData( int nPage, int nWhich ) const;
        unsigned long GetNumDuplexFiles(int nWhich) const;
        void         RemoveDuplexFileData();
        void         DeleteDuplexFiles(int nWhich);
        void         SetImageInfoRetrieved(bool bSet) { m_bImageInfoRetrieved = bSet; }
        bool         IsImageInfoRetrieved() const {return m_bImageInfoRetrieved; }
        void         ProcessMultipageFile();
        LONG         GetForcedImageBpp() const { return m_nForcedBpp; }
        void         SetForcedImageBpp(LONG bpp) { m_nForcedBpp = bpp; }
        void         SetFileIncompleteSaveMode( bool bSaveIncomplete ) { m_bIsFileSaveIncomplete = bSaveIncomplete; }
        bool         IsFileIncompleteSave() const { return m_bIsFileSaveIncomplete; }
        bool         IsBlankPageDetectionOn() const { return m_bIsBlankPageDetectionOn &&
                            m_nJobControl < DTWAIN_JCBP_JSIC || IsBlankPageDetectionSampleOn() ||IsBlankPageDetectionNoSampleOn(); }
        void         SetBlankPageDetectionOn(bool bSet=true) { m_bIsBlankPageDetectionOn = bSet; }
        bool         IsBlankPageDetectionNoSampleOn() const { return m_bIsBlankPageDetectionNoSampleOn &&
                                                                    m_nJobControl < DTWAIN_JCBP_JSIC; }
        bool         IsBlankPageDetectionSampleOn() const { return m_bIsBlankPageDetectionSampleOn && m_nJobControl < DTWAIN_JCBP_JSIC; }
        void         SetBlankPageDetectionNoSampleOn(bool bSet = true) { m_bIsBlankPageDetectionNoSampleOn = bSet; }
        void         SetBlankPageDetectionSampleOn(bool bSet = true) { m_bIsBlankPageDetectionSampleOn = bSet; }
        double       GetBlankPageThreshold() const { return m_dBlankPageThreshold; }
        void         SetBlankPageThreshold(double threshold) { m_dBlankPageThreshold = threshold; }
        void         SetBlankPageAutoDetect(LONG bSet) { m_lBlankPageAutoDetect = bSet;}
        bool         IsBlankPageAutoDetectOn() const { return m_lBlankPageAutoDetect != DTWAIN_BP_AUTODISCARD_NONE; }
        LONG         GetBlankPageAutoDetect() const { return m_lBlankPageAutoDetect;}
        LONG         GetBlankPageCount() const { return m_nBlankPageCount; }
        void         SetBlankPageCount(LONG nCount) { m_nBlankPageCount = nCount; }
        void         SetImageNegative(bool bSet=true) { m_bImageNegative = bSet; }
        bool         IsImageNegativeOn() const { return m_bImageNegative; }
        bool         IsXferReadySent() const { return m_bXferReadySent; }
        void         SetXferReadySent(bool bSet) { m_bXferReadySent = bSet;  }
        bool         IsCurrentlyProcessingPixelInfo() const { return m_bProcessingPixelInfo; }
        void         SetCurrentlyProcessingPixelInfo(bool bSet=true) { m_bProcessingPixelInfo = bSet; }
        void         ClearPDFText();
        bool         IsTwainVersion2() const { return m_bDSMVersion2; }
        void         SetTwainVersion2(bool bSet = true) { m_bDSMVersion2 = bSet;  }
        void         SetActualFileName(CTL_StringType sName) { m_ActualFileName = std::move(sName);  }
        CTL_StringType GetActualFileName() const { return m_ActualFileName;  }
        void         SetOpenFlag(bool bOpened) { m_bIsOpened = bOpened; }
        bool         CloseSource(bool bForce);
        const std::vector<int>& GetSupportedTransferMechanisms() const { return m_aTransferMechanisms; }
        const std::vector<TW_UINT32>& GetSupportedDATS() const { return m_aSupportedDATS; }
        void         SetSupportedTransferMechanisms(const std::vector<int>& aTransferMechanisms)
                            { m_aTransferMechanisms = aTransferMechanisms; }
        void         SetSupportedDATS(const std::vector<TW_UINT32>& aSupportedDATS)
                            { m_aSupportedDATS = aSupportedDATS; }
        void         SetDoublePageCountOnDuplex(bool bSet) { m_bDoublePageCountOnDuplex = bSet; }
        bool         IsDoublePageCountOnDuplex() const { return m_bDoublePageCountOnDuplex; }
        CapList&     GetCustomCapCache() { return m_aSupportedCustomCapCache; }
        boost::logic::tribool IsFileSystemSupported() const { return m_tbIsFileSystemSupported; }
        boost::logic::tribool IsBufferedTileModeSupported() const { return m_tbIsTileModeSupported; }
        boost::logic::tribool IsFeederSupported() const { return m_tbIsFeederSupported; }
        std::pair<boost::logic::tribool, int> GetDuplexSupport() const { return m_tbIsDuplexSupported; }
        boost::logic::tribool IsAudioTransferSupported() const { return m_tbIsAudioTransferSupported; }
        boost::logic::tribool IsUIControllable() const { return m_tbUIControllable; }
        void         SetFileSystemSupported(bool bSet) { m_tbIsFileSystemSupported = bSet; }
        void         SetBufferedTileModeSupported(bool bSet) { m_tbIsTileModeSupported = bSet; }
        void         SetFeederSupported(bool bSet) { m_tbIsFeederSupported = bSet; }
        void         SetDuplexSupport(bool bSet, int duplexType) { m_tbIsDuplexSupported = { bSet,duplexType }; }
        void         SetAudioTransferSupported(bool bSet) { m_tbIsAudioTransferSupported = bSet; }
        void         SetUIControllable(bool bSet) { m_tbUIControllable = bSet; }
        TW_IMAGEMEMXFER& GetBufferedXFerInfo() { return m_BufferedXFerInfo; }
        CTL_ExtImageInfoTriplet* GetExtImageInfoTriplet();
        void        SetShutdownAcquire(bool bSet) { m_bShutdownAcquire = bSet; }
        bool        IsShutdownAcquire() const { return m_bShutdownAcquire; }
        int         GetLastAcquireError() const { return m_nLastAcquireError; }
        void        SetLastAcquireError(int err) { m_nLastAcquireError = err; }
        void        SetUsePeekMessage(bool bSet) { m_bUsePeekMessage = bSet; }
        bool        IsUsePeekMessage() const { return m_bUsePeekMessage;  }
        bool        IsTwainLoopStarted() const { return m_bTwainMsgLoopStarted; }
        void        SetTwainLoopStarted(bool bSet) { m_bTwainMsgLoopStarted = bSet; }
        // Only public member
        void *      m_pUserPtr;

    private:
        static void  AddCapToList(CapList& vList, TW_UINT16 nCap);
        static bool  IsCapInList(const CapList& vList, TW_UINT16 nCap);


    protected:
        CTL_ITwainSource( CTL_ITwainSession* pSession, LPCTSTR lpszProductName, CTL_TwainDLLHandle* pHandle );
        void SetDibHandleProc(HANDLE hDib, size_t nWhich, bool bCreatePalette) const;

    private:

        struct tagCapCacheInfo {
                double Contrast;
                double Brightness;
                double XResolution;
                double YResolution;
                double XNativeResolution;
                int    PixelFlavor;
                int    BitDepth;
                int    PixelType;
                unsigned int UseContrast:1;
                unsigned int UseBrightness:1;
                unsigned int UseXResolution:1;
                unsigned int UseYResolution:1;
                unsigned int UsePixelFlavor:1;
                unsigned int UseXNativeResolution:1;
                unsigned int UseBitDepth:1;
                unsigned int UsePixelType:1;
            } CapCacheInfo;

        bool            m_bDSMVersion2;
        bool            m_bXferReadySent;
        bool            m_bIsOpened;
        bool            m_bIsSelected;
        CTL_TwainIdentity  m_SourceId;
        CTL_ITwainSession* m_pSession;
        bool            m_bUIOpened;
        bool            m_bPromptPending;
        bool            m_bActive;
        HWND            m_hOutWnd;
        std::shared_ptr<CTL_TwainDibArray> m_DibArray;
        bool            m_bUseFeeder;
        bool            m_bUseAutomaticSenseMediumEnabledMode;
        bool            m_bDibAutoDelete;
        CTL_StringType  m_strAcquireFile;
        CTL_StringType  m_strLastAcquiredFile;
        CTL_StringType  m_ActualFileName;
        CTL_TwainAcquireEnum m_AcquireType;
        long            m_nImageNum;
        int             m_nCurDibPage;
        bool            m_bDeleteOnScan;
        bool            m_bUIOnAcquire;
        CTL_TwainFileFormatEnum m_nFileAcquireType;
        long            m_lFileFlags;
        bool            m_bAcquireAttempt;
        int             m_nAcquireCount;
        LONG_PTR        m_lAcquireNum;
        DTWAIN_ARRAY    m_pFileEnumerator;
        bool            m_bTransferDone;
        bool            m_bAcquireStarted;
        bool            m_bDialogModal;
        bool            m_bOpenAfterAcquire;
        bool            m_bAcquireAutoClose;
        int             m_nMaxAcquisitions;
        int             m_nUIMaxAcquisitions;
        int             m_nNumAcquires;
        LONG            m_nSpecialMode;
        TW_USERINTERFACE m_UserInterface;
        DTWAIN_ARRAY      m_aAcqAttempts;
        CTL_DeviceEvent m_DeviceEvent;
        bool            m_bShowUIOnly;
        LONG            m_nCompression;
        SourceState     m_nState;
        TW_UINT32       m_nCompressBytes;
        CapToStateMap   m_mapCapToState;
        bool            m_bCapCached;
        bool            m_bRetrievedAllCaps;
        bool            m_bFastCapRetrieval;
        LONG            m_nJpegQuality;
        bool            m_bJpegProgressive;
        bool            m_bAutoFeed;
        TW_UINT16       m_nJobControl;
        int             m_nFailAction;
        int             m_nMaxRetryAttempts;
        int             m_nCurRetryCount;
        CTL_ImageIOHandlerPtr m_pImageHandler;
        HANDLE          m_hAcquireStrip;
        bool            m_bUserStripUsed;
        SIZE_T          m_nUserStripSize;
        bool            m_bImagesStored;
        bool            m_bAutoIncrementFile;
        int             m_nCurFileNum;
        int             m_nFileNameBaseNum;
        int             m_nFileIncrement;
        size_t          m_nFileDigits;
        LONG            m_nAutoIncrementFlags;
        int             m_nStartFileNum;
        bool            m_bManualDuplexModeOn;
        LONG            m_nManualDuplexModeFlags;
        LONG            m_nMultiPageScanMode;
        LONG            m_nCurrentSideAcquired;
        std::array<bool, 2> m_bDuplexSideDone;
        TW_UINT32       m_EOJDetectedValue;
        bool            m_bIsFileSaveIncomplete;
        int             m_nJobNum;
        bool            m_bJobStarted;
        bool            m_bJobFileHandling;
        bool            m_bImageLayoutValid;
        bool            m_bIsBlankPageDetectionOn;
        bool            m_bIsBlankPageDetectionNoSampleOn;
        bool            m_bIsBlankPageDetectionSampleOn;
        LONG            m_lBlankPageAutoDetect;
        double          m_dBlankPageThreshold;
        LONG            m_nBlankPageCount;
        bool            m_bForceScanOnNoUI;
        bool            m_bImageNegative;
        bool            m_bProcessingPixelInfo;
        bool            m_bSkipImageInfoErrors;
        bool            m_bDoublePageCountOnDuplex;
        LONG            m_nForcedBpp;
        bool            m_bTileMode;
        std::vector<int> m_aTransferMechanisms;
        std::vector<TW_UINT32> m_aSupportedDATS;
        bool            m_bExtendedCapsRetrieved;
        bool            m_bShutdownAcquire;
        bool            m_bUsePeekMessage;
        long            m_FileSavePageCount;
        int             m_nLastAcquireError;
        bool            m_bTwainMsgLoopStarted;
        boost::logic::tribool m_tbIsFileSystemSupported;
        boost::logic::tribool m_tbIsTileModeSupported;
        boost::logic::tribool m_tbIsFeederSupported;
        std::pair<boost::logic::tribool, int> m_tbIsDuplexSupported;
        boost::logic::tribool m_tbIsAudioTransferSupported;
        boost::logic::tribool m_tbUIControllable;
        CTL_TwainDLLHandle* m_pDLLHandle;
        TW_IMAGEMEMXFER m_BufferedXFerInfo;

        struct tagCapCachInfo {
            TW_UINT16 nCap;
            bool      m_bSupported;
        };
        typedef boost::container::flat_map<TW_UINT16, bool> CachedCapMap;

        public:
            typedef boost::container::flat_map<int, std::vector<int> > CachedPixelTypeMap;
            void        AddPixelTypeAndBitDepth(int PixelType, int BitDepth);
            CachedPixelTypeMap::iterator FindPixelType(int PixelType);
            bool IsBitDepthSupported(int PixelType, int BitDepth);
            bool IsPixelTypeSupported(int PixelType);
            bool PixelTypesRetrieved() const;
            const CachedPixelTypeMap& GetPixelTypeMap() const { return m_aPixelTypeMap; }


        private:
        struct AltAcquireArea {
            FloatRect m_rect;
            FloatRect m_rectScaling;
            FloatRect m_rectResample;
            LONG flags;
            LONG UnitOfMeasure;
            AltAcquireArea() : flags(0), UnitOfMeasure(0) {}
        };

        AltAcquireArea m_AltAcquireArea;
        CachedCapMap  m_aCapCache;
        CachedPixelTypeMap m_aPixelTypeMap;
        CapList m_aUnsupportedCapCache;
        CapList m_aSupportedCapCache;
        CapList m_aSupportedCustomCapCache;
        TW_IMAGEINFO  m_ImageInfo;
        FloatRect     m_ImageLayout;
        TW_FILESYSTEM   m_FileSystem;
        DTWAINImageInfoEx m_ImageInfoEx;
        TW_IMAGEMEMXFER* m_pImageMemXfer;
        std::unique_ptr<CTL_ExtImageInfoTriplet> m_pExtImageTriplet;
        TWINFOVector m_ExtImageVector;
        DTWAIN_ARRAY    m_PersistentArray;
        CapList    m_aExtendedCaps;
        DuplexData m_DuplexFileData;
        bool    m_bImageInfoRetrieved;
        bool    m_bExtendedImageInfoSupported;
        bool    m_bSupportedCustomCapsRetrieved;
    };
}
#endif