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

#include "monitorutils.h"
#include "dtwaindefs.h"

namespace dynarithmic
{
    HWND GetEffectiveConsoleWindow()
    {
        // 1) Already attached?
        HWND hwnd = GetConsoleWindow();
        if (hwnd)
            return hwnd;

        // 2) Try attaching to parent console
        if (AttachConsole(ATTACH_PARENT_PROCESS))
        {
            hwnd = GetConsoleWindow();
            if (hwnd)
                return hwnd;
        }
        return NULL;
    }

    bool CenterWindow(HWND hWnd, HWND hParent)
    {
        if (!hParent)
            hParent = GetDesktopWindow();

        RECT rcChild;
        RECT rcParent;

        GetWindowRect(hWnd, &rcChild);     // SCREEN coords
        GetWindowRect(hParent, &rcParent); // SCREEN coords

        int childW = rcChild.right - rcChild.left;
        int childH = rcChild.bottom - rcChild.top;

        int parentW = rcParent.right - rcParent.left;
        int parentH = rcParent.bottom - rcParent.top;

        int x = rcParent.left + (parentW - childW) / 2;
        int y = rcParent.top + (parentH - childH) / 2;

        SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        return true;
    }

    HMONITOR GetPreferredMonitor(HWND hDialog, int options)
    {
        if (options & DTWAIN_DLG_CONSOLEASPARENT)
        {
            HWND hConsole = GetConsoleWindow();
            if (hConsole)
            {
                return MonitorFromWindow(hConsole, MONITOR_DEFAULTTONEAREST);
            }
        }
        return MonitorFromWindow(hDialog, MONITOR_DEFAULTTONEAREST);
    }

    void CenterWindowOnMonitor(HWND hWnd, HMONITOR hMonitor)
    {
        MONITORINFO mi = { sizeof(mi) };
        GetMonitorInfo(hMonitor, &mi);

        RECT rcWork = mi.rcWork;

        RECT rcWindow;
        GetWindowRect(hWnd, &rcWindow);

        int windowWidth = rcWindow.right - rcWindow.left;
        int windowHeight = rcWindow.bottom - rcWindow.top;

        int x = rcWork.left +
            ((rcWork.right - rcWork.left) - windowWidth) / 2;

        int y = rcWork.top +
            ((rcWork.bottom - rcWork.top) - windowHeight) / 2;

        SetWindowPos(hWnd, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
    }

    void CenterWindowSmart(HWND hWnd, int options)
    {
        HMONITOR hMonitor = GetPreferredMonitor(hWnd, options);
        CenterWindowOnMonitor(hWnd, hMonitor);
    }
}