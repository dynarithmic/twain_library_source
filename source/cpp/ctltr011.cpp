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
#include <sstream>
#include "ctltr011.h"

#include "ctliface.h"
#include "ctltwainmanager.h"
#include "cppfunc.h"
using namespace dynarithmic;
CTL_CapabilityGetTriplet::CTL_CapabilityGetTriplet(CTL_ITwainSession* pSession,
                                                    CTL_ITwainSource* pSource,
                                                    CTL_EnumGetType gType,
                                                    TW_UINT16 gCap,
                                                    TW_UINT16 TwainDataType)
                       :  CTL_CapabilityTriplet(pSession, pSource, static_cast<TW_UINT16>(gType), TwainDataType, true)
                       , m_gCap(gCap), m_gType(gType)
{
    TW_CAPABILITY* pCap = GetCapabilityBuffer();
    pCap->Cap = gCap;
    pCap->ConType = TWON_DONTCARE16;
    pCap->hContainer = nullptr;
    m_nContainerToUse = TwainContainer_INVALID;
    SetItemType(TwainDataType);
}

CTL_EnumGetType CTL_CapabilityGetTriplet::CapRetrievalType()const
{
    return m_gType;
}

CTL_EnumCapability CTL_CapabilityGetTriplet::CapToRetrieve()const
{
    return m_gCap;
}

TW_UINT16 CTL_CapabilityGetTriplet::GetEffectiveItemType(TW_UINT16 curDataType) const
{
    const TW_UINT16 itemType = GetItemType();
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
    {
        StringStreamA strm;
        strm << "Getting cap value: " << m_gCap << "  requested item type: " << itemType <<
                "  Observed item type: " << curDataType;
        LogWriterUtils::WriteLogInfoIndentedA(strm.str());

        std::string msg = "Observed and requested data types " + std::string(itemType ==
            curDataType ? "equal" : "not equal");

        LogWriterUtils::WriteLogInfoIndentedA(msg);
    }

    // If we're the same, or if we must return the correct type as returned by the Source ...
    if (itemType == curDataType || itemType == 0xFFFF)
        return curDataType;

    // return whatever the itemtype is overriding the curDataType
    return itemType;
}

TW_UINT16 CTL_CapabilityGetTriplet::Execute()
{
    const TW_UINT16 rc = CTL_CapabilityTriplet::Execute();
    if (rc == TWRC_SUCCESS)
    {
        if (IsTesting())
        {
            switch (GetCapabilityBuffer()->ConType)
            {
                case TWON_ONEVALUE:
                    m_nContainerToUse = TwainContainer_ONEVALUE;
                    break;
                case TWON_ENUMERATION:
                    m_nContainerToUse = TwainContainer_ENUMERATION;
                    break;
                case TWON_ARRAY:
                    m_nContainerToUse = TwainContainer_ARRAY;
                    break;
                case TWON_RANGE:
                    m_nContainerToUse = TwainContainer_RANGE;
                    break;
            }
        }
    }
    return rc;
}

void CTL_CapabilityGetTriplet::Decode(void * p)
{
    // determine what kind of retrieval was done
    switch (m_gType)
    {
        // Get all capabilities
        case CTL_GetTypeGET:
        case CTL_GetTypeGETCURRENT:
        case CTL_GetTypeGETDEFAULT:
        case CTL_GetTypeQUERYSUPPORT:
            EnumCapValues(p);
            return ;
        default:
            return;
    }
}
