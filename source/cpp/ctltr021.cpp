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
#include "ctltr021.h"
#include "ctltwainmanager.h"

using namespace dynarithmic;

CTL_UserInterfaceTriplet::CTL_UserInterfaceTriplet(CTL_ITwainSession *pSession,
                                                   CTL_ITwainSource* pSource,
                                                   TW_UINT16 nMsg,
                                                   TW_USERINTERFACE *pTWUI,
                                                   TW_BOOL bShowUI/*=TRUE*/)
                       :  CTL_TwainTriplet(), m_pUserInterface(pTWUI)
{
    SetSessionPtr(pSession);
    SetSourcePtr( pSource );
    m_pUserInterface->ShowUI  = bShowUI;
    m_pUserInterface->ModalUI = 0;
    const HWND *pWnd = pSession->GetWindowHandlePtr();

    m_pUserInterface->hParent = static_cast<TW_HANDLE>(*pWnd);
    // Get the app manager's AppID
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
    if ( pMgr && pMgr->IsValidTwainSession( pSession ))
    {
        if ( pSource )
        {
            Init( pSession->GetAppIDPtr(), pSource->GetSourceIDPtr(),
                    DG_CONTROL, DAT_USERINTERFACE, nMsg,
                    static_cast<TW_MEMREF>(m_pUserInterface));
            SetAlive (true);
        }
    }
}
