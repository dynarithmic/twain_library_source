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
#include "ctltr015.h"
#include "ctlobtyp.h"
#include "twainfix32.h"

using namespace dynarithmic;

CTL_CapabilityGetRangeTriplet::CTL_CapabilityGetRangeTriplet(
                                    CTL_ITwainSession *pSession,
                                    CTL_ITwainSource* pSource,
                                    TW_UINT16 gType,
                                    TW_UINT16   gCap,
                                    TW_UINT16 TwainDataType=0xFFFF)

                       :  CTL_CapabilityGetTriplet(pSession,
                                                   pSource,
                                                   gType,
                                                   gCap,
                                                   TwainDataType),
                                                m_FirstVal{}, m_LastVal{}, m_StepVal{}, m_DefaultVal{}, m_CurrentVal{}
{
}


void CTL_CapabilityGetRangeTriplet::Decode( void *pCapData )
{
    EnumCapValues( pCapData );
}


bool CTL_CapabilityGetRangeTriplet::EnumCapValues( void *pCapData )
{
    RemoveAllTypeObjects();

    CTL_TwainTypeArray *pArray = GetTwainTypeArray();
    auto sessionHandle = GetSessionPtr()->GetTwainDLLHandle();
    // dereference to a TW_RANGE structure
    const pTW_RANGE pRange = static_cast<pTW_RANGE>(pCapData);

    // get item type
    const int nItemType = GetEffectiveItemType(pRange->ItemType);

    // get min value

    // Create a new object for this item
    const CTL_TwainTypeObPtr pOb = std::make_shared<CTL_TwainTypeOb>(sessionHandle, static_cast<TW_UINT16>(sizeof(TW_RANGE)), false);

    // Copy Data to pOb
    pOb->CopyData( static_cast<void*>(pRange) );

    // Store this object in object array
    pArray->push_back( pOb );

    // Get range stats
    if ( nItemType == TWTY_FIX32 )
    {
        auto pFixFirst = reinterpret_cast<pTW_FIX32>(&pRange->MinValue);
        auto pFixLast = reinterpret_cast<pTW_FIX32>(&pRange->MaxValue);
        auto pFixStep = reinterpret_cast<pTW_FIX32>(&pRange->StepSize);
        auto pFixDefVal = reinterpret_cast<pTW_FIX32>(&pRange->DefaultValue);
        auto pFixCurVal = reinterpret_cast<pTW_FIX32>(&pRange->CurrentValue);

        m_FirstVal.fval  = Fix32ToFloat( *pFixFirst );
        m_LastVal.fval   = Fix32ToFloat( *pFixLast  );
        m_StepVal.fval   = Fix32ToFloat( *pFixStep  );
        m_CurrentVal.fval = Fix32ToFloat( *pFixCurVal );
        m_DefaultVal.fval = Fix32ToFloat( *pFixDefVal );
    }
    else
    {
        m_FirstVal.ival = pRange->MinValue;
        m_LastVal.ival  = pRange->MaxValue;
        m_StepVal.ival  = pRange->StepSize;
        m_CurrentVal.ival = pRange->CurrentValue;
        m_DefaultVal.ival = pRange->DefaultValue;
    }

    m_nNumItems = m_nNumRangeItems;
    return true;
}


pTW_RANGE CTL_CapabilityGetRangeTriplet::GetRangePtr()
{
    const CTL_TwainTypeArray *pArray = GetTwainTypeArray();
    if (pArray->empty())
        return nullptr;
    const CTL_TwainTypeOb *pOb = pArray->front().get();
    return static_cast<pTW_RANGE>(pOb->GetDataRaw());
}


TW_UINT16 CTL_CapabilityGetRangeTriplet::GetDataType()
{
    const pTW_RANGE pRange = GetRangePtr();
    if ( !pRange )
        return static_cast<TW_UINT16>(-1);
    return pRange->ItemType;
}

bool CTL_CapabilityGetRangeTriplet::GetValue(void *pData, size_t nWhichVal)
{
    if ( nWhichVal >= m_nNumRangeItems )
        return false;
    const int nDataType = GetDataType();
    if ( nDataType == TWTY_FIX32 )
    {
        auto pFloat = static_cast<double*>(pData);
        switch (static_cast<CTL_EnumTwainRange>(nWhichVal))
        {
            case TwainRange_MIN:
                *pFloat = m_FirstVal.fval;
            break;
            case TwainRange_MAX:
                *pFloat = m_LastVal.fval;
            break;
            case TwainRange_STEP:
                *pFloat = m_StepVal.fval;
            break;
            case TwainRange_DEFAULT:
                *pFloat = m_DefaultVal.fval;
            break;
            case TwainRange_CURRENT:
                *pFloat = m_CurrentVal.fval;
            break;
            default:
                return false;
        }
        return true;
    }

    auto pInt32 = static_cast<pTW_UINT32>(pData);
    switch (static_cast<CTL_EnumTwainRange>(nWhichVal))
    {
        case TwainRange_MIN:
            *pInt32 = m_FirstVal.ival;
        break;
        case TwainRange_MAX:
            *pInt32 = m_LastVal.ival;
        break;
        case TwainRange_STEP:
            *pInt32 = m_StepVal.ival;
        break;
        case TwainRange_DEFAULT:
            *pInt32 = m_DefaultVal.ival;
        break;
        case TwainRange_CURRENT:
            *pInt32 = m_CurrentVal.ival;
        break;
        default:
            return false;
    }
    return true;
}



