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
#include "ctltr032.h"
#include "ctltwainmanager.h"
#include "twainfix32.h"

using namespace dynarithmic;
CTL_ImageLayoutTriplet::CTL_ImageLayoutTriplet(CTL_ITwainSession *pSession,
                                               CTL_ITwainSource* pSource,
                                               TW_UINT16 GetType)
                                               : CTL_TwainTriplet(), m_ImageLayout{}
{
    SetSessionPtr(pSession);
    SetSourcePtr( pSource );

    // Get the app manager's AppID
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
    if ( pMgr && pMgr->IsValidTwainSession( pSession ))
    {
        if ( pSource )
        {
            Init( pSession->GetAppIDPtr(),
                  pSource->GetSourceIDPtr(),
                  DG_IMAGE,
                  DAT_IMAGELAYOUT,
                  GetType,
                  static_cast<TW_MEMREF>(&m_ImageLayout));
            SetAlive (true);
        }
    }
}


double CTL_ImageLayoutTriplet::GetLeft() const
{
    return Fix32ToFloat(m_ImageLayout.Frame.Left);
}


double CTL_ImageLayoutTriplet::GetRight() const
{
    return Fix32ToFloat(m_ImageLayout.Frame.Right);
}


double CTL_ImageLayoutTriplet::GetTop() const
{
    return Fix32ToFloat(m_ImageLayout.Frame.Top );
}


double CTL_ImageLayoutTriplet::GetBottom() const
{
    return Fix32ToFloat(m_ImageLayout.Frame.Bottom);
}


TW_UINT32 CTL_ImageLayoutTriplet::GetDocumentNumber() const
{
    return m_ImageLayout.DocumentNumber;
}


TW_UINT32 CTL_ImageLayoutTriplet::GetPageNumber() const
{
    return m_ImageLayout.PageNumber;
}


TW_UINT32 CTL_ImageLayoutTriplet::GetFrameNumber() const
{
    return m_ImageLayout.FrameNumber;
}

TW_FRAME CTL_ImageLayoutTriplet::GetFrame() const
{
    return m_ImageLayout.Frame;
}
////////////////////////////////////////////////////////////
CTL_ImageSetLayoutTriplet::CTL_ImageSetLayoutTriplet(
                               CTL_ITwainSession *pSession,
                               CTL_ITwainSource* pSource,
                               const CTL_RealArray &rArray,
                               TW_UINT16 SetType) :
CTL_ImageLayoutTriplet( pSession, pSource, SetType )
{
    TW_IMAGELAYOUT *pLayout = GetImageLayout();
    if ( SetType != MSG_RESET)
    {
        pLayout->Frame.Left = FloatToFix32(static_cast<float>(rArray[0]));
        pLayout->Frame.Top =  FloatToFix32(static_cast<float>(rArray[1]));
        pLayout->Frame.Right = FloatToFix32(static_cast<float>(rArray[2]));
        pLayout->Frame.Bottom = FloatToFix32(static_cast<float>(rArray[3]));
    }
    pLayout->DocumentNumber = static_cast<TW_UINT32>(-1);
    pLayout->PageNumber = static_cast<TW_UINT32>(-1);
    pLayout->FrameNumber = static_cast<TW_UINT32>(-1);
}


