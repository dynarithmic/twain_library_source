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
#include <cstdio>
#include <cstring>
#include <sstream>
#include <boost/scope_exit.hpp>
#include <functional>
#include <algorithm>
#include "cppfunc.h"
#include "ctldib.h"
#include "ctltwainmanager.h"
#include "ctltrall.h"
#include "ctltmpl5.h"
#include "errorcheck.h"
#include "ctlutils.h"
#include "ctlsetgetcaps.h"
using namespace dynarithmic;

static bool GetCapValuesEx_Internal( CTL_ITwainSource* pSource, TW_UINT16 lCap,
                                     LONG lGetType, LONG lContainerType,
                                     LONG nDataType, LPDTWAIN_ARRAY pArray,
                                     bool bOverrideDataType );

template <typename T, typename ConvertTo=T>
struct NullGetCapConverter
{
    static ConvertTo Convert(T value) { return value; }
};

template <typename T, typename ConvertTo = T>
struct NullSetCapConverter
{
    static ConvertTo convert(T value, DTWAIN_ARRAY) { return value; }
};

struct StringGetCapConverter
{
    static CTL_StringType Convert(std::string& value)
    { return StringConversion::Convert_Ansi_To_Native(value, value.length()); }
};

struct StringSetCapConverter
{
    static std::string convert(CTL_StringType& value, DTWAIN_ARRAY)
    {
        return StringConversion::Convert_Native_To_Ansi(value, value.length());
    }
};

struct StringSetCapConverterA
{
    static std::string& convert(std::string& value, DTWAIN_ARRAY)
    {
        return value;
    }
};

struct FrameGetCapConverter
{
    static TwainFrameInternal Convert(TW_FRAME fValue)
    {
        return fValue;
    }
};

struct FrameSetCapConverter
{
    static TW_FRAME convert(TwainFrameInternal& fValue, DTWAIN_ARRAY )
    {
        return fValue.To_TWFRAME();
    }
};

template <typename DataType,
          typename ConvertTo = DataType,
          typename ConverterFn=NullGetCapConverter<DataType, ConvertTo> >
static DTWAIN_ARRAY PerformGetCap(CTL_ITwainSource* pSource,
                           TW_UINT16 lCap, LONG nDataType, LONG lContainerType,
                           LONG lGetType, LONG overrideDataType, CTL_ArrayType eType,
                           LONG oneCapFlag=0)
{
    DataType dValue = {};
    DTWAIN_ARRAY ThisArray = nullptr;
    auto pHandle = pSource->GetDTWAINHandle();
    if (overrideDataType == 0xFFFF)
    {
        const LONG nArrayType = GetArrayTypeFromCapType(static_cast<TW_UINT16>(nDataType));
        ThisArray = CreateArrayFromFactory(pHandle, nArrayType, 0);
        overrideDataType = nDataType;
    }
    else
        ThisArray = CreateArrayFromCap(pHandle, pSource, lCap, 0);

    if (!ThisArray)
        return nullptr;

    DTWAINArrayLowLevel_RAII raii(pHandle, ThisArray);

    DTWAIN_ARRAY pDTWAINArray = ThisArray;
    pHandle->m_ArrayFactory->clear(pDTWAINArray);
    int bOk = 0;
    if (lContainerType == DTWAIN_CONTONEVALUE)
    {
        bOk = GetOneCapValue<DataType>(pHandle,
                                        pSource,
                                        static_cast<UINT>(lCap),
                                        static_cast<TW_UINT16>(lGetType),
                                        oneCapFlag,
                                        &dValue,
                                        static_cast<TW_UINT16>(overrideDataType));
        if (!bOk)
            return nullptr;

        ConvertTo convValue = ConverterFn::Convert(dValue);
        pHandle->m_ArrayFactory->add_to_back(pDTWAINArray, &convValue, 1);
    }
    else
    {
        bOk = GetMultiCapValues(pHandle,
                                pSource,
                                pDTWAINArray,
                                eType,
                                static_cast<UINT>(lCap),
                                static_cast<TW_UINT16>(lGetType),
                                ConvertTo(),
                                static_cast<TW_UINT16>(overrideDataType),
                                static_cast<UINT>(lContainerType),
                                true,
                                DataType());
        if (!bOk)
            return nullptr;
    }
    raii.SetDestroy(false);
    return ThisArray;
}


template <typename DataType,
          typename TwainType,
          typename ConvertFrom = DataType,
          typename ConverterFn = NullSetCapConverter<DataType, ConvertFrom> >
static bool performSetCap(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source, TW_UINT16 lCap, DTWAIN_ARRAY pArray,
                          LONG lContainerType, LONG lSetType, LONG nDTWAIN_ArrayType, CTL_ArrayType eType,
                          LONG TwainTypeValue)
{
    bool bOk = false;
    DTWAIN_ARRAY pDTWAINArray = pArray;
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    if (lSetType != DTWAIN_CAPRESET && lSetType != DTWAIN_CAPRESETALL)
    {
        auto tagType = pHandle->m_ArrayFactory->tag_type(pArray);
        const bool isArrayOk = (tagType == nDTWAIN_ArrayType);
        if (!isArrayOk && CTL_CapabilityTriplet::IsCapOperationSet(lSetType))
            return false;
    }

    const bool isResetOp = CTL_CapabilityTriplet::IsCapOperationReset(lSetType);
    if (lContainerType == DTWAIN_CONTONEVALUE || isResetOp)
    {
        DataType lValue{};

        if (isResetOp)
            lValue = DataType();
        else
        {
            ConvertFrom lVal;
            pHandle->m_ArrayFactory->get_value(pDTWAINArray, 0, &lVal);
            lValue = ConverterFn::convert(lVal, nullptr);
        }

        bOk = SetOneCapValue(DLLHandle, Source, static_cast<TW_UINT16>(lCap), static_cast<TW_UINT16>(lSetType), lValue, static_cast<TW_UINT16>(TwainTypeValue))?true:false;
        return bOk;
    }
    else
    {
        bOk = SetMultiCapValues<DataType, ConvertFrom, ConverterFn>
            (DLLHandle, Source, pArray, eType, static_cast<UINT>(lCap), static_cast<TW_UINT16>(lSetType), static_cast<UINT>(lContainerType), true, static_cast<TW_UINT16>(TwainTypeValue))?true:false;

    }
    return bOk;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCapValues( DTWAIN_SOURCE Source, LONG lCap, LONG lGetType, LPDTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lGetType, pArray))
    const DTWAIN_BOOL bRet = DTWAIN_GetCapValuesEx2(Source, lCap, lGetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}


// Gets capability values.  This function does not test if the capability exists, or if the container type is valid.  Use
// with caution!!
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCapValuesEx( DTWAIN_SOURCE Source, LONG lCap, LONG lGetType, LONG lContainerType, LPDTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lGetType, lContainerType, pArray))
    const DTWAIN_BOOL bRet = DTWAIN_GetCapValuesEx2(Source, lCap, lGetType, lContainerType, DTWAIN_DEFAULT, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

// Gets capability values.  This function does not test if the capability exists, or if the container type is valid.  Use
// with caution!!
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetCapValuesEx2( DTWAIN_SOURCE Source, LONG lCap, LONG lGetType, LONG lContainerType,
                                                 LONG nDataType, LPDTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lGetType, lContainerType, nDataType,pArray))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    BOOL bRet = GetCapValuesEx2_Internal(pSource, lCap, lGetType, lContainerType, nDataType, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

// Gets capability values.  This function does not test if the capability exists, or if the container type is valid.  Use
// with caution!!
bool dynarithmic::GetCapValuesEx2_Internal( CTL_ITwainSource* pSource,
                                            LONG lCap, LONG lGetType, LONG lContainerType,
                                            LONG nDataType, LPDTWAIN_ARRAY pArray )
{
    DTWAIN_BOOL bRet = FALSE;
    bool overrideDataType = true;
    auto* pHandle = pSource->GetDTWAINHandle();

    if (nDataType == DTWAIN_DEFAULT)
    {
        nDataType = CTL_TwainAppMgr::GetDataTypeFromCap(static_cast<TW_UINT16 >(lCap), pSource);
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return nDataType < 0 || nDataType == (std::numeric_limits<int>::min)(); }, nDataType, false, FUNC_MACRO);
        overrideDataType = false;
    }
    return bRet = GetCapValuesEx_Internal(pSource, static_cast<TW_UINT16>(lCap), lGetType, lContainerType, nDataType, pArray, overrideDataType);
}

bool GetCapValuesEx_Internal( CTL_ITwainSource* pSource, TW_UINT16 lCap, LONG lGetType, LONG lContainerType,
                                     LONG nDataType, LPDTWAIN_ARRAY pArray, bool bOverrideDataType )
{
    LOG_FUNC_ENTRY_PARAMS((pSource, lCap, lGetType, lContainerType, nDataType, pArray, bOverrideDataType))

    const auto pHandle = pSource->GetDTWAINHandle();

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return pArray == nullptr; },
                                      DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    // We clear the user array here, since we do not want to 
    // report information back to user if capability is not supported
    bool bEnumeratorExists = pHandle->m_ArrayFactory->is_valid(*pArray);
    if ( bEnumeratorExists )
        pHandle->m_ArrayFactory->clear(*pArray);
    else
        *pArray = nullptr;

	CHECK_IF_CAP_SUPPORTED(pSource, pHandle, lCap, false)

    LONG overrideDataType = nDataType;
    if ( bOverrideDataType )
        overrideDataType = 0xFFFF;


    if( !pSource->IsCapNegotiableInState(static_cast<TW_UINT16>(lCap), pSource->GetState()) )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    DTWAIN_ARRAY ThisArray=nullptr;
    DTWAINArrayPtr_RAII arr(pHandle, &ThisArray);
    if( nDataType == 0xFFFF )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return nDataType == DTWAIN_CAPDATATYPE_UNKNOWN;},
                                        DTWAIN_ERR_UNKNOWN_CAPDATATYPE, false, FUNC_MACRO);

    // adjust the container types for GETHELP, GETLABEL, and GETLABELENUM
    switch (lGetType)
    {
        case DTWAIN_CAPGETHELP:
        case DTWAIN_CAPGETLABEL:
            lContainerType = DTWAIN_CONTONEVALUE;
        break;
        case DTWAIN_CAPGETLABELENUM:
            lContainerType = DTWAIN_CONTARRAY;
        break;
    }

    // get the default container type if specified
    if (lContainerType == DTWAIN_CONTDEFAULT)
    {
        switch (lGetType)
        {
            // skip these, as they're already set up
            case DTWAIN_CAPGETHELP:
            case DTWAIN_CAPGETLABEL:
            case DTWAIN_CAPGETLABELENUM:
            break;

            default:
                lContainerType = GetCapContainer(pSource, lCap, lGetType);
            break;
        }
    }

    if (lGetType == DTWAIN_CAPGETHELP || lGetType == DTWAIN_CAPGETLABEL)
        ThisArray = PerformGetCap<HANDLE>(pSource, lCap, nDataType, lContainerType, GetTwainGetType(lGetType), overrideDataType, CTL_ArrayHandleType);
    else
    if ( lGetType == DTWAIN_CAPGETLABELENUM )
        ThisArray = PerformGetCap<std::string>(pSource, lCap, nDataType, lContainerType, GetTwainGetType(lGetType), overrideDataType, CTL_ArrayStringType);
    else
    {
        if (dynarithmic::IsTwainIntegralType(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<LONG>(pSource, lCap, nDataType, lContainerType, lGetType, overrideDataType, CTL_ArrayIntType);
            if ( !ThisArray )
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
        if (dynarithmic::IsTwainFix32Type(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<double>(
                pSource, lCap, nDataType, lContainerType, lGetType, overrideDataType,
                CTL_ArrayDoubleType);
            if (!ThisArray)
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
        if (dynarithmic::IsTwainStringType(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<std::string/*, NullGetCapConverter*/>
                        (pSource, lCap, nDataType, lContainerType, lGetType, overrideDataType, CTL_ArrayANSIStringType);
            if (!ThisArray)
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
        if (dynarithmic::IsTwainFrameType(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<TW_FRAME, TwainFrameInternal, FrameGetCapConverter>
                (pSource, lCap, nDataType, lContainerType, lGetType, overrideDataType, CTL_ArrayDTWAINFrameType);
            if (!ThisArray)
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    if (lCap >= CAP_CUSTOMBASE)
    {
        // Save the data type information in the general cap information structure,
        // since the getting of the cap values was successful.
        switch (lGetType)
        {
            case DTWAIN_CAPGET:
                SetCapabilityInfo<CAPINFO_IDX_GETCONTAINER>(pHandle, pSource, lContainerType, lCap);
            break;
            case DTWAIN_CAPGETCURRENT:
                SetCapabilityInfo<CAPINFO_IDX_GETCURRENTCONTAINER>(pHandle, pSource, lContainerType, lCap);
            break;
            case DTWAIN_CAPGETDEFAULT:
                SetCapabilityInfo<CAPINFO_IDX_GETDEFAULTCONTAINER>(pHandle, pSource, lContainerType, lCap);
            break;
        }
        SetCapabilityInfo<CAPINFO_IDX_DATATYPE>(pHandle, pSource, nDataType, lCap);
    }

    arr.SetDestroy(false);
    if ( bEnumeratorExists )
        pHandle->m_ArrayFactory->destroy(*pArray);
    *pArray = ThisArray;
    DumpArrayContents(*pArray, lCap);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


// Sets capability values.  This function does not test if the
// capability exists, or if the container type is valid.  Use
// with caution!!
bool dynarithmic::SetCapValuesEx2_Internal( CTL_ITwainSource* pSource, LONG lCap, LONG lSetType, LONG lContainerType,
                                            LONG nDataType, DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((pSource, lCap, lSetType, lContainerType, nDataType, pArray))
    auto pHandle = pSource->GetDTWAINHandle();

    bool bOk = false;
    CHECK_IF_CAP_SUPPORTED(pSource, pHandle, static_cast<TW_UINT16>(lCap), false)

    if (nDataType == DTWAIN_DEFAULT)
    {
        nDataType = DTWAIN_GetCapDataType(pSource, static_cast<TW_UINT16 >(lCap));
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return nDataType < 0; }, nDataType, false, FUNC_MACRO);
    }

    if ( !CTL_CapabilityTriplet::IsCapOperationReset(lSetType) )
    {
        bool bFoundType = false;

        // Get the array type, given the tag type of the DTWAIN Array
        const LONG DTwainArrayType = CTL_ArrayFactory::tagtype_to_arraytype(pHandle->m_ArrayFactory->tag_type(pArray));

        const auto it1 = pHandle->m_mapDTWAINArrayToTwainType.find(DTwainArrayType);
        if ( it1 != pHandle->m_mapDTWAINArrayToTwainType.end() )
        {
            // Search the array for the Twain Type
            const std::vector<LONG>::iterator it2 =
                std::find(it1->second.begin(), it1->second.end(), nDataType);
            if ( it2 != it1->second.end())
                bFoundType = true;
        }
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !bFoundType;}, DTWAIN_ERR_UNKNOWN_CAPDATATYPE, false, FUNC_MACRO);
    }
    if( nDataType == 0xFFFF )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return nDataType == DTWAIN_CAPDATATYPE_UNKNOWN;}, DTWAIN_ERR_UNKNOWN_CAPDATATYPE, false, FUNC_MACRO);

    LONG TestContainer;

    if( lContainerType == DTWAIN_CONTDEFAULT )
        TestContainer = GetCapContainer(pSource, lCap, lSetType); 
    else
        TestContainer = lContainerType;

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return TestContainer == 0 && !CTL_CapabilityTriplet::IsCapOperationReset(lSetType);}, DTWAIN_ERR_CAPSET_NOSUPPORT, false, FUNC_MACRO);

    if( lContainerType == DTWAIN_CONTDEFAULT )
        lContainerType = TestContainer;

    if (lSetType == DTWAIN_CAPSETCURRENT)
        lSetType = DTWAIN_CAPSET;
    else
    if (lSetType == DTWAIN_CAPSETCONSTRAINT)
        lSetType = MSG_SETCONSTRAINT;

    DumpArrayContents(pArray, lCap);

    // If the container type has multiple options, we will try each container until we get an ok or we get a failure of all attempts of setting the
    // capability.
    auto allContainers = dynarithmic::getSetBitsAsVector(lContainerType);
    for (auto& containerType : allContainers)
    {
        if (dynarithmic::IsTwainIntegralType(static_cast<TW_UINT16>(nDataType)))
            bOk = performSetCap<LONG, TW_UINT32>(pHandle, pSource, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::LongType, CTL_ArrayIntType, nDataType);
        else
        if (dynarithmic::IsTwainFix32Type(static_cast<TW_UINT16>(nDataType)))
            bOk = performSetCap<double, double>(pHandle, pSource, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::DoubleType, CTL_ArrayDoubleType, nDataType);
        else
        if (dynarithmic::IsTwainStringType(static_cast<TW_UINT16>(nDataType)))
            bOk = performSetCap<std::string, std::string, std::string, StringSetCapConverterA>
                            (pHandle, pSource, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::StringType, CTL_ArrayANSIStringType, nDataType);
        else
        if (dynarithmic::IsTwainFrameType(static_cast<TW_UINT16>(nDataType)))
        {
            bOk = performSetCap<TW_FRAME, TW_FRAME, TwainFrameInternal, FrameSetCapConverter>
            (pHandle, pSource, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::FrameSingleType, CTL_ArrayDTWAINFrameType /*CTL_ArrayTWFrameType*/, nDataType);
        }
        if (bOk)
        {
            if (lCap >= CAP_CUSTOMBASE)
            {
                // Since this is a custom capability, we will save the information that worked (capability type and
                // container type)
                switch (lSetType)
                {
                    case DTWAIN_CAPSET:
                    case DTWAIN_CAPSETCURRENT:
                        SetCapabilityInfo<CAPINFO_IDX_SETCONTAINER>(pHandle, pSource, containerType, lCap);
                    break;
                    case DTWAIN_CAPSETAVAILABLE:
                    case DTWAIN_CAPSETCONSTRAINT:
                        SetCapabilityInfo<CAPINFO_IDX_SETCONSTRAINTCONTAINER>(pHandle, pSource, containerType, lCap);
                    break;
                }
                SetCapabilityInfo<CAPINFO_IDX_DATATYPE>(pHandle, pSource, nDataType, lCap);
            }
            break; // get out now
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bOk)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCapValues( DTWAIN_SOURCE Source, LONG lCap, LONG lSetType, DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lSetType, pArray))
    const DTWAIN_BOOL bRet = DTWAIN_SetCapValuesEx2(Source, lCap, lSetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCapValuesEx2( DTWAIN_SOURCE Source, LONG lCap, LONG lSetType, LONG lContainerType,
                                                LONG nDataType, DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lSetType, lContainerType, nDataType, pArray))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    bool bRet = dynarithmic::SetCapValuesEx2_Internal(pSource, lCap, lSetType, lContainerType, nDataType, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCapValuesEx( DTWAIN_SOURCE Source, LONG lCap, LONG lSetType, LONG lContainerType,
                                                DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lSetType, lContainerType, pArray))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    DTWAIN_BOOL bRet = FALSE;
    auto nDataType = CTL_TwainAppMgr::GetDataTypeFromCap(static_cast<TW_UINT16 >(lCap), pSource);
    if ( nDataType < 0)
        DTWAIN_Check_Error_Condition_2_Ex(pHandle, [&] { return true;} , DTWAIN_ERR_BAD_CAP, false, FUNC_MACRO);
    else
        bRet = dynarithmic::SetCapValuesEx2_Internal(pSource, lCap, lSetType, lContainerType, nDataType, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

