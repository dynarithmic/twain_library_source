/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

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
#ifndef CTLIFACE_H
#define CTLIFACE_H

#ifdef _MSC_VER
#pragma warning( disable : 4786)
#pragma warning (disable : 4127)
#endif

#include <mutex>
#include <memory>
#include <functional>
#include <array>
#include <string_view>
#include <deque>
#include <map>
#include <boost/functional/hash.hpp>
#include "ctltripletbase.h"
#include "dtwain_raii.h"
#include "ocrinterface.h"
#include "pdffont_basic.h"
#include "ctlloadresources.h"
#include "dtwain.h"
#include "twainframe.h"
#include "../simpleini/simpleini.h"
#include "notimpl.h"
#include "sourceacquireopts.h"
#include "ctlconstexprutils.h"
#include "ctllogfunctioncall.h"
#include "capinfomap.h"
#include "ctlbimap.h"

#ifdef _WIN32
    #include "winlibraryloader_impl.inl"
#else
    #include "linuxlibraryloader_impl.inl"
#endif

#include "capstruc.h"
#include "ctltmpl4.h"
#include "ctltwainsession.h"
#include "dtwain_resource_constants.h"
#include "ctltwaindecoder.h"
#include "logmsg.h"
#include "winconst.h"
#include "ctlguiddef.h"
#include "ctlsourceselect.h"
#include "ctltwainmemoryimpl.h"
#include "ctlsourceinfo.h"
#include "ctlmapdefs.h"

namespace dynarithmic
{
    template <typename T>
    struct dtwain_library_loader : library_loader_impl
    {
        static T get_func_ptr(void* handle, const char* name)
        {
            return static_cast<T>(get(handle, name));
        }
    };

    class CTL_TwainDLLHandle;
    class CTL_ITwainSource;
    class CTL_TwainAppMgr;
    class CTL_ITwainSession;
    class CTL_TwainDibArray;
    struct SourceSelectionOptions;
    struct SourceAcquireOptions;

    #define DTWAIN_INTERNAL_NOTIFICATION   10000
    /* DTWAIN Source UI Close Modes */
    #define DTWAIN_SourceCloseModeFORCE           0
    #define DTWAIN_SourceCloseModeBYPASS          1

    #define DSM_STATE_NONE      1
    #define DSM_STATE_LOADED    2
    #define DSM_STATE_OPENED    3

    // Select source wParam's 
    #define  DTWAIN_SelectSourceFailed             (DTWAIN_INTERNAL_NOTIFICATION + 1)
    #define  DTWAIN_AcquireSourceClosed            (DTWAIN_INTERNAL_NOTIFICATION + 2)
    #define  DTWAIN_TN_ACQUIRECANCELLED_EX         (DTWAIN_INTERNAL_NOTIFICATION + 3)
    #define  DTWAIN_TN_ACQUIREDONE_EX              (DTWAIN_INTERNAL_NOTIFICATION + 4)
    #define  DTWAIN_RETRY_EX                       (DTWAIN_INTERNAL_NOTIFICATION + 5)

    // modal processing messages
    #define DTWAIN_TN_MESSAGELOOPERROR             (DTWAIN_INTERNAL_NOTIFICATION + 6)
    #define REGISTERED_DTWAIN_MSG _T("DTWAIN_NOTIFY-{37AE5C3E-34B6-472f-A0BC-74F3CB199F2B}")

    /* Transfer started */
    /* Scanner already has physically scanned a page.
        This is sent only once (when TWAIN actually does the transformation of the
        scanned image to the DIB) */
    #define  DTWAIN_TWAINAcquireStarted          (DTWAIN_INTERNAL_NOTIFICATION + 7)

    /* Sent when DTWAIN_Acquire...() functions are about to return */
    #define  DTWAIN_AcquireTerminated            (DTWAIN_INTERNAL_NOTIFICATION + 8)
    #ifdef _WIN32
    #define  TWAINDLLVERSION_1    _T(TWAINDLL_VERSION1)
    #define  TWAINDLLVERSION_2    _T(TWAINDLL_VERSION2)
    #else
    #define  TWAINDLLVERSION_1    ""
    #define  TWAINDLLVERSION_2    "/usr/local/lib/libtwaindsm.so"
    #endif

    #include "capstruc.h"
    #include "capinfomap.h"

    // Create this statically when initializing.  Initialize the second
    // value with the dynamically created CTL_CapInfoMap above
    typedef CTL_ClassValues10<CTL_StringType, /* Product Name */
                             CTL_CapInfoMapPtr, /* Array of cap info*/
                             int,       /* dummy */
                             int,        /* dummy */
                             char,
                             char,
                             char,
                             char,
                             char,
                             char> CTL_SourceCapInfo;

    // Add the statically created CTL_SourceCapInfo to this array
    typedef std::vector<CTL_SourceCapInfo> CTL_SourceCapInfoArray;

    constexpr int DTWAIN_MaxErrorSize=256;
    class CTLTwainDibArray;

    // define a vector that holds OCREngine interfaces
    typedef std::vector<OCREnginePtr> OCRInterfaceContainer;
    typedef std::unordered_map<std::string, OCREnginePtr> OCRProductNameToEngineMap;
    typedef std::shared_ptr<CTL_TwainDLLHandle> CTL_TwainDLLHandlePtr;
    typedef CTL_ITwainSource* CTL_ITwainSourcePtr;

    struct CTL_ArrayFactory;
    typedef std::shared_ptr<CTL_ArrayFactory> CTL_ArrayFactoryPtr;

    struct CTL_LoggerCallbackInfo
    {
        DTWAIN_LOGGER_PROC  m_pLoggerCallback = nullptr;
        DTWAIN_LOGGER_PROCA  m_pLoggerCallbackA = nullptr;
        DTWAIN_LOGGER_PROCW  m_pLoggerCallbackW = nullptr;
        DTWAIN_LONG64  m_pLoggerCallback_UserData = 0;
        DTWAIN_LONG64  m_pLoggerCallback_UserDataA = 0;
        DTWAIN_LONG64  m_pLoggerCallback_UserDataW = 0;
    };

    struct CTL_OnSourceOpenProperties
    {
        bool m_bCheckFeederStatusOnOpen;
        bool m_bQueryBestCapContainer;
        bool m_bQueryCapOperations;
        bool m_bSaveDefaultToINI;
    };

    class CTL_TwainDLLHandle
    {
        public:
            static constexpr int NumTwainMapValues = DTWAIN_CONSTANT_LAST;

            CTL_TwainDLLHandle();
            ~CTL_TwainDLLHandle() = default;
            static void    NotifyWindows( UINT nMsg, WPARAM wParam, LPARAM lParam );
            void    RemoveAllArrays();
            void    RemoveAllSourceCapInfo();
            void    RemoveAllSourceMaps();
            void    RemoveAllPDFTextElements();
            void    InitializeResourceRegistry();
            std::pair<CTL_ResourceRegistryMap::iterator, bool> AddResourceToRegistry(LPCSTR pLangDLL, bool bClear);
            CTL_ResourceRegistryMap& GetResourceRegistry() { return m_ResourceRegistry; }
            CTL_StringType GetVersionString() const { return  m_VersionString; }
            void        SetVersionString(CTL_StringType s) { m_VersionString = std::move(s); }
            DTWAIN_GUID& GetGUID() { return m_uuid; }
            auto& GetGUIDMap(int nWhich) { return m_arrayMapGUID[nWhich]; }
            auto& GetGUIDMap() { return m_arrayMapGUID; }

            DTWAIN_ACQUIRE          GetNewAcquireNum();
            void                    EraseAcquireNum(DTWAIN_ACQUIRE nNum);
            CTL_TwainAppMgr* m_pAppMgr;

            struct tagSessionStruct
            {
                TW_UINT16 nMajorNum;
                TW_UINT16 nMinorNum;
                TW_UINT16 nLanguage;
                TW_UINT16 nCountry;
                CTL_StringType szVersion;
                CTL_StringType szManufact;
                CTL_StringType szFamily;
                CTL_StringType szProduct;
                CTL_StringType DSMName;
                int nSessionType;
                tagSessionStruct() : nMajorNum(1),
                                     nMinorNum(0),
                                     nLanguage(TWLG_USA),
                                     nCountry(TWCY_USA),
                                     szVersion(_T("<?>")),
                                     szManufact(_T("<?>")),
                                     szFamily(_T("<?>")),
                                     szProduct(_T("<?>")),
                                     #ifdef _WIN64
                                     DSMName(TWAINDLLVERSION_2),
                                     nSessionType(DTWAIN_TWAINDSM_VERSION2)
                                     #else
                                     DSMName(TWAINDLLVERSION_1),
                                     nSessionType(DTWAIN_TWAINDSM_LEGACY)
                                     #endif
                                    {}
            };

            tagSessionStruct m_SessionStruct;
            DTWAIN_GUID m_uuid = {};
            std::array<DTWAIN_GUID_MAP, GUID_DEF_TOTAL> m_arrayMapGUID = {};
            CTL_ResourceRegistryMap m_ResourceRegistry;
            CTL_ITwainSession* m_pTwainSession;
            CTL_StringType   m_VersionString;
            CTL_StringType   m_strTWAINPath;     // path to the TWAIN Data Source Manager that is being used
            CTL_StringType   m_strTWAINPath2;   // path to the TWAIN Data Source Manager 2.x that is being used
            LONG             m_nTwainPathLocation = -1; // constant denoting where active Twain DataSource manager was found
            CTL_StringType   m_strLibraryPath;   // path to the DTWAIN Library being used
            CTL_StringType   m_sWindowsVersionInfo; // Windows version information, cached.
            CTL_StringType   m_strDefaultSource; // Current default TWAIN source
            CTL_LoggerCallbackInfo m_LoggerCallbackInfo;

            HINSTANCE           m_hInstance;
            HWND                m_hWndTwain;
            HWND                m_hNotifyWnd;
            #ifdef _WIN32
            WNDPROC             m_hOrigProc;
            HWND                m_hWndDummy;
            #endif
            int                 m_nCurrentDibPos;
            bool                m_bSessionAllocated;
            bool                m_bDummyWindowCreated;
            bool                m_bTransferDone;
            bool                m_bSourceClosed;    // Used for "WAIT" mode
            DTWAIN_CALLBACK     m_CallbackMsg;
            DTWAIN_CALLBACK     m_CallbackError;
            LONG                m_lLastError;
            LONG                m_lLastAcqError;
            LONG                m_lAcquireMode;
            bool                m_nSourceCloseMode;
            int                 m_nUIMode;
            bool                m_bNotificationsUsed;
            bool                m_bNotifyTripletsUsed;
            std::deque<int>     m_vErrorBuffer;
            unsigned int        m_nErrorBufferThreshold = 50;
            unsigned int        m_nErrorBufferReserve = 1000;
            DTWAIN_CALLBACK_PROC m_pCallbackFn;
            DTWAIN_CALLBACK_PROC64 m_pCallbackFn64;
            DTWAIN_ERROR_PROC   m_pErrorProcFn;
            DTWAIN_ERROR_PROC64 m_pErrorProcFn64;
            LONG                m_lErrorProcUserData;
            LONG64              m_lErrorProcUserData64;
            LONG                m_lCallbackData;
            LONGLONG            m_lCallbackData64;
            OCRInterfaceContainer m_OCRInterfaceArray;
            OCRProductNameToEngineMap m_OCRProdNameToEngine;
            OCREnginePtr          m_pOCRDefaultEngine;
            CTL_OnSourceOpenProperties  m_OnSourceOpenProperties = {};

            // File Save As information
            #ifdef _WIN32
            std::unique_ptr<OPENFILENAME>  m_pofn;
            LONG                m_nSaveAsFlags;
            POINT               m_SaveAsPos;
            LPOFNHOOKPROC       m_pSaveAsDlgProc;
            CustomPlacement     m_CustomPlacement;
            #endif
            CTL_TEXTELEMENTMAP m_mapPDFTextElement;
            bool                m_bUseProxy;
            CTL_SourceCapInfoArray   m_aSourceCapInfo;
            CTL_StringToSourcePtrMap       m_mapStringToSource;
            std::vector<LONG_PTR>           m_aAcquireNum;
            short int                       m_nDSMState = DSM_STATE_NONE;
            int                             m_nDSMVersion = DTWAIN_TWAINDSM_LEGACY;
            bool                            m_bOpenSourceOnSelect;
            CTL_StringType                  m_sTempFilePath;
            CTL_ArrayFactoryPtr             m_ArrayFactory;
            CTL_LongToVectorLongMap         m_mapDTWAINArrayToTwainType;
            CTL_TwainMemoryFunctions* m_TwainMemoryFunc = nullptr;
            CTL_LegacyTwainMemoryFunctions  m_TwainLegacyFunc;
            CTL_Twain2MemoryFunctions       m_Twain2Func;
            DTWAIN_DIBUPDATE_PROC           m_pDibUpdateProc = nullptr;
            std::unordered_set<DTWAIN_SOURCE> m_aFeederSources;
            int                             m_TwainDSMSearchOrder = DTWAIN_TWAINDSMSEARCH_WSO;
            CTL_StringType                  m_TwainDSMSearchOrderStr = _T("CWSOU");
            CTL_StringType                  m_TwainDSMUserDirectory;
            CTL_StringType                  m_strSessionDetails;
            CTL_StringType                  m_strSourceDetails;
            const CTL_ITwainSession* GetTwainSession() const { return m_pTwainSession; }
    };

    template <typename T>
    T IsDLLHandleValid(CTL_TwainDLLHandle *pHandle, T bCheckSession = T(1))
    {
        // See if DLL Handle exists
        if (!pHandle)
            return {};
        // Check handles registered to the thread id's
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        if (std::find_if(threadMap.begin(), threadMap.end(), [&](auto& pr) { return pr.second.get() == pHandle; }) ==
            threadMap.end())
            return {};
        if (!pHandle->m_bSessionAllocated && bCheckSession)
            return {};
        return {1};
    }

    struct SysInitializeOptions
    {
        bool showErrorBox = false;
        bool createErrorLog = false;
        bool createMinimalSetup = false;
    };

    DTWAIN_HANDLE SysInitializeImpl(const SysInitializeOptions& initOptions);
    std::vector<CTL_ITwainSource*> GetOpenSources(CTL_TwainDLLHandle* pHandle);
    bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId);
    LONG DTWAIN_CloseAllSources();
    HMODULE GetDLLInstance();

    #ifdef __cplusplus
    extern "C" {
    #endif
    #ifdef _WIN32
    LRESULT DLLENTRY_DEF DTWAIN_WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    #endif
    void DTWAIN_AcquireProc(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source, WPARAM Data1, LPARAM Data2);
    #ifdef __cplusplus
    }
    #endif

    void LoadOCRInterfaces(CTL_TwainDLLHandle *pHandle);
    void UnloadOCRInterfaces(CTL_TwainDLLHandle *pHandle);

    DTWAIN_HANDLE GetDTWAINHandle_Internal();
    void SysDestroyNoCheck();

    #define THIS_FUNCTION_PROTO_THROWS  ;
    #define THIS_FUNCTION_THROWS

    void  DTWAIN_InternalThrowException() THIS_FUNCTION_PROTO_THROWS

    #define IDS_DTWAIN_APPTITLE       9700
    #define IDS_DTWAIN_APPTITLE_HTML  9701
}

#endif
