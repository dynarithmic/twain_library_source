/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#include "ctltrp.h"
#include "ctltwmgr.h"
#include "dtwain_resource_constants.h"

using namespace dynarithmic;
///////////////////////////////////////////////////////////////////////////////
/// Classes that do the TWAIN triplet protocol

CTL_TwainTriplet::CTL_TwainTriplet() : m_TwainTripletArg({nullptr, nullptr, {0,0,0}, nullptr}),
                                       m_bInit(false),
                                       m_bAlive(false),
                                       m_pSource(nullptr),
                                       m_pSession(nullptr)
{}

CTL_TwainTriplet::CTL_TwainTriplet(  pTW_IDENTITY pOrigin,
                                     pTW_IDENTITY pDest,
                                     TW_UINT32    nDG,
                                     TW_UINT16    nDAT,
                                     TW_UINT16    nMSG,
                                     TW_MEMREF    pData) : m_bInit(false),
                                                           m_bAlive(false),
                                                           m_pSource(nullptr),
                                                           m_pSession(nullptr)
{
    Init(pOrigin, pDest, nDG, nDAT,  nMSG, pData);
}

void CTL_TwainTriplet::Init( pTW_IDENTITY pOrigin,
                             pTW_IDENTITY pDest,
                             TW_UINT32    nDG,
                             TW_UINT16    nDAT,
                             TW_UINT16    nMSG,
                             TW_MEMREF    pData)
{
    m_TwainTripletArg = {pOrigin, pDest, {nDG, nDAT, nMSG}, pData};
    m_bInit     = true;
}

bool CTL_TwainTriplet::IsMSGGetType() const
{
    const TW_UINT16 msgType = GetMSG();
    return  msgType == MSG_GET ||
            msgType == MSG_GETCURRENT ||
            msgType == MSG_GETDEFAULT ||
            msgType == MSG_GETHELP ||
            msgType == MSG_GETLABEL ||
            msgType == MSG_GETLABELENUM;
}

bool CTL_TwainTriplet::IsMSGSetType() const
{
    const TW_UINT16 msgType = GetMSG();
    return msgType == MSG_SET || msgType == MSG_SETCONSTRAINT;
}

bool CTL_TwainTriplet::IsMSGResetType() const
{
    const TW_UINT16 msgType = GetMSG();
    return msgType == MSG_RESET || msgType == MSG_RESETALL;
}

TW_UINT16 CTL_TwainTriplet::Execute()
{
    // Get the Main App Ptr
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
    const CTL_TwainAppMgrPtr pTemp = pMgr;

    const TW_UINT16 nFail = TWRC_FAILURE;
    if ( !pTemp )
    {
        DTWAIN_ERROR_CONDITION(IDS_ErrTwainMgrInvalid, nFail);
    }
    if ( !m_bAlive )
    {
        DTWAIN_ERROR_CONDITION(IDS_ErrTripletNotExecuted, nFail);
    }
    return pTemp->CallDSMEntryProc( *this );
}

void CTL_TwainTriplet::SetAlive( bool bSet )
{
    m_bAlive = bSet;
}

bool CTL_TwainTriplet::IsAlive() const
{
    return m_bAlive;
}

