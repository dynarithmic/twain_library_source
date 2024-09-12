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
void LogDTWAINErrorToMsgBox(int nError, LPCSTR func, const std::string& s)
{
    CTL_StringStreamType strm;
    CTL_StringType funcstr;
    if (func)
        funcstr = func;
    else
        func = _T("(Uninitialized DTWAIN DLL)");
    strm << _T("DTWAIN Function ") << func << _T(" returned error code ") << nError << std::endl << std::endl;
    strm << s.c_str();
    CTL_StringType st = strm.str();
    std::cerr << StringConversion::Convert_Native_To_Ansi(st) << "\n";
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePos(HWND hWndParent, LPCTSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nFlags))
    #if 0
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if (nFlags & DTWAIN_DLG_CLEAR_PARAMS)
        pHandle->m_CustomPlacement.nOptions = 0;
    else
    {
        pHandle->m_CustomPlacement.xpos = xPos;
        pHandle->m_CustomPlacement.ypos = yPos;
        pHandle->m_CustomPlacement.nOptions = nFlags;
        pHandle->m_CustomPlacement.hWndParent = hWndParent;
        if (szTitle)
            pHandle->m_CustomPlacement.sTitle = szTitle;
        else
            pHandle->m_CustomPlacement.sTitle = _T("");
    }
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCustomFileSave(OPENFILENAME* lpOpenFileName)
{
    LOG_FUNC_ENTRY_PARAMS(lpOpenFileName)
    #if 0
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_pofn = lpOpenFileName;
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

void dynarithmic::LogToDebugMonitorA(std::string sMsg)
{
    if (sMsg.back() != '\n')
        sMsg.push_back('\n');
    ErrorStream::StreamMe(sMsg);
}

void dynarithmic::LogToDebugMonitorW(std::wstring sMsg)
{
    if (sMsg.back() != L'\n')
        sMsg.push_back(L'\n');
    ErrorStream::StreamMe(sMsg);
}

void dynarithmic::LogToDebugMonitor(CTL_StringType sMsg)
{
    if (sMsg.back() != StringWrapper::traits_type::GetNewLineChar())
        sMsg.push_back(StringWrapper::traits_type::GetNewLineChar());
    ErrorStream::StreamMe(sMsg);
}