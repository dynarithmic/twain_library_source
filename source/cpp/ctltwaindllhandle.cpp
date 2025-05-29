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
#include "ctltwainmanager.h"
#include "ctlloadresources.h"
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
    // This is a new resource that may not have shown up in the "official" list of resources
    return m_ResourceRegistry.insert({ pLangDLL, filesys::exists(GetResourceFileNameA(pLangDLL, DTWAINLANGRESOURCEFILE)) });
}

////////////////////////////////////////////////////////////////////////////////////////
CTL_StaticDataStruct        CTL_StaticData::s_StaticData;
std::unique_ptr<CSimpleIniA>   CTL_StaticData::s_iniInterface;
std::mutex                  CTL_StaticData::s_mutexInitDestroy;

FileSaveNode::FileSaveNode() : m_FileType(0) {}

FileSaveNode::FileSaveNode(int fType, CTL_StringType filter1, CTL_StringType filter2, CTL_StringType ext) :
                            m_FileType(fType), m_sTotalFilter(filter1), m_sExtension(ext)
{
    m_sTotalFilter += _T('\0');
    m_sTotalFilter += filter2;
    m_sTotalFilter.append(_T("\0\0"), 2);
}

CTL_StaticDataStruct::CTL_StaticDataStruct() : 
    s_aINIKeys{ 
                {{INI_SOURCEXFERWAITINFO_KEY,    "SourceXferWaitInfo",   },
                {INI_TWAINLOOPPEEK_KEY,          "TwainLoopPeek",        },
                {INI_PAPERDETECTIONSTATUS_KEY,   "PaperDetectionStatus", },
                {INI_FLATBEDONLY_KEY,            "FlatbedOnly",          },
                {INI_SOURCEOPENPROPS_KEY,        "SourceOpenProps",      },
                {INI_CHECKFEEDERSTATUS_ITEM,     "CheckFeederStatus",    },
                {INI_QUERYBESTCAPCONTAINER_ITEM, "QueryBestCapContainer",},
                {INI_QUERYCAPOPERATIONS_ITEM,    "QueryCapOperations",   },
                {INI_IMAGEGILE_KEY,              "ImageFile",            },
                {INI_MISCELLANEOUS_KEY,          "Miscellaneous",        },
                {INI_RESOURCECHECK_ITEM,         "resourcecheck",        },
                {INI_RESAMPLE_ITEM,              "resample",             },
                {INI_OCRLIBRARY_KEY,             "OCRLibrary",           },
                {INI_LANGUAGE_KEY,               "language",             },
                {INI_DEFAULT_ITEM,               "default",              },
                {INI_SOURCES_KEY,                "Sources",              },
                {INI_DSMERRORLOGGING_KEY,        "DSMErrorLogging"}}
             } {}

std::pair<bool, std::string> CTL_StaticData::GetTwainNameFromConstantA(int lConstantType, TwainConstantType lTwainConstant)
{
    // Get the map of constant types
    auto& constantsmap = CTL_StaticData::GetTwainConstantsMap();
    auto iter1 = constantsmap.find(lConstantType);
    if (iter1 == constantsmap.end())
        return { false, std::to_string(lTwainConstant) };

    // Now get the map of the constant value(s)
    auto iter2 = iter1->second.find(lTwainConstant);
    if (iter2 == iter1->second.end())
        return { false, std::to_string(lTwainConstant) };

    // Return the first constant name (the primary name)
    return { true, iter2->second.front() };
}

std::pair<bool, CTL_StringType> CTL_StaticData::GetTwainNameFromConstant(int lConstantType, TwainConstantType lTwainConstant)
{
    auto pr = CTL_StaticData::GetTwainNameFromConstantA(lConstantType, lTwainConstant);
    return { pr.first, StringConversion::Convert_Ansi_To_Native(pr.second) };
}

std::pair<bool, std::wstring> CTL_StaticData::GetTwainNameFromConstantW(int lConstantType, TwainConstantType lTwainConstant)
{
    auto pr = CTL_StaticData::GetTwainNameFromConstantA(lConstantType, lTwainConstant);
    return { pr.first, StringConversion::Convert_Ansi_To_Wide(pr.second) };
}

CTL_LongToStringMap* CTL_StaticData::GetLanguageResource(std::string_view sLang)
{
    auto iter = s_StaticData.s_AllLoadedResourcesMap.find(sLang.data());
    if (iter != s_StaticData.s_AllLoadedResourcesMap.end())
        return &iter->second;
    return nullptr;
}

CTL_LongToStringMap* CTL_StaticData::GetCurrentLanguageResource()
{
    return CTL_StaticData::GetLanguageResource(s_StaticData.s_CurrentResourceKey);
}


///////////////////////////////////////////////////////////////////////////
void CTL_TwainDLLHandle::NotifyWindows(UINT /*nMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
}

std::pair<bool, TwainConstantType> CTL_StaticData::GetIDFromTwainName(std::string_view sName)
{
    auto& constantsMap = CTL_StaticData::GetStringToConstantMap();
    auto iter = constantsMap.find(sName.data());
    if (iter != constantsMap.end())
        return { true, iter->second };
    return { false, (std::numeric_limits<TwainConstantType>::min)() };
}

/////////////////////////////////////////////////////////////////////////
// static definitions
CTL_TwainDLLHandle* dynarithmic::FindHandle(HWND hWnd, bool bIsDisplay)
{
    auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
    const auto it = std::find_if(threadMap.begin(), threadMap.end(),
                                 [&](auto& ptr)
                                 {
                                     if ( bIsDisplay)
                                         return false;
                                     return ptr.second.get() && ptr.second.get()->m_hWndTwain == hWnd;
                                 });
    if (it != threadMap.end())
        return it->second.get();
    return nullptr;
}

CTL_TwainDLLHandle* dynarithmic::FindHandle(HINSTANCE hInst)
{
    auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
    const auto it = std::find_if(threadMap.begin(), threadMap.end(),
                                 [&](auto& ptr)
                                 { return ptr.second.get() && ptr.second.get()->m_hInstance == hInst; });
    if (it != threadMap.end())
        return it->second.get();
    return nullptr;
}
