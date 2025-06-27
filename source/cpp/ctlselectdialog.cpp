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
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "ctltmpl5.h"
#include "sourceselectopts.h"
#include "errorcheck.h"
#include "../simpleini/simpleini.h"
#include <boost/logic/tribool.hpp>

using namespace dynarithmic;
using namespace boost::logic;

CTL_StringType dynarithmic::LLSelectionDialog(CTL_TwainDLLHandle* pHandle, const SourceSelectionOptions& opts)
{
    #ifndef _WIN32
    return {};
    #else
    // Get the resource for the Twain dialog
    auto dllHandle = CTL_StaticData::GetDLLInstanceHandle();
    const HGLOBAL hglb = LoadResource(dllHandle, static_cast<HRSRC>(FindResource(dllHandle,
                                                                      MAKEINTRESOURCE(10000), RT_DIALOG)));
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{ return !hglb;}, DTWAIN_ERR_NULL_WINDOW, NULL, FUNC_MACRO);

    const auto lpTemplate = static_cast<LPDLGTEMPLATE>(LockResource(hglb));

    SelectStruct selectStruct;
    selectStruct.CS.xpos = opts.xPos;
    selectStruct.CS.ypos = opts.yPos;
    selectStruct.CS.nOptions = opts.nOptions;
    selectStruct.CS.hWndParent = opts.hWndParent;
    selectStruct.getDefaultFunc = opts.getDefaultFunc;
    selectStruct.getNameListFunc = opts.getNameListFunc;
    selectStruct.nItems = 0;
    selectStruct.pHandle = pHandle;
    if ( opts.szTitle )
        selectStruct.CS.sTitle = opts.szTitle;
    else
    {
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            LogWriterUtils::WriteLogInfoIndentedA("Retrieving Dialog Resources...");
        selectStruct.CS.sTitle = GetResourceStringFromMap_Native(opts.selectionResourceID);
        if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            LogWriterUtils::WriteLogInfoIndentedA("Retrieved Dialog Resources successfully...");
        if (selectStruct.CS.sTitle.empty() )
            selectStruct.CS.sTitle = _T("Select Source");
    }

    if (opts.szIncludeNames)
        StringWrapper::Tokenize(opts.szIncludeNames, _T("|"), selectStruct.CS.aIncludeNames);
    if (opts.szExcludeNames)
        StringWrapper::Tokenize(opts.szExcludeNames, _T("|"), selectStruct.CS.aExcludeNames);
    if (opts.szNameMapping)
    {
        std::vector<CTL_StringType> mapPairs;
        StringWrapper::Tokenize(opts.szNameMapping, _T("|"), mapPairs);
        for (auto& m : mapPairs)
        {
            std::vector<CTL_StringType> onePair;
            StringWrapper::Tokenize(m, _T("="), onePair);
            if (onePair.size() == 2)
                selectStruct.CS.mapNames.insert({ onePair.front(), onePair.back() });
        }
    }
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
        LogWriterUtils::WriteLogInfoIndentedA("Displaying TWAIN Dialog...");
    const INT_PTR bRet = DialogBoxIndirectParam(dllHandle, lpTemplate, opts.hWndParent,
                                                reinterpret_cast<DLGPROC>(DisplayTwainDlgProc), reinterpret_cast<LPARAM>(&selectStruct));
    if (bRet == -1)
    {
        CTL_TwainAppMgr::SetError(DTWAIN_ERR_WIN32_ERROR, LogWin32Error(::GetLastError()).c_str(), false);
        return {};
    }

    // See if cancel was selected
    if (selectStruct.SourceName.empty() || selectStruct.nItems == 0 )
    {
        CTL_TwainAppMgr::SetError(DTWAIN_ERR_SOURCESELECTION_CANCELED, "", false);
        return {};
    }

    // Could be a mapped name, so need to check
    auto actualSourceName = selectStruct.SourceName;
    if (opts.nOptions & DTWAIN_DLG_USENAMEMAPPING)
    {
        const auto iter = std::find_if(selectStruct.CS.mapNames.begin(), selectStruct.CS.mapNames.end(),
                                       [&](auto& pr)
                                       { return pr.second == actualSourceName;}
        );
        if (iter != selectStruct.CS.mapNames.end())
            actualSourceName = iter->first;
    }
    return actualSourceName;
    #endif
}


static void DisplayLocalString(HWND hWnd, int nID, int resID)
{
    std::string sText;
    sText = GetResourceStringFromMap(resID);
    if (!sText.empty())
    {
        const HWND hWndControl = GetDlgItem(hWnd, nID);
        if (hWndControl)
            SetDlgItemTextA(hWnd, nID, sText.c_str());
    }
}

#ifdef _WIN32
/////////////////////////////////////////////////////////////////////////////////
/// TWAIN Dialog procedure
static BOOL CALLBACK ChildEnumFontProc(HWND hWnd, LPARAM lParam)
{
    SendMessage(hWnd, WM_SETFONT, static_cast<WPARAM>(lParam), 0);
    return TRUE;
}

static std::vector<CTL_StringType> AdjustSourceNames(std::vector<CTL_StringType>& vSourceNames, CustomPlacement CS)
{
    if (vSourceNames.empty())
        return {};

    const bool doExclude = !CS.aExcludeNames.empty(); // Use an include list
    const bool doInclude = !CS.aIncludeNames.empty(); // Use an exclude list
    const bool doMapping = !CS.mapNames.empty();  // Use a name mapping list

    if (!doInclude && !doExclude && !doMapping)
        return vSourceNames;

    for (auto& sName : vSourceNames)
        sName = StringWrapper::TrimAll(sName);

    if (doInclude)
    {
        for (auto& sName : CS.aIncludeNames)
            sName = StringWrapper::TrimAll(sName);

        // Create a list of the names to include (extract only those names)            
        std::vector<CTL_StringType> vReturn2;
        std::sort(vSourceNames.begin(), vSourceNames.end());
        std::sort(CS.aIncludeNames.begin(), CS.aIncludeNames.end());
        std::set_intersection(vSourceNames.begin(), vSourceNames.end(),
            CS.aIncludeNames.begin(), CS.aIncludeNames.end(), std::back_inserter(vReturn2));
        if (!vReturn2.empty())
            vSourceNames = vReturn2;
    }

    auto vReturn = vSourceNames;

    if (doExclude)
    {
        for (auto& sName : CS.aExcludeNames)
            sName = StringWrapper::TrimAll(sName);

        // Create a list of the names to include if we remove the excluded names
        std::vector<CTL_StringType> vReturn2;
        std::sort(vSourceNames.begin(), vSourceNames.end());
        std::sort(CS.aExcludeNames.begin(), CS.aExcludeNames.end());

        // This does the magic of removing the excluded names
        std::set_difference(vSourceNames.begin(), vSourceNames.end(),
            CS.aExcludeNames.begin(), CS.aExcludeNames.end(), std::back_inserter(vReturn2));
        if (!vReturn2.empty())
            vReturn = vReturn2;
    }

    if (doMapping)
    {
        // Check if a mapped name should be used
        std::vector<CTL_StringType> vMapped;
        for (auto& sName : vReturn)
        {
            auto iter = CS.mapNames.find(sName);
            if (iter != CS.mapNames.end())
                // replace real source name with mapped name
                vMapped.push_back(iter->second);
            else
                // just use the real source name
                vMapped.push_back(sName);
        }
        vReturn = vMapped;
    }

    // Return the new vector of Source names to display in the
    // Select Source dialog.
    return vReturn;
}

// Determine if the selected source name is actually a mapped name
static CTL_StringType GetPossibleMappedName(CustomPlacement CS, TCHAR* szSelectedSourceName)
{
    if (CS.mapNames.empty())
        return szSelectedSourceName;

    auto& mapping = CS.mapNames;

    // check if selected source name is a map key
    auto iter = mapping.find(szSelectedSourceName);
    if (iter != mapping.end())
        return iter->first;  // return that a Source "by coincidence" is the name of an actual source

    // Go through map to see what the real source name is of the mapped, selected source name
    for (auto& it : mapping)
    {
        if (it.second == szSelectedSourceName)
            return it.first; // return the real Source name
    }

    // No name matches
    return {};
}

LRESULT CALLBACK dynarithmic::DisplayTwainDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static SelectStruct* pS;
    bool bLogMessages = (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS) ? true : false;
    switch (message)
    {
        case WM_INITDIALOG:
        {
            DTWAINDeviceContextRelease_RAII contextRAII;
            if (CTL_StaticData::GetDialogFont())
            {
                SendMessage(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(CTL_StaticData::GetDialogFont()), 0);
                EnumChildWindows(hWnd, ChildEnumFontProc, reinterpret_cast<LPARAM>(CTL_StaticData::GetDialogFont()));
            }

            HWND lstSources;
            pS = reinterpret_cast<SelectStruct*>(lParam);

            if (pS->CS.nOptions & DTWAIN_DLG_CENTER_SCREEN)
                CenterWindow(hWnd, nullptr);
            else
            if (pS->CS.nOptions & DTWAIN_DLG_CENTER)
                CenterWindow(hWnd, GetParent(hWnd));
            else
                SetWindowPos(hWnd, nullptr, pS->CS.xpos, pS->CS.ypos, 0, 0, SWP_NOSIZE);
            lstSources = GetDlgItem(hWnd, IDC_LSTSOURCES);

            // Set the title
            ::SetWindowText(hWnd, pS->CS.sTitle.c_str());

            // Fill the list box with the sources
            auto vValues = pS->getNameListFunc(*pS);
            int nCount;
            if (vValues.empty())
                nCount = 0;
            else
                nCount = static_cast<int>(vValues.size());
            pS->nItems = nCount;
            if (nCount == 0)
            {
                HWND hWndSelect = GetDlgItem(hWnd, IDOK);
                if (hWndSelect)
                    EnableWindow(hWndSelect, FALSE);

                // Display the local strings if they are available:
                DisplayLocalString(hWnd, IDOK, IDS_SELECT_TEXT);
                DisplayLocalString(hWnd, IDCANCEL, IDS_CANCEL_TEXT);
                DisplayLocalString(hWnd, IDC_SOURCETEXT, IDS_SOURCES_TEXT);
                SetFocus(hWnd);
                return TRUE;
            }

            // Get the default Source
            std::vector<TCHAR> DefName;
            bool bAlwaysHighlightFirst = pS->CS.nOptions & DTWAIN_DLG_HIGHLIGHTFIRST;
            DefName = pS->getDefaultFunc(*pS);
            if (bLogMessages)
            {
                StringStreamA strm;
                strm << GetResourceStringFromMap(IDS_LOGMSG_DEFAULTSOURCE) << ": \"" <<
                    StringConversion::Convert_NativePtr_To_Ansi(DefName.data()).c_str() << "\" ...";
                LogWriterUtils::WriteLogInfoIndentedA(strm.str());
            }

            std::vector<SIZE> TextExtents;
            HDC hdcList = nullptr;
            if (pS->CS.nOptions & DTWAIN_DLG_HORIZONTALSCROLL)
            {
                hdcList = GetDC(lstSources);
                auto pr = std::make_pair(lstSources, hdcList);
                contextRAII.reset(&pr);
            }

            std::vector<CTL_StringType> vSourceNames = vValues;
            std::vector<CTL_StringType> vNewSourceNames;

            if (!vValues.empty())
            {
                // Remove and rename sources depending on the options
                vNewSourceNames = AdjustSourceNames(vSourceNames, pS->CS);
                for (auto& sName : vNewSourceNames)
                {
                    SIZE szType;
                    if (hdcList)
                    {
                        auto cstr = sName.c_str();
                        ::GetTextExtentPoint32(hdcList, cstr, static_cast<int>(StringWrapper::traits_type::Length(cstr)), &szType);
                        TextExtents.push_back(szType);
                    }
                }

                if (hdcList)
                {
                    sort(TextExtents.begin(), TextExtents.end(), [](SIZE sz1, SIZE sz2) { return sz1.cx > sz2.cx; });
                }

                // Sort the names
                if (pS->CS.nOptions & DTWAIN_DLG_SORTNAMES)
                {
                    sort(vNewSourceNames.begin(), vNewSourceNames.end());
                }
            }
            LRESULT index;
            LRESULT DefIndex = 0;
            if (bLogMessages)
            {
                int nCounter = 1;
                CTL_StringStreamType strm2;
                auto nl = StringWrapper::traits_type::GetNewLineString();
                strm2 << "----- " << GetResourceStringFromMap_Native(IDS_SOURCES_TEXT) <<
                    StringWrapper::JoinEx(vNewSourceNames.begin(), vNewSourceNames.end(),
                        [&](const CTL_StringType& str, const CTL_StringType& val)
                        {
                            CTL_StringStreamType strmInner;
                            strmInner << str << nl << nCounter << _T(") ") << val;
                            ++nCounter;
                            return strmInner.str();
                        }
                    ) << nl << "-----";
                LogWriterUtils::WriteMultiLineInfoIndented(strm2.str(), nl);
            }
            for (auto& sName : vNewSourceNames)
            {
                index = SendMessage(lstSources, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(sName.c_str()));
                if (!DefName.empty())
                {
                    if (StringWrapper::Compare(sName, static_cast<LPCTSTR>(DefName.data())) == 0)
                        DefIndex = index;
                }
            }
            if (bAlwaysHighlightFirst || DefName.empty())
                DefIndex = 0;

            if (!TextExtents.empty())
            {
                SendMessage(lstSources, LB_SETCURSEL, DefIndex, 0);
                if (pS->CS.nOptions & DTWAIN_DLG_HORIZONTALSCROLL)
                    SendMessage(lstSources, LB_SETHORIZONTALEXTENT, TextExtents[0].cx, 0);
            }
            else
                SendMessage(lstSources, LB_SETCURSEL, DefIndex, 0);

            // Display the local strings if they are available:
            DisplayLocalString(hWnd, IDOK, IDS_SELECT_TEXT);
            DisplayLocalString(hWnd, IDCANCEL, IDS_CANCEL_TEXT);
            DisplayLocalString(hWnd, IDC_SOURCETEXT, IDS_SOURCES_TEXT);

            // Display the window as topmost if topmost flag is on
            if (pS->CS.nOptions & DTWAIN_DLG_TOPMOSTWINDOW)
            {
                ::SetForegroundWindow(hWnd);
                ::SetWindowPos(hWnd,       // handle to window
                    HWND_TOPMOST,  // placement-order handle
                    0,     // horizontal position
                    0,      // vertical position
                    0,  // width
                    0, // height
                    SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE// window-positioning options
                );
            }
            SetFocus(hWnd);
            return TRUE;
        }

        case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            HWND lstSources = GetDlgItem(hWnd, IDC_LSTSOURCES);
            TCHAR sz[255];
            LRESULT nSel = SendMessage(lstSources, LB_GETCURSEL, 0, 0);
            SendMessage(lstSources, LB_GETTEXT, nSel, reinterpret_cast<LPARAM>(sz));

            // Check if this is a mapped name
            pS->SourceName = GetPossibleMappedName(pS->CS, sz);

            if (bLogMessages)
            {
                StringWrapper::traits_type::outputstream_type strm;
                strm << _T("Selected Source name in dialog = \"") << sz << _T("\", Actual Source name = \"") << pS->SourceName << _T("\"");
                LogWriterUtils::WriteLogInfoIndented(strm.str());
            }
            EndDialog(hWnd, LOWORD(wParam));
            return TRUE;
        }
        else
        if (LOWORD(wParam) == IDCANCEL)
        {
            pS->SourceName.clear();
            EndDialog(hWnd, LOWORD(wParam));
            return TRUE;
        }
        break;
    }
    return FALSE;
}
#endif
