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
#ifndef SOURCESELECTOPTS_H
#define SOURCESELECTOPTS_H

#include "ctliface.h"

enum {SELECTSOURCE=1, SELECTDEFAULTSOURCE, SELECTSOURCEBYNAME, SELECTSOURCE2};

namespace dynarithmic
{
    struct SourceSelectionOptions : NotImpl<SourceSelectionOptions>
    {
        int nWhich;
        LPCTSTR szProduct;
        HWND hWndParent;
        LPCTSTR szTitle;
        LONG xPos;
        LONG yPos;
        LPCTSTR szIncludeNames;
        LPCTSTR szExcludeNames;
        LPCTSTR szNameMapping;
        LONG nOptions;
        int selectionResourceID;
        std::function<std::vector<TCHAR>(SelectStruct&)> getDefaultFunc;
        std::function<std::vector<CTL_StringType>(SelectStruct&)> getNameListFunc;

        SourceSelectionOptions(int n, int resourceID,  LPCTSTR sProd=nullptr, HWND parent=nullptr, LPCTSTR title=nullptr, LONG xP = 0, LONG yP = 0,
                               LPCTSTR sIncludeNames=nullptr, LPCTSTR sExcludeNames=nullptr, LPCTSTR sNameMapping=nullptr, LONG opt = 0) :
                               nWhich(n),
                               szProduct(sProd),
                               hWndParent(parent),
                               szTitle(title),
                               xPos(xP),
                               yPos(yP),
                               szIncludeNames(sIncludeNames),
                               szExcludeNames(sExcludeNames),
                               szNameMapping(sNameMapping),
                               nOptions(opt),
                               selectionResourceID(resourceID) {}
        friend OutputBaseStreamA& operator << (OutputBaseStreamA& strm, const SourceSelectionOptions& src);
        friend OutputBaseStreamW& operator << (OutputBaseStreamW& strm, const SourceSelectionOptions& src);
    };

    inline OutputBaseStreamA& operator << (OutputBaseStreamA& strm, const SourceSelectionOptions& src)
    {
        LPCSTR nuller = "null";
        strm << ("whichOption=") << src.nWhich
            << (", productName=") << (src.szProduct ? StringConversion::Convert_NativePtr_To_Ansi(src.szProduct) : nuller)
            << (", parentWindow=") << src.hWndParent
            << (", title=") << (src.szTitle ? StringConversion::Convert_NativePtr_To_Ansi(src.szTitle) : nuller)
            << (", xPos=") << src.xPos
            << (", yPos=") << src.yPos
            << (", includeNames=") << (src.szIncludeNames ? StringConversion::Convert_NativePtr_To_Ansi(src.szIncludeNames) : nuller)
            << (", excludeNames=") << (src.szExcludeNames ? StringConversion::Convert_NativePtr_To_Ansi(src.szExcludeNames) : nuller)
            << (", nameMapping=") << (src.szNameMapping ? StringConversion::Convert_NativePtr_To_Ansi(src.szNameMapping) : nuller)
            << (", options=") << src.nOptions;
        return strm;
    }

    inline OutputBaseStreamW& operator << (OutputBaseStreamW& strm, const SourceSelectionOptions& src)
    {
        LPCTSTR nuller = _T("null");
        strm << _T("whichOption=") << src.nWhich
            << _T(", productName=") << (src.szProduct ? src.szProduct : nuller)
            << _T(", parentWindow=") << src.hWndParent
            << _T(", title=") << (src.szTitle ? src.szTitle : nuller)
            << _T(", xPos=") << src.xPos
            << _T(", yPos=") << src.yPos
            << _T(", includeNames=") << (src.szIncludeNames ? src.szIncludeNames : nuller)
            << _T(", excludeNames=") << (src.szExcludeNames ? src.szExcludeNames : nuller)
            << _T(", nameMapping=") << (src.szNameMapping ? src.szNameMapping : nuller)
            << _T(", options=") << src.nOptions;
        return strm;
    }

    DTWAIN_SOURCE     DTWAIN_LLSelectSource(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& opts);
    DTWAIN_SOURCE     DTWAIN_LLSelectSourceByName(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& opts);
    DTWAIN_SOURCE     DTWAIN_LLSelectDefaultSource(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& opts);
    DTWAIN_SOURCE     DTWAIN_LLSelectSource2(CTL_TwainDLLHandle* pHandle, SourceSelectionOptions& opts);
    LRESULT CALLBACK  DisplayTwainDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    CTL_StringType    LLSelectionDialog(CTL_TwainDLLHandle* pHandle, const SourceSelectionOptions& opts);

    struct AcquireAttemptRAII
    {
        CTL_ITwainSource* m_pSource;
        AcquireAttemptRAII(CTL_ITwainSource* pSource) :m_pSource(pSource) {}
        ~AcquireAttemptRAII()
        {
            if (m_pSource->GetDTWAINHandle()->m_lAcquireMode == DTWAIN_MODAL) m_pSource->SetAcquireAttempt(false);
        }
    };

}
#endif
