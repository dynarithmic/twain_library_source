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
#include "ctltr027.h"
#include "ctltwainmanager.h"

using namespace dynarithmic;
CTL_ImagePendingTriplet::CTL_ImagePendingTriplet(CTL_ITwainSession *pSession,
                                                 CTL_ITwainSource* pSource,
                                                 TW_UINT16 nMsg)
    : CTL_TwainTriplet(), m_nMsg(nMsg), m_PendingXfers{}
{
    InitGeneric(pSession, pSource, DG_CONTROL, DAT_PENDINGXFERS, m_nMsg, &m_PendingXfers);
}

bool CTL_ImagePendingTriplet::Reset( TW_UINT16 nMsg )
{
    CTL_ITwainSession *pSession = GetSessionPtr();
    CTL_ITwainSource *pSource = GetSourcePtr();
    if ( IsAlive() )
    {
        m_nMsg = nMsg;
        Init( pSession->GetAppIDPtr(),
              pSource->GetSourceIDPtr(),
              DG_CONTROL,
              DAT_PENDINGXFERS,
              m_nMsg,
              static_cast<TW_MEMREF>(&m_PendingXfers));
        return true;
    }
    return false;
}

TW_PENDINGXFERS* CTL_ImagePendingTriplet::GetPendingXferBuffer()
{
    return &m_PendingXfers;
}








