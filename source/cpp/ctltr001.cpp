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
//#include <stdafx.h>
#include "ctltr001.h"
#include "ctltwainmanager.h"
using namespace dynarithmic;
//////////////////////////////////////////////////////////////////////////////
CTL_SourceTriplet::CTL_SourceTriplet(CTL_ITwainSession *pSession,
                                     LPCTSTR pProduct,
                                     TW_UINT16 nMsg)
                                     :  CTL_TwainTriplet(), m_bSourceCreated(false)
{
    SetSessionPtr( nullptr );
    SetSourcePtr(  nullptr );

    // Get the app manager's AppID
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

    if ( pMgr && pMgr->IsValidTwainSession( pSession ))
    {
        // Don't add this source to permanent list
        CTL_ITwainSource* pSource = CTL_ITwainSource::Create(pSession, pProduct);
        SetSourcePtr(pSource);
        m_bSourceCreated = true;
        SetSessionPtr(pSession);
        if ( pSource )
        {
            Init( pSession->GetAppIDPtr(), nullptr, DG_CONTROL, DAT_IDENTITY,
                  nMsg, static_cast<TW_MEMREF>(pSource->GetSourceIDPtr()) );
            SetAlive (true);
        }
    }
}


CTL_SourceTriplet::CTL_SourceTriplet(CTL_ITwainSession *pSession,
                                     CTL_ITwainSource* pSource,
                                     TW_UINT16 nMsg)
{
    SetSessionPtr(pSession);
    SetSourcePtr(pSource);
    m_bSourceCreated = false;
    // Get the app manager's AppID
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

    if ( pMgr && pMgr->IsValidTwainSession( pSession ))
    {
        // Don't add this source to permanent list
        Init( pSession->GetAppIDPtr(), nullptr, DG_CONTROL, DAT_IDENTITY,
              nMsg, static_cast<TW_MEMREF>(pSource->GetSourceIDPtr()) );
        SetAlive (true);
    }
}

