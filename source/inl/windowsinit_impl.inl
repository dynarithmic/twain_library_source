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
static UINT_PTR APIENTRY FileSaveAsHookProc(HWND hWnd, UINT msg, WPARAM /*w*/, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    {
        // Get the parameters
        const HWND ThisWnd = GetParent(hWnd);
        const OPENFILENAME *pofs = reinterpret_cast<OPENFILENAME *>(lParam);
        CustomPlacement *pCS = reinterpret_cast<CustomPlacement *>(pofs->lCustData);
        if (pCS->nOptions & DTWAIN_DLG_CENTER_SCREEN)
            CenterWindow(ThisWnd, nullptr);
        else
            if (pCS->nOptions & DTWAIN_DLG_CENTER)
                CenterWindow(ThisWnd, pCS->hWndParent);
            else
                SetWindowPos(ThisWnd, nullptr, pCS->xpos, pCS->ypos, 0, 0, SWP_NOSIZE);
    }
    break;
    }
    return TRUE;
}

bool dynarithmic::CenterWindow(HWND hwnd, HWND hwndParent)
{
    RECT rect, rectP;

    if (!hwndParent)
        hwndParent = GetDesktopWindow();
    GetWindowRect(hwnd, &rect);
    GetWindowRect(hwndParent, &rectP);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    int x = (rectP.right - rectP.left - width) / 2 + rectP.left;
    int y = (rectP.bottom - rectP.top - height) / 2 + rectP.top;

    int screenwidth = GetSystemMetrics(SM_CXSCREEN);
    int screenheight = GetSystemMetrics(SM_CYSCREEN);

    //make sure that the dialog box never moves outside of
    //the screen
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width  > screenwidth)  x = screenwidth - width;
    if (y + height > screenheight) y = screenheight - height;

    // Convert x,y to screen and then to parent coordinates

    MoveWindow(hwnd, x, y, width, height, FALSE);
    return true;
}

////////// Function to subclass the window ////////////////////////
WNDPROC SubclassTwainMsgWindow(HWND hWnd, WNDPROC wProcIn /*=NULL*/)
{
    WNDPROC wProc = nullptr;
    WNDPROC wProcToUse = nullptr;
    if (wProcIn == nullptr)
        wProcToUse = static_cast<WNDPROC>(DTWAIN_WindowProc);
    else
        wProcToUse = wProcIn;
    if (IsWindow(hWnd))
    {
        if (reinterpret_cast<WNDPROC>(::GetWindowLongPtr(hWnd, GWLP_WNDPROC)) != wProcToUse)
        {
            wProc = TWSubclassWindow(hWnd, wProcToUse);
            if (!wProc)
                LogWin32Error(GetLastError());
        }
    }
    // Already equal, so return the original
    else
        wProc = wProcToUse;
    return wProc;
}


static HWND CreateTwainWindow(CTL_TwainDLLHandle * /*pHandle*/,
    HINSTANCE hInstance/*=NULL*/,
    HWND hWndParent)
{
    if (hInstance == nullptr)
        hInstance = CTL_StaticData::s_DLLInstance;
    HWND hWndP;
    if (!hWndParent)
        hWndP = GetDesktopWindow();
    else
        hWndP = hWndParent;

    RECT rect;

    GetWindowRect(hWndP, &rect);
    const HWND hwnd = CreateWindow(_T("DTWAINWindowClass"),              // class
        _T("Twain Window"),                 // title
        WS_OVERLAPPED,    // style
        0, 0,   // x, y
        0, 0,   // width, height
        hWndParent ? hWndP : NULL,
        NULL,                            // hmenu
        hInstance,
        NULL);                          // lpvparam
    return hwnd;
}

void dynarithmic::DTWAIN_InvokeCallback(int nWhich, DTWAIN_HANDLE p, DTWAIN_SOURCE pSource, WPARAM lData1, LPARAM lData2)
{
    DTWAIN_CALLBACK cProc;
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(p);
    if (pHandle)
    {
        switch (nWhich)
        {
        case DTWAIN_CallbackMESSAGE:
            cProc = pHandle->m_CallbackMsg;
            break;
        case DTWAIN_CallbackERROR:
            cProc = pHandle->m_CallbackError;
            break;
        default:
            return;
        }

        if (!cProc)
            return;

        (*cProc)(pHandle, pSource, lData1, lData2);
    }
}

void RegisterTwainWindowClass()
{
    CTL_StaticData::s_nRegisteredDTWAINMsg = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
    WNDCLASS wndclass;
    memset(&wndclass, 0, sizeof(WNDCLASS));
#ifdef DTWAIN_LIB
    wndclass.style = 0;
#else
    wndclass.style = CS_GLOBALCLASS;
#endif
    wndclass.lpfnWndProc = DTWAIN_WindowProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hInstance = CTL_StaticData::s_DLLInstance;
    wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    wndclass.lpszMenuName = nullptr;
    wndclass.lpszClassName = _T("DTWAINWindowClass");
    RegisterClass(&wndclass);
}

void UnregisterTwainWindowClass()
{
    UnregisterClass(_T("DTWAINWindowClass"), static_cast<HINSTANCE>(CTL_StaticData::s_DLLInstance));
}

#ifndef DTWAIN_LIB
BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID /*plvReserved*/)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    {
        if (fdwReason == DLL_PROCESS_ATTACH)
        {
            CTL_StaticData::s_logFilterFlags = 0;
        }
        CTL_StaticData::s_DLLInstance = hinstDll;
    }
    return TRUE;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:

#ifndef DTWAIN_RETAIL
        if (fdwReason == DLL_PROCESS_DETACH)
        {
            DTWAINScopedLogController sLogContoller(0);
            if (GetDTWAINHandle_Internal())
                DTWAIN_SysDestroy();
        }
#endif
        return TRUE;
    }
    return TRUE;
}
#endif

std::string dynarithmic::LogWin32Error(DWORD lastError)
{
    LPSTR lpMsgBuf = nullptr;

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        nullptr,
        lastError,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        (LPSTR)&lpMsgBuf,
        0,
        nullptr
    );

    // Display the string.
    std::string sError = lpMsgBuf;
    while (!sError.empty())
    {
        if (std::iscntrl(sError.back()))
            sError.pop_back();
        else
            break;
    }
    StringStreamA strm;
    strm << boost::format("Win32 Error: %1% (%2%)") % lastError % sError;
    CTL_TwainAppMgr::WriteLogInfoA(strm.str());

    // Free the buffer.
    LocalFree(lpMsgBuf);

    return strm.str();
}

void LogDTWAINErrorToMsgBox(int nError, LPCSTR func, const std::string& s)
{
    StringStreamA strm;
    if (!func)
        func = "(Uninitialized DTWAIN DLL)";
    strm << "DTWAIN Function " << func << " returned error code " << nError << std::endl << std::endl;
    strm << s.c_str();
    const std::string st = strm.str();
    MessageBoxA(nullptr, st.c_str(), "DTWAIN Error", MB_ICONSTOP);
}

void dynarithmic::LogToDebugMonitorA(std::string sMsg)
{
    if (sMsg.back() != '\n')
        sMsg.push_back('\n');
    OutputDebugStringA(sMsg.c_str());
}

void dynarithmic::LogToDebugMonitorW(std::wstring sMsg)
{
    if (sMsg.back() != L'\n')
        sMsg.push_back(L'\n');
    OutputDebugStringW(sMsg.c_str());
}

void dynarithmic::LogToDebugMonitor(CTL_StringType sMsg)
{
    if (sMsg.back() != StringWrapper::traits_type::GetNewLineChar())
        sMsg.push_back(StringWrapper::traits_type::GetNewLineChar());
    ::OutputDebugString(sMsg.c_str());
}

DTWAIN_BOOL dynarithmic::DTWAIN_SetCallbackProc(DTWAIN_CALLBACK fnCall, LONG nWhich)
{
    LOG_FUNC_ENTRY_PARAMS((fnCall, nWhich))
        // See if DLL Handle exists
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    switch (nWhich)
    {
    case DTWAIN_CallbackERROR:
        pHandle->m_CallbackError = fnCall;
        break;

    case DTWAIN_CallbackMESSAGE:
        pHandle->m_CallbackMsg = fnCall;
        break;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileSavePos(HWND hWndParent, LPCTSTR szTitle, LONG xPos, LONG yPos, LONG nFlags)
{
    LOG_FUNC_ENTRY_PARAMS((hWndParent, szTitle, xPos, yPos, nFlags))
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
            pHandle->m_CustomPlacement.sTitle.clear();
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCustomFileSave(OPENFILENAME* lpOpenFileName)
{
    LOG_FUNC_ENTRY_PARAMS((lpOpenFileName))
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
	pHandle->m_pofn = std::make_unique<OPENFILENAME>();
	memcpy(pHandle->m_pofn.get(), lpOpenFileName, sizeof(OPENFILENAME));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

