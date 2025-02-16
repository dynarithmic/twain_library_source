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
#include "ctltr010.h"

#include "ctliface.h"
#include "ctltwainmanager.h"
#include "twainfix32.h"
using namespace dynarithmic;

CTL_CapabilityTriplet::CTL_CapabilityTriplet(CTL_ITwainSession *pSession,
                                             CTL_ITwainSource* pSource,
                                             TW_UINT16 nMsg,
                                             TW_UINT16 TwainType,
                                             bool bReading)
                       :  CTL_TwainTriplet(),
                            m_Capability(),
                            m_bReading(bReading),
                            m_bTesting(false),
                            m_bSupported(true),
                            m_nItemType(TwainType),
                            m_bGetDefaultType(false)
{
    SetSessionPtr( pSession );
    SetSourcePtr( pSource );

    // Get the app manager's AppID
    const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
    if ( pMgr && pMgr->IsValidTwainSession( pSession ))
    {
        if ( pSource )
        {
            Init( pSession->GetAppIDPtr(), pSource->GetSourceIDPtr(),
                    DG_CONTROL, DAT_CAPABILITY, nMsg,
                    static_cast<TW_MEMREF>(&m_Capability));
            SetAlive (true);
        }
    }
}

CTL_CapabilityTriplet::~CTL_CapabilityTriplet()
{
    RemoveAllTypeObjects();
}

void CTL_CapabilityTriplet::RemoveAllTypeObjects()
{
    m_ObArray.clear();
 }

bool CTL_CapabilityTriplet::IsCapabilitySupported()
{
    const bool oldTesting = m_bTesting;
    m_bTesting = true;
    // Call base class
    const TW_UINT16 rc = Execute();
    m_bTesting = oldTesting;
    // Return FALSE if capability is not supported
    if ( rc != TWRC_SUCCESS )
        return false;

    return true;
}


TW_UINT16 CTL_CapabilityTriplet::Execute()
{
    RemoveAllTypeObjects();
    // Call base class
    TW_UINT16 rc = CTL_TwainTriplet::Execute();

    if ( rc != TWRC_SUCCESS )
    {
        m_bSupported = FALSE;
        return rc;
    }

    // Determine return type
    // Get the pointer to the data returned and let it decode
    // if the capability is to be read
    auto memfnptr = GetSessionPtr()->GetTwainDLLHandle()->m_TwainMemoryFunc;
    if ( m_bTesting ) // Don't decode if in test mode
    {
        if ( m_Capability.hContainer)
        {
            if ( m_Capability.Cap >= CAP_CUSTOMBASE )
            {
                // Need to specially decode this to determine ItemType
                // Get pointer to data
                void *pCapData = static_cast<void*>(memfnptr->LockMemory(m_Capability.hContainer));

                // dereference to a TW_ONEVALUE structure.  Don't really
                // care if item is not really TW_ONEVALUE since first
                // item in structure is the same for all types (the item type)
                const pTW_ONEVALUE pValOne = static_cast<pTW_ONEVALUE>(pCapData);

                // Get item type
                const TW_UINT16 nItemType = pValOne->ItemType;

                SetItemType(nItemType);
            }

            if (memfnptr == &GetSessionPtr()->GetTwainDLLHandle()->m_TwainLegacyFunc)
            {
                #ifdef _WIN32
                const UINT nCount = GlobalFlags(m_Capability.hContainer) & GMEM_LOCKCOUNT;
                for ( UINT i = 0; i < nCount; i++ )
                      GlobalUnlock(m_Capability.hContainer);
                GlobalFree( m_Capability.hContainer );
                #endif
            }
            else
            {
                memfnptr->UnlockMemory(m_Capability.hContainer);
                memfnptr->FreeMemory(m_Capability.hContainer);
            }
        }
    }
    else
    if ( m_bReading )
    {
        if ( !m_Capability.hContainer )
        {
            // No capability data
            rc = 1;
            CTL_TwainAppMgr::ProcessConditionCodeError(-TWAIN_ERR_NULL_CONTAINER);
            return rc;
        }

        try
        {
            const TW_MEMREF p = memfnptr->LockMemory( m_Capability.hContainer );
            Decode(p);
            memfnptr->UnlockMemory(m_Capability.hContainer);
            memfnptr->FreeMemory( m_Capability.hContainer ); // Test
        }
        catch(...)
        {
            return TWRC_FAILURE;
        }
    }
    return rc;
}

void CTL_CapabilityTriplet::Decode( void * /*p*/ )
{}

bool CTL_CapabilityTriplet::IsReading() const
{
    return m_bReading;
}

void CTL_CapabilityTriplet::SetTestMode( bool bSet/* = true */)
{
    m_bTesting = bSet;
}


bool CTL_CapabilityTriplet::IsTesting() const
{
    return m_bTesting;
}

TW_CAPABILITY*  CTL_CapabilityTriplet::GetCapabilityBuffer()
{
    return &m_Capability;
}

CTL_TwainTypeArray* CTL_CapabilityTriplet::GetTwainTypeArray()
{
    return &m_ObArray;
}
////////////////////////////////////////////////////////////////////////////
bool CTL_CapabilityTriplet::IsSupported()
{
    return m_bSupported;
}
