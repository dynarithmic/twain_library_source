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
using namespace dynarithmic;

static DTWAIN_BOOL DTWAIN_GetCapValuesEx_Internal( DTWAIN_SOURCE Source, TW_UINT16 lCap,
                                                   LONG lGetType, LONG lContainerType,
                                                   LONG nDataType, LPDTWAIN_ARRAY pArray,
                                                   bool bOverrideDataType );

// Overload for string caps
static int GetMultiCapValues(DTWAIN_HANDLE DLLHandle, 
                        DTWAIN_SOURCE Source,
                        DTWAIN_ARRAY pArray,
                        CTL_ArrayType,
                        TW_UINT16 nCap,
                        TW_UINT16 GetType,
                        CTL_StringType AValue,
                        TW_UINT16 TwainDataType,
                        UINT nContainerVal/* = 0*/,
                        bool bUseContainer/* = false */,
                        std::string dummy = std::string()
                        );

// overload for TW_FRAME caps
static int GetMultiCapValues(DTWAIN_HANDLE DLLHandle,
                        DTWAIN_SOURCE Source,
                        DTWAIN_ARRAY pArray,
                        CTL_ArrayType EnumType,
                        TW_UINT16 nCap,
                        TW_UINT16 GetType,
                        DTWAIN_FRAME frm,
                        TW_UINT16 TwainDataType,
                        UINT nContainerVal,
                        bool bUseContainer,
                        TW_FRAME dummy);

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
static DTWAIN_ARRAY PerformGetCap(DTWAIN_HANDLE DLLHandle, DTWAIN_SOURCE Source,
                           TW_UINT16 lCap, LONG nDataType, LONG lContainerType,
                           LONG lGetType, LONG overrideDataType, CTL_ArrayType eType,
                           LONG oneCapFlag=0)
{
    DataType dValue = {};
    DTWAIN_ARRAY ThisArray = nullptr;
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(DLLHandle);
    if (overrideDataType == 0xFFFF)
    {
        const LONG nArrayType = GetArrayTypeFromCapType(static_cast<TW_UINT16>(nDataType));
        ThisArray = CreateArrayFromFactory(pHandle, nArrayType, 0);
        overrideDataType = nDataType;
    }
    else
        ThisArray = CreateArrayFromCap(pHandle, static_cast<CTL_ITwainSource*>(Source), lCap, 0);

    if (!ThisArray)
        return nullptr;

    DTWAINArrayLowLevel_RAII raii(pHandle, ThisArray);

    DTWAIN_ARRAY pDTWAINArray = ThisArray;
    pHandle->m_ArrayFactory->clear(pDTWAINArray);
    int bOk = 0;
    if (lContainerType == DTWAIN_CONTONEVALUE)
    {
        bOk = GetOneCapValue<DataType>(DLLHandle,
                                        Source,
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
        bOk = GetMultiCapValues(DLLHandle,
                                Source,
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
    DTWAIN_BOOL bRet = FALSE;
    bool overrideDataType = true;
    if (nDataType == DTWAIN_DEFAULT)
    {
        nDataType = CTL_TwainAppMgr::GetDataTypeFromCap(static_cast<TW_UINT16 >(lCap), pSource);
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return nDataType < 0 || nDataType == (std::numeric_limits<int>::min)(); }, nDataType, false, FUNC_MACRO);
        overrideDataType = false;
    }
    bRet = DTWAIN_GetCapValuesEx_Internal(Source, static_cast<TW_UINT16>(lCap), lGetType, lContainerType, nDataType, pArray, overrideDataType);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DTWAIN_GetCapValuesEx_Internal( DTWAIN_SOURCE Source, TW_UINT16 lCap, LONG lGetType, LONG lContainerType,
                                            LONG nDataType, LPDTWAIN_ARRAY pArray, bool bOverrideDataType )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lGetType, lContainerType, nDataType, pArray, bOverrideDataType))

    CTL_ITwainSource* p = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = p->GetDTWAINHandle();

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return pArray == nullptr; },
                                      DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    // We clear the user array here, since we do not want to 
    // report information back to user if capability is not supported
    bool bEnumeratorExists = pHandle->m_ArrayFactory->is_valid(*pArray);
    if ( bEnumeratorExists )
        pHandle->m_ArrayFactory->clear(*pArray);
    else
        *pArray = nullptr;

	CHECK_IF_CAP_SUPPORTED(p, pHandle, lCap, false)

    LONG overrideDataType = nDataType;
    if ( bOverrideDataType )
        overrideDataType = 0xFFFF;


    if( !p->IsCapNegotiableInState(static_cast<TW_UINT16>(lCap), p->GetState()) )
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
                lContainerType = GetCapContainer(p, lCap, lGetType);
            break;
        }
    }

    if (lGetType == DTWAIN_CAPGETHELP || lGetType == DTWAIN_CAPGETLABEL)
        ThisArray = PerformGetCap<HANDLE>(pHandle, Source, lCap, nDataType, lContainerType, GetTwainGetType(lGetType), overrideDataType, CTL_ArrayHandleType);
    else
    if ( lGetType == DTWAIN_CAPGETLABELENUM )
        ThisArray = PerformGetCap<std::string>(pHandle, Source, lCap, nDataType, lContainerType, GetTwainGetType(lGetType), overrideDataType, CTL_ArrayStringType);
    else
    {
        if (dynarithmic::IsTwainIntegralType(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<LONG>(pHandle, Source, lCap, nDataType, lContainerType, lGetType, overrideDataType, CTL_ArrayIntType);
            if ( !ThisArray )
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
        if (dynarithmic::IsTwainFix32Type(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<double>(
                pHandle, Source, lCap, nDataType, lContainerType, lGetType, overrideDataType,
                CTL_ArrayDoubleType);
            if (!ThisArray)
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
        if (dynarithmic::IsTwainStringType(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<std::string/*, NullGetCapConverter*/>
                        (pHandle, Source, lCap, nDataType, lContainerType, lGetType, overrideDataType, CTL_ArrayANSIStringType);
            if (!ThisArray)
                LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }
        else
        if (dynarithmic::IsTwainFrameType(static_cast<TW_UINT16>(nDataType)))
        {
            ThisArray = PerformGetCap<TW_FRAME, TwainFrameInternal, FrameGetCapConverter>
                (pHandle, Source, lCap, nDataType, lContainerType, lGetType, overrideDataType, CTL_ArrayDTWAINFrameType);
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
                SetCapabilityInfo<CAPINFO_IDX_GETCONTAINER>(pHandle, Source, lContainerType, lCap);
            break;
            case DTWAIN_CAPGETCURRENT:
                SetCapabilityInfo<CAPINFO_IDX_GETCURRENTCONTAINER>(pHandle, Source, lContainerType, lCap);
            break;
            case DTWAIN_CAPGETDEFAULT:
                SetCapabilityInfo<CAPINFO_IDX_GETDEFAULTCONTAINER>(pHandle, Source, lContainerType, lCap);
            break;
        }
        SetCapabilityInfo<CAPINFO_IDX_DATATYPE>(pHandle, Source, nDataType, lCap);
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
static DTWAIN_BOOL SetCapValuesEx2_Internal( DTWAIN_SOURCE Source, LONG lCap, LONG lSetType, LONG lContainerType,
                                             LONG nDataType, DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCap, lSetType, lContainerType, nDataType, pArray))
    auto pSource = static_cast<CTL_ITwainSource*>(Source);
    auto pHandle = pSource->GetDTWAINHandle();

    bool bOk = false;
    CHECK_IF_CAP_SUPPORTED(pSource, pHandle, static_cast<TW_UINT16>(lCap), false)

    if (nDataType == DTWAIN_DEFAULT)
    {
        nDataType = DTWAIN_GetCapDataType(Source, static_cast<TW_UINT16 >(lCap));
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

    // Change to the real TWAIN set-constraint type

    DumpArrayContents(pArray, lCap);

    // If the container type has multiple options, we will try each container until we get an ok or we get a failure of all attempts of setting the
    // capability.
    auto allContainers = dynarithmic::getSetBitsAsVector(lContainerType);
    for (auto& containerType : allContainers)
    {
        if (dynarithmic::IsTwainIntegralType(static_cast<TW_UINT16>(nDataType)))
            bOk = performSetCap<LONG, TW_UINT32>(pHandle, Source, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::LongType, CTL_ArrayIntType, nDataType);
        else
        if (dynarithmic::IsTwainFix32Type(static_cast<TW_UINT16>(nDataType)))
            bOk = performSetCap<double, double>(pHandle, Source, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::DoubleType, CTL_ArrayDoubleType, nDataType);
        else
        if (dynarithmic::IsTwainStringType(static_cast<TW_UINT16>(nDataType)))
            bOk = performSetCap<std::string, std::string, std::string, StringSetCapConverterA>
                            (pHandle, Source, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::StringType, CTL_ArrayANSIStringType, nDataType);
        else
        if (dynarithmic::IsTwainFrameType(static_cast<TW_UINT16>(nDataType)))
        {
            bOk = performSetCap<TW_FRAME, TW_FRAME, TwainFrameInternal, FrameSetCapConverter>
            (pHandle, Source, static_cast<TW_UINT16>(lCap), pArray, containerType, lSetType, CTL_ArrayFactory::arrayTag::FrameSingleType, CTL_ArrayDTWAINFrameType /*CTL_ArrayTWFrameType*/, nDataType);
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
                        SetCapabilityInfo<CAPINFO_IDX_SETCONTAINER>(pHandle, Source, containerType, lCap);
                    break;
                    case DTWAIN_CAPSETAVAILABLE:
                    case DTWAIN_CAPSETCONSTRAINT:
                        SetCapabilityInfo<CAPINFO_IDX_SETCONSTRAINTCONTAINER>(pHandle, Source, containerType, lCap);
                    break;
                }
                SetCapabilityInfo<CAPINFO_IDX_DATATYPE>(pHandle, Source, nDataType, lCap);
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
    VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    bool bRet = SetCapValuesEx2_Internal(Source, lCap, lSetType, lContainerType, nDataType, pArray);
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
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return nDataType < 0;} , DTWAIN_ERR_BAD_CAP, false, FUNC_MACRO);
    bRet = SetCapValuesEx2_Internal(Source, lCap, lSetType, lContainerType, nDataType, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

/////////////////////////////////////////////////////////////
// Custom cap data functions
#include "ctltr036.h"

HANDLE DLLENTRY_DEF DTWAIN_GetCustomDSData( DTWAIN_SOURCE Source, LPBYTE Data, DWORD dSize, LPDWORD pActualSize, LONG nFlags )
{
    LOG_FUNC_ENTRY_PARAMS((Source, Data, dSize, pActualSize, nFlags))
    const bool bSupported = DTWAIN_IsCapSupported(Source, CAP_CUSTOMDSDATA)?true:false;

    if( !bSupported )
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)

    auto *p = static_cast<CTL_ITwainSource*>(Source);
    // Call TWAIN to get the custom data
    const auto pSession = p->GetTwainSession();
    CTL_GetCustomDSTriplet DST(pSession, p);

    // Get the custom data
    const int ret = DST.Execute();

    // return if TWAIN cannot complete this request
    if( ret != TWRC_SUCCESS )
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)

    // Copy actual size data to parameter
    if( pActualSize )
        *pActualSize = DST.GetDataSize();
    auto localActualSize = DST.GetDataSize();

    // Get the returned handle from TWAIN
    const HANDLE h = DST.GetData();

    // Return the handle if that is all user wants to do
    if( nFlags & DTWAINGCD_RETURNHANDLE )
        LOG_FUNC_EXIT_NONAME_PARAMS(h)

    // Copy data to user's data area.
    if( Data && (nFlags & DTWAINGCD_COPYDATA))
    {
        const char *pData = static_cast<char *>(ImageMemoryHandler::GlobalLock(h));
        auto nMinCopy = (std::max)((std::min<DWORD>)(dSize, localActualSize), 0UL);
        memcpy(Data, pData, nMinCopy);
        ImageMemoryHandler::GlobalUnlock(h);
        ImageMemoryHandler::GlobalFree(h);
        LOG_FUNC_EXIT_NONAME_PARAMS(HANDLE(1))
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((Data, pActualSize))
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK(HANDLE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCustomDSData( DTWAIN_SOURCE Source, HANDLE hData, const BYTE* Data, DWORD dSize, LONG nFlags )
{
    LOG_FUNC_ENTRY_PARAMS((Source, hData, Data, dSize, nFlags))
    const bool bSupported = DTWAIN_IsCapSupported(Source, CAP_CUSTOMDSDATA)?true:false;

    if (!bSupported)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    auto p = static_cast<CTL_ITwainSource*>(Source);

    // Set up triplet for CUSTOMDSDATA call
    const auto pSession = p->GetTwainSession();
    auto pHandle = p->GetDTWAINHandle();

    CTL_SetCustomDSTriplet DST(pSession, p);

    // Check what options the user wants to do
    char *pData = nullptr;

    // Set data to the handle passed in
    if( nFlags & DTWAINSCD_USEHANDLE )
        DST.SetData(hData, dSize);
    else
    if( dSize == -1 )
    {
        if( !DTWAIN_GetCustomDSData(Source, nullptr, 0, &dSize, DTWAINGCD_COPYDATA) )
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }

    // Set data to the data passed in
    DTWAINGlobalHandleUnlockFree_RAII memHandler;

    if( Data && (nFlags & DTWAINSCD_USEDATA ))
    {
        // Allocate local copy of handle
        pData = static_cast<char*>(ImageMemoryHandler::GlobalAllocPr(GMEM_DDESHARE, dSize));
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return pData == NULL; }, DTWAIN_ERR_OUT_OF_MEMORY, false, FUNC_MACRO);

        // Make sure memory is cleaned up at the end
        memHandler.reset(ImageMemoryHandler::GlobalHandle(pData));
        memcpy(pData, Data, dSize);
        DST.SetData(ImageMemoryHandler::GlobalHandle(pData), dSize);
    }

    // Call TWAIN
    const int ret = DST.Execute();

    // return TRUE or FALSE depending on return code of TWAIN
    if( ret != TWRC_SUCCESS )
    {
        pHandle->m_lLastError = -(IDS_TWRC_ERRORSTART + ret);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireStripSizes( DTWAIN_SOURCE Source, LPDWORD lpMin, LPDWORD lpMax, LPDWORD lpPreferred )
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpMin, lpMax, lpPreferred))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    TW_SETUPMEMXFER Xfer;
    const bool bRet = CTL_TwainAppMgr::GetMemXferValues(pSource, &Xfer)?true:false;

    if( bRet )
    {
        if( lpMin )
            *lpMin = Xfer.MinBufSize;

        if( lpMax )
            *lpMax = Xfer.MaxBufSize;

        if( lpPreferred )
            *lpPreferred = Xfer.Preferred;
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpMin, lpMax, lpPreferred))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireStripBuffer(DTWAIN_SOURCE Source, HANDLE hMem)
{
    LOG_FUNC_ENTRY_PARAMS((Source,hMem))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    if ( !hMem )
    {
        pSource->SetUserStripBuffer(nullptr);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    else
    {
        const SIZE_T dSize = ImageMemoryHandler::GlobalSize( hMem );
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !dSize; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
        pSource->SetUserStripBuffer(hMem);
        pSource->SetUserStripBufSize(dSize);
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireStripSize(DTWAIN_SOURCE Source, DWORD StripSize)
{
    LOG_FUNC_ENTRY_PARAMS((Source, StripSize))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    DWORD MinSize, MaxSize;
    if ( StripSize == 0 )
    {
        pSource->SetUserStripBufSize(StripSize);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }

    if ( DTWAIN_GetAcquireStripSizes(Source, &MinSize, &MaxSize, nullptr) )
    {
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return StripSize < MinSize || StripSize > MaxSize;},
                                                    DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    else
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{ return true;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    pSource->SetUserStripBufSize(StripSize);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

HANDLE DLLENTRY_DEF DTWAIN_GetAcquireStripBuffer(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const HANDLE h = pSource->GetUserStripBuffer();
    LOG_FUNC_EXIT_NONAME_PARAMS(h)
    CATCH_BLOCK_LOG_PARAMS(HANDLE())
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireStripData(DTWAIN_SOURCE Source, LPLONG lpCompression, LPDWORD lpBytesPerRow,
                                                    LPDWORD lpColumns, LPDWORD lpRows, LPDWORD XOffset,
                                                    LPDWORD YOffset, LPDWORD lpBytesWritten)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpCompression, lpBytesPerRow,lpColumns, lpRows, XOffset,YOffset, lpBytesWritten))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const TW_IMAGEMEMXFER* pBuffer = pSource->GetBufferStripData();

    if ( lpCompression )
        *lpCompression = pBuffer->Compression;
    if ( lpBytesPerRow)
        *lpBytesPerRow = pBuffer->BytesPerRow;
    if ( lpColumns )
        *lpColumns = pBuffer->Columns;
    if ( lpRows )
        *lpRows = pBuffer->Rows;
    if ( XOffset )
        *XOffset = pBuffer->XOffset;
    if ( YOffset )
        *YOffset = pBuffer->YOffset;
    if ( lpBytesWritten)
        *lpBytesWritten = pBuffer->BytesWritten;
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpCompression, lpBytesPerRow, lpColumns, lpRows, XOffset, YOffset, lpBytesWritten))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

template <typename T>
struct StreamerImpl
{
    static void streamMe(OutputBaseStreamA* strm, size_t* pCur, T& val)
    { 
        *strm << "Array[" << *pCur << "] = " << val << "\n"; 
    }
};

struct StreamerImplFrame
{
    CTL_OutputBaseStreamType* m_pStrm;
    size_t* m_pCurItem;
    StreamerImplFrame(CTL_OutputBaseStreamType* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }

    void operator()(const TwainFrameInternal& pPtr) const
    {
        *m_pStrm << _T("Array[") << *m_pCurItem << _T("]");
        *m_pStrm << _T("{left=" <<   pPtr.m_FrameComponent[0] << _T("}\n"));
        *m_pStrm << _T("{top=" <<    pPtr.m_FrameComponent[1] << _T("}\n"));
        *m_pStrm << _T("{right=" <<  pPtr.m_FrameComponent[2] << _T("}\n"));
        *m_pStrm << _T("{bottom=" << pPtr.m_FrameComponent[3] << _T("}\n"));
        ++*m_pCurItem;
    }
};

struct StreamerImplNativeString
{
    CTL_OutputBaseStreamType* m_pStrm;
    size_t* m_pCurItem;
    StreamerImplNativeString(CTL_OutputBaseStreamType* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }

    void operator()(CTL_StringType& pPtr) const
    {
        *m_pStrm << pPtr << _T("\n");
        ++*m_pCurItem;
    }
};

template <typename T, typename StreamFn = StreamerImpl<T> >
struct oStreamer
{

    OutputBaseStreamA* m_pStrm;
    size_t* m_pCurItem;
    oStreamer(OutputBaseStreamA* strm, size_t* curItem) : m_pStrm(strm), m_pCurItem(curItem) { *curItem = 0; }
    void operator()(T& n)
    {
        StreamFn::streamMe(m_pStrm, m_pCurItem, n);
        ++*m_pCurItem;
    }
};

template <typename T>
static void genericDumper(DTWAIN_ARRAY Array)
{
    // Get the array contents as a vector
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<typename T::value_type>(Array);

    StringStreamA strm;
    size_t n;

    std::for_each(vCaps.begin(), vCaps.end(), oStreamer<typename T::value_type>(&strm, &n));
    LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
}

static void DumpArrayLONG(DTWAIN_ARRAY Array, LONG lCap)
{
    if ( lCap != CAP_SUPPORTEDCAPS )
        genericDumper<CTL_ArrayFactory::tagged_array_long>(Array);

    else
    {
        // Get the array contents as a vector
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& vCaps = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Array);

        StringStreamA strm;
        size_t n;
        strm << "\n";

        // if the cap is for supported caps, then output the strings.
        // vector of names
        std::vector<std::string> CapNames;

        // get the vector of cap names given cap number
        std::transform(vCaps.begin(), vCaps.end(), std::back_inserter(CapNames),
                        [] (LONG n) {return CTL_TwainAppMgr::GetCapNameFromCap(n);});

        // stream the cap information from the cap names
        std::for_each(CapNames.begin(), CapNames.end(), oStreamer<std::string>(&strm, &n));
        LogWriterUtils::WriteMultiLineInfoIndentedA(strm.str(), "\n");
    }
}

static void DumpArrayFLOAT(DTWAIN_ARRAY Array)
{
    genericDumper<CTL_ArrayFactory::tagged_array_double>(Array);
}
static void DumpArrayWideString(DTWAIN_ARRAY Array);
static void DumpArrayAnsiString(DTWAIN_ARRAY Array);

static void DumpArrayNativeString(DTWAIN_ARRAY Array)
{
#ifdef _UNICODE
    DumpArrayWideString(Array);
#else
    DumpArrayAnsiString(Array);
#endif
}

static void DumpArrayWideString(DTWAIN_ARRAY Array)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<std::wstring>(Array);
    std::wstring allValues = StringWrapperW::Join(vData.begin(), vData.end(), L"\n");
    LogWriterUtils::WriteMultiLineInfoIndentedW(allValues, L"\n");
}

static void DumpArrayAnsiString(DTWAIN_ARRAY Array)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<std::string>(Array);
    std::string allValues = StringWrapperA::Join(vData.begin(), vData.end(), "\n");
    LogWriterUtils::WriteMultiLineInfoIndentedA(allValues, "\n");
}

static void DumpArrayFrame(DTWAIN_ARRAY Array)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& vData = pHandle->m_ArrayFactory->underlying_container_t<TwainFrameInternal>(Array);
    size_t n;
    CTL_StringStreamType strm;
    std::for_each(vData.begin(), vData.end(), StreamerImplFrame(&strm, &n));
    LogWriterUtils::WriteMultiLineInfoIndented(strm.str(), _T("\n"));
}

void dynarithmic::DumpArrayContents(DTWAIN_ARRAY Array, LONG lCap)
{
    if ( !(CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS ))
        return;

    std::string szBuf;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    // This dumps contents of array to log file
    {
        // Turn off the error logging flags temporarily
        {
            DTWAINScopedLogController sLogger(0);
            if (!Array)
            {
                szBuf = "DTWAIN_ARRAY is NULL\n";
                // Turn on the error logging flags
                LogWriterUtils::WriteLogInfoIndentedA(szBuf);
                return;
            }
        }

        const LONG nCount = static_cast<LONG>(pHandle->m_ArrayFactory->size(Array));
        StringStreamA strm;
        if ( nCount < 0 )
        {
            strm << "Could not dump contents of DTWAIN_ARRAY " << Array << "\nNumber of elements: " << nCount;
            return;
        }
        strm << "Dumping contents of DTWAIN_ARRAY " << Array << "   : Number of elements: " << nCount;
        szBuf = strm.str();
    }

    LogWriterUtils::WriteLogInfoIndentedA(szBuf);

    // determine the type
    const LONG nType = CTL_ArrayFactory::tagtype_to_arraytype(pHandle->m_ArrayFactory->tag_type(Array));
    switch (nType)
    {
        case DTWAIN_ARRAYLONG:
            DumpArrayLONG(Array, lCap);
            break;

        case DTWAIN_ARRAYFLOAT:
            DumpArrayFLOAT(Array);
            break;

        case DTWAIN_ARRAYSTRING:
            DumpArrayNativeString(Array);
            break;

        case DTWAIN_ARRAYWIDESTRING:
            DumpArrayWideString(Array);
            break;

        case DTWAIN_ARRAYANSISTRING:
            DumpArrayAnsiString(Array);
            break;

        case DTWAIN_ARRAYFRAME:
            DumpArrayFrame(Array);
            break;
    }
}


int GetMultiCapValues(DTWAIN_HANDLE DLLHandle,
                       DTWAIN_SOURCE Source,
                       DTWAIN_ARRAY pArray,
                       CTL_ArrayType EnumType,
                       TW_UINT16 nCap,
                       TW_UINT16 GetType,
                       std::string cStr,
                       TW_UINT16 TwainDataType,
                       UINT nContainerVal,
                       bool bUseContainer,
                       std::string )
{
    return GetMultiCapValuesImpl<std::string, std::string, CTL_ArrayType, VectorAdderFn3>::GetMultiCapValues
        (DLLHandle, Source, pArray, EnumType, nCap, GetType, {}, TwainDataType, nContainerVal, bUseContainer, std::string());
}

