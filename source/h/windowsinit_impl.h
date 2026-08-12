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
#ifndef WINDOWSINITIMPL_H
#define WINDOWSINITIMPL_H
#include "dtwain_standard_defs.h"
#include "dtwainc.h"

namespace dynarithmic
{
    UINT_PTR APIENTRY FileSaveAsHookProc(HWND hWnd, UINT msg, WPARAM w, LPARAM lparam);
    void RegisterTwainWindowClass();
    void UnregisterTwainWindowClass();
    void DTWAIN_InvokeCallback(int nWhich, DTWAIN_HANDLE p, DTWAIN_SOURCE pSource, WPARAM lData1, LPARAM lData2);
    DTWAIN_BOOL DTWAIN_SetCallbackProc(DTWAIN_CALLBACK fnCall, LONG nWhich);
    void DTWAIN_InvokeCallback( int nWhich, DTWAIN_HANDLE pHandle, DTWAIN_SOURCE pSource, WPARAM lData1, LPARAM lData2 );
}
#endif
