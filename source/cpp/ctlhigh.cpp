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
#include <boost/format.hpp>
#include <type_traits>

#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "ctlsupport.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif
using namespace dynarithmic;

static bool GetDoubleCap( DTWAIN_SOURCE Source, LONG lCap, double *pValue );
static LONG GetAllCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, DTWAIN_BOOL bExpandRange);
static LONG GetCurrentCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, DTWAIN_BOOL bExpandRange);
static LONG GetDefaultCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, DTWAIN_BOOL bExpandRange);

typedef bool (*SetDoubleCapFn)(DTWAIN_SOURCE, LONG, double);
typedef bool (*GetDoubleCapFn)(DTWAIN_SOURCE, LONG, double *);
typedef LONG (*GetCapValuesFn)(DTWAIN_SOURCE, LPDTWAIN_ARRAY, LONG, DTWAIN_BOOL);

static LONG EnumCapInternal(DTWAIN_SOURCE Source,
                            TW_UINT16 Cap,
                            LPDTWAIN_ARRAY arr,
                            DTWAIN_BOOL expandRange,
                            GetCapValuesFn fn,
                            const std::string& func,
                            const std::string& paramLog);

#if DTWAIN_BUILD_LOGCALLSTACK == 1
    #define GENERATE_PARAM_LOG(argVals) \
            (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK) ? (ParamOutputter((#argVals)).outputParam argVals.getString()) : ("")
#else
    #define GENERATE_PARAM_LOG(argVals) {}
#endif

template <typename CapArrayType>
static bool GetCapability(DTWAIN_SOURCE Source, TW_UINT16 Cap, typename CapArrayType::value_type* value,
                                 GetCapValuesFn /*capFn*/, const std::string& func, const std::string& paramLog)
{
    DTWAIN_ARRAY ArrayValues = nullptr;
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    const LONG retVal = EnumCapInternal(Source, Cap, &ArrayValues, false, GetCurrentCapValues, func, paramLog);
    DTWAINArrayLowLevel_RAII arr(pHandle, ArrayValues);
    if (retVal > 0)
    {
        auto& vOut = pHandle->m_ArrayFactory->underlying_container_t<typename CapArrayType::value_type>(ArrayValues);
        *value = vOut[0];
        return true;
    }
    return false;
}

template <typename CapDataType>
struct SetSupportFn1
{
    DTWAIN_SOURCE theSource;
    CapDataType theValue;
    TW_UINT16 theCap;
    bool setType;

    SetSupportFn1(DTWAIN_SOURCE s, CapDataType v, TW_UINT16 Cap, bool setCurrent)
    { SetAll(s, v ,Cap, setCurrent); }

    SetSupportFn1()
    { SetAll(0, 0, 0, false); }

    void SetAll(DTWAIN_SOURCE s, CapDataType v, TW_UINT16 Cap, bool setCurrent)
    {
        theSource = s;
        theCap = Cap;
        theValue = v;
        setType = setCurrent;
    }

    bool operator()()
    { return SetSupport<CapDataType>(theSource, &theValue, theCap, setType ? true : false); }
};

template <typename T>
struct SetSupportFn2 : public SetSupportFn1<T>
{
    SetSupportFn2(DTWAIN_SOURCE s, T v, TW_UINT16 Cap, bool setCurrent) : SetSupportFn1<T>(s,v,Cap, setCurrent) {}

    bool operator()()
    { return dynarithmic::SetSupportArray(this->theSource, this->theValue, this->theCap); }
};

template <typename T, typename FnToCall>
static T FunctionCaller(FnToCall fn, const std::string& func, const std::string& paramLog)
{
    try
    {
        T bRet {};
        #if DTWAIN_BUILD_LOGCALLSTACK == 1
        const bool doLog = CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CALLSTACK ? true : false;
        if (doLog)
        {
            try { LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallA(func.c_str(), LOG_INDENT_IN) + paramLog); }
            catch (...) {}
        }
        #endif
        bRet = fn();

        #if DTWAIN_BUILD_LOGCALLSTACK == 1
        if (doLog)
        {
            try
            {
                LogWriterUtils::WriteLogInfoA(CTL_LogFunctionCallA(func.c_str(), LOG_INDENT_OUT) + ParamOutputter("", true).outputParam(bRet).getString());
            }
            catch (...)
            { return bRet; }
        }
        #endif
        return bRet;
    }
    catch (T var) { return var; }
    catch (...)
    {
        LogExceptionErrorA(func.c_str());
        if (CTL_StaticData::IsThrowExceptions())
            DTWAIN_InternalThrowException();
        return {};
    }
}

struct GetDeviceCapsByStringFn
{
    DTWAIN_SOURCE theSource;
    LPTSTR theString;
    GetByStringFn theFn;

    GetDeviceCapsByStringFn(DTWAIN_SOURCE Src, LPTSTR value, GetByStringFn fn) :
        theSource(Src), theString(value), theFn(fn) {}
    DTWAIN_BOOL operator()() const { return DTWAIN_GetDeviceCapByString(theSource, theString, theFn); }
};

struct CapSetterFn
{
    DTWAIN_SOURCE Source;
    LPCTSTR value;
    SetByStringFn fn;
    CapSetterFn(DTWAIN_SOURCE src, LPCTSTR val, SetByStringFn theFn) : Source(src), value(val), fn(theFn) {}
    DTWAIN_BOOL operator()() const
    {
        return DTWAIN_SetDeviceCapByString(Source, value, fn);
    }
};

struct CapSetterFn2
{
    DTWAIN_SOURCE Source;
    LPCTSTR value;
    SetByStringFn2 fn;
    bool extra;
    CapSetterFn2(DTWAIN_SOURCE src, LPCTSTR val, bool ex, SetByStringFn2 theFn) : Source(src), value(val), fn(theFn), extra(ex) {}
    DTWAIN_BOOL operator()()
    {
        return DTWAIN_SetDeviceCapByString2(Source, value, extra, fn);
    }
};

struct IsEnabledImplFn
{
    DTWAIN_SOURCE theSource;
    TW_UINT16 theCap;
    IsEnabledImplFn(DTWAIN_SOURCE Src, TW_UINT16 cap) : theSource(Src), theCap(cap) {}
    bool operator()() { return CheckEnabled(theSource, theCap) ? true : false; }
};

template <typename T>
struct IsSupportedImplFn
{
    DTWAIN_SOURCE theSource;
    TW_UINT16 theCap;
    bool supportFlag;
    T theSupportVal;

    IsSupportedImplFn(DTWAIN_SOURCE Src, TW_UINT16 cap, bool sup, T supVal) :
        theSource(Src), theCap(cap), supportFlag(sup), theSupportVal(supVal) {}
    bool operator()() { return IsSupported(theSource, supportFlag, theSupportVal, theCap); }
};

struct EnumCapInternalFn
{
    DTWAIN_SOURCE theSource;
    TW_UINT16 theCap;
    LPDTWAIN_ARRAY theArray = {};
    bool theExpand;
    GetCapValuesFn theFn;
    EnumCapInternalFn(DTWAIN_SOURCE Src, TW_UINT16 cap, LPDTWAIN_ARRAY arr, bool expand, GetCapValuesFn fn) :
        theSource(Src), theCap(cap), theArray(arr), theExpand(expand), theFn(fn) {}
    LONG operator()() { return theFn(theSource, theArray, theCap, theExpand); }
};

template <typename CapDataType, typename SetterFn>
static DTWAIN_BOOL SetCapability(SetterFn fn, const std::string& func, const std::string& paramLog)
{ return FunctionCaller<DTWAIN_BOOL>(fn, func, paramLog); }

static DTWAIN_BOOL GetCapabilityByString(GetDeviceCapsByStringFn fn,
                                        const std::string& func,
                                        const std::string& paramLog)
{ return FunctionCaller<DTWAIN_BOOL>(fn, func, paramLog); }

static bool IsEnabledImpl(DTWAIN_SOURCE Source, TW_UINT16 Cap, const std::string& func, const std::string& paramLog)
{ return FunctionCaller<bool>(IsEnabledImplFn(Source, Cap), func, paramLog); }

template <typename T>
static bool IsSupportedImpl(DTWAIN_SOURCE Source, TW_UINT16 Cap, bool anySupport, T SupportVal,
                            const std::string& func, const std::string& paramLog)
{ return FunctionCaller<bool>(IsSupportedImplFn<T>(Source, Cap, anySupport, SupportVal), func, paramLog); }

static LONG EnumCapInternal(DTWAIN_SOURCE Source,
                            TW_UINT16 Cap,
                            LPDTWAIN_ARRAY arr,
                            DTWAIN_BOOL expandRange,
                            GetCapValuesFn fn,
                            const std::string& func,
                            const std::string& paramLog)
{ return FunctionCaller<LONG>(EnumCapInternalFn(Source, Cap, arr, expandRange?true:false, fn), func, paramLog); }

template <typename SetFn>
static DTWAIN_BOOL SetCapabilityByString(SetFn fn,
                                         const std::string& func,
                                         const std::string& paramLog)
{ return FunctionCaller<DTWAIN_BOOL>(fn, func, paramLog); }

static bool GetStringCapability(DTWAIN_SOURCE Source, TW_UINT16 Cap, LPSTR value, LONG nLen, GetCapValuesFn fn, const std::string& func, const std::string& paramLog)
{

    DTWAIN_ARRAY ArrayValues = nullptr;
    const LONG retVal = EnumCapInternal(Source, Cap, &ArrayValues, false, fn, func, paramLog);
    if (nLen > 0)
        value[0] = '\0';
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    DTWAINArrayLowLevel_RAII arr(pHandle, ArrayValues);
    if (retVal > 0)
    {
        std::string sVal;
        pHandle->m_ArrayFactory->get_value(ArrayValues, 0, &sVal);
        StringWrapperA::CopyInfoToCString(sVal, value, nLen);
        return true;
    }
    return false;
}


#define EXPORT_ENUM_CAP_VALUES(FuncName, Cap) \
    LONG DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, DTWAIN_BOOL bExpandIfRange) {\
        return EnumCapInternal(Source, Cap, pArray, bExpandIfRange, GetAllCapValues, __FUNCTION__, GENERATE_PARAM_LOG((Source, pArray, bExpandIfRange))); }

#define EXPORT_ENUM_CAP_VALUES_NOEXPAND(FuncName, Cap) \
    LONG DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray) {\
            return EnumCapInternal(Source, Cap, pArray, false, GetAllCapValues, __FUNCTION__, GENERATE_PARAM_LOG((Source, pArray))); }

#define EXPORT_ENUM_CAP_VALUES_EX(FuncName, Cap) \
    DTWAIN_ARRAY DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, DTWAIN_BOOL bExpandIfRange) {\
        DTWAIN_ARRAY pArray = 0; \
        EnumCapInternal(Source, Cap, &pArray, bExpandIfRange, GetAllCapValues, __FUNCTION__, GENERATE_PARAM_LOG((Source, bExpandIfRange))); \
        return pArray; }

#define EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(FuncName, Cap) \
    DTWAIN_ARRAY DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source) {\
            DTWAIN_ARRAY pArray = 0; \
            EnumCapInternal(Source, Cap, &pArray, false, GetAllCapValues, __FUNCTION__, GENERATE_PARAM_LOG((Source))); \
            return pArray; }

#define EXPORT_SET_CAP_VALUE(FuncName, Cap, CapDataType, CapFn) \
    DTWAIN_BOOL  DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, CapDataType value) \
    {\
       return SetCapability <CapDataType>(CapFn(Source, value, Cap, true), __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
    }


#define EXPORT_SET_CAP_VALUE_PARM3_2(FuncName, Cap1, Cap2, CapDataType1, CapDataType2, value2, CapFn) \
    DTWAIN_BOOL  DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, CapDataType1 value, DTWAIN_BOOL condType ) \
{\
    if ( condType )\
        SetCapability <CapDataType2>(CapFn(Source, value2, Cap2, true), __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
    return SetCapability <CapDataType1>(CapFn(Source, value, Cap1, true), __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
}

#define EXPORT_SET_CAP_VALUE_2(FuncName, Cap1, Cap2, CapDataType1, CapDataType2, CapFn) \
    DTWAIN_BOOL  DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, CapDataType1 value) \
{\
    CapFn theFn, theFn2;\
    theFn.SetAll(Source, value, Cap1, true);\
    theFn2.SetAll(Source, value, Cap2, true);\
    if (SetCapability <CapDataType1>(theFn, __FUNCTION__, GENERATE_PARAM_LOG((Source, value)))) \
        return SetCapability<CapDataType2>(theFn2, __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
    return FALSE;\
}

#define EXPORT_SET_CAP_VALUE_D_D(FuncName, Cap1, Cap2) \
    EXPORT_SET_CAP_VALUE_2(FuncName, Cap1, Cap2, DTWAIN_FLOAT, DTWAIN_FLOAT, SetSupportFn1<DTWAIN_FLOAT>)

#define EXPORT_GET_CAP_VALUE(FuncName, Cap, CapDataType, CapFn) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, CapDataType::value_type* value)\
    {\
        return GetCapability<CapDataType>(Source, Cap, value, CapFn, __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
    }

#define EXPORT_GET_CAP_VALUE_OPT_CURRENT(FuncName, Cap, CapDataType) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, CapDataType::value_type* value, DTWAIN_BOOL bCurrent)\
    {\
        std::string sLog = GENERATE_PARAM_LOG((Source, value, bCurrent));\
        std::string func = __FUNCTION__;\
        if ( bCurrent ) \
            return GetCapability<CapDataType>(Source, Cap, value, GetCurrentCapValues, func, sLog); \
        return GetCapability<CapDataType>(Source, Cap, value, GetDefaultCapValues, func, sLog); \
    }

#define EXPORT_SET_CAP_VALUE_OPT_CURRENT(FuncName, Cap, CapDataType, CapFn) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, CapDataType value, DTWAIN_BOOL bCurrent)\
    {\
        std::string sLog = GENERATE_PARAM_LOG((Source, value, bCurrent));\
        std::string func = __FUNCTION__;\
        return SetCapability<CapDataType>(CapFn(Source, value, Cap, bCurrent?true:false), func, sLog); \
    }

#define EXPORT_GET_CAP_VALUE_STRING(FuncName) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName##String(DTWAIN_SOURCE Source, LPTSTR value)\
    {\
        auto retVal = GetCapabilityByString(GetDeviceCapsByStringFn(Source, value, FuncName), __FUNCTION__, GENERATE_PARAM_LOG((Source, value)));\
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((value)) \
        return retVal; \
    }

#define EXPORT_SET_CAP_VALUE_STRING(FuncName) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName##String(DTWAIN_SOURCE Source, LPCTSTR value)\
   {\
       CapSetterFn fn(Source, value, &FuncName);\
       return  SetCapabilityByString(fn, __FUNCTION__, GENERATE_PARAM_LOG((Source, value)));\
   }

#define EXPORT_SET_CAP_VALUE_STRING_2(FuncName) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName##String(DTWAIN_SOURCE Source, LPCTSTR value, DTWAIN_BOOL condition)\
{\
    CapSetterFn2 fn(Source, value, condition?true:false, &FuncName);\
    return  SetCapabilityByString(fn, __FUNCTION__, GENERATE_PARAM_LOG((Source, value, condition)));\
}

#define EXPORT_GET_CAP_VALUE_D(FuncName, Cap) EXPORT_GET_CAP_VALUE(FuncName, Cap, CTL_ArrayFactory::tagged_array_double, GetCurrentCapValues)
#define EXPORT_GET_CAP_VALUE_I(FuncName, Cap) EXPORT_GET_CAP_VALUE(FuncName, Cap, CTL_ArrayFactory::tagged_array_long, GetCurrentCapValues)
#define EXPORT_GET_CAP_VALUE_A(FuncName, Cap) EXPORT_GET_CAP_VALUE(FuncName, Cap, CTL_ArrayFactory::tagged_array_voidptr, GetCurrentCapValues)
#define EXPORT_GET_CAP_VALUE_S(FuncName, Cap, NumChars) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, LPTSTR value)\
    {\
        std::string valueTemp((NumChars) + 1, '\0');\
        auto retVal = GetStringCapability(Source, Cap, &valueTemp[0], NumChars, \
                                          GetCurrentCapValues, __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
        valueTemp.resize(NumChars); \
        StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(valueTemp, valueTemp.size()), value, NumChars); \
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((value)) \
        return retVal;\
    }

#define EXPORT_GET_VALUE_OPT_MAXLENGTH_S(FuncName, Cap) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, LPTSTR value, LONG MaxLen)\
    {\
        std::string valueTemp(MaxLen + 1, '\0');\
        auto retVal = GetStringCapability(Source, Cap, &valueTemp[0], MaxLen, GetCurrentCapValues,\
                                         __FUNCTION__, GENERATE_PARAM_LOG((Source, value))); \
        valueTemp.resize(MaxLen); \
        StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(valueTemp, valueTemp.size()), value, MaxLen); \
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((value)) \
        return retVal;\
    }

#define EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(FuncName, Cap) EXPORT_GET_CAP_VALUE_OPT_CURRENT(FuncName, Cap, CTL_ArrayFactory::tagged_array_long)
#define EXPORT_GET_CAP_VALUE_OPT_CURRENT_D(FuncName, Cap) EXPORT_GET_CAP_VALUE_OPT_CURRENT(FuncName, Cap, CTL_ArrayFactory::tagged_array_double)
#define EXPORT_GET_CAP_VALUE_OPT_CURRENT_S(FuncName, Cap, NumChars) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, LPTSTR value, DTWAIN_LONG GetType)\
    {\
        GetCapValuesFn fn = GetAllCapValues;\
        if ( GetType == DTWAIN_CAPGETCURRENT ) \
           fn = GetCurrentCapValues;\
        else\
        if ( GetType == DTWAIN_CAPGETDEFAULT) \
           fn = GetDefaultCapValues;\
        std::string valueTemp((NumChars) + 1, '\0');\
        auto retval = GetStringCapability(Source, Cap, &valueTemp[0], NumChars, fn, __FUNCTION__, \
                            GENERATE_PARAM_LOG((Source, value, GetType))); \
        valueTemp.resize(NumChars); \
        StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(valueTemp, valueTemp.size()), value, NumChars); \
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((value)) \
        return retval; \
    }

#define EXPORT_SET_CAP_VALUE_D(FuncName, Cap) EXPORT_SET_CAP_VALUE(FuncName, Cap, DTWAIN_FLOAT, SetSupportFn1<DTWAIN_FLOAT>)
#define EXPORT_SET_CAP_VALUE_I(FuncName, Cap) EXPORT_SET_CAP_VALUE(FuncName, Cap, LONG, SetSupportFn1<LONG>)
#define EXPORT_SET_CAP_VALUE_S(FuncName, Cap) EXPORT_SET_CAP_VALUE(FuncName, Cap, LPCTSTR, SetSupportFn1<LPCTSTR>)
#define EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(FuncName, Cap) EXPORT_SET_CAP_VALUE_OPT_CURRENT(FuncName, Cap, LONG, SetSupportFn1<LONG>)
#define EXPORT_SET_CAP_VALUE_A(FuncName, Cap) EXPORT_SET_CAP_VALUE(FuncName, Cap, DTWAIN_ARRAY, SetSupportFn2<DTWAIN_ARRAY>)

#define EXPORT_SET_CAP_VALUE_D_I_EX(FuncName, Cap1, Cap2, paramEx) \
    EXPORT_SET_CAP_VALUE_PARM3_2(FuncName, Cap1, Cap2, DTWAIN_FLOAT, LONG, paramEx, SetSupportFn1<DTWAIN_FLOAT>)

#define EXPORT_IS_CAP_SUPPORTED_I(FuncName, Cap) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, LONG CapValue)\
    { return IsSupportedImpl(Source, Cap, (CapValue==DTWAIN_ANYSUPPORT), CapValue, __FUNCTION__, GENERATE_PARAM_LOG((Source, CapValue)))?TRUE:FALSE; }

#define EXPORT_IS_CAP_SUPPORTED_D(FuncName, Cap) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source, DTWAIN_FLOAT CapValue)\
    { return IsSupportedImpl(Source, Cap, false, CapValue, __FUNCTION__, GENERATE_PARAM_LOG((Source, CapValue)))?TRUE:FALSE; }

#define EXPORT_IS_CAP_SUPPORTED_I_1(FuncName, Cap) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source)\
    { return IsSupportedImpl(Source, Cap, true, 0L, __FUNCTION__, GENERATE_PARAM_LOG((Source)))?TRUE:FALSE; }

#define EXPORT_IS_CAP_ENABLED(FuncName, Cap) \
    DTWAIN_BOOL DLLENTRY_DEF FuncName(DTWAIN_SOURCE Source)\
    { return IsEnabledImpl(Source, Cap, __FUNCTION__, GENERATE_PARAM_LOG((Source)))?TRUE:FALSE; }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsAudioXferSupported, ACAP_XFERMECH)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsAutoBorderDetectSupported, ICAP_AUTOMATICBORDERDETECTION)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsAutoBrightSupported, ICAP_AUTOBRIGHT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsAutoDeskewSupported, ICAP_AUTOMATICDESKEW)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsAutoFeedSupported,  CAP_AUTOFEED)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsAutoRotateSupported, DTWAIN_CV_ICAPAUTOROTATE)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsCompressionSupported, ICAP_COMPRESSION)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsCustomDSDataSupported, CAP_CUSTOMDSDATA)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsDeviceEventSupported, CAP_DEVICEEVENT)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsDoubleFeedDetectSupported, CAP_DOUBLEFEEDDETECTION)
EXPORT_IS_CAP_SUPPORTED_D(DTWAIN_IsDoubleFeedDetectLengthSupported, CAP_DOUBLEFEEDDETECTIONLENGTH)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsExtImageInfoSupported, ICAP_EXTIMAGEINFO)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsFileXferSupported,  ICAP_IMAGEFILEFORMAT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIndicatorSupported, CAP_INDICATORS)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsLampSupported, ICAP_LAMPSTATE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsLightPathSupported, ICAP_LIGHTPATH)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsLightSourceSupported, ICAP_LIGHTSOURCE)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsMaxBuffersSupported, CAP_MAXBATCHBUFFERS)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsOrientationSupported, ICAP_ORIENTATION)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsOverscanSupported, ICAP_OVERSCAN)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsPaperSizeSupported, ICAP_SUPPORTEDSIZES)
EXPORT_IS_CAP_SUPPORTED_I(DTWAIN_IsPatchSupported, ICAP_SUPPORTEDPATCHCODETYPES)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsPrinterSupported, CAP_PRINTER)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsRotationSupported, ICAP_ROTATION)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsThumbnailSupported, CAP_THUMBNAILSENABLED)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsUIOnlySupported, CAP_ENABLEDSUIONLY)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsAutomaticSenseMediumSupported, CAP_AUTOMATICSENSEMEDIUM)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsImageAddressingSupported, CAP_IMAGEADDRESSENABLED)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldALevelSupported, CAP_IAFIELDA_LEVEL)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldBLevelSupported, CAP_IAFIELDB_LEVEL)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldCLevelSupported, CAP_IAFIELDC_LEVEL)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldDLevelSupported, CAP_IAFIELDD_LEVEL)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldELevelSupported, CAP_IAFIELDE_LEVEL)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldAPrintFormatSupported, CAP_IAFIELDA_PRINTFORMAT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldBPrintFormatSupported, CAP_IAFIELDB_PRINTFORMAT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldCPrintFormatSupported, CAP_IAFIELDC_PRINTFORMAT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldDPrintFormatSupported, CAP_IAFIELDD_PRINTFORMAT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldEPrintFormatSupported, CAP_IAFIELDE_PRINTFORMAT)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldAValueSupported, CAP_IAFIELDA_VALUE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldBValueSupported, CAP_IAFIELDB_VALUE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldCValueSupported, CAP_IAFIELDC_VALUE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldDValueSupported, CAP_IAFIELDD_VALUE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldEValueSupported, CAP_IAFIELDE_VALUE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldALastPageSupported, CAP_IAFIELDA_LASTPAGE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldBLastPageSupported, CAP_IAFIELDB_LASTPAGE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldCLastPageSupported, CAP_IAFIELDC_LASTPAGE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldDLastPageSupported, CAP_IAFIELDD_LASTPAGE)
EXPORT_IS_CAP_SUPPORTED_I_1(DTWAIN_IsIAFieldELastPageSupported, CAP_IAFIELDE_LASTPAGE)

EXPORT_IS_CAP_ENABLED(DTWAIN_IsAutoBorderDetectEnabled, ICAP_AUTOMATICBORDERDETECTION)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsAutoBrightEnabled, ICAP_AUTOBRIGHT)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsAutoDeskewEnabled, ICAP_AUTOMATICDESKEW)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsAutoRotateEnabled, ICAP_AUTOMATICROTATE)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsAutoScanEnabled, CAP_AUTOSCAN)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsDeviceOnLine, CAP_DEVICEONLINE)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsDuplexEnabled, CAP_DUPLEXENABLED)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsIndicatorEnabled, CAP_INDICATORS)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsLampEnabled, ICAP_LAMPSTATE)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsPaperDetectable, CAP_PAPERDETECTABLE)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsPatchDetectEnabled, ICAP_PATCHCODEDETECTIONENABLED)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsThumbnailEnabled, CAP_THUMBNAILSENABLED)
EXPORT_IS_CAP_ENABLED(DTWAIN_IsAutomaticSenseMediumEnabled, CAP_AUTOMATICSENSEMEDIUM)

EXPORT_GET_CAP_VALUE_I(DTWAIN_GetAlarmVolume, CAP_ALARMVOLUME)
EXPORT_GET_CAP_VALUE_S(DTWAIN_GetAuthor, CAP_AUTHOR, 128)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetBatteryMinutes, CAP_BATTERYMINUTES)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetBatteryPercent, CAP_BATTERYPERCENTAGE)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetBrightness, ICAP_BRIGHTNESS)
EXPORT_GET_CAP_VALUE_S(DTWAIN_GetCaption, CAP_CAPTION, 255)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetCompressionType, ICAP_COMPRESSION)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetContrast, ICAP_CONTRAST)
EXPORT_GET_CAP_VALUE_S(DTWAIN_GetDeviceTimeDate, CAP_DEVICETIMEDATE, 32)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_D(DTWAIN_GetDoubleFeedDetectLength, CAP_DOUBLEFEEDDETECTIONLENGTH)
EXPORT_GET_CAP_VALUE_A(DTWAIN_GetDoubleFeedDetectValues, CAP_DOUBLEFEEDDETECTION)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetFeederAlignment,  CAP_FEEDERALIGNMENT)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetFeederOrder, CAP_FEEDERORDER)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_S(DTWAIN_GetHalftone, ICAP_HALFTONES, 128)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetHighlight, ICAP_HIGHLIGHT)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetJobControl, CAP_JOBCONTROL)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetLightPath, ICAP_LIGHTPATH)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetLightSource, ICAP_LIGHTSOURCE)
EXPORT_GET_CAP_VALUE_A(DTWAIN_GetLightSources, ICAP_LIGHTSOURCE)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetNoiseFilter,  ICAP_NOISEFILTER)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetOrientation, ICAP_ORIENTATION)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetMaxBuffers, CAP_MAXBATCHBUFFERS)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPaperSize, ICAP_SUPPORTEDSIZES)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPatchMaxPriorities, ICAP_PATCHCODEMAXSEARCHPRIORITIES)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPatchMaxRetries, ICAP_PATCHCODEMAXRETRIES)
EXPORT_GET_CAP_VALUE_A(DTWAIN_GetPatchPriorities, ICAP_PATCHCODESEARCHPRIORITIES)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPatchSearchMode, ICAP_PATCHCODESEARCHMODE)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPatchTimeOut, ICAP_PATCHCODETIMEOUT)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetPixelFlavor, ICAP_PIXELFLAVOR)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPrinter, CAP_PRINTER)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetPrinterStartNumber, CAP_PRINTERINDEX)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetPrinterStringMode, CAP_PRINTERMODE)
EXPORT_GET_CAP_VALUE_A(DTWAIN_GetPrinterStrings, CAP_PRINTERSTRING)
EXPORT_GET_VALUE_OPT_MAXLENGTH_S(DTWAIN_GetPrinterSuffixString, CAP_PRINTERSUFFIX)
EXPORT_GET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_GetOverscan, ICAP_OVERSCAN)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetRotation, ICAP_ROTATION)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetShadow, ICAP_SHADOW)
EXPORT_GET_CAP_VALUE_I(DTWAIN_GetSourceUnit, ICAP_UNITS)

EXPORT_GET_CAP_VALUE_D(DTWAIN_GetThreshold, ICAP_THRESHOLD)
EXPORT_GET_CAP_VALUE_S(DTWAIN_GetTimeDate, CAP_TIMEDATE, 32)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetXResolution, ICAP_XRESOLUTION)
EXPORT_GET_CAP_VALUE_D(DTWAIN_GetYResolution, ICAP_YRESOLUTION)

EXPORT_SET_CAP_VALUE_I(DTWAIN_ClearBuffers, CAP_CLEARBUFFERS)

EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableAutoBorderDetect, ICAP_AUTOMATICBORDERDETECTION)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableAutoBright, ICAP_AUTOBRIGHT)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableAutoDeskew, ICAP_AUTOMATICDESKEW)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableAutoRotate, ICAP_AUTOMATICROTATE)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableAutoScan, CAP_AUTOSCAN)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableDuplex, CAP_DUPLEXENABLED)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableIndicator, CAP_INDICATORS)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableLamp, ICAP_LAMPSTATE)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnablePatchDetect, ICAP_PATCHCODEDETECTIONENABLED)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnablePrinter, CAP_PRINTERENABLED)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableThumbnail, CAP_THUMBNAILSENABLED)
EXPORT_SET_CAP_VALUE_A(DTWAIN_SetAlarms, CAP_ALARMS)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetAlarmVolume, CAP_ALARMVOLUME)
EXPORT_SET_CAP_VALUE_S(DTWAIN_SetAuthor, CAP_AUTHOR)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetBrightness, ICAP_BRIGHTNESS)
EXPORT_SET_CAP_VALUE_S(DTWAIN_SetCaption, CAP_CAPTION)
EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_SetCompressionType, ICAP_COMPRESSION)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetContrast, ICAP_CONTRAST)
EXPORT_SET_CAP_VALUE_S(DTWAIN_SetDeviceTimeDate, CAP_DEVICETIMEDATE)
EXPORT_SET_CAP_VALUE_A(DTWAIN_SetDoubleFeedDetectValues, CAP_DOUBLEFEEDDETECTION)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetDoubleFeedDetectLength, CAP_DOUBLEFEEDDETECTIONLENGTH)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetFeederAlignment, CAP_FEEDERALIGNMENT)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetFeederOrder,  CAP_FEEDERORDER)
EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_SetFileXferFormat, ICAP_IMAGEFILEFORMAT)
EXPORT_SET_CAP_VALUE_S(DTWAIN_SetHalftone, ICAP_HALFTONES)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetHighlight, ICAP_HIGHLIGHT)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetLightPath, ICAP_LIGHTPATH)
EXPORT_SET_CAP_VALUE_A(DTWAIN_SetLightPathEx, ICAP_LIGHTPATH)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetLightSource, ICAP_LIGHTSOURCE)
EXPORT_SET_CAP_VALUE_A(DTWAIN_SetLightSources, ICAP_LIGHTSOURCE)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetMaxBuffers, CAP_MAXBATCHBUFFERS)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetNoiseFilter, ICAP_NOISEFILTER)
EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_SetOrientation, ICAP_ORIENTATION)
EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_SetOverscan, ICAP_OVERSCAN)
EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_SetPaperSize, ICAP_SUPPORTEDSIZES)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetPatchMaxPriorities, ICAP_PATCHCODEMAXSEARCHPRIORITIES)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetPatchMaxRetries,  ICAP_PATCHCODEMAXRETRIES)
EXPORT_SET_CAP_VALUE_A(DTWAIN_SetPatchPriorities, ICAP_PATCHCODESEARCHPRIORITIES)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetPatchSearchMode, ICAP_PATCHCODESEARCHMODE)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetPatchTimeOut, ICAP_PATCHCODETIMEOUT)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetPixelFlavor, ICAP_PIXELFLAVOR)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetPrinterStartNumber, CAP_PRINTERINDEX)
EXPORT_SET_CAP_VALUE_S(DTWAIN_SetPrinterSuffixString, CAP_PRINTERSUFFIX)
EXPORT_SET_CAP_VALUE_OPT_CURRENT_I(DTWAIN_SetPrinterStringMode, CAP_PRINTERMODE)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetRotation, ICAP_ROTATION)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetShadow, ICAP_SHADOW)
EXPORT_SET_CAP_VALUE_I(DTWAIN_SetSourceUnit, ICAP_UNITS)
EXPORT_SET_CAP_VALUE_D_I_EX(DTWAIN_SetThreshold, ICAP_THRESHOLD, ICAP_BITDEPTHREDUCTION, TWBR_THRESHOLD)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetXResolution, ICAP_XRESOLUTION)
EXPORT_SET_CAP_VALUE_D(DTWAIN_SetYResolution, ICAP_YRESOLUTION)
EXPORT_SET_CAP_VALUE_I(DTWAIN_EnableAutomaticSenseMedium, CAP_AUTOMATICSENSEMEDIUM)

EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetBrightness)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetContrast)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetDoubleFeedDetectLength)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetHighlight)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetResolution)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetRotation)
EXPORT_SET_CAP_VALUE_STRING_2(DTWAIN_SetThreshold)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetShadow)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetXResolution)
EXPORT_SET_CAP_VALUE_STRING(DTWAIN_SetYResolution)

EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetBrightness)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetContrast)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetHighlight)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetResolution)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetRotation)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetShadow)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetThreshold)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetXResolution)
EXPORT_GET_CAP_VALUE_STRING(DTWAIN_GetYResolution)

// Exported function implementations for getting the cap values
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumDoubleFeedDetectLengths, CAP_DOUBLEFEEDDETECTIONLENGTH)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumAudioXferMechs, ACAP_XFERMECH)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumAlarms, CAP_ALARMS)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumAlarmVolumes, CAP_ALARMVOLUME)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumAutoFeedValues, CAP_AUTOFEED)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumAutomaticCaptures, CAP_AUTOMATICCAPTURE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumAutomaticSenseMedium, CAP_AUTOMATICSENSEMEDIUM)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumBitDepths, ICAP_BITDEPTH)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumBrightnessValues, ICAP_BRIGHTNESS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumCompressionTypes, ICAP_COMPRESSION)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumContrastValues, ICAP_CONTRAST)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumDoubleFeedDetectValues, CAP_DOUBLEFEEDDETECTION)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumExtendedCapsEx, CAP_EXTENDEDCAPS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumFileXferFormats,ICAP_IMAGEFILEFORMAT)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumHalftones, ICAP_HALFTONES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumJobControls, CAP_JOBCONTROL)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumLightPaths, ICAP_LIGHTPATH)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumLightSources, ICAP_LIGHTSOURCE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumNoiseFilters, ICAP_NOISEFILTER)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumOrientations, ICAP_ORIENTATION)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumOverscanValues, ICAP_OVERSCAN)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPatchCodes, ICAP_SUPPORTEDPATCHCODETYPES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPatchMaxPriorities, ICAP_PATCHCODEMAXSEARCHPRIORITIES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPatchPriorities, ICAP_PATCHCODESEARCHPRIORITIES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPatchSearchModes, ICAP_PATCHCODESEARCHMODE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPaperSizes, ICAP_SUPPORTEDSIZES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPatchMaxRetries, ICAP_PATCHCODEMAXRETRIES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPatchTimeOutValues, ICAP_PATCHCODETIMEOUT)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumPrinterStringModes, CAP_PRINTERMODE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumSourceUnits, ICAP_UNITS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumSupportedCapsEx, CAP_SUPPORTEDCAPS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumTwainPrinters, CAP_PRINTER)
EXPORT_ENUM_CAP_VALUES_NOEXPAND(DTWAIN_EnumTwainPrintersArray, CAP_PRINTER)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumHighlightValues, ICAP_HIGHLIGHT)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumMaxBuffers, CAP_MAXBATCHBUFFERS)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumResolutionValues, ICAP_XRESOLUTION)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumXResolutionValues, ICAP_XRESOLUTION)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumYResolutionValues, ICAP_YRESOLUTION)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumShadowValues, ICAP_SHADOW)
EXPORT_ENUM_CAP_VALUES(DTWAIN_EnumThresholdValues, ICAP_THRESHOLD)

EXPORT_SET_CAP_VALUE_D_D(DTWAIN_SetResolution, ICAP_XRESOLUTION, ICAP_YRESOLUTION)


EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumDoubleFeedDetectLengthsEx, CAP_DOUBLEFEEDDETECTIONLENGTH)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumAudioXferMechsEx, ACAP_XFERMECH)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumAlarmsEx, CAP_ALARMS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumSupportedCapsEx2, CAP_SUPPORTEDCAPS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumExtendedCapsEx2, CAP_EXTENDEDCAPS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPaperSizesEx, ICAP_SUPPORTEDSIZES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumSourceUnitsEx, ICAP_UNITS)

EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumAlarmVolumesEx, CAP_ALARMVOLUME)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumAutoFeedValuesEx, CAP_AUTOFEED)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumAutomaticCapturesEx, CAP_AUTOMATICCAPTURE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumAutomaticSenseMediumEx, CAP_AUTOMATICSENSEMEDIUM)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumBitDepthsEx2, ICAP_BITDEPTH)

EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumBrightnessValuesEx, ICAP_BRIGHTNESS)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumCompressionTypesEx, ICAP_COMPRESSION)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumContrastValuesEx, ICAP_CONTRAST)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumDoubleFeedDetectValuesEx, CAP_DOUBLEFEEDDETECTION)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumFileXferFormatsEx, ICAP_IMAGEFILEFORMAT)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumHalftonesEx, ICAP_HALFTONES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumJobControlsEx, CAP_JOBCONTROL)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumLightPathsEx, ICAP_LIGHTPATH)

EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumLightSourcesEx, ICAP_LIGHTSOURCE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumNoiseFiltersEx, ICAP_NOISEFILTER)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumOrientationsEx, ICAP_ORIENTATION)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumOverscanValuesEx, ICAP_OVERSCAN)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPatchCodesEx, ICAP_SUPPORTEDPATCHCODETYPES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPatchMaxPrioritiesEx, ICAP_PATCHCODEMAXSEARCHPRIORITIES)

EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPatchPrioritiesEx, ICAP_PATCHCODESEARCHPRIORITIES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPatchSearchModesEx, ICAP_PATCHCODESEARCHMODE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPatchMaxRetriesEx, ICAP_PATCHCODEMAXRETRIES)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPatchTimeOutValuesEx, ICAP_PATCHCODETIMEOUT)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumPrinterStringModesEx, CAP_PRINTERMODE)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumTwainPrintersEx, CAP_PRINTER)
EXPORT_ENUM_CAP_VALUES_NOEXPAND_EX(DTWAIN_EnumTwainPrintersArrayEx, CAP_PRINTER)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumHighlightValuesEx, ICAP_HIGHLIGHT)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumMaxBuffersEx, CAP_MAXBATCHBUFFERS)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumResolutionValuesEx, ICAP_XRESOLUTION)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumXResolutionValuesEx, ICAP_XRESOLUTION)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumYResolutionValuesEx, ICAP_YRESOLUTION)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumShadowValuesEx, ICAP_SHADOW)
EXPORT_ENUM_CAP_VALUES_EX(DTWAIN_EnumThresholdValuesEx, ICAP_THRESHOLD)


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumSourceValues(DTWAIN_SOURCE Source, LPCTSTR capName, LPDTWAIN_ARRAY pArray, DTWAIN_BOOL expandIfRange)
{
    LOG_FUNC_ENTRY_PARAMS((Source, capName, pArray, expandIfRange))
    const DTWAIN_BOOL retVal = DTWAIN_GetCapValuesEx2(Source, CTL_TwainAppMgr::GetCapFromCapName(StringConversion::Convert_NativePtr_To_Ansi(capName).c_str()), 
        DTWAIN_CAPGET, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, pArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(false)
}

///////////////// These functions are high-level capability functions ///////////////
DTWAIN_BOOL dynarithmic::DTWAIN_SetDeviceCapByString(DTWAIN_SOURCE Source, LPCTSTR strVal, SetByStringFn fn)
{
    DTWAIN_FLOAT value = 0.0;
    if ( strVal )
        value = StringWrapper::ToDouble(strVal);
    return fn(Source, value);
}

DTWAIN_BOOL dynarithmic::DTWAIN_SetDeviceCapByString2(DTWAIN_SOURCE Source, LPCTSTR strVal, bool bExtra, SetByStringFn2 fn)
{
    DTWAIN_FLOAT value = 0.0;
    if ( strVal )
        value = StringWrapper::ToDouble(strVal);
    return fn(Source, value, bExtra);
}

DTWAIN_BOOL dynarithmic::DTWAIN_GetDeviceCapByString(DTWAIN_SOURCE Source, LPTSTR strVal, GetByStringFn fn)
{
    DTWAIN_FLOAT tempR;
    const DTWAIN_BOOL retVal = fn(Source, &tempR);
    if ( retVal )
    {
        StringStreamA strm;
        strm << boost::format("%1%") % tempR;
        const auto srcStr = StringConversion::Convert_Ansi_To_Native(strm.str());
        StringWrapper::SafeStrcpy(strVal, srcStr.c_str());
    }
    return retVal;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetResolution(DTWAIN_SOURCE Source, LPDTWAIN_FLOAT Resolution)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Resolution))
    LONG lCap = 0;
    if ( DTWAIN_IsCapSupported( Source, ICAP_XRESOLUTION))
        lCap = ICAP_XRESOLUTION;
    else
    if ( DTWAIN_IsCapSupported( Source, ICAP_XNATIVERESOLUTION))
         lCap = ICAP_XNATIVERESOLUTION;
    else
        LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
    const DTWAIN_BOOL bRet = GetDoubleCap( Source, lCap, Resolution);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(FALSE)
}

static bool GetDoubleCap( DTWAIN_SOURCE Source, LONG lCap, double *pValue )
{
    double *pRealValue = pValue;
    if (DTWAIN_GetCapDataType(Source, lCap) != TWTY_FIX32)
        return false;
    DTWAIN_ARRAY Array = nullptr;
    bool bRet = DTWAIN_GetCapValuesEx2(Source, lCap, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &Array) ? true : false;
    if (!bRet)
        return false;
    const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
    DTWAINArrayLowLevel_RAII arr(pHandle, Array);
    const auto& vIn = pHandle->m_ArrayFactory->underlying_container_t<double>(Array);
    if ( bRet && Array )
    {
        if ( vIn.empty() )
            bRet = false;
        else
            *pRealValue = vIn[0];
    }
    return bRet;
}

static LONG GetCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, LONG GetType, DTWAIN_BOOL bExpandRange)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pArray, lCap, bExpandRange))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    LONG nValues = 0;

    DTWAIN_ARRAY OrigVals = nullptr;

    // we may use a brand new array
    LPDTWAIN_ARRAY arrayToUse = &OrigVals;

    // use what was passed in if it isn't null
    if ( pArray )
        arrayToUse = pArray;

    DTWAINArrayPtr_RAII a(pHandle, &OrigVals);

    // get the capability values
    if (DTWAIN_GetCapValuesEx2(Source, lCap, GetType, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, arrayToUse))
    {
        // Gotten the value.  Check what container type holds the data
        const LONG lContainer = DTWAIN_GetCapContainer(Source, lCap, GetType);
        switch (lContainer)
        {
            case DTWAIN_CONTRANGE:
            {
                if (bExpandRange)
                {
                    // we need to expand to a temporary
                    DTWAIN_ARRAY tempArray = nullptr;

                    // throw this away when done
                    DTWAINArrayPtr_RAII aTemp(pHandle, &tempArray);

                    // expand the range into a temp array
                    DTWAIN_RangeExpand(*arrayToUse, &tempArray);

                    // destroy original and copy new values
                    pHandle->m_ArrayFactory->destroy(*arrayToUse);
                    *arrayToUse = CreateArrayCopyFromFactory(pHandle, tempArray);

                    // get the count
                    nValues = static_cast<LONG>(pHandle->m_ArrayFactory->size(*arrayToUse)); 
                }
                else
                {
                    nValues = static_cast<LONG>(pHandle->m_ArrayFactory->size(*arrayToUse)); 
                }
            }
            break;
            case DTWAIN_CONTENUMERATION:
            case DTWAIN_CONTONEVALUE:
            case DTWAIN_CONTARRAY:
            {
                nValues = static_cast<LONG>(pHandle->m_ArrayFactory->size(*arrayToUse)); 
            }
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(nValues)
    CATCH_BLOCK_LOG_PARAMS(0) //DTWAIN_FAILURE1)
}

static LONG GetCurrentCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, DTWAIN_BOOL bExpandRange)
{ return GetCapValues(Source, pArray, lCap, DTWAIN_CAPGETCURRENT, bExpandRange); }

static LONG GetDefaultCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, DTWAIN_BOOL bExpandRange)
{ return GetCapValues(Source, pArray, lCap, DTWAIN_CAPGETDEFAULT, bExpandRange); }

static LONG GetAllCapValues(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY pArray, LONG lCap, DTWAIN_BOOL bExpandRange)
{ return GetCapValues(Source, pArray, lCap, DTWAIN_CAPGET, bExpandRange); }
