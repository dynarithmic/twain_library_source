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
#ifndef CTLTWAINDLLHANDLE_H
#define CTLTWAINDLLHANDLE_H

#include <utility>
#include <array>
#include <deque>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <vector>

#include "dtwaindefs.h"
#include "dtwain_standard_defs.h"
#include "ctlloadresources.h"
#include "ctlstringdefs.h"
#include "ctlguiddef.h"
#include "dtwtype.h"
#include "twain.h"
#include "ctlloggercallback.h"
#include "ctlsourceselect.h"
#include "pdffont_basic.h"
#include "ctltwainmemoryimpl.h"
#include "ctlmapdefs.h"
#include "capstructdefs.h"
#include "ocrinterface.h"
#include "ctlinternalconstants.h"

namespace dynarithmic
{
    class CTL_TwainAppMgr;
    class CTL_ITwainSession;

    struct CTL_ArrayFactory;
    using  CTL_ArrayFactoryPtr = std::shared_ptr<CTL_ArrayFactory>;
    using  CTL_SourceCapInfoArray = std::vector<CTL_SourceCapInfo>;

    class CTL_TwainDLLHandle
    {
        using OCRInterfaceContainer = std::vector<OCREnginePtr>;
        using OCRProductNameToEngineMap = std::unordered_map<std::string, OCREnginePtr>;
        using CTL_TwainDLLHandlePtr = std::shared_ptr<CTL_TwainDLLHandle>;

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

    using CTL_TwainDLLHandlePtr = std::shared_ptr<CTL_TwainDLLHandle>;
}
#endif
