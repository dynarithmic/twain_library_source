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
#include "windowsinit_impl.h"
#include "monitorutils.h"
#include "dtwaindefs.h"
#include "ctlsourceselect.h"
#include "cppfunc.h"
#include "ctldtwainhandle.h"
#include "ctltwainlogging.h"
#include "ctlinternalconstants.h"
#include "ctltwaindllhandle.h"
#include "windowsinit_impl.h"
#include "dtwainx.h"
#include "ctliface.h"

namespace dynarithmic
{
    UINT_PTR APIENTRY FileSaveAsHookProc(HWND hWnd, UINT msg, WPARAM /*w*/, LPARAM lParam)
    {
        switch (msg)
        {
            case WM_INITDIALOG:
            {
                // Get the parameters
                const HWND ThisWnd = GetParent(hWnd);
                const OPENFILENAME* pofs = reinterpret_cast<OPENFILENAME*>(lParam);
                CustomPlacement* pCS = reinterpret_cast<CustomPlacement*>(pofs->lCustData);
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

    void RegisterTwainWindowClass()
    {
        CTL_StaticData::GetRegisteredMessage() = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
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
        wndclass.hInstance = CTL_StaticData::GetDLLInstanceHandle();
        wndclass.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
        wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndclass.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
        wndclass.lpszMenuName = nullptr;
        wndclass.lpszClassName = _T("DTWAINWindowClass");
        RegisterClass(&wndclass);
    }

    void UnregisterTwainWindowClass()
    {
        UnregisterClass(_T("DTWAINWindowClass"), CTL_StaticData::GetDLLInstanceHandle());
    }

    DTWAIN_BOOL DTWAIN_SetCallbackProc(DTWAIN_CALLBACK fnCall, LONG nWhich)
    {
        LOG_FUNC_ENTRY_PARAMS((fnCall, nWhich))
            // See if DLL Handle exists
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
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

    void DTWAIN_InvokeCallback(int nWhich, DTWAIN_HANDLE p, DTWAIN_SOURCE pSource, WPARAM lData1, LPARAM lData2)
    {
        DTWAIN_CALLBACK cProc;
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(p);
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
}

using namespace dynarithmic;

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
            CTL_StaticData::GetLogFilterFlags() = 0;
        }
        CTL_StaticData::SetDLLInstanceHandle(hinstDll);
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
