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
#include "ctltwmgr.h"
#include "ctltr016.h"

#include "ctliface.h"

using namespace dynarithmic;

/////////////////////////////////////////////////////////////////////////////////////
CTL_CapabilitySetTripletBase::CTL_CapabilitySetTripletBase(CTL_ITwainSession *pSession,
                                                           CTL_ITwainSource* pSource,
                                                           CTL_EnumSetType sType,
                                                           TW_UINT16    sCap,
                                                           TW_UINT16 TwainType) :
                    CTL_CapabilityTriplet(pSession, pSource, static_cast<TW_UINT16>(sType), TwainType, false),
                        m_gType(sType), m_gCap(sCap), m_nTwainType(TwainType)
{
    TW_CAPABILITY *pCap = GetCapabilityBuffer();

    pCap->Cap = static_cast<TW_UINT16>(sCap);
    pCap->ConType = static_cast<TW_UINT16>(-1);
    pCap->hContainer = nullptr;
}

CTL_EnumSetType CTL_CapabilitySetTripletBase::CapSetType() const
{
    return m_gType;
}

CTL_EnumCapability CTL_CapabilitySetTripletBase::CapToSet() const
{
    return m_gCap;
}

TW_UINT16 CTL_CapabilitySetTripletBase::GetTwainType() const
{
    return m_nTwainType;
}

void * CTL_CapabilitySetTripletBase::PreEncode()
{
    TW_CAPABILITY *pCap = GetCapabilityBuffer();

    // Get the cap container
    const TW_UINT16 nContainerSize = GetContainerTypeSize();

    // Get the container type
    const TW_UINT16 nConType = GetContainerType();
    pCap->ConType = nConType;

    // Allocate proper amount of memory
    size_t nAggSize = GetAggregateSize();
    if ( nAggSize == 0 )
        nAggSize = 1;
    const auto dMem = static_cast<DWORD>(nContainerSize + GetItemSize(m_nTwainType) * nAggSize);

    pCap->hContainer = CTL_TwainDLLHandle::s_TwainMemoryFunc->AllocateMemory(dMem );
    return CTL_TwainDLLHandle::s_TwainMemoryFunc->LockMemory( pCap->hContainer );
}

TW_UINT16 CTL_CapabilitySetTripletBase::PostEncode(TW_UINT16 rc)
{
    const TW_CAPABILITY *pCap = GetCapabilityBuffer();
    CTL_TwainDLLHandle::s_TwainMemoryFunc->UnlockMemory( pCap->hContainer );
    CTL_TwainDLLHandle::s_TwainMemoryFunc->FreeMemory( pCap->hContainer );
    if ( rc != TWRC_SUCCESS )
    {
        const TW_UINT16 cc = CTL_TwainAppMgr::GetConditionCode( GetSessionPtr(), GetSourcePtr() );
        CTL_TwainAppMgr::ProcessConditionCodeError(cc);
    }
    return rc;
}

void CTL_CapabilitySetTripletBase::EncodeOneValue(pTW_ONEVALUE pVal, void *pData)
{
    // Do Fix32 special case
    switch (pVal->ItemType )
    {
        case TWTY_FIX32:
        {
            const float fnum = static_cast<float>(*static_cast<double*>(pData));
            TW_FIX32 ffix32;
            FloatToTwain32( fnum, ffix32 );
            memcpy(&pVal->Item, &ffix32, sizeof(TW_FIX32));
        }
        break;

        case TWTY_STR32:
        case TWTY_STR64:
        case TWTY_STR128:
        case TWTY_STR255:
        case TWTY_STR1024:
        {
            // The data is in the CTL_StringType type.  Must extract
            // Copy data to TW_CONTAINER
            // Make sure that string is fully null terminated
            TW_STR1024 TempString = {};
            auto ptrString = static_cast<std::string*>(pData);
            std::copy(ptrString->begin(), ptrString->end(), TempString);
            memcpy(&pVal->Item, TempString,GetItemSize( pVal->ItemType) );
        }
        break;

        default:
        // Copy data to TW_CONTAINER
            memcpy(&pVal->Item, pData, GetItemSize( pVal->ItemType) );
        break;
    }
}

void CTL_CapabilitySetTripletBase::EncodeEnumValue(pTW_ENUMERATION pArray,
                                                   int valuePos,
                                                   size_t nItemSize,
                                                   void *pData)
{
    if ( pArray->ItemType == TWTY_FIX32 )
    {
        // floats are stored as doubles in CTL
        const float fnum = static_cast<float>(*static_cast<double*>(pData));
        TW_FIX32 ffix32;
        FloatToTwain32( fnum, ffix32 );
        memcpy(&pArray->ItemList[valuePos], &ffix32, sizeof(TW_FIX32));
    }
    else
    if ( TwainUtils::IsTwainStringType(pArray->ItemType) )
    {
        TW_STR1024 TempString = {0};
        std::string *ptrString = reinterpret_cast<std::string*>(pData);
        std::copy(ptrString->begin(), ptrString->end(), TempString);
        memcpy(&pArray->ItemList[valuePos], TempString, GetItemSize( pArray->ItemType) );
    }
    else
        memcpy( &pArray->ItemList[valuePos], pData, nItemSize );
}


void CTL_CapabilitySetTripletBase::EncodeRange(pTW_RANGE pVal,
                                               void *pData1,
                                               void *pData2,
                                               void *pData3) const
{
    pVal->ItemType = GetTwainType();
    const size_t nItemSize = GetItemSize( pVal->ItemType );

    if ( pVal->ItemType == TWTY_FIX32 )
    {
        auto fnum = static_cast<float>(*static_cast<double*>(pData1));   // Min Value
        TW_FIX32 ffix32;
        FloatToTwain32( fnum, ffix32 );
        memcpy(&pVal->MinValue, &ffix32, sizeof(TW_FIX32));

        fnum = static_cast<float>(*static_cast<double*>(pData2));
        FloatToTwain32( fnum, ffix32 );
        memcpy(&pVal->MaxValue, &ffix32, sizeof(TW_FIX32));

        fnum = static_cast<float>(*static_cast<double*>(pData3));
        FloatToTwain32( fnum, ffix32 );
        memcpy(&pVal->StepSize, &ffix32, sizeof(TW_FIX32));
    }
    else
    {
        memcpy( &pVal->MinValue, pData1, nItemSize );
        memcpy( &pVal->MaxValue, pData2, nItemSize );
        memcpy( &pVal->StepSize, pData3, nItemSize );
    }

    pVal->DefaultValue = TWON_DONTCARE32;
}

void CTL_CapabilitySetTripletBase::EncodeArrayValue(pTW_ARRAY pArray,
                                                    size_t valuePos,
                                                    void *pData)
{
    // Get size of datatype
    const TW_UINT16 nItemSize = GetItemSize( pArray->ItemType );
    if ( pArray->ItemType == TWTY_FIX32 )
    {
        // floats are stored as doubles in CTL
        const float fnum = static_cast<float>(*static_cast<double*>(pData));
        TW_FIX32 ffix32;
        FloatToTwain32( fnum, ffix32 );
        memcpy(&pArray->ItemList[valuePos], &ffix32, sizeof(TW_FIX32));
    }
    else
    if ( TwainUtils::IsTwainStringType(pArray->ItemType) )
    {
        TW_STR1024 TempString = {0};
        const auto pStrData = static_cast<std::string*>(pData);
        std::copy(pStrData->begin(), pStrData->end(), TempString);
        memcpy(&pArray->ItemList[valuePos], TempString, GetItemSize( pArray->ItemType) );
    }
    else
        memcpy( &pArray->ItemList[valuePos], pData, nItemSize );
}


///////////////////////////////////////////////////////////////////////////////////
CTL_CapabilityResetTriplet::CTL_CapabilityResetTriplet(CTL_ITwainSession *pSession,
                                   CTL_ITwainSource* pSource,
                                   CTL_EnumCapability sCap,
                                   TW_UINT16 SetType /* = CTL_SetTypeRESET */) :
            CTL_CapabilityTriplet(pSession, pSource, SetType, 0, true)
{
    TW_CAPABILITY *pCap = GetCapabilityBuffer();

    pCap->Cap = static_cast<TW_UINT16>(sCap);
    pCap->ConType = TWON_DONTCARE16;
    pCap->hContainer = nullptr;
    SetTestMode( true );  // No decoding done for this triplet type
}

#ifdef USE_EXPLICIT_TEMPLATE_INSTANTIATIONS
    #include "ctltr016.inl"
    template  CTL_CapabilitySetTriplet<int>;
    template  CTL_CapabilitySetTriplet<long>;
    template  CTL_CapabilitySetTriplet<unsigned int>;
    template  CTL_CapabilitySetTriplet<unsigned long>;
    template  CTL_CapabilitySetTriplet<unsigned short>;
    template  CTL_CapabilitySetTriplet<double>;
    template  CTL_CapabilitySetTriplet<std::string>;
    template  CTL_CapabilitySetTriplet<char *>;
    template  CTL_CapabilitySetTriplet<TW_FRAME>;
    template  CTL_CapabilitySetTriplet<unsigned short *>;
    template  CTL_CapabilitySetTriplet<int *>;
    template  CTL_CapabilitySetTriplet<short *>;
#endif

