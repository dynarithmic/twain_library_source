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
#ifndef WINGET_TWAIN_INL
#define WINGET_TWAIN_INL
#ifdef _WIN32
static CTL_StringType GetTwainDirFullNameEx(CTL_TwainDLLHandle* pHandle, LPCTSTR strTwainDLLName,
									bool bLeaveLoaded = false,
									boost::dll::shared_library *pModule = nullptr);

CTL_StringType GetTwainDirFullName(LPCTSTR strTwainDLLName, 
                                    LPLONG pWhichSearch, 
                                    bool bLeaveLoaded = false, 
                                    boost::dll::shared_library *pModule = nullptr)
{
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    static constexpr std::array<std::pair<LONG, const char*>, 15> searchOrderMap = { {
		{DTWAIN_TWAINDSMSEARCH_WSO,"WSO"},
		{ DTWAIN_TWAINDSMSEARCH_WOS,"WOS" },
		{ DTWAIN_TWAINDSMSEARCH_SWO,"SWO" },
        { DTWAIN_TWAINDSMSEARCH_SOW,"SOW" },
        { DTWAIN_TWAINDSMSEARCH_OWS,"OWS" },
        { DTWAIN_TWAINDSMSEARCH_OSW,"OSW" },
        { DTWAIN_TWAINDSMSEARCH_W,"W" },
		{ DTWAIN_TWAINDSMSEARCH_S,"S" },
		{ DTWAIN_TWAINDSMSEARCH_O,"O" },
		{ DTWAIN_TWAINDSMSEARCH_WS,"WS" },
		{ DTWAIN_TWAINDSMSEARCH_WO,"WO" },
		{ DTWAIN_TWAINDSMSEARCH_SW,"SW" },
		{ DTWAIN_TWAINDSMSEARCH_SO,"SO" },
		{ DTWAIN_TWAINDSMSEARCH_OW,"OW" },
        { DTWAIN_TWAINDSMSEARCH_OS,"OS" } } };

    auto iter = dynarithmic::generic_array_finder_if(searchOrderMap, [&](const auto& pr) { return pr.first == pHandle->m_TwainDSMSearchOrder; });
	if (iter.first)
	{
        pHandle->m_TwainDSMSearchOrderStr = searchOrderMap[iter.second].second + std::string("CU");
        return GetTwainDirFullNameEx(pHandle, strTwainDLLName, bLeaveLoaded, pModule);
	}
    // This will completely use the Ex version of finding the directory
    return GetTwainDirFullNameEx(pHandle, strTwainDLLName, bLeaveLoaded, pModule);
}

CTL_StringType GetTwainDirFullNameEx(CTL_TwainDLLHandle* pHandle, LPCTSTR strTwainDLLName, bool bLeaveLoaded, boost::dll::shared_library *pModule)
{
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
    static constexpr std::array<std::pair<StringWrapperA::traits_type::char_type, int>, 5> searchMap = { {
		{'C',CurDirPos},
		{'W',WinDirPos},
		{'S',SysDirPos},
		{'O',SysPathPos},
        {'U',UserDefPos },
     } };

    std::vector<CTL_StringType> dirNames(searchMap.size());

    const auto dllPath = GetDTWAINDLLPath();
    dirNames[WinDirPos] = StringWrapper::GetWindowsDirectory();
    dirNames[SysDirPos] = StringWrapper::GetSystemDirectory();
    dirNames[SysPathPos] = {};
	dirNames[CurDirPos] = StringWrapper::SplitPath(dllPath)[StringWrapper::DIRECTORY_POS];
    dirNames[UserDefPos] = StringWrapper::SplitPath(pHandle->m_TwainDSMUserDirectory)[StringWrapper::DIRECTORY_POS];

    const std::string curSearchOrder = pHandle->m_TwainDSMSearchOrderStr;
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
        #ifdef _WIN32
        const UINT nOldError = SetErrorMode(SEM_NOOPENFILEERRORBOX);
        #endif

        if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS )
        { 
            CTL_StringType msg = _T("Testing TWAIN availability for file \"") + fNameTotal + _T("\" ...");
            CTL_TwainAppMgr::WriteLogInfo(msg);
        }
        boost::dll::shared_library libloader;
        boost::system::error_code ec;
        libloader.load(fNameTotal, ec, boost::dll::load_mode::search_system_folders);

        #ifdef _WIN32
        SetErrorMode(nOldError);
        #endif

        if (ec != boost::system::errc::success)
        {
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing TWAIN availability for file \"") + fNameTotal + _T("\" failed");
                CTL_TwainAppMgr::WriteLogInfo(msg);
            }
            continue;
        }

        // Try to load the source manager
        DSMENTRYPROC lpDSMEntry = nullptr;
        try 
        {
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing if file \"") + fNameTotal + _T("\" is a valid Twain DSM...");
                CTL_TwainAppMgr::WriteLogInfo(msg);
            }
            lpDSMEntry = dtwain_library_loader<DSMENTRYPROC>::get_func_ptr(libloader.native(), "DSM_Entry");
        }
        catch (boost::exception&)
        {
        }

        if (lpDSMEntry)
        {
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing if file \"") + fNameTotal + _T("\" is a valid Twain DSM (success) ...");
                CTL_TwainAppMgr::WriteLogInfo(msg);
            }
            // We need the full module name
            fNameTotal = StringWrapper::traits_type::PathGenericString(libloader.location());
            if (!bLeaveLoaded)
                // Unload the library
                libloader.unload();
            if (pModule)
            {
                if (bLeaveLoaded)
                    *pModule = libloader;
            }
            return fNameTotal;
        }
        else
        {
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)
            {
                CTL_StringType msg = _T("Testing if file \"") + fNameTotal + _T("\" is a valid DSM (failed) ...");
                CTL_TwainAppMgr::WriteLogInfo(msg);
            }
        }
        libloader.unload();
    }
    return {};
}

#endif
#endif // WINGET_TWAIN_INL
