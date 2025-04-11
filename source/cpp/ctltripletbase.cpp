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
#include "ctltripletbase.h"
#include "ctltwainmanager.h"

using namespace dynarithmic;
///////////////////////////////////////////////////////////////////////////////
/// Classes that do the TWAIN triplet protocol

CTL_TwainTriplet::CTL_TwainTriplet() : m_TwainTripletArg({ {}, {}, {{},{},{}}, {}}),
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

// High-level initialization of triplet components.  
bool CTL_TwainTriplet::InitGeneric(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource, 
                                   TW_UINT32 nDG, TW_UINT16 nDat, TW_UINT16 MsgType, TW_MEMREF pType, 
                                    std::pair<bool, bool> prInit)
{
    // Optionally set session and source pointers
    if ( prInit.first )
        SetSessionPtr(pSession);
    if ( prInit.second )
        SetSourcePtr(pSource);

    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

    // Only Initialize if App Manager is valid
    if (pMgr && pMgr->IsValidTwainSession(pSession))
    {
        Init(pSession?pSession->GetAppIDPtr():nullptr,
            pSource?pSource->GetSourceIDPtr():nullptr,
            nDG,
            nDat,
            MsgType,
            pType);
        SetAlive(true);
        return true;
    }
    return false;
}


void CTL_TwainTriplet::Init( const pTW_IDENTITY pOrigin,
                             const pTW_IDENTITY pDest,
                             TW_UINT32 nDG,
                             TW_UINT16 nDAT,
                             TW_UINT16 nMSG,
                             TW_MEMREF pData)
{
    m_TwainTripletArg = {pOrigin, pDest, {nDG, nDAT, nMSG}, pData};
    m_bInit     = true;
}

bool CTL_TwainTriplet::IsMSGGetType() const
{
    return ::IsMSGGetType(GetMSG());
}

bool CTL_TwainTriplet::IsMSGSetType() const
{
    return ::IsMSGSetType(GetMSG());
}

bool CTL_TwainTriplet::IsMSGResetType() const
{
    return ::IsMSGResetType(GetMSG());
}

bool CTL_TwainTriplet::IsMSGSetOrResetType() const
{
    return ::IsMSGSetOrResetType(GetMSG());
}

TW_UINT16 CTL_TwainTriplet::Execute()
{
    // Get the Main App Ptr
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

    const TW_UINT16 nFail = TWRC_FAILURE;
    if ( !pMgr )
    {
        DTWAIN_ERROR_CONDITION(IDS_ErrTwainMgrInvalid, nFail, true)
    }
    if ( !m_bAlive )
    {
        DTWAIN_ERROR_CONDITION(IDS_ErrTripletNotExecuted, nFail, false)
    }
    return pMgr->CallDSMEntryProc( *this );
}

void CTL_TwainTriplet::SetAlive( bool bSet )
{
    m_bAlive = bSet;
}

bool CTL_TwainTriplet::IsAlive() const
{
    return m_bAlive;
}
