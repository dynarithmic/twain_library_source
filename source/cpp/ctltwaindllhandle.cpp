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
#include "ctltwmgr.h"
#include "ctlres.h"
#include "ctliface.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;
////////////////////////////////////////////////////////////////////////
CTL_TwainDLLHandle::CTL_TwainDLLHandle() : m_hWndTwain(nullptr),
                                           m_hInstance(nullptr),
                                           m_nCurrentDibPos(0),
                                           m_bSessionAllocated(false),
                                           m_hNotifyWnd(nullptr),
                                           m_bDummyWindowCreated(false),
                                           m_bTransferDone(false),
                                           m_bSourceClosed(false),
                                           m_CallbackMsg(nullptr),
                                           m_CallbackError(nullptr),
                                           m_lLastError(0),
                                           m_lLastAcqError(0),
                                           m_lAcquireMode(DTWAIN_MODAL),
                                           m_nSourceCloseMode(DTWAIN_SourceCloseModeFORCE),
                                           m_nUIMode(DTWAIN_UIModeOPEN),
                                           m_bNotificationsUsed(false),
                                           m_bNotifyTripletsUsed(false),
                                           m_bOpenSourceOnSelect(true),
                                           m_pAppMgr(nullptr),
                                           m_pTwainSession(nullptr),
                                           m_nSaveAsFlags(0),
                                           m_SaveAsPos{},
                                           m_pSaveAsDlgProc(nullptr),
                                           m_bUseProxy(false),
                                           m_pCallbackFn(nullptr),
                                           m_pCallbackFn64(nullptr),
                                           m_lCallbackData(0),
                                           m_lCallbackData64(0),
                                           m_pErrorProcFn(nullptr),
                                           m_pErrorProcFn64(nullptr),
                                           m_lErrorProcUserData(0),
                                           m_lErrorProcUserData64(0),
                                           m_pOCRDefaultEngine(nullptr),
                                           m_Twain2Func{}
#ifdef _WIN32
                                           , m_hOrigProc(nullptr)
                                           , m_hWndDummy(nullptr)
                                           , m_mapPDFTextElement{{nullptr, {}}}
#endif
{
}

void CTL_TwainDLLHandle::RemoveAllSourceCapInfo()
{
    m_aSourceCapInfo.clear();
}

void CTL_TwainDLLHandle::RemoveAllSourceMaps()
{
    m_mapStringToSource.clear();
}

void CTL_TwainDLLHandle::InitializeResourceRegistry()
{
    auto default_values = GetLangResourceNames();
    m_ResourceRegistry.clear();
    for (size_t i = 0; i < default_values.size(); ++i)
        m_ResourceRegistry.insert({ default_values[i], filesys::exists(GetResourceFileNameA(default_values[i].c_str(), DTWAINLANGRESOURCEFILE)) });
    }

std::pair<CTL_ResourceRegistryMap::iterator, bool> CTL_TwainDLLHandle::AddResourceToRegistry(LPCSTR pLangDLL, bool bClear)
{
    if (bClear)
    {
        m_ResourceRegistry.erase(pLangDLL);
        return m_ResourceRegistry.insert({ pLangDLL, filesys::exists(GetResourceFileNameA(pLangDLL, DTWAINLANGRESOURCEFILE)) });
    }
    auto iter = m_ResourceRegistry.find(pLangDLL);
    if (iter != m_ResourceRegistry.end())
        return { iter, true };
    return { iter, false };
}

////////////////////////////////////////////////////////////////////////////////////////
int                         CTL_StaticData::s_nLoadingError = DTWAIN_NO_ERROR;
std::unique_ptr<CSimpleIniA>   CTL_StaticData::s_iniInterface;
bool                         CTL_StaticData::s_bINIFileLoaded = false;
bool                         CTL_StaticData::s_bDoResampling = true;
CTL_StringToMapLongToStringMap CTL_StaticData::s_AllLoadedResourcesMap;
CTL_PairToStringMap         CTL_StaticData::s_ResourceCache;
std::string                 CTL_StaticData::s_CurrentResourceKey;
CTL_GeneralResourceInfo     CTL_StaticData::s_ResourceInfo;
CTL_PDFMediaMap             CTL_StaticData::s_PDFMediaMap;
CTL_TwainLongToStringMap    CTL_StaticData::s_TwainCountryMap;
CTL_TwainNameMap            CTL_StaticData::s_TwainNameMap;
CTL_AvailableFileFormatsMap CTL_StaticData::s_AvailableFileFormatsMap;
CTL_TwainConstantsMap       CTL_StaticData::s_TwainConstantsMap;
bool                        CTL_StaticData::s_bCheckHandles = true;
CTL_TwainLongToStringMap    CTL_StaticData::s_TwainLanguageMap;
CTL_StringType              CTL_StaticData::s_strResourcePath;
CTL_StringType              CTL_StaticData::s_ResourceVersion;
CTL_StringType              CTL_StaticData::s_DLLPath;
CTL_StringType              CTL_StaticData::s_sINIPath;
bool                        CTL_StaticData::s_multipleThreads = false;
CTL_LongToStringMap         CTL_StaticData::s_ErrorCodes;
CTL_StringType              CTL_StaticData::s_VersionString;
HFONT                       CTL_StaticData::s_DialogFont = nullptr;
CTL_ErrorToExtraInfoMap     CTL_StaticData::s_mapExtraErrorInfo;
CTL_GeneralCapInfo          CTL_StaticData::s_mapGeneralCapInfo;
LONG                        CTL_StaticData::s_nRegisteredDTWAINMsg = 0;
std::mutex                  CTL_StaticData::s_mutexInitDestroy;
CTL_MapThreadToDLLHandle    CTL_StaticData::s_mapThreadToDLLHandle;
CTL_ThreadMap               CTL_StaticData::s_ThreadMap;
bool                        CTL_StaticData::s_bThrowExceptions = false;
HINSTANCE                   CTL_StaticData::s_DLLInstance = nullptr;
std::unordered_set<HWND>    CTL_StaticData::s_appWindowsToDisable;
CTL_CallbackProcArray       CTL_StaticData::s_aAllCallbacks;
CTL_StringType              CTL_StaticData::s_strLangResourcePath;
CTL_GeneralErrorInfo        CTL_StaticData::s_mapGeneralErrorInfo;
long                        CTL_StaticData::s_lErrorFilterFlags = 0;
UINT_PTR                    CTL_StaticData::s_nTimeoutID = 0;
bool                        CTL_StaticData::s_bTimerIDSet = false;
UINT                        CTL_StaticData::s_nTimeoutMilliseconds = 0;
CLogSystem                  CTL_StaticData::s_appLog;
bool                        CTL_StaticData::s_ResourcesInitialized = false;
ImageResamplerMap           CTL_StaticData::s_ImageResamplerMap;
SourceStatusMap             CTL_StaticData::s_SourceStatusMap;

CTL_StringType CTL_StaticData::GetTwainNameFromConstant(int lConstantType, int lTwainConstant)
{
    auto& constantsmap = CTL_StaticData::GetTwainConstantsMap();
    auto iter1 = constantsmap.find(lConstantType);
    if (iter1 == constantsmap.end())
        return {};
    auto iter2 = iter1->second.find(lTwainConstant);
    if (iter2 == iter1->second.end())
        return {};
    return StringConversion::Convert_Ansi_To_Native(iter2->second);
}

CTL_LongToStringMap* CTL_StaticData::GetLanguageResource(std::string sLang)
{
    auto iter = s_AllLoadedResourcesMap.find(sLang);
    if (iter != s_AllLoadedResourcesMap.end())
        return &iter->second;
    return nullptr;
}

CTL_LongToStringMap* CTL_StaticData::GetCurrentLanguageResource()
{
    return CTL_StaticData::GetLanguageResource(s_CurrentResourceKey);
}

///////////////////////////////////////////////////////////////////////////
void CTL_TwainDLLHandle::NotifyWindows(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
}

std::string CTL_StaticData::GetTwainNameFromResource(int nWhichResourceID, int nWhichItem)
{
    auto& name_map = CTL_StaticData::GetTwainNameMap();
    const auto iter = name_map.Left().find({ nWhichResourceID,nWhichItem });
    if (iter != name_map.Left().end())
        return iter->second;
    return {};
}

int CTL_StaticData::GetIDFromTwainName(std::string sName)
{
    auto& name_map = CTL_StaticData::GetTwainNameMap();
    StringWrapperA::MakeUpperCase(StringWrapperA::TrimAll(sName));
    const auto iter = name_map.Right().find(sName);
    if (iter != name_map.Right().end())
        return iter->second.second;
    return{};
}

/////////////////////////////////////////////////////////////////////////
// static definitions
CTL_TwainDLLHandle* dynarithmic::FindHandle(HWND hWnd, bool bIsDisplay)
{
    const auto it = std::find_if(CTL_StaticData::s_mapThreadToDLLHandle.begin(), 
                                 CTL_StaticData::s_mapThreadToDLLHandle.end(),
                                 [&](auto& ptr)
                                 {
                                     if ( bIsDisplay)
                                         return false;
                                     return ptr.second.get() && ptr.second.get()->m_hWndTwain == hWnd;
                                 });
    if (it != CTL_StaticData::s_mapThreadToDLLHandle.end())
        return it->second.get();
    return nullptr;
}

CTL_TwainDLLHandle* dynarithmic::FindHandle(HINSTANCE hInst)
{
    const auto it = std::find_if(CTL_StaticData::s_mapThreadToDLLHandle.begin(), 
                                 CTL_StaticData::s_mapThreadToDLLHandle.end(),
                                 [&](auto& ptr)
                                 { return ptr.second.get() && ptr.second.get()->m_hInstance == hInst; });
    if (it != CTL_StaticData::s_mapThreadToDLLHandle.end())
        return it->second.get();
    return nullptr;
}
