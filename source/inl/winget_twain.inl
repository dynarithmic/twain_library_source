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
#ifndef WINGET_TWAIN_INL
#define WINGET_TWAIN_INL
#ifdef _WIN32
static CTL_StringType GetTwainDirFullNameEx(CTL_TwainDLLHandle* pHandle, LPCTSTR strTwainDLLName,
    bool bLeaveLoaded = false,
    boost::dll::shared_library* pModule = nullptr);

CTL_StringType GetTwainDirFullName(LPCTSTR strTwainDLLName,
    LPLONG pWhichSearch,
    bool bLeaveLoaded = false,
    boost::dll::shared_library* pModule = nullptr)
{
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    static constexpr std::array<std::pair<LONG, const TCHAR*>, 15> searchOrderMap = { {
        {DTWAIN_TWAINDSMSEARCH_WSO, _T("WSO")},
        { DTWAIN_TWAINDSMSEARCH_WOS,_T("WOS") },
        { DTWAIN_TWAINDSMSEARCH_SWO,_T("SWO") },
        { DTWAIN_TWAINDSMSEARCH_SOW,_T("SOW") },
        { DTWAIN_TWAINDSMSEARCH_OWS,_T("OWS") },
        { DTWAIN_TWAINDSMSEARCH_OSW,_T("OSW") },
        { DTWAIN_TWAINDSMSEARCH_W,_T("W") },
        { DTWAIN_TWAINDSMSEARCH_S,_T("S") },
        { DTWAIN_TWAINDSMSEARCH_O,_T("O") },
        { DTWAIN_TWAINDSMSEARCH_WS,_T("WS") },
        { DTWAIN_TWAINDSMSEARCH_WO,_T("WO") },
        { DTWAIN_TWAINDSMSEARCH_SW,_T("SW") },
        { DTWAIN_TWAINDSMSEARCH_SO,_T("SO") },
        { DTWAIN_TWAINDSMSEARCH_OW,_T("OW") },
        { DTWAIN_TWAINDSMSEARCH_OS,_T("OS") } } };

    auto iter = dynarithmic::generic_array_finder_if(searchOrderMap, [&](const auto& pr) { return pr.first == pHandle->m_TwainDSMSearchOrder; });
    if (iter.first)
    {
        pHandle->m_TwainDSMSearchOrderStr = searchOrderMap[iter.second].second + CTL_StringType(_T("C"));
        return GetTwainDirFullNameEx(pHandle, strTwainDLLName, bLeaveLoaded, pModule);
    }
    // This will completely use the Ex version of finding the directory
    return GetTwainDirFullNameEx(pHandle, strTwainDLLName, bLeaveLoaded, pModule);
}

enum { dll_already_loaded, dll_loaded, dll_notfound };

template <typename ErrorCodeType>
static std::pair<int, int> LoadTwainDLL(boost::dll::shared_library& libloader, const CTL_StringType& fNameTotal)
{
    ErrorCodeType ec;
    HMODULE hMod = ::GetModuleHandle(fNameTotal.c_str());
    if (hMod)
        return { dll_already_loaded, boost::system::errc::success };
    libloader.load(fNameTotal, ec, boost::dll::load_mode::search_system_folders);
    int val = ec.value();
    if (val != boost::system::errc::success)
        return { dll_notfound, val };
    return { dll_loaded, val };
}

CTL_StringType GetTwainDirFullNameEx(CTL_TwainDLLHandle* pHandle, LPCTSTR strTwainDLLName, bool bLeaveLoaded, boost::dll::shared_library* pModule)
{
    struct libLoadRAII
    {
        bool* m_pLeaveLoaded = nullptr;
        boost::dll::shared_library* m_pSharedLibrary = nullptr;
        libLoadRAII(boost::dll::shared_library* pSharedLibrary, bool* pLeaveLoaded) :
            m_pSharedLibrary(pSharedLibrary), m_pLeaveLoaded(pLeaveLoaded) {}
        void LeaveLoaded(bool bSet) { *m_pLeaveLoaded = bSet; }
        ~libLoadRAII()
        {
            try
            {
                if (!(*m_pLeaveLoaded))
                    m_pSharedLibrary->unload();
            }
            catch (...)
            {
            }
        }
    };

    static constexpr int WinDirPos = 0;
    static constexpr int SysDirPos = 1;
    static constexpr int SysPathPos = 2;
    static constexpr int CurDirPos = 3;
    static constexpr int UserDefPos = 4;

    // make sure we get only the file name.  If a directory path
    // is given in the strTwainDLLName argument, it is ignored.
    StringWrapper::StringArrayType fComponents;
    StringWrapper::SplitPath(strTwainDLLName, fComponents);
    const auto fName = fComponents[StringWrapper::NAME_POS] + fComponents[StringWrapper::EXTENSION_POS];

    // we first search the Windows directory.
    // if TWAIN isn't found there, check system directory.
    // if not there, then use the Windows path search logic
    std::set<CTL_StringType> strSet;
    static constexpr std::array<std::pair<StringWrapper::traits_type::char_type, int>, 5> searchMap = { {
        {_T('C'),CurDirPos},
        {_T('W'),WinDirPos},
        {_T('S'),SysDirPos},
        {_T('O'),SysPathPos},
        {_T('U'),UserDefPos },
     } };

    std::vector<CTL_StringType> dirNames(searchMap.size());

    const auto dllPath = GetDTWAINDLLPath();
    constexpr int boost_version_minor = (BOOST_VERSION / 100) % 1000;
    constexpr int boost_version_major = BOOST_VERSION / 100000;

    dirNames[WinDirPos] = StringWrapper::GetWindowsDirectory();
    dirNames[SysDirPos] = StringWrapper::GetSystemDirectory();
    dirNames[SysPathPos] = {};
    dirNames[CurDirPos] = StringWrapper::SplitPath(dllPath)[StringWrapper::DIRECTORY_POS];
    auto& startupSearchOrder = CTL_StaticData::GetStartupDSMSearchOrder();
    auto& startupSearchOrderDir = CTL_StaticData::GetStartupDSMSearchOrderDir();
    CTL_StringType dirToUse = pHandle->m_TwainDSMUserDirectory;
    CTL_StringType searchOrderToUse = pHandle->m_TwainDSMSearchOrderStr;
    if (!startupSearchOrder.empty())
    {
        searchOrderToUse = startupSearchOrder;
        dirToUse = startupSearchOrderDir;
    }
    dirNames[UserDefPos] = StringWrapper::SplitPath(dirToUse)[StringWrapper::DIRECTORY_POS];

    struct SetErrorModeRAII
    {
        UINT m_OldError;
        SetErrorModeRAII()
        {
#ifdef _WIN32
            m_OldError = SetErrorMode(SEM_NOOPENFILEERRORBOX);
#endif
        }
        ~SetErrorModeRAII()
        {
#ifdef _WIN32
            SetErrorMode(m_OldError);
#endif
        }
    };

    const CTL_StringType curSearchOrder = searchOrderToUse;
    CTL_StringType fNameTotal;
    const int minSize = static_cast<int>((std::min)(dirNames.size(), curSearchOrder.size()));
    for (int i = 0; i < minSize; ++i)
    {
        // skip this search if -1 is given
        auto curOrder = dynarithmic::generic_array_finder_if(searchMap, [&](const auto& pr) { return pr.first == curSearchOrder[i]; });
        const int nCurDir = searchMap[curOrder.second].second;
        if (nCurDir == -1)
            continue;

        // only do this if we haven't checked the directory
        auto dirNameToUse = dirNames[nCurDir];
        if (strSet.find(dirNameToUse) != strSet.end())
            continue;

        // record that we are trying this directory
        strSet.insert(dirNameToUse);

        if (!dirNameToUse.empty())
            fNameTotal = StringWrapper::AddBackslashToDirectory(dirNameToUse) + fName;
        else
            fNameTotal = fName;

        // Set the DLL load error code, and ensure that the error mode
        // is set back to its original state when done searching for 
        // the TWAIN DSM.
        SetErrorModeRAII errMode;

        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
        {
            CTL_StringType msg = _T("Testing TWAIN availability for file \"") + fNameTotal + _T("\" ...");
            LogWriterUtils::WriteLogInfo(msg);
        }
        boost::dll::shared_library libloader;
        libLoadRAII raii(&libloader, &bLeaveLoaded);

        std::pair<int, int> loadReturnCode;

        if constexpr (boost_version_major == 1 && boost_version_minor >= 88)
        {
            // Boost has changed return code status for version 1.88 and higher
            loadReturnCode = LoadTwainDLL<std::error_code>(libloader, fNameTotal);
        }
        else
        {
            // Use boost 1.87 and below compatible code
            loadReturnCode = LoadTwainDLL<boost::system::error_code>(libloader, fNameTotal);
        }

        if (loadReturnCode.second != boost::system::errc::success)
        {
            // Error loading the source manager DLL
            if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing TWAIN availability for file \"") + fNameTotal + _T("\" failed with error code: ");
                msg += StringWrapper::ToString(loadReturnCode.second);
                LogWriterUtils::WriteLogInfo(msg);
            }
            continue;
        }

        // Leave the DLL loaded if it was already loaded, otherwise respect the bLeaveLoaded setting
        if (loadReturnCode.first == dll_already_loaded)
            bLeaveLoaded = true;

        // Try to load the source manager
        DSMENTRYPROC lpDSMEntry = nullptr;
        try
        {
            if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing if file \"") + fNameTotal + _T("\" is a valid Twain DSM...");
                LogWriterUtils::WriteLogInfo(msg);
            }
            lpDSMEntry = dtwain_library_loader<DSMENTRYPROC>::get_func_ptr(libloader.native(), "DSM_Entry");
        }
        catch (boost::exception&)
        {
        }

        if (lpDSMEntry)
        {
            if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing if file \"") + fNameTotal + _T("\" is a valid Twain DSM (success) ...");
                LogWriterUtils::WriteLogInfo(msg);
            }
            // We need the full module name
            fNameTotal = StringWrapper::traits_type::PathGenericString(libloader.location());
            if (pModule)
            {
                if (bLeaveLoaded)
                    *pModule = libloader;
            }
            return fNameTotal;
        }
        else
        {
            // This is not a TWAIN DSM.  Unload the library
            libloader.unload();
            if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing if file \"") + fNameTotal + _T("\" is a valid DSM (failed) ...");
                LogWriterUtils::WriteLogInfo(msg);
            }
        }
    }
    return {};
}

#endif
#endif // WINGET_TWAIN_INL