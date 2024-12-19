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
#ifndef CTLTWAINMANAGER_H
#define CTLTWAINMANAGER_H

#include <memory>
#include <map>
#include <string>
#include "ctlobstr.h"
#include "ctlarray.h"
#include "ctltwainsession.h"
#include "ctlenum.h"
#include "capstruc.h"
#include "errstruc.h"
#include "ctltripletbase.h"
#include "ctltr011.h"
#include "ctltr012.h"
#include "ctltr014.h"
#include "ctltr015.h"
#include <boost/dll/shared_library.hpp>
namespace dynarithmic
{
    class CTL_TwainDibArray;
    class CTL_CapabilityGetTriplet;
    class CTL_ImageXferTriplet;
    class CTL_ImageInfoTriplet;

    struct RawTwainTriplet
    {
        TW_UINT32    nDG;
        TW_UINT16    nDAT;
        TW_UINT16    nMSG;
    };

    struct SourceXferReadyOverride
    {
        uint32_t m_MaxThreshold = 0;
        uint32_t m_CurrentCount = 0;
        bool m_bSeenUIClose = false;
        bool m_bSeenXferReady = false;
    };

    using SourceToXferReadyMap = std::map<std::string, SourceXferReadyOverride>;
    using SourceToXferReadyList = std::vector<std::pair<std::string, uint32_t>>;
    using SourceFlatbedOnlyList = std::unordered_set<std::string>;
    using SourcePeekMessageList = std::unordered_set<std::string>;

    class CTL_TwainAppMgr;
    typedef std::shared_ptr<CTL_TwainAppMgr> CTL_TwainAppMgrPtr;

    static constexpr int INVALID_CONDITION_CODE = -9999;

    class CTL_TwainAppMgr
    {
        public:
            CTL_TwainAppMgr(const CTL_TwainAppMgr& ) = delete;
            CTL_TwainAppMgr& operator=(const CTL_TwainAppMgr& ) = delete;

            // Application manager functions.
            // for 16-bit DLL's, this is global and can only be one of
            // them.
            static CTL_TwainAppMgrPtr Create( CTL_TwainDLLHandle* pHandle,
                                              HINSTANCE hInstance,
                                              HINSTANCE hThisInstance,
                                              LPCTSTR lpszDLLName = nullptr);

            static CTL_TwainAppMgrPtr GetInstance() noexcept
            {
                return s_pGlobalAppMgr;
            }

            static void Destroy();
            static HINSTANCE GetAppInstance();
            static std::pair<bool, CTL_StringType> CheckTwainExistence(CTL_StringType DLLName, LPLONG pWhichSearch=nullptr);


            // Twain session management functions.  Each App utilizing
            // this DLL will get its own session.
            static CTL_ITwainSession* CreateTwainSession(CTL_TwainDLLHandle* pHandle,
                                             LPCTSTR pAppName = nullptr,
                                             HWND* hAppWnd = nullptr,
                                             TW_UINT16 nMajorNum    = 1,
                                             TW_UINT16 nMinorNum    = 0,
                                             CTL_TwainLanguageEnum nLanguage  =
                                             TwainLanguage_USAENGLISH,
                                             CTL_TwainCountryEnum nCountry   =
                                             TwainCountry_USA,
                                             LPCTSTR lpszVersion  = _T("<?>"),
                                             LPCTSTR lpszMfg      = _T("<?>"),
                                             LPCTSTR lpszFamily   = _T("<?>"),
                                             LPCTSTR lpszProduct  = _T("<?>")
                                  );

            static void DestroyTwainSession(const CTL_ITwainSession* pSession);
            static bool IsValidTwainSession(const CTL_ITwainSession* pSession);
            static bool IsValidTwainSource( const CTL_ITwainSession* pSession,
                                            const CTL_ITwainSource *pSource);
            static void UnloadSourceManager();
            static HWND* GetWindowHandlePtr( const CTL_ITwainSession* pSession );
            static bool OpenSourceManager( CTL_ITwainSession* pSession );
            static bool CloseSourceManager(CTL_ITwainSession* pSession);
            static bool IsTwainMsg(MSG *pMsg, bool bFromUserQueue=false);
            static bool IsValidConditionCode(int nCode) { return nCode != INVALID_CONDITION_CODE; }
            static unsigned int GetRegisteredMsg();
            static bool IsVersion2DSMUsed();
            static void GatherCapabilityInfo(CTL_ITwainSource* pSource);

            // Source management functions
            // Get all the sources in an array
            static void EnumSources( CTL_ITwainSession* pSession, CTL_TwainSourceSet & rArray );

            // Select a source from the TWAIN source dialog
            static const CTL_ITwainSource* SelectSourceDlg(  CTL_ITwainSession *pSession, LPCTSTR pProduct=nullptr);

            // Select a source from a source object (NULL opens default
            // source)
            static CTL_ITwainSource*  SelectSource(CTL_ITwainSession* pSession,
                                                   const CTL_ITwainSource* pSource=nullptr);

            // Select a source via Product Name
            static CTL_ITwainSource*  SelectSource( CTL_ITwainSession* pSession, LPCTSTR strSource);

            static CTL_ITwainSource*  GetDefaultSource(CTL_ITwainSession* pSession);
            static bool SetDefaultSource(CTL_ITwainSource* pSource);


            // Select a source from a source object (NULL opens default
            // source)
            static bool OpenSource( CTL_ITwainSession* pSession, const CTL_ITwainSource* pSource=nullptr);
            static bool CloseSource(CTL_ITwainSession* pSession, const CTL_ITwainSource* pSource=nullptr, bool bForce=true);
            static CTL_TwainAcquireEnum GetCompatibleFileTransferType( const CTL_ITwainSource *pSource );
            static bool IsMemFileTransferSupported(const CTL_ITwainSource *pSource);
            static TW_UINT16 GetConditionCode( CTL_ITwainSession *pSession, CTL_ITwainSource *pSource=nullptr, TW_UINT16 rc=1);

            // Get the current session in use
            static CTL_ITwainSession* GetCurrentSession();

            // Get the nth registered session (only use with 0)
            static CTL_ITwainSession* GetNthSession(int nSession);

            // Gets the transfer count for a selected source
            static int GetTransferCount( const CTL_ITwainSource *pSource );
            static int SetTransferCount( const CTL_ITwainSource *pSource, int nCount );

            // Get capabilities for selected source
            static void GetCapabilities(const CTL_ITwainSource *pSource,CTL_TwainCapArray& rArray);

            static constexpr bool IsCustomCapability(LONG nCap) { return nCap >= DTWAIN_CV_CAPCUSTOMBASE; }

            static void GetExtendedCapabilities(const CTL_ITwainSource *pSource, CTL_IntArray& rArray);

            static UINT GetCapOps(const CTL_ITwainSource *pSource, int nCap, bool bCanQuery); // Does extra checking here

            static UINT GetCapabilityOperations(const CTL_ITwainSource *pSource, // Uses the MSG_QUERYSUPPORT triplet
                                                int nCap);

            static CTL_IntArray EnumTransferMechanisms( const CTL_ITwainSource *pSource );
            static bool GetFileTransferDefaults( CTL_ITwainSource *pSource, int &nFileType);
            static int SetTransferMechanism( const CTL_ITwainSource *pSource, CTL_TwainAcquireEnum AcquireType,
                                            LONG ClipboardTransferType);

            static void SetPixelAndBitDepth(const CTL_ITwainSource *pSource);
            static bool IsSourceOpen( const CTL_ITwainSource *pSource );
            static void GetPixelTypes( const CTL_ITwainSource *pSource, CTL_IntArray & rArray );
            static CTL_TwainUnitEnum GetCurrentUnitMeasure(const CTL_ITwainSource *pSource);
            static void GetCompressionTypes( const CTL_ITwainSource *pSource, CTL_IntArray & rArray );
            static void GetUnitTypes( const CTL_ITwainSource *pSource, CTL_IntArray & rArray );
            static bool GetImageLayoutSize(const CTL_ITwainSource* pSource, CTL_RealArray& rArray, CTL_EnumGetType GetType);
            static bool SetImageLayoutSize(const CTL_ITwainSource* pSource, const CTL_RealArray& rArray, CTL_RealArray& rActual,
                                             CTL_EnumSetType SetType);

            static bool StoreImageLayout(CTL_ITwainSource* pSource);
            static bool IsFeederLoaded( const CTL_ITwainSource *pSource );
            static bool IsFeederEnabled( const CTL_ITwainSource *pSource, TW_UINT16& nValue );
            static bool SetupFeeder( const CTL_ITwainSource *pSource, int maxpages, bool bSet = true);

            // User Interface functions
            static bool ShowUserInterface(CTL_ITwainSource *pSource, bool bTest = false, bool bShowUIOnly = false);
            static bool DisableUserInterface( const CTL_ITwainSource *pSource );
            static bool GetImageInfo(CTL_ITwainSource *pSource, CTL_ImageInfoTriplet *pTrip=nullptr);
            static int  TransferImage(const CTL_ITwainSource *pSource, int nImageNum=0);
            static bool SetFeederEnableMode( CTL_ITwainSource *pSource, bool bMode=true);
            static void NotifyFeederStatus();
            static bool ShowProgressIndicator(const CTL_ITwainSource* pSource, bool bShow = true);
            static bool IsProgressIndicatorOn(const CTL_ITwainSource* pSource);
            static bool IsJobControlSupported( const CTL_ITwainSource *pSource, TW_UINT16& nValue );

            static void     SetError(int nError, const std::string& extraInfo, bool bMustReportGeneralError);
            static int      GetLastError();
            static LPSTR    GetLastErrorString(LPSTR lpszBuffer, int nSize);
            static LPSTR    GetErrorString(int nError, LPSTR lpszBuffer, int nSize);
            static void     SetAndLogError(int nError, const std::string& extraInfo, bool bMustReportGeneralError);

            static void     SetDLLInstance(HINSTANCE hDLLInstance);
            // Generic capability setting functions

            // Message sending functions
            static int  SendTwainMsgToWindow(const CTL_ITwainSession *pSession,
                                             HWND hWndWhich,
                                             WPARAM wParam,
                                             LPARAM lParam = 0L);

            static bool ProcessConditionCodeError(TW_UINT16 nError);
            static int  FindConditionCode(TW_UINT16 nCode);
            static bool IsCapabilitySupported(const CTL_ITwainSource *pSource,
                                              TW_UINT16 nCap,
                                              int nType=CTL_GetTypeGET);

            static bool IsCapabilitySupported(const CTL_ITwainSource *pSource,
                                              TW_UINT16 nCap,
                                              bool bRetest,
                                              int nType);

            static bool GetOneTwainCapValue( const CTL_ITwainSource *pSource,
                                             void *pValue,
                                             TW_UINT16 Cap,
                                             CTL_EnumGetType GetType,
                                             TW_UINT16 nDataType );

            static bool GetOneCapValue(const CTL_ITwainSource *pSource,
                                       void *pValue,
                                       TW_UINT16 Cap,
                                       TW_UINT16 nDataType );

            static TW_UINT16 ProcessReturnCodeOneValue(CTL_ITwainSource *pSource,
                                                         TW_UINT16 rc);

            static TW_UINT16 GetMemXferValues(CTL_ITwainSource *pSource, TW_SETUPMEMXFER *pXfer);
            static bool IsCapMaskOn( CTL_EnumCapability Cap, CTL_EnumGetType GetType);
            static bool IsCapMaskOn( CTL_EnumCapability Cap, CTL_EnumSetType SetType);
            static bool IsSourceCompliant( const CTL_ITwainSource *pSource,
                                           CTL_EnumTwainVersion TVersion,
                                           CTL_TwainCapArray & rArray);
            static std::string  GetCapNameFromCap( LONG Cap );
            static int          GetDataTypeFromCap( CTL_EnumCapability Cap, CTL_ITwainSource *pSource=nullptr);
            static UINT         GetContainerTypesFromCap( CTL_EnumCapability Cap, bool nType );
            static bool         GetBestContainerType(CTL_TwainDLLHandle *pHandle,
                                                     const CTL_ITwainSource* pSource,
                                                     CTL_EnumCapability nCap,
                                                     UINT &rGet,
                                                     UINT &rSet,
                                                     UINT &nDataType,
                                                     UINT lGetType,
                                                     bool* flags);
            static bool         GetBestCapDataType(const CTL_ITwainSource* pSource,
                                                     CTL_EnumCapability nCap,
                                                     UINT &nDataType);
            static LONG DoCapContainerTest(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, TW_UINT16 nCap, LONG lGetType);

            static void         GetContainerNamesFromType( int nType, StringArray &rArray );
            static void         EndTwainUI(const CTL_ITwainSession* pSession, CTL_ITwainSource* pSource);

            static int          CopyFile(CTL_StringType strIn, CTL_StringType strOut);
            static LONG         GetCapFromCapName( const char *szCapName );
            static bool         SetDefaultSource( CTL_ITwainSession *pSession,
                                                  const CTL_ITwainSource *pSource );

            static TW_UINT16 CallDSMEntryProc( TW_IDENTITY *pOrigin, TW_IDENTITY* pDest,
                                               TW_UINT32 dg, TW_UINT16 dat, TW_UINT16 msg, TW_MEMREF pMemref);

            static CTL_StringType GetTwainDirFullName(LPCTSTR szTwainDLLName,
                                                      LPLONG pWhichSearch,
                                                      bool leaveLoaded = false,
                                                      boost::dll::shared_library* pModule = nullptr);
            static CTL_StringType GetTwainDirFullNameEx(LPCTSTR szTwainDLLName,
                                                      bool leaveLoaded = false,
                                                      boost::dll::shared_library* pModule = nullptr);

            static CTL_CapStruct GetGeneralCapInfo(LONG Cap);
            static bool GetCurrentOneCapValue(const CTL_ITwainSource *pSource, void *pValue, TW_UINT16 Cap, TW_UINT16 nDataType );
            static CTL_StringType GetDSMPath();
            static CTL_StringType GetDSMVersionInfo();
            auto GetDSMModuleHandle() const { return m_hLibModule.native(); }
            static SourceToXferReadyMap& GetSourceToXferReadyMap() { return s_SourceToXferReadyMap; }
            static SourceToXferReadyList& GetSourceToXferReadyList() { return s_SourceToXferReadyList; }
            static SourceFlatbedOnlyList& GetSourceFlatbedOnlyList() { return s_SourceFlatbedOnlyList; }
            static SourcePeekMessageList& GetSourcePeekMessageList() { return s_SourcePeekMessageList; }
            const CTL_TwainTriplet* GetCurrentTriplet() const { return m_pCurrentTriplet;}

        private:
            friend class CTL_TwainTriplet;
            CTL_TwainAppMgr( CTL_TwainDLLHandle* pHandle, LPCTSTR lpszDLLName, HINSTANCE hInstance,HINSTANCE hThisInstance );

            void        SetLastTwainError( TW_UINT16 nError,int nErrorType );

            TW_UINT16 CallDSMEntryProc(const CTL_TwainTriplet & pTriplet);

            // Get the default DLL name
            static CTL_StringType GetDefaultDLLName();
            static CTL_StringType GetLatestDSMVersion();

            // Load the source manager
            bool LoadSourceManager(  LPCTSTR pszDLLName=nullptr );

            // Load the data source manager
            bool LoadDSM();

            // single Application manager
            static CTL_TwainAppMgrPtr    s_pGlobalAppMgr;

            // Array of Twain Sessions
            CTL_TwainSessionArray m_arrTwainSession;

            static bool GetMultipleIntValues( const CTL_ITwainSource *pSource,
                                              CTL_IntArray & pArray,
                                              CTL_CapabilityGetTriplet *pTrip);

            static bool GetMultipleRealValues( const CTL_ITwainSource *pSource,
                                              CTL_RealArray & pArray,
                                              CTL_CapabilityGetTriplet *pTrip);
            static bool GetOneIntValue( const CTL_ITwainSource *pSource,
                                        TW_UINT16 *pInt,
                                        CTL_CapabilityGetTriplet *pTrip);

            static TW_INT32 AllocateBufferStrip(const TW_IMAGEINFO *pImgInfo,
                                                const TW_SETUPMEMXFER *pSetupInfo,
                                                HGLOBAL *pGlobal,
                                                DWORD *pSize,
                                                DWORD SizeToUse,
                                                LONG nCompression);

            static int  NativeTransfer( CTL_ITwainSession *pSession,
                                        CTL_ITwainSource  *pSource);

            static int AudioNativeTransfer(CTL_ITwainSession *pSession,
                                            CTL_ITwainSource  *pSource);

            static int  FileTransfer( CTL_ITwainSession *pSession,
                                      CTL_ITwainSource  *pSource,
                                      CTL_TwainAcquireEnum AcquireType);

            static int  BufferTransfer( CTL_ITwainSession *pSession,
                                        CTL_ITwainSource  *pSource,
                                        bool bIsMememoryFile = false );

            static int  ClipboardTransfer( CTL_ITwainSession *pSession,
                                           CTL_ITwainSource *pSource );

            static int  StartTransfer( CTL_ITwainSession *pSession,
                                       CTL_ITwainSource * pSource,
                                       CTL_ImageXferTriplet *pTrip);

            static bool SetupMemXferDIB(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource,
                                        HGLOBAL hGlobal, const TW_IMAGEINFO* pImgInfo, TW_INT32 nSize);

            static CTL_ITwainSource* GenericSourceSelector(CTL_ITwainSession* pSession, const CTL_ITwainSource* pSource, LPCTSTR lpszSource, int nWhich);

            template<typename ArrayType, typename DataType>
            static bool GetMultipleValues( const CTL_ITwainSource *pSource,ArrayType& pArray,CTL_CapabilityGetTriplet *pTrip)
            {
                pArray.clear();
                if ( !IsSourceOpen( pSource ) )
                    return false;

                // Execute the TWAIN triplet
                const TW_UINT16 rc = pTrip->Execute();

                const CTL_ITwainSource *pTempSource = const_cast<CTL_ITwainSource*>(pSource);

                if ( rc == TWRC_FAILURE ) // Check if there is a real failure
                {
                    const auto pSession = pTempSource->GetTwainSession();
                    const TW_UINT16 ccode = GetConditionCode(pSession, nullptr);
                    ProcessConditionCodeError(ccode);
                    return false;
                }

                switch (rc)
                {
                    case TWRC_SUCCESS:
                    {
                        DataType pData;
                        const size_t nNumItems = pTrip->GetNumItems();
                        for ( size_t i = 0; i < nNumItems; i++ )
                        {
                            pTrip->GetValue( &pData, i );
                            pArray.push_back( pData );
                        }
                        return true;
                    }
                default:
                    ;
                }
                return false;
            }

            template <typename ArrayType, typename fn>
            struct MultiCapCopier
            {
                ArrayType* m_Array;
                CTL_ITwainSource *m_pSource;
                CTL_CapabilityGetTriplet* m_pTriplet;
                MultiCapCopier(CTL_ITwainSource* pSource, ArrayType* arr, CTL_CapabilityGetTriplet* pTriplet) :
                        m_Array(arr), m_pSource(pSource), m_pTriplet(pTriplet) {}

                void CopyValues()
                {
                    ArrayType pTemp;
                    GetMultipleValues<ArrayType, fn>(m_pSource, pTemp, m_pTriplet);
                    std::copy(pTemp.begin(), pTemp.end(), std::back_inserter(*m_Array));
                }
            };

            template <typename T, typename fnType>
            struct GetMultiValuesImpl
            {
            static void GetMultipleTwainCapValues(const CTL_ITwainSource *pSource,
                                                  T& pArray,
                                                  TW_UINT16 Cap,
                                                  TW_UINT16 nDataType,
                                                  CTL_EnumContainer Container=TwainContainer_ENUMERATION)
               {
                   auto pTempSource = const_cast<CTL_ITwainSource*>(pSource);
                   auto pSession = pTempSource->GetTwainSession();

                   std::unique_ptr<CTL_CapabilityGetTriplet> pGetTriplet;

                   switch(Container)
                   {
                       case TwainContainer_ARRAY:
                       {
                           pGetTriplet = std::make_unique<CTL_CapabilityGetArrayTriplet>(pSession,
                               pTempSource,
                               CTL_GetTypeGET,
                               Cap, nDataType);
                       }
                       break;

                       case TwainContainer_ENUMERATION:
                       {
                           pGetTriplet = std::make_unique<CTL_CapabilityGetEnumTriplet>( pSession,
                                               pTempSource,
                                               CTL_GetTypeGET,
                                               Cap,
                                               nDataType);
                       }
                       break;

                       case TwainContainer_RANGE:
                       {
                           pGetTriplet = std::make_unique<CTL_CapabilityGetRangeTriplet>( pSession,
                                               pTempSource,
                                               CTL_GetTypeGET,
                                               Cap,nDataType);
                       }
                       break;

                       default:
                           return;
                       }

                       MultiCapCopier<T, fnType> copier(pTempSource, &pArray, pGetTriplet.get());
                       copier.CopyValues();
                   }
            };

            static CTL_ErrorStruct GetGeneralErrorInfo(TW_UINT32 nDG, TW_UINT16 nDAT, TW_UINT16 nMSG);

            void DestroySession(const CTL_ITwainSession* pSession);
            void DestroyAllTwainSessions();
            void WriteToLogFile(int rc);
            void OpenLogFile(LPCSTR lpszFile);
            void CloseLogFile();
            CTL_TwainDLLHandle* GetDLLHandle() const { return m_pDLLHandle; }
            static bool SetDependentCaps( const CTL_ITwainSource *pSource, CTL_EnumCapability Cap );
            static void EnumNoTimeoutTriplets();
            static CTL_TwainSessionArray::iterator FindSession(const CTL_ITwainSession* pSession);

            static TW_IDENTITY s_AppId;          // Twain Identity structure
            static CTL_ITwainSession* s_pSelectedSession; // Current selected
                                                                // session
            CTL_StringType  m_strTwainDSMPath;   // Twain DLL path
            CTL_StringType  m_strTwainDSMVersionInfo; // TWAIN DLL version information
            boost::dll::shared_library m_hLibModule;         // Twain DLL module handle
            DSMENTRYPROC    m_lpDSMEntry;        // Proc entry point for DSM_ENTRY
            TW_UINT16       m_nErrorTWRC;
            TW_UINT16       m_nErrorTWCC;
            unsigned int    m_nTwainMsg;
            HINSTANCE       m_Instance;
            const CTL_TwainTriplet* m_pCurrentTriplet;
            CTL_TwainDLLHandle* m_pDLLHandle;
            static int               s_nLastError;
            static std::string        s_strLastError;
            static HINSTANCE         s_ThisInstance;
            static std::vector<RawTwainTriplet> s_NoTimeoutTriplets;
            static VOID CALLBACK TwainTimeOutProc(HWND, UINT, ULONG, DWORD);
            static SourceToXferReadyMap s_SourceToXferReadyMap;
            static SourceToXferReadyList s_SourceToXferReadyList;
            static SourceFlatbedOnlyList s_SourceFlatbedOnlyList;
            static SourcePeekMessageList s_SourcePeekMessageList;
    };

    #define DTWAIN_ERROR_CONDITION(Err, RetVal, mustReport) {               \
            CTL_TwainAppMgr::SetError(Err, "", mustReport);               \
            return(RetVal); }

    #define DTWAIN_ERROR_CONDITION_EX(Err, ExtraInfo, RetVal, mustReport) {               \
        CTL_TwainAppMgr::SetError(Err, ExtraInfo, mustReport);               \
        return(RetVal); }

    /////////////////// DLL stuff /////////////////////
}
#endif


