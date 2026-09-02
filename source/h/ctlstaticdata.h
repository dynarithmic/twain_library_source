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
#ifndef CTLSTATICDATA_H
#define CTLSTATICDATA_H

#include <array>
#include <string_view>
#include <windef.h>
#include <mutex>
#include <map>
#include <vector>
#include <tchar.h>
#include "ctlstringdefs.h"
#include "ctlmapdefs.h"
#include "logmsg.h"
#include "SimpleIni.h"
#include "ctltwaindecoder.h"
#include "pdffont_basic.h"
#include "ctlfilesavedefs.h"
#include "dtwaindefs.h"

namespace dynarithmic
{
    struct CTL_GeneralResourceInfo
    {
        CTL_StringType sResourceName;
        bool bIsFromRC = false;
    };

    struct ImageResamplerData
    {
        std::string m_sImageType;
        std::vector<uint16_t> m_vNoSamples;
        std::map<uint16_t, uint16_t> m_mapFromTo;
    };

    using ImageResamplerMap = std::map<int, ImageResamplerData>;

    struct CTL_StaticDataStruct
    {
        enum { INI_SOURCEXFERWAITINFO_KEY, 
               INI_TWAINLOOPPEEK_KEY, 
               INI_PAPERDETECTIONSTATUS_KEY, 
               INI_FLATBEDONLY_KEY, 
               INI_SOURCEOPENPROPS_KEY, 
               INI_CHECKFEEDERSTATUS_ITEM, 
               INI_QUERYBESTCAPCONTAINER_ITEM, 
               INI_QUERYCAPOPERATIONS_ITEM, 
               INI_IMAGEGILE_KEY, 
               INI_MISCELLANEOUS_KEY, 
               INI_RESOURCECHECK_ITEM,
               INI_RESAMPLE_ITEM, 
               INI_OCRLIBRARY_KEY, 
               INI_LANGUAGE_KEY, 
               INI_DEFAULT_ITEM,
               INI_SOURCES_KEY, 
               INI_DSMERRORLOGGING_KEY, 
               INI_ALLOWDUP_RESOURCE,
               INI_SOURCE_SAVEDEFAULT,
               INI_SELECTSOURCEPOS_KEY,
               INI_SAVESELECTSOURCEPOS_KEY,
               INI_TWAINLOOPGETMSG_KEY,
               INI_SHEETCOUNT_KEY,
               INI_TESTGET_ITEM,
               INI_AUTOCLOSEUI_KEY,
               INI_PARSEDELIMS_ITEM,
               INI_USEEXTERNALRC_ITEM,
               LASTINIENTRY };

        std::array<std::pair<int, std::string_view>, LASTINIENTRY> s_aINIKeys;
        int32_t                      s_nExtImageInfoOffset = 0;
        int                          s_nLoadingError = DTWAIN_ERR_BAD_HANDLE;
        bool                         s_bINIFileLoaded = false;
        bool                         s_bDoResampling = true;
        bool                         s_bCheckHandles = true;
        bool                         s_multipleThreads = false;
        HFONT                        s_DialogFont = nullptr;
        LONG                         s_nRegisteredDTWAINMsg = 0;
        bool                         s_bThrowExceptions = false;
        HINSTANCE                    s_DLLInstance = nullptr;
        uint32_t                     s_logFilterFlags = 0;
        bool                         s_ResourcesInitialized = false;
        bool                         s_bTimerIDSet = false;
        CTL_StringType               s_FileParseDelimiters;
        CTL_UINT16ToInfoMap          s_IntToTwainInfoMap;
        CTL_StringToConstantMap      s_MapStringToConstant;
        CTL_TwainIDToStringMap     s_MapExtendedImageInfo;
        CTL_StringToMapLongToStringMap s_AllLanguageResourcesMap;
        CTL_GeneralResourceInfo         s_ResourceInfo;
        CTL_PDFMediaMap          s_PDFMediaMap;
        CTL_TwainConstantsMap s_TwainConstantsMap;
        CTL_StringType           s_strResourcePath;  // path to the DTWAIN resource strings
        CTL_StringType           s_DLLPath;
        CTL_StringType           s_DLLParentPath;
        CTL_StringType           s_sINIPath;
        CTL_StringType           s_StartupDSMSearchOrder = _T("CWSOU");
        CTL_StringType           s_StartupDSMSearchOrderDir;
        CTL_LongToStringMap      s_ErrorCodes;
        CTL_StringType           s_VersionString;
        CTL_ErrorToExtraInfoMap  s_mapExtraErrorInfo;
        CTL_GeneralCapInfo       s_mapGeneralCapInfo;
        CTL_MapThreadToDLLHandle s_mapThreadToDLLHandle;
        CTL_ThreadMap            s_ThreadMap;
        std::unordered_set<HWND> s_appWindowsToDisable;
        CTL_StringType           s_strLangResourcePath;
        CTL_GeneralErrorInfo     s_mapGeneralErrorInfo;
        CTL_AvailableFileFormatsMap s_AvailableFileFormatsMap;
        CLogSystem               s_appLog;
        ImageResamplerMap        s_ImageResamplerMap;
        SourceStatusMap          s_SourceStatusMap;
        CTL_StringType           s_ResourceVersion;
        std::string              s_CurrentResourceKey;
        CTL_PairToStringMap      s_ResourceCache;
        CTL_FileSaveMap          s_FileSaveMap;
        CTL_CompressionMap       s_CompressionMap;
        std::string              s_AppTitle;
        std::string              s_AppTitleHTML;
        std::pair<int32_t, int32_t> s_SavedSelectSourcePos;
        CTL_TEXTELEMENTPTRLIST   s_PDFTextElementList;
        int64_t                  s_logFileSaveThreshold = -1LL;
        bool                     s_bTestGetMessage = true;
        SourceToXferReadyMap     s_SourceToXferReadyMap;
        SourceToXferReadyList    s_SourceToXferReadyList;
        SourceFlatbedOnlyList    s_SourceFlatbedOnlyList;
        SourceGetMessageList     s_SourceGetMessageList;
        SourcePaperDetectableMap s_SourcePaperDetectableMap;
        SourceSheetcountMap      s_SourceSheetcountList;
        SourceToUIAutocloseMap   s_SourceToAutocloseMap;
        CTL_StringType           s_ApplicationName;
        SupportedBitDepthsMap    s_supportedBitDepths;
        CTL_StaticDataStruct();
    };

    struct CTL_StaticData
    {
        static std::mutex               s_mutexInitDestroy;
        static std::unique_ptr<CSimpleIniA>    s_iniInterface;
        static CTL_StaticDataStruct& Get()
        {
            static CTL_StaticDataStruct data;
            return data;
        }
        static CTL_ThreadMap& GetThreadMap() { return Get().s_ThreadMap; }
        static HFONT& GetDialogFont() { return Get().s_DialogFont; }
        static CLogSystem& GetLogger() { return Get().s_appLog; }
        static LONG& GetRegisteredMessage() { return Get().s_nRegisteredDTWAINMsg; }
        static bool IsResamplingDone() { return Get().s_bDoResampling;  }
        static void SetResamplingDone(bool bSet) { Get().s_bDoResampling = bSet; }
        static CTL_StringType& GetVersionString() { return Get().s_VersionString; }
        static bool IsINIFileLoaded() { return Get().s_bINIFileLoaded; }
        static void SetINIFileLoaded(bool bSet) { Get().s_bINIFileLoaded = bSet; }
        static CTL_StringType& GetLanguageResourcePath() { return Get().s_strLangResourcePath; }
        static CTL_ErrorToExtraInfoMap& GetExtraErrorInfoMap() { return Get().s_mapExtraErrorInfo; }
        static CTL_MapThreadToDLLHandle& GetThreadToDLLHandleMap() { return Get().s_mapThreadToDLLHandle; }
        static CTL_FileSaveMap& GetFileSaveMap() { return Get().s_FileSaveMap; }
        static CTL_CompressionMap& GetCompressionMap() { return Get().s_CompressionMap; }
        static bool ResourcesLoaded() { return Get().s_ResourcesInitialized; }
        static void Reset() 
        { 
            CTL_StaticDataStruct tempStruct; 
            auto& static_struct = Get();
            tempStruct.s_DLLPath = static_struct.s_DLLPath;
            tempStruct.s_DLLParentPath = static_struct.s_DLLParentPath;
            tempStruct.s_DLLInstance = static_struct.s_DLLInstance;
            tempStruct.s_StartupDSMSearchOrder = static_struct.s_StartupDSMSearchOrder;
            tempStruct.s_StartupDSMSearchOrderDir = static_struct.s_StartupDSMSearchOrderDir;
            tempStruct.s_SavedSelectSourcePos = static_struct.s_SavedSelectSourcePos;
            static_struct = tempStruct;
        }
        static auto& GetLogFilterFlags() { return Get().s_logFilterFlags; }
        static bool IsThrowExceptions() { return Get().s_bThrowExceptions; }
        static void SetThrowExceptions(bool bSet) { Get().s_bThrowExceptions = bSet; }
        static CTL_UINT16ToInfoMap& GetIntToTwainInfoMap() { return Get().s_IntToTwainInfoMap; }
        static int32_t GetExtImageInfoOffset() { return Get().s_nExtImageInfoOffset; }
        static void SetExtImageInfoOffset(int32_t offset) { Get().s_nExtImageInfoOffset = offset; }
        static CTL_StringToConstantMap& GetStringToConstantMap() { return Get().s_MapStringToConstant; }
        static CTL_TwainIDToStringMap& GetExtendedImageInfoMap() { return Get().s_MapExtendedImageInfo; }
        static int GetResourceLoadError() { return Get().s_nLoadingError; }
        static void SetResourceLoadError(int errNum) { Get().s_nLoadingError = errNum; }
        static CSimpleIniA* GetINIInterface() { return s_iniInterface.get(); }
        static CTL_PairToStringMap& GetResourceCache() { return Get().s_ResourceCache; }
        static CTL_StringToMapLongToStringMap& GetAllLanguagesResourceMap() { return Get().s_AllLanguageResourcesMap; }
        static CTL_LongToStringMap* GetLanguageResource(std::string_view sLang);
        static std::string&         GetCurrentLanguageResourceKey() { return Get().s_CurrentResourceKey; }
        static void SetCurrentLanguageResourceKey(const std::string& sLang) { Get().s_CurrentResourceKey = sLang; }
        static CTL_LongToStringMap* GetCurrentLanguageResource();
        static CTL_GeneralResourceInfo& GetGeneralResourceInfo() { return Get().s_ResourceInfo; }
        static CTL_PDFMediaMap& GetPDFMediaMap() { return Get().s_PDFMediaMap; }
        static CTL_TwainConstantsMap& GetTwainConstantsMap() { return Get().s_TwainConstantsMap; }
        static CTL_TwainConstantToStringMapNode& GetTwainConstantsStrings(LONG nWhich) { return Get().s_TwainConstantsMap[nWhich]; }
        static bool IsCheckHandles() { return Get().s_bCheckHandles; }
        static void SetCheckHandles(bool bSet) { Get().s_bCheckHandles = bSet; }
        static std::pair<bool, TwainConstantType> GetIDFromTwainName(std::string_view sName);
        static constexpr int GetDGResourceID() { return 8890; }
        static constexpr int GetDATResourceID() { return 8891; }
        static constexpr int GetMSGResourceID() { return 8892; }
        static CTL_StringType& GetResourcePath() { return Get().s_strResourcePath; }
        static CTL_StringType& GetDLLPath() { return Get().s_DLLPath; }
        static CTL_StringType& GetINIPath() { return Get().s_sINIPath; }
        static CTL_StringType& GetStartupDSMSearchOrder() { return Get().s_StartupDSMSearchOrder; }
        static CTL_StringType& GetStartupDSMSearchOrderDir() { return Get().s_StartupDSMSearchOrderDir; }
        static bool IsUsingMultipleThreads() { return Get().s_multipleThreads; }
        static void SetUseMultipleThreads(bool bSet) { Get().s_multipleThreads = bSet; }
        static CTL_LongToStringMap& GetErrorCodes() { return Get().s_ErrorCodes; }
        static CTL_GeneralCapInfo& GetGeneralCapInfo() { return Get().s_mapGeneralCapInfo; }
        static HINSTANCE GetDLLInstanceHandle() { return Get().s_DLLInstance; }
        static CTL_GeneralErrorInfo& GetGeneralErrorInfoMap() { return Get().s_mapGeneralErrorInfo; }
        static void SetDLLInstanceHandle(HINSTANCE h) { Get().s_DLLInstance = h; }
        static ImageResamplerMap& GetImageResamplerMap() { return Get().s_ImageResamplerMap; }
        static SourceStatusMap& GetSourceStatusMap() { return Get().s_SourceStatusMap;  }
        static CTL_StringType& GetResourceVersion() { return Get().s_ResourceVersion; }
        static std::pair<bool, CTL_StringType> GetTwainNameFromConstant(int lConstantType, TwainConstantType lTwainConstant);
        static std::pair<bool, std::string> GetTwainNameFromConstantA(int lConstantType, TwainConstantType lTwainConstant);
        static std::pair<bool, std::wstring> GetTwainNameFromConstantW(int lConstantType, TwainConstantType lTwainConstant);
        static auto& GetAppWindowsToDisable() { return Get().s_appWindowsToDisable; }
        static constexpr std::string_view GetINIKey(int nWhich) { return Get().s_aINIKeys[nWhich].second; }
        static std::string& GetAppTitle() { return Get().s_AppTitle; }
        static std::string& GetAppTitleHTML() { return Get().s_AppTitleHTML; }
        static std::pair<int32_t, int32_t>& GetSelectSourcePos() { return Get().s_SavedSelectSourcePos; }
        static auto& GetPDFTextElementList() { return Get().s_PDFTextElementList; }
        static auto& GetLogFileSaveThreshold() { return Get().s_logFileSaveThreshold; }
        static bool& IsTestForGetMessage() { return Get().s_bTestGetMessage; }
        static SourceToXferReadyMap& GetSourceToXferReadyMap() { return Get().s_SourceToXferReadyMap; }
        static SourceToXferReadyList& GetSourceToXferReadyList() { return Get().s_SourceToXferReadyList; }
        static SourceFlatbedOnlyList& GetSourceFlatbedOnlyList() { return Get().s_SourceFlatbedOnlyList; }
        static SourceGetMessageList& GetSourceGetMessageList() { return Get().s_SourceGetMessageList; }
        static SourcePaperDetectableMap& GetSourcePaperDetectionMap() { return Get().s_SourcePaperDetectableMap; }
        static SourceSheetcountMap& GetSourceSheetcountMap() { return Get().s_SourceSheetcountList; }
        static SourceToUIAutocloseMap& GetSourceToUIAutocloseMap() { return Get().s_SourceToAutocloseMap; }
        static CTL_StringType& GetFileParseDelimiters() { return Get().s_FileParseDelimiters; }
        static CTL_StringType& GetApplicationName() { return Get().s_ApplicationName;  }
        static CTL_StringType& GetDLLParentPath() { return Get().s_DLLParentPath; }
        static CTL_AvailableFileFormatsMap& GetAvailableFileFormatsMap() { return Get().s_AvailableFileFormatsMap; }
        static SupportedBitDepthsMap& GetSupportedBPPMap() { return Get().s_supportedBitDepths; }
    };
}
#endif