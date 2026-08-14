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
#include <cstring>
#include <algorithm>
#include <string>
#include <string_view>
#include <sstream>
#include <set>
#include <vector>

#ifdef _MSC_VER
    #pragma warning (disable:4702)
    #pragma comment (lib, "shlwapi")
#endif
#include "ctllogsourcecaps.h"
#include "ctlgetversion.h"
#include "ctltwainlogging.h"
#include "ctldtwainhandle.h"
#include "ctlstringutils.h"
#include "ctlthreadutils.h"
#include "dtwain_version.h"
#include "cppfunc.h"
#include "ctlwindowsimpl.h"
#include "ctlguiddef.h"
#include "ctltwaindllhandle.h"
#include "dtwainx.h"
#include <arrayfactory.h>
#include "ctlfileutils.h"
#include "ctlguidimpl.h"
#include "ctltwaindllpath.h"
#include "ctldefsource.h"
#include "windowsinit_impl.h"
#include "ctltwainsource.h"
#include "ctltwainmanager.h"
#include "ctlclosesource.h"
#include "errorcheck.h"
#include "dtwain_config.h"

#ifdef _WIN64
    #pragma message ("Compiling 64-bit DTWAIN")
#else
#ifdef WIN32
    #pragma message ("Compiling 32-bit DTWAIN")
#endif
#endif

using namespace dynarithmic;
namespace stringutils = basicstringutils;

namespace
{
    bool RemoveThreadIdFromAssociation(unsigned long threadId);
    void LogDTWAINErrorToMsgBox(int nError, LPCSTR pFunc, std::string_view s);
    HWND CreateTwainWindow(CTL_TwainDLLHandle* /*pHandle*/, HINSTANCE hInstance/*=NULL*/, HWND hWndParent);
    void RegisterTwainWindowClass();
    void UnhookAllDisplays();
    bool SysDestroyHelper(const char* pParentFunc, CTL_TwainDLLHandle* pHandle, bool bCheck=true);
    std::string GetStaticLibVer();
}

namespace dynarithmic
{
    DTWAIN_HANDLE GetDTWAINHandle_Internal()
    {
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        auto iter = threadMap.find(getThreadId());
        if (iter != threadMap.end())
            return iter->second.get();
        return nullptr;
    }

    void SysDestroyNoCheck()
    {
    }
        
    HMODULE GetDLLInstance()
    {
        #ifdef _WIN32
        return GetModuleHandleA(DTWAIN_DLLNAME);
        #else
        return 0;
        #endif
    }
}

namespace
{
    DTWAIN_BOOL SetLangResourcePath(LPCTSTR szPath)
    {
        LOG_FUNC_ENTRY_PARAMS((szPath))
        CTL_StaticData::GetLanguageResourcePath() = WindowsAPIImplDef::AddBackslashToDirectory(szPath);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }


    bool FindTask( DWORD hTask )
    {
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        return threadMap.find(hTask) != threadMap.end();
    }

    bool FindTask(const DTWAIN_GUID& guid)
    {
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        auto it = std::find_if(threadMap.begin(), threadMap.end(), [&](const auto& pr) { return pr.second->GetGUID() == guid; });
        return it != threadMap.end();
    }

    template <class TypeInfo, class TypeArray>
    bool FindFirstValue( TypeInfo SearchVal,
                        std::vector<TypeArray> *pSearchArray,
                        int *pWhere/*=NULL*/ )
    {
        if ( pWhere )
            *pWhere = -1;
        auto it = std::find_if(pSearchArray->begin(), pSearchArray->end(), [&](const TypeArray& val) { return val.GetValue1() == SearchVal;}); //Searcher(SearchVal));
        if ( it != pSearchArray->end() )
        {
            if (pWhere)
                *pWhere = static_cast<int>(std::distance(pSearchArray->begin(), it));
            return true;
        }
        return false;
    }
}

// Load resource functions
namespace
{
    void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL, bool bClear)
    {
        // Load the resources
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;

        std::string szStr = customProfile->GetValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_LANGUAGE_KEY).data(),
            CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_DEFAULT_ITEM).data(),
            stringconversion::Convert_NativePtr_To_Ansi(szLangDLL).c_str());
        if (!LoadLanguageResourceA(szStr, pHandle->GetResourceRegistry(), bClear))
        {
            // Use the English resources by default
            if (!LoadLanguageResourceA("english", pHandle->GetResourceRegistry(), bClear))
            {
                // Too bad.  Last chance -- load english resources directly from internal rc.
                // Note that unlike the text resources that should have been loaded, 
                // these internal resources should not be able to be edited without
                // corrupting the DLL in terms of checking the hash value of the DLL file.
                LoadLanguageResourceFromRC();
            }
        }
        // Load error flags if defined
        struct ProfileSettingsInt
        {
            LPCSTR section;
            LPCSTR name;
            long orValue;
        };
        constexpr ProfileSettingsInt allIntProfiles[] = {
            { "DSMErrorLogging", "Decode_Identity1", DTWAIN_LOG_DECODE_SOURCE },
            { "DSMErrorLogging", "Decode_Identity2", DTWAIN_LOG_DECODE_DEST },
            { "DSMErrorLogging", "Decode_Data", DTWAIN_LOG_DECODE_TWMEMREF },
            { "DSMErrorLogging", "Decode_Events", DTWAIN_LOG_DECODE_TWEVENT },
            { "DSMErrorLogging", "CallStack", DTWAIN_LOG_CALLSTACK },
            { "DSMErrorLogging", "ShowExceptions", DTWAIN_LOG_SHOWEXCEPTIONS },
            { "DSMErrorLogging", "DisplayLibErrors", DTWAIN_LOG_ERRORMSGBOX },
            { "DSMErrorLogging", "DisplayInitFailErrors", DTWAIN_LOG_INITFAILURE },
            { "DSMErrorLogging", "BufferErrors", DTWAIN_LOG_USEBUFFER } };

        auto& logFilterFlags = CTL_StaticData::GetLogFilterFlags();
        std::for_each(allIntProfiles, std::end(allIntProfiles), [&](const ProfileSettingsInt& ps)
            {
                const auto nVal = customProfile->GetLongValue(ps.section, ps.name, 0);
                if (nVal != 0)
                    logFilterFlags |= ps.orValue;
            });

        auto nVal = customProfile->GetLongValue("DSMErrorLogging", "EnableNone", 0);
        if (nVal == 1)
            logFilterFlags = 0;

        nVal = customProfile->GetLongValue("DSMErrorLogging", "EnableAll", 0);
        if (nVal != 0)
            logFilterFlags = 0xFFFFFFFFL & ~DTWAIN_LOG_USEFILE;

        szStr = customProfile->GetValue("DSMErrorLogging", "File", "");
        if (!szStr.empty())
        {
            logFilterFlags |= DTWAIN_LOG_USEFILE;
            OpenLogging(stringconversion::Convert_Ansi_To_Native(szStr, szStr.size()).c_str(), logFilterFlags);
            CTL_StaticData::GetLogger().StatusOutFast("In DTWAIN_SysInitialize()");
        }

        nVal = customProfile->GetLongValue("DSMErrorLogging", "BufferErrorThreshold", 50);
        if (logFilterFlags & DTWAIN_LOG_USEBUFFER)
            DTWAIN_SetErrorBufferThreshold(nVal);

        nVal = customProfile->GetLongValue("DSMErrorLogging", "AppHandlesExceptions", 0);
        logFilterFlags = nVal == 0 ? false : true;
    }

    void LoadStaticData(CTL_TwainDLLHandle* pHandle)
    {
        if (pHandle->m_mapDTWAINArrayToTwainType.empty())
        {
            static constexpr int TwainIntTypes[] = { TWTY_INT8, TWTY_UINT8, TWTY_BOOL, TWTY_INT16, TWTY_INT32, TWTY_UINT16, TWTY_UINT32 };
            static constexpr int TwainStringTypes[] = { TWTY_STR32, TWTY_STR64, TWTY_STR128, TWTY_STR128, TWTY_STR255, TWTY_STR1024 };
            static constexpr int TwainFloatTypes[] = { TWTY_FIX32 };
            static constexpr int TwainFrameTypes[] = { TWTY_FRAME };
            static constexpr int TwainWideStringTypes[] = { TWTY_UNI512 };
            static constexpr int DTwainArrayTypes[] = { DTWAIN_ARRAYLONG, DTWAIN_ARRAYANSISTRING, DTWAIN_ARRAYFLOAT, DTWAIN_ARRAYFRAME,
                                                       CTL_ArrayFrameSingleType, DTWAIN_ARRAYWIDESTRING };
            static constexpr size_t NumTwainTypes[] = { std::size(TwainIntTypes),
                                             std::size(TwainStringTypes),
                                             std::size(TwainFloatTypes),
                                             std::size(TwainFrameTypes),
                                             std::size(TwainFrameTypes),
                                             std::size(TwainWideStringTypes) };

            static constexpr std::array<const int*, 6> intArray =
            { TwainIntTypes, TwainStringTypes, TwainFloatTypes, TwainFrameTypes, TwainFrameTypes, TwainWideStringTypes };

            static constexpr size_t nNumAllTypes = std::size(NumTwainTypes);
            for (size_t i = 0; i < nNumAllTypes; ++i)
            {
                std::vector<LONG> TwainType(intArray[i], intArray[i] + NumTwainTypes[i]);
                pHandle->m_mapDTWAINArrayToTwainType.insert(make_pair(DTwainArrayTypes[i], TwainType));
            }
        }
    }

    // This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [SourceXferWaitInfo]
    // section for TWAIN sources that may potentially send "close source" requests
    // before sending the "start transfer" request when acquiring images.
    void LoadTransferReadyOverrides()
    {
        auto& xfer_map = CTL_StaticData::GetSourceToXferReadyMap();
        xfer_map.clear();
        auto& xfer_list = CTL_StaticData::GetSourceToXferReadyList();
        xfer_list.clear();

        // Get the section name
        auto *customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CSimpleIniA::TNamesDepend keys;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCEXFERWAITINFO_KEY).data();
        customProfile->GetAllKeys(iniKey, keys);
        auto iter = keys.begin();
        while (iter != keys.end())
        {
            CSimpleIniA::TNamesDepend vals;
            customProfile->GetAllValues(iniKey, iter->pItem, vals);
            if (!vals.empty())
            {
                auto iter2 = vals.begin();
                if ( !vals.empty())
                {
                    try
                    {
                        uint32_t valueToUse = std::stoi(iter2->pItem);
                        xfer_list.push_back({ iter->pItem, valueToUse });
                    }
                    catch (const std::invalid_argument& /*ex*/)
                    {
                        // We can get here if std::stoi detects that the value is not 
                        // a valid integer. 
                        xfer_list.push_back({ iter->pItem, 0 });
                    }
                    catch (const std::out_of_range& /*ex*/)
                    {
                        // We can get here if std::stoi detects that the value is not 
                        // a valid integer. 
                        xfer_list.push_back({ iter->pItem, 0 });
                    }
                }
            }
            ++iter;
        }
    }

    // This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [AutocloseUI]
    // section for TWAIN sources that will require the source UI to autoclose
    // after a single acquisition
    void LoadAutocloseUIOverrides()
    {
        auto& autoclose_map = CTL_StaticData::GetSourceToUIAutocloseMap();
        autoclose_map.clear();

        // Get the section name
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CSimpleIniA::TNamesDepend keys;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_AUTOCLOSEUI_KEY).data();
        customProfile->GetAllKeys(iniKey, keys);
        auto iter = keys.begin();
        while (iter != keys.end())
        {
            CSimpleIniA::TNamesDepend vals;
            customProfile->GetAllValues(iniKey, iter->pItem, vals);
            if (!vals.empty())
            {
                auto iter2 = vals.begin();
                if (!vals.empty())
                {
                    try
                    {
                        uint32_t valueToUse = std::stoi(iter2->pItem);
                        autoclose_map.insert({ iter->pItem, (valueToUse != 0)?true:false });
                    }
                    catch (const std::invalid_argument& /*ex*/)
                    {
                    }
                    catch (const std::out_of_range& /*ex*/)
                    {
                    }
                }
            }
            ++iter;
        }
    }

    // This loads the sources that rely on the TWAIN loop when processing the acquisitions
    // to use GetMessage() instead of PeekMessage().
    void LoadTwainLoopOverrides()
    {
        auto& getmsgloop_list = CTL_StaticData::GetSourceGetMessageList();
        getmsgloop_list.clear();

        // Get the section name
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CSimpleIniA::TNamesDepend keys;
        customProfile->GetAllKeys(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_TWAINLOOPGETMSG_KEY).data(), keys);
        auto iter = keys.begin();
        while (iter != keys.end())
        {
            getmsgloop_list.insert(iter->pItem);
            ++iter;
        }
    }

    void LoadGetMessageTestOverride()
    {
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_TWAINLOOPPEEK_KEY).data();
        CTL_StaticData::IsTestForGetMessage() = customProfile->GetBoolValue(iniKey, CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_TESTGET_ITEM).data(), true);
    }

    // This loads the sources that will override DTWAIN_IsFeederSensitive() with 
    // whether the source supports checking for paper loaded in feeder
    void LoadPaperDetectionOverrides()
    {
        auto& paperdetectable_map = CTL_StaticData::GetSourcePaperDetectionMap();
        paperdetectable_map.clear();

        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CSimpleIniA::TNamesDepend keys;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_PAPERDETECTIONSTATUS_KEY).data();
        customProfile->GetAllKeys(iniKey, keys);
        auto iter = keys.begin();
        while (iter != keys.end())
        {
            bool isPaperDetectable = customProfile->GetBoolValue(iniKey, iter->pItem, true);
            paperdetectable_map[iter->pItem] = isPaperDetectable;
            ++iter;
        }
    }

    // This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [SourceOpenProps]
    // section.  This section determines the activities to perform after successfully
    // opening a TWAIN Source
    void LoadOnSourceOpenProperties(CTL_TwainDLLHandle* pHandle)
    {
        // Get the section name
        auto* iniInterface = CTL_StaticData::GetINIInterface();
        if (!iniInterface)
            return;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCEOPENPROPS_KEY).data();
        pHandle->m_OnSourceOpenProperties.m_bCheckFeederStatusOnOpen = iniInterface->GetBoolValue(iniKey, CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_CHECKFEEDERSTATUS_ITEM).data(), true);
        pHandle->m_OnSourceOpenProperties.m_bQueryBestCapContainer = iniInterface->GetBoolValue(iniKey, CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_QUERYBESTCAPCONTAINER_ITEM).data(), true);
        pHandle->m_OnSourceOpenProperties.m_bQueryCapOperations = iniInterface->GetBoolValue(iniKey, CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_QUERYBESTCAPCONTAINER_ITEM).data(), true);

        // Check if the default opened source name is saved to the INI file when a source is opened
        iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCES_KEY).data();
        pHandle->m_OnSourceOpenProperties.m_bSaveDefaultToINI = iniInterface->GetBoolValue(iniKey, CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCE_SAVEDEFAULT).data(), false);
    }

    // This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [SheetCount]
    // section.  This section determines whether the TWAIN driver supports
    // CAP_SHEETCOUNT correctly (interprets the CAP_SHEETCOUNT as the number
    // of sheets of paper, not the number of images)
    void LoadSheetcountProperties(CTL_TwainDLLHandle* pHandle)
    {
        auto& sheetcount_map = CTL_StaticData::GetSourceSheetcountMap();
        sheetcount_map.clear();

        // Get the section name
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CSimpleIniA::TNamesDepend keys;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SHEETCOUNT_KEY).data();
        customProfile->GetAllKeys(iniKey, keys);
        auto iter = keys.begin();
        while (iter != keys.end())
        {
            CSimpleIniA::TNamesDepend vals;
            customProfile->GetAllValues(iniKey, iter->pItem, vals);
            if ( !vals.empty() )
                sheetcount_map.push_back({ iter->pItem,vals.front().pItem });
            ++iter;
        }
    }

    void LoadImageFileOptions(CTL_TwainDLLHandle* pHandle)
    {
        auto *customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CTL_StaticData::SetResamplingDone(customProfile->GetBoolValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_IMAGEGILE_KEY).data(), 
                                            CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_RESAMPLE_ITEM).data(), true));
        auto& parseDelims = CTL_StaticData::GetFileParseDelimiters();
        const char* defName = "|*;";
        std::string sParseDelims = customProfile->GetValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_MISCELLANEOUS_KEY).data(),
                                                           CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_PARSEDELIMS_ITEM).data(), defName);
        parseDelims = stringconversion::Convert_Ansi_To_Native(sParseDelims);
    }

    // This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [FlatbedOnly]
    // section for TWAIN sources that only work if the source is preset to use
    // only the flatbed portion of the device.  
    void LoadFlatbedOnlyOverrides()
    {
        auto& flatbed_list = CTL_StaticData::GetSourceFlatbedOnlyList();
        flatbed_list.clear();

        // Get the section name
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;
        CSimpleIniA::TNamesDepend keys;
        auto iniKey = CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_FLATBEDONLY_KEY).data();
        customProfile->GetAllKeys(iniKey, keys);
        auto iter = keys.begin();
        while (iter != keys.end())
        {
            CSimpleIniA::TNamesDepend vals;
            customProfile->GetAllValues(iniKey, iter->pItem, vals);
            flatbed_list.insert(iter->pItem);
            ++iter;
        }
    }

    bool LoadGeneralResources(const SysInitializeOptions& initOptions)
    {
        bool bResourcesLoaded = false;
        CTL_StaticData::SetResourceLoadError(DTWAIN_NO_ERROR);
        typedef std::function<bool(ResourceLoadingInfo&)> boolFuncs;
        boolFuncs bf[] = { &LoadTwainResources };
        for (auto& fnBool : bf)
        {
            ResourceLoadingInfo ret;
            fnBool(ret);

            // If there are any errors loading the twaininfo.txt or INI files, report them here.
            if (std::any_of(ret.errorValue.begin(), ret.errorValue.end(), [](bool b) { return b == false; }))
            {
                CTL_StringType versionErrorMessage;
                std::vector<CTL_StringType> vErrors;
                if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED])
                {
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_NOT_FOUND);
                }
                if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED])
                {
                    #if defined (WIN64) || defined(_WIN64)
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_INI64_NOT_FOUND);
                    #else
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_INI32_NOT_FOUND);
                    #endif
                }
                if ( !ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_EXCEPTION_OK])
                {
                    CTL_StringStreamType strm;
                    strm << _T("\r\n\r\nBad data found:\r\n") << _T("Line number: ") << ret.m_dupInfo.lineNumber;
                    strm << _T("\r\nLine starts with: ") << stringconversion::Convert_Ansi_To_Native(ret.m_dupInfo.line, ret.m_dupInfo.line.length());
                    versionErrorMessage = strm.str();
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_DATA_EXCEPTION);
                }
                if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_VERSION_READ])
                {
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_BAD_VERSION);
                    versionErrorMessage = _T("\r\nBad or outdated TWAIN version of resources used: (");
                    versionErrorMessage += ret.errorMessage;
                    versionErrorMessage += _T(").  Expected version: ");
                    versionErrorMessage += _T(DTWAIN_TEXTRESOURCE_FILEVERSION);
                    versionErrorMessage += _T("\r\nPlease use the latest text resources found at \"https://github.com/dynarithmic/twain_library/tree/master/text_resources\"");
                }
                if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_CRC_CHECK])
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_CRC_CHECK);
                if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_NODUPLICATE_ID])
                {
                    CTL_StringStreamType strm;
                    strm << _T("\r\n\r\nDuplicate ID Information:\r\n") << _T("Line number: ") << ret.m_dupInfo.lineNumber;
                    strm << _T("\r\nLine: ") << stringconversion::Convert_Ansi_To_Native(ret.m_dupInfo.line, ret.m_dupInfo.line.length());
                    strm << _T("\r\nID: ") << ret.m_dupInfo.duplicateID;
                    versionErrorMessage = strm.str();
                    CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_DUPLICATEID_FOUND);
                }

                if (!versionErrorMessage.empty())
                    versionErrorMessage += _T("\r\nCurrent running DTWAIN Version: ") + CTL_StringType(_T(DTWAIN_SHORT_VERSION));

                CTL_StringType szBuf(DTWAIN_USERRES_MAXSIZE + 1, L' ');
                LONG nChars = DTWAIN_GetErrorString(CTL_StaticData::GetResourceLoadError(), szBuf.data(), DTWAIN_USERRES_MAXSIZE);
                szBuf.resize(nChars);
                CTL_StringType errorMsg = _T("Error.  DTWAIN Resource file(s) not found or corrupted:\r\n");
                CTL_StringType sAllErrors = _T("Error in reading resource file:\r\n") + ret.resourcePath + _T("\r\n") +
                    errorMsg + szBuf + versionErrorMessage;

                if (initOptions.showErrorBox)
                {
                    // Only display the error message box if DTWAIN_SysInitialize() was called
                    // instead of DTWAIN_SysInitialNoBlocking()
                    #ifdef _WIN32
                    MessageBox(nullptr, sAllErrors.c_str(), _T("DTWAIN Resource Error"), MB_ICONERROR);
                    #endif
                }
                else
                {
                    bool bWroteInfoToFile = false;
                    std::string sErr = stringconversion::Convert_Native_To_Ansi(
                        stringutils::ReplaceAll<CTL_StringType>(sAllErrors, _T("\r"), _T(" ")));
                    if (initOptions.createErrorLog)
                    {
                        // Write the information to errorlog_*.txt located in the resource directory
                        // Get the base file name with time stamp
                        auto baseFileName = filenameutils::CreateFileNameWithDateTime<CTL_StringType>(_T("errorlog_"), _T("txt"));

                        // Create the file name with the path of the resources
                        auto errorName = stringconversion::Convert_Native_To_Ansi(CreateResourceFileName(baseFileName.c_str()));

                        // Write the information to the error log file
                        std::ofstream ofs(errorName);
                        if (ofs)
                        {
                            ofs << sErr;
                            bWroteInfoToFile = true;
                        }
                    }
                    if ( !bWroteInfoToFile )
                    {
                        // Show error in debug console
                        OutputDebugStringA(sErr.c_str());
                    }
                }
            }
            else
                bResourcesLoaded = true;
        }
        return bResourcesLoaded;
    }

    // This loads the last select source save position
    void LoadSelectSourcePosition()
    {
        // Get the section name
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (!customProfile)
            return;

        const char* pLastPos = customProfile->GetValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCES_KEY).data(),
            CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SELECTSOURCEPOS_KEY).data(), "");
        if (pLastPos && pLastPos[0] != 0)
        {
            std::vector<CTL_StringTypeA> arr;
            auto numTokens = stringutils::Tokenize(pLastPos, " ", arr);
            if (numTokens >= 2)
            {
                auto& lastPos = CTL_StaticData::GetSelectSourcePos();
                try
                {
                    lastPos.first = stoi(arr[0]);
                    lastPos.second = stoi(arr[1]);
                }
                catch (...)
                {
                }
            }
        }
    }
}

#ifdef _WIN32
namespace
{
//    UINT_PTR APIENTRY FileSaveAsHookProc(HWND hWnd, UINT msg, WPARAM w, LPARAM lparam);
    WNDPROC SubclassTwainMsgWindow(HWND hWnd, WNDPROC wProcIn = nullptr);
}
#define     TWSubclassWindow(hwnd, lpfn)  \
                (reinterpret_cast<WNDPROC>(SetWindowLongPtr((hwnd), GWLP_WNDPROC, (LONG_PTR)(WNDPROC)(lpfn))))
#endif

#ifdef DTWAIN_LIB
    static void GetVersionFromResource(LPLONG lMajor, LPLONG lMinor, LPLONG patch);
#endif

namespace dynarithmic
{
    DTWAIN_HANDLE SysInitializeImpl(const SysInitializeOptions& initOptions)
    {
        std::lock_guard<std::mutex> lg(CTL_StaticData::s_mutexInitDestroy);
    #ifdef DTWAIN_LIB
        if ( CTL_StaticData::s_DLLInstance == NULL )
        {
            // Get the instance handle of the application
            TCHAR szName[1024];
            ::GetModuleFileName(NULL, szName, 1023);

            CTL_StaticData::s_DLLInstance = ::GetModuleHandle( szName );
        }
        CTL_StaticData::GetLogFilterFlags() = 0;
        CTL_StaticData::s_nRegisteredDTWAINMsg = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
    #else
        auto& sAppName = CTL_StaticData::GetApplicationName();
        if ( sAppName.empty() )
            sAppName = WindowsAPIImplDef::GetModuleFileName(nullptr);
    #endif    
        LOG_FUNC_ENTRY_PARAMS(())

        CTL_TwainDLLHandlePtr pHandlePtr;
        // Return handle if already registered
        const DTWAIN_HANDLE DLLHandle = GetDTWAINHandle_Internal();
        if (DLLHandle)
            LOG_FUNC_EXIT_NONAME_PARAMS(DLLHandle)

        // This must be checked if this is used in a static library
        #ifdef DTWAIN_LIB
        if ( CTL_StaticData::s_DLLInstance == NULL )
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        #endif

        // Load resources only if first time
        // Return a new DTWAIN_HANDLE
        try
        {
            pHandlePtr = std::make_shared<CTL_TwainDLLHandle>();
            auto threadId = getThreadId();
            AssociateThreadToTwainDLL(pHandlePtr, threadId);
            CTL_TwainDLLHandle* pHandle = pHandlePtr.get();

            // Associate a GUID with the handle
            pHandle->GetGUID() = GenerateUUIDv4Impl<std::string>();

            if (!initOptions.createMinimalSetup)
            {
                // Open dtwain32.ini or dtwain64.ini
                if ( !CTL_StaticData::s_iniInterface )
                    CTL_StaticData::s_iniInterface = std::make_unique<CSimpleIniA>();
                auto* ptrIni = CTL_StaticData::GetINIInterface();
                if (!CTL_StaticData::IsINIFileLoaded())
                {
                    auto err = ptrIni->LoadFile(GetDTWAININIPathA().c_str());
                    CTL_StaticData::SetINIFileLoaded(err == SI_OK);
                    CTL_StaticData::GetINIPath() = GetDTWAININIPath();
                }

                bool resourcesLoaded = LoadGeneralResources(initOptions);
                if (!resourcesLoaded)
                {
                    RemoveThreadIdFromAssociation(threadId);
                    LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
                }
    #ifdef _WIN32
                pHandle->m_pSaveAsDlgProc = FileSaveAsHookProc;
                RegisterTwainWindowClass();
    #endif
                LoadStaticData(pHandle);
                auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
                if (!threadMap.empty())
                {
                    const CTL_StringType szLangDLL = _T("english");

                    // Initialize the enumerator factory
                    pHandlePtr->m_ArrayFactory = std::make_shared<CTL_ArrayFactory>();

                    // Initialize the resource registry
                    pHandle->InitializeResourceRegistry();

                    // Load customized resources from INI
                    LoadCustomResourcesFromIni(pHandle, szLangDLL.c_str(), true);

                    // Load DS overrides for transfer ready / close UI requests
                    LoadTransferReadyOverrides();

                    // Load auto close UI overrides
                    LoadAutocloseUIOverrides();

                    // Load Twain message loop overrides for peek message
                    LoadTwainLoopOverrides();

                    // Load whether paper detection is supported
                    LoadPaperDetectionOverrides();

                    // Load whether there will be an explicit test for GetMessage()
                    LoadGetMessageTestOverride();

                    // Load flatbed only list of devices
                    LoadFlatbedOnlyOverrides();

                    // Load check feeder on open status
                    LoadOnSourceOpenProperties(pHandle);

                    // Load the CAP_SHEETCOUNT behavior
                    LoadSheetcountProperties(pHandle);

                    // Load image file related options
                    LoadImageFileOptions(pHandle);

                    // Load the last "Select Source" file dialog position
                    LoadSelectSourcePosition();
                
                    WriteVersionToLog(pHandle);
                    // Store the user defined search order for the
                    // TWAIN datasource manager
                    auto& searchOrder = CTL_StaticData::GetStartupDSMSearchOrder();
                    auto& searchOrderDir = CTL_StaticData::GetStartupDSMSearchOrderDir();
                    if (!searchOrder.empty())
                    {
                        pHandle->m_TwainDSMSearchOrderStr = searchOrder;
                        pHandle->m_TwainDSMUserDirectory = searchOrderDir;
                        pHandle->m_TwainDSMSearchOrder = -1;
                    }
                    pHandle->SetVersionString(GetVersionString());

                    // Get the application title, now that all the resources have been loaded
                    auto& appTitle = CTL_StaticData::GetAppTitle();
                    appTitle.resize(256, '\0');
                    auto nSize = GetResourceStringA(IDS_DTWAIN_APPTITLE, &appTitle[0], 255);
                    if ( nSize != 0)
                        appTitle.resize(nSize - 1);

                    auto& appTitleHTML = CTL_StaticData::GetAppTitleHTML();
                    appTitleHTML.resize(256, '\0');
                    nSize = GetResourceStringA(IDS_DTWAIN_APPTITLE_HTML, &appTitleHTML[0], 255);
                    if (nSize != 0)
                        appTitleHTML.resize(nSize - 1);
                }
                LOG_FUNC_ENTRY_PARAMS(())
                LOG_FUNC_EXIT_NONAME_PARAMS(static_cast<DTWAIN_HANDLE>(pHandle))
                CATCH_BLOCK(nullptr)
            }
            else
            {
                LOG_FUNC_ENTRY_PARAMS(())
                LOG_FUNC_EXIT_NONAME_PARAMS(static_cast<DTWAIN_HANDLE>(pHandle))
                CATCH_BLOCK(nullptr)
            }
            CATCH_BLOCK(nullptr)
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        }
        catch (std::exception& ex)
        {
            if (initOptions.showErrorBox)
            {
                MessageBoxA(nullptr, "DTWAIN Initialization Error", ex.what(), MB_ICONERROR);
            }
            else
            {
                LogToDebugMonitorA("DTWAIN Initialization Error\n");
                LogToDebugMonitorA(ex.what());
            }
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        }
        catch (...)
        {
            LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
        }
    }


    bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId)
    {
        if ( !pHandle )
             return false;
        return CTL_StaticData::GetThreadToDLLHandleMap().insert({threadId, pHandle}).second;
    }

    LONG DTWAIN_CloseAllSources()
    {
        LOG_FUNC_ENTRY_PARAMS(())

        // See if DLL Handle exists
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if ( pHandle->m_mapStringToSource.empty() )
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
        CTL_StringToSourcePtrMap m_mapTemp = pHandle->m_mapStringToSource;
        auto* pHandle2 = pHandle;
        std::for_each(m_mapTemp.begin(), m_mapTemp.end(), [&](CTL_StringToSourcePtrMap::value_type& vt)
        {
            CTL_ITwainSource *pTheSource = vt.second;
            if (pTheSource->IsAcquireAttempt())
                CTL_TwainAppMgr::DisableUserInterface(pTheSource);
            CloseSourceInternal(pHandle2, pTheSource);
        });

        LOG_FUNC_EXIT_NONAME_PARAMS(0)
        CATCH_BLOCK(0)
    }
}

namespace
{
    bool SysDestroyHelper(const char* pParentFunc, CTL_TwainDLLHandle* pHandle, bool bCheck)
    {
        #ifdef _WIN32
        // Unload the OCR interfaces
        unsigned long threadId = getThreadId();

        UnloadOCRInterfaces(pHandle);
        #endif

        // Close out any INI changes
        auto* customProfile = CTL_StaticData::GetINIInterface();
        if (customProfile)
        {
            // Write the last select source save position
            auto& lastPos = CTL_StaticData::GetSelectSourcePos();

            // Check if the "saveselectsourcepos" key value is in INI file, and if so, ifthe value is true
            bool bSaveLastPos = customProfile->GetBoolValue(CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCES_KEY).data(),
                                                            CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SAVESELECTSOURCEPOS_KEY).data(), false);

            if (bSaveLastPos && lastPos != std::make_pair(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max()))
            {
                // Save the last value
                std::ostringstream strm;
                strm << lastPos.first << " " << lastPos.second;
                customProfile->SetValue(
                    CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SOURCES_KEY).data(),
                    CTL_StaticData::GetINIKey(CTL_StaticDataStruct::INI_SELECTSOURCEPOS_KEY).data(),
                    strm.str().c_str());
            }

            // Close out the other INI changes
            customProfile->SaveFile(CTL_StaticData::GetINIPath().c_str());
            CTL_StaticData::s_iniInterface.reset();
            CTL_StaticData::SetINIFileLoaded(false);
        }

        // Remove the handle for this thread
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        auto it = threadMap.find(threadId);
        if ( it == threadMap.end() )
            return false;
        #if DTWAIN_BUILD_LOGCALLSTACK
        // Log the parent function, and the assumption that the 
        // return value from this point on is "true". 
        LogValue(pParentFunc, false, true);
        #endif
        try
        {
            // Note that the assumption is that no exception is thrown from
            // this point on.
            UnhookAllDisplays();
            pHandle->RemoveAllArrays();
            pHandle->RemoveAllSourceCapInfo();
            pHandle->RemoveAllSourceMaps();
            pHandle->RemoveAllPDFTextElements();
            pHandle->m_CallbackMsg = nullptr;
            pHandle->m_CallbackError = nullptr;
            RemoveThreadIdFromAssociation(threadId);
            CTL_StaticData::Reset();
            return true;
        }
        catch (...)
        {
            OutputDebugString(_T("Exception error in DTWAIN_SysDestroy()"));
        }
        return false;
    }

    void UnhookAllDisplays()
    {
    #ifdef _WIN32
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CONSOLE)
            FreeConsole();
    #endif
    }

    bool RemoveThreadIdFromAssociation(unsigned long threadId)
    {
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        auto iter = threadMap.find(threadId);
        if ( iter != threadMap.end())
        {
            threadMap.erase(iter);
            return true;
        }
        return false;
    }

}

extern "C"
{
    //////////////////////////////// Initialize DLL /////////////////////////////
    LONG DLLENTRY_DEF DTWAIN_GetAPIHandleStatus(DTWAIN_HANDLE pHandle)
    {
        LOG_FUNC_ENTRY_PARAMS((pHandle))
        LONG retVal = 0;
        if (!IsDLLHandleValid(static_cast<CTL_TwainDLLHandle*>(pHandle), FALSE))
            LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        retVal = IsDLLHandleValid(static_cast<CTL_TwainDLLHandle*>(pHandle), TRUE) ? DTWAIN_TWAINSESSIONOK : DTWAIN_APIHANDLEOK;
        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        CATCH_BLOCK(0)
    }

    LONG DLLENTRY_DEF DTWAIN_GetConditionCodeString(LONG CC, LPTSTR lpszBuffer, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((CC, lpszBuffer, nMaxLen))
        const LONG lError = -(IDS_TWCC_ERRORSTART + CC);
        const LONG Retval = DTWAIN_GetErrorString(lError, lpszBuffer, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
        LOG_FUNC_EXIT_NONAME_PARAMS(Retval)
        CATCH_BLOCK(0)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsInitialized()
    {
        // Get the Current task
        LOG_FUNC_ENTRY_PARAMS(())
        const DWORD hTask = getThreadId();

    #ifdef DTWAIN_LIB
        if ( CTL_StaticData::s_DLLInstance == NULL )
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        if ( !CTL_StaticData::s_DLLHandles.empty() )
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    #else
        // Check if this task has already been hooked
        if (FindTask( hTask ) )
            // Already hooked.  No need to do this again
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    #endif
        CATCH_BLOCK(false)
    }


    DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_GetDTWAINHandle()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        LOG_FUNC_EXIT_NONAME_PARAMS(::GetDTWAINHandle_Internal())
        CATCH_BLOCK(nullptr)
    }

    DTWAIN_HANDLE DLLENTRY_DEF  DTWAIN_SysInitializeLib(HINSTANCE hInstance)
    {
        LOG_FUNC_ENTRY_PARAMS((hInstance))

    #ifdef DTWAIN_LIB
        CTL_StaticData::s_DLLInstance = hInstance;
    #endif
        const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
        LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
        CATCH_BLOCK(nullptr)
    }

    DTWAIN_HANDLE DLLENTRY_DEF  DTWAIN_SysInitializeLibEx2(HINSTANCE hInstance,
                                                           LPCTSTR szINIPath,
                                                           LPCTSTR szImageDLLPath,
                                                           LPCTSTR szLangResourcePath)
    {
        LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath, szImageDLLPath,szLangResourcePath))

        SetLangResourcePath(szLangResourcePath);

        const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLibEx(hInstance, szINIPath);
        LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
        CATCH_BLOCK(nullptr)
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibEx(HINSTANCE hInstance, LPCTSTR szINIPath)
    {
        LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath))

        CTL_StaticData::GetINIPath() = WindowsAPIImplDef::AddBackslashToDirectory(szINIPath);

        const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLib(hInstance);
        LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
        CATCH_BLOCK(nullptr)
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////// Initialize Library EX2 code //////////////////////////////////////
    DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx2(LPCTSTR szINIPath,
                                                       LPCTSTR szImageDLLPath,
                                                       LPCTSTR szLangResourcePath)
    {
        LOG_FUNC_ENTRY_PARAMS((szINIPath, szImageDLLPath, szLangResourcePath))

        SetLangResourcePath(szLangResourcePath);
        const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeEx(szINIPath);
        LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
        CATCH_BLOCK(nullptr)
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////
    DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx(LPCTSTR szINIPath)
    {
        LOG_FUNC_ENTRY_PARAMS((szINIPath))
        CTL_StaticData::GetINIPath() = WindowsAPIImplDef::AddBackslashToDirectory(szINIPath);
        const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
        LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
        CATCH_BLOCK(nullptr)
    }

    DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeNoBlocking()
    {
        return SysInitializeImpl({ false, false , false });
    }

    DTWAIN_HANDLE  DLLENTRY_DEF DTWAIN_SysInitializeNoBlockingEx(DTWAIN_BOOL bCreateLogFile)
    {
        return SysInitializeImpl({ false, bCreateLogFile ? true : false , false });
    }

    DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitialize()
    {
        return SysInitializeImpl({ true, false , false });
    }
}

DTWAIN_BOOL DTWAIN_SetSourceCloseMode(LONG lCloseMode)
{
    LOG_FUNC_ENTRY_PARAMS((lCloseMode))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    pHandle->m_nSourceCloseMode = lCloseMode?true:false;
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(FALSE)
}

LONG DTWAIN_GetSourceCloseMode()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_nSourceCloseMode)
    CATCH_BLOCK(0)
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SysDestroy()
    {
        std::lock_guard<std::mutex> lg(CTL_StaticData::s_mutexInitDestroy);
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        if (!DTWAIN_EndTwainSession())
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        const DTWAIN_BOOL bRet = SysDestroyHelper(FUNC_MACRO, pHandle);
        if (!bRet)
        {
            #if DTWAIN_BUILD_LOGCALLSTACK == 1
            LogValue(FUNC_MACRO, false, false);
            #endif
        }
        #if DTWAIN_BUILD_LOGCALLSTACK == 1
        if (CTL_StaticData::GetLogFilterFlags())
            CTL_LogFunctionCallA(CTL_StaticData::GetLogFilterFlags(), FUNC_MACRO, 1);
        #endif
        return bRet;
        CATCH_BLOCK(false)
    }


    /* This function tests all open DLL handles to see if any source is acquiring */
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsAcquiring()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        const auto iter = std::find_if(pHandle->m_mapStringToSource.begin(),
                                       pHandle->m_mapStringToSource.end(), 
                                    [&](const CTL_StringToSourcePtrMap::value_type& vt) {return vt.second->IsAcquireAttempt(); });
        if ( iter != pHandle->m_mapStringToSource.end())
             LOG_FUNC_EXIT_NONAME_PARAMS(true)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
        CATCH_BLOCK(false)
    }

    HWND  DLLENTRY_DEF  DTWAIN_GetTwainHwnd()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_hWndTwain)
        CATCH_BLOCK(nullptr)
    }


    LONG DLLENTRY_DEF DTWAIN_GetTwainMode()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_lAcquireMode)
        CATCH_BLOCK(-1L)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainMode(LONG lMode)
    {
        LOG_FUNC_ENTRY_PARAMS((lMode))
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        switch (lMode)
        {
            case DTWAIN_MODELESS:
            case DTWAIN_MODAL:
                pHandle->m_lAcquireMode = lMode;
                LOG_FUNC_EXIT_NONAME_PARAMS(true)
            break;
        }
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, []{return 0;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AppHandlesExceptions(DTWAIN_BOOL bSet)
    {
        CTL_StaticData::SetThrowExceptions(bSet?true:false);
        return TRUE;
    }
}

#ifdef _WIN32
#else
#include "linuxinit_impl.inl"
#endif
