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
#include "ctlfilesave.h"
#include "ctliface.h"
#include "cppfunc.h"
#include "ctldtwainhandle.h"

namespace dynarithmic
{
    CTL_StringType PromptForFileName(CTL_TwainDLLHandle* pHandle, CTL_TwainFileFormatEnum nFileAcquireType)
    {
        CTL_StringType szFilter;
        LPCTSTR szExt = nullptr;

        auto& filesaveMap = CTL_StaticData::GetFileSaveMap();
        const auto it = filesaveMap.find(static_cast<int>(nFileAcquireType));
        if (it != filesaveMap.end())
        {
            szFilter = it->second.GetTotalFilter();
            szExt = it->second.GetExtension().c_str();
        }
        else
        {
            CTL_StringStreamType strm;
            strm << nFileAcquireType << _T(" format");
            szFilter = strm.str();
            szFilter.append(_T("*\0\0"), 3);
            szExt = _T(".");
        }
#ifdef _WIN32
        TCHAR szFile[1024] = {};
        // prompt for filename

        OPENFILENAME ofn = {};
        OPENFILENAME* pOfn = &ofn;

        if (pHandle->m_pofn)
            pOfn = pHandle->m_pofn.get();
        szFile[0] = _T('\0');
        pOfn->lStructSize = sizeof(OPENFILENAME);
        const auto sTitle = pHandle->m_CustomPlacement.sTitle;
        auto sTitleAnsi = basicstringutils::Narrow(sTitle);
        CTL_StringType sActualTitle;
        if (pOfn == &ofn)
        {
            pOfn->hwndOwner = nullptr;
            pOfn->lpstrFilter = szFilter.data();
            pOfn->lpstrFile = szFile;
            pOfn->nMaxFile = sizeof szFile - 5;
            pOfn->Flags = OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY |
                OFN_NOREADONLYRETURN | OFN_EXPLORER;
            if (pHandle->m_CustomPlacement.nOptions != 0)
            {
                pOfn->lpfnHook = pHandle->m_pSaveAsDlgProc;
                pOfn->Flags |= OFN_ENABLEHOOK;
                pOfn->lCustData = reinterpret_cast<LPARAM>(&pHandle->m_CustomPlacement);
                if (!sTitleAnsi.empty())
                {
                    sActualTitle = stringconversion::Convert_Ansi_To_Native(sTitleAnsi);
                    pOfn->lpstrTitle = sActualTitle.c_str();
                }
            }
        }

        if (!GetSaveFileName(pOfn))
        {
            return {};                    // user canceled dialog
        }

        // supply default extension - GetOpenFileName doesn't seem to do it!
        int nExt = pOfn->nFileExtension;
        if (nExt && !szFile[nExt])
        {
            // no extension
            lstrcat(szFile, szExt);
        }
        return szFile;
#else
        return {};
#endif
    }
}

using namespace dynarithmic;

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePos(HWND hWndParent, LPCTSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nFlags))
    // See if DLL Handle exists
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if (nFlags & DTWAIN_DLG_CLEAR_PARAMS)
        pHandle->m_CustomPlacement.nOptions = 0;
    else
    {
        pHandle->m_CustomPlacement.xpos = xPos;
        pHandle->m_CustomPlacement.ypos = yPos;
        pHandle->m_CustomPlacement.nOptions = nFlags;
        pHandle->m_CustomPlacement.hWndParent = hWndParent;
        if (szTitle)
            pHandle->m_CustomPlacement.sTitle = stringconversion::Convert_NativePtr_To_Wide(szTitle);
        else
            pHandle->m_CustomPlacement.sTitle.clear();
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCustomFileSave(OPENFILENAME* lpOpenFileName)
{
    LOG_FUNC_ENTRY_PARAMS((lpOpenFileName))
    // See if DLL Handle exists
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);

    pHandle->m_pofn = std::make_unique<OPENFILENAME>();
    memcpy(pHandle->m_pofn.get(), lpOpenFileName, sizeof(OPENFILENAME));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}




