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
#include "ctltwainmanager.h"
#include "errorcheck.h"
#include "cppfunc.h"
#include "arrayfactory.h"
#include "ctllogfunctioncall.h"
#ifdef _MSC_VER
#pragma warning (disable : 4702)
#pragma warning (disable : 4714)
#endif

using namespace dynarithmic;

static bool GetImageSize(CTL_TwainDLLHandle* pHandle,  DTWAIN_SOURCE Source, LPDTWAIN_ARRAY FloatArray, TW_UINT16 GetType);

static bool GetImageSize2(CTL_ITwainSource *p,
    LPDTWAIN_FLOAT left,
    LPDTWAIN_FLOAT top,
    LPDTWAIN_FLOAT right,
    LPDTWAIN_FLOAT bottom,
    LPLONG Unit);

static bool SetImageSize(DTWAIN_SOURCE Source,
    DTWAIN_ARRAY FloatArray,
    DTWAIN_ARRAY ActualArray,
    TW_UINT16 SetType);

static bool SetImageSize2(CTL_ITwainSource *p,
    DTWAIN_FLOAT left,
    DTWAIN_FLOAT top,
    DTWAIN_FLOAT right,
    DTWAIN_FLOAT bottom,
    LONG Unit,
    LONG flags);

static constexpr bool IsValidUnit(LONG Unit)
{
    return Unit == DTWAIN_INCHES ||
        Unit == DTWAIN_CENTIMETERS ||
        Unit == DTWAIN_PICAS ||
        Unit == DTWAIN_POINTS ||
        Unit == DTWAIN_TWIPS ||
        Unit == DTWAIN_PIXELS;
}

///////////////////////////////////////////////////////////////////////
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea(DTWAIN_SOURCE Source, LONG lGetType, LPDTWAIN_ARRAY FloatArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lGetType, FloatArray))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const DTWAIN_BOOL bRet = GetImageSize(pHandle, Source, FloatArray, static_cast<TW_UINT16>(lGetType));
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea(DTWAIN_SOURCE Source, LONG lSetType, DTWAIN_ARRAY FloatArray, DTWAIN_ARRAY ActualArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lSetType, FloatArray, ActualArray))
    VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const DTWAIN_BOOL bRet = SetImageSize(Source, FloatArray, ActualArray,static_cast<TW_UINT16>(lSetType));
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea2String(DTWAIN_SOURCE Source, LPCTSTR left, LPCTSTR top, LPCTSTR right, LPCTSTR bottom, LONG Unit, LONG flags)
{
    LOG_FUNC_ENTRY_PARAMS((Source, left, top, right, bottom, Unit, flags))
    const DTWAIN_FLOAT val1 = StringWrapper::ToDouble(left);
    const DTWAIN_FLOAT val2 = StringWrapper::ToDouble(top);
    const DTWAIN_FLOAT val3 = StringWrapper::ToDouble(right);
    const DTWAIN_FLOAT val4 = StringWrapper::ToDouble(bottom);
    const DTWAIN_BOOL bRet = DTWAIN_SetAcquireArea2(Source, val1, val2, val3, val4, Unit, flags);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAcquireArea2(DTWAIN_SOURCE Source, DTWAIN_FLOAT left, DTWAIN_FLOAT top, DTWAIN_FLOAT right, DTWAIN_FLOAT bottom, LONG Unit, LONG flags)
{
    LOG_FUNC_ENTRY_PARAMS((Source, left, top, right, bottom, Unit, flags))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return !IsValidUnit(Unit); },
                                    DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    const DTWAIN_BOOL bRet = SetImageSize2(pSource, left, top, right, bottom, Unit, flags);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea2String(DTWAIN_SOURCE Source, LPTSTR left, LPTSTR top, LPTSTR right, LPTSTR bottom, LPLONG Unit)
{
    LOG_FUNC_ENTRY_PARAMS((Source, left, top, right, bottom, Unit))
    std::array<DTWAIN_FLOAT, 4> val = {};
    std::array<LPTSTR, 4> pStr = { left?left:nullptr, top?top:nullptr, right?right:nullptr, bottom?bottom:nullptr };
    const DTWAIN_BOOL bRet = DTWAIN_GetAcquireArea2(Source, &val[0], &val[1], &val[2], &val[3], Unit);
    if (bRet)
    {
        std::ostringstream strm;
        auto numDigits = strm.precision();
        for (size_t i = 0; i < val.size(); ++i)
        {
            strm << std::setprecision(numDigits) << val[i];
            if (pStr[i])
            {
                std::string sResult = strm.str();
                StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(sResult), pStr[i], static_cast<int32_t>(sResult.size()) + 1);
            }
            strm.str("");
        }
    }
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((left, top, right, bottom, Unit))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAcquireArea2(DTWAIN_SOURCE Source, LPDTWAIN_FLOAT  left, LPDTWAIN_FLOAT  top, LPDTWAIN_FLOAT  right, LPDTWAIN_FLOAT  bottom, LPLONG Unit)
{
    LOG_FUNC_ENTRY_PARAMS((Source, left, top, right, bottom, Unit))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const DTWAIN_BOOL bRet = GetImageSize2(pSource, left, top, right, bottom, Unit);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

static bool GetImageSize(CTL_TwainDLLHandle* pHandle, DTWAIN_SOURCE Source, LPDTWAIN_ARRAY FloatArray, TW_UINT16 GetType)
{
    CTL_ITwainSource* p = static_cast<CTL_ITwainSource*>(Source);
    DTWAIN_ARRAY FloatArrayOut = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYFLOAT, 4);
    if (!FloatArrayOut)
        return false;
    DTWAINArrayLowLevel_RAII aFloat(pHandle, FloatArrayOut);
    CTL_RealArray Array;
    if (GetType == MSG_GETCURRENT)
        GetType = MSG_GET;

    const bool bOk = CTL_TwainAppMgr::GetImageLayoutSize(p, Array, GetType);
    if (!bOk)
        return false;
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<double>(FloatArrayOut);
    std::copy(Array.begin(), Array.end(), vValues.begin());
    const DTWAIN_ARRAY temp = CreateArrayCopyFromFactory(pHandle, FloatArrayOut);
    if  (pHandle->m_ArrayFactory->is_valid(static_cast<void*>(FloatArray)))
        pHandle->m_ArrayFactory->destroy(static_cast<void*>(FloatArray));
    if (temp)
    {
        *FloatArray = temp;
        return true;
    }
    return false;
}


static bool FillActualArray(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ActualArray, const std::vector<double>& vValues)
{
    if (ActualArray != nullptr)
    {
        auto& vActual = pHandle->m_ArrayFactory->underlying_container_t<double>(ActualArray);
        DTWAIN_Check_Error_Condition_0_Ex(pHandle,
            [&] {return !pHandle->m_ArrayFactory->is_valid(ActualArray, CTL_ArrayFactory::arrayTag::DoubleType); },
            DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
        vActual.clear();
        std::copy_n(vValues.begin(), std::min(std::size_t(4), vValues.size()), std::back_inserter(vActual));
        return true;
    }
    return false;
}

static bool SetImageSize(DTWAIN_SOURCE Source, DTWAIN_ARRAY FloatArray, DTWAIN_ARRAY ActualArray, TW_UINT16 SetType)
{
    LOG_FUNC_ENTRY_PARAMS((Source, FloatArray, ActualArray, SetType))
    CTL_ITwainSource* p = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = p->GetDTWAINHandle();
    if (SetType == MSG_RESET)
    {
        CTL_RealArray dummy;
        const bool bOk = CTL_TwainAppMgr::SetImageLayoutSize(p, {}, dummy, MSG_RESET);
        if ( bOk )
            FillActualArray(pHandle, ActualArray, dummy);
        LOG_FUNC_EXIT_NONAME_PARAMS(bOk)
    }

    const DTWAIN_ARRAY pArray = FloatArray;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle,
        [&] { return !pHandle->m_ArrayFactory->is_valid(pArray, CTL_ArrayFactory::arrayTag::DoubleType); },
        DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
    static const size_t minValue = 4;
    const auto& vFloat = pHandle->m_ArrayFactory->underlying_container_t<double>(FloatArray);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle,
        [&] { return vFloat.size() < minValue; },
        DTWAIN_ERR_AREA_ARRAY_TOO_SMALL, false, FUNC_MACRO);

    CTL_RealArray Array;
    CTL_RealArray rArray;
    std::copy_n(vFloat.begin(), minValue, std::back_inserter(Array));
    const bool bOk = CTL_TwainAppMgr::SetImageLayoutSize(p, Array, rArray, SetType);
    if (bOk)
        FillActualArray(pHandle, ActualArray, rArray);
    LOG_FUNC_EXIT_NONAME_PARAMS(bOk)
    CATCH_BLOCK(false)
}


static bool SetImageSize2(CTL_ITwainSource *p,
                            DTWAIN_FLOAT left,
                            DTWAIN_FLOAT top,
                            DTWAIN_FLOAT right,
                            DTWAIN_FLOAT bottom,
                            LONG Unit,
                            LONG flags)
{
    LOG_FUNC_ENTRY_PARAMS((p, left, top, right, bottom, Unit, flags))
    p->SetAlternateAcquireArea(left, top, right, bottom, flags, Unit, flags ? true : false);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

static bool GetImageSize2(CTL_ITwainSource *p,
                            LPDTWAIN_FLOAT left,
                            LPDTWAIN_FLOAT top,
                            LPDTWAIN_FLOAT right,
                            LPDTWAIN_FLOAT bottom,
                            LPLONG Unit)
{
    LOG_FUNC_ENTRY_PARAMS((p, left, top, right, bottom, Unit))
    FloatRect r;
    LONG flags;
    p->GetAlternateAcquireArea(r, *Unit, flags);
    if (!flags)
    {
        *Unit = -1;
        *left = -1.0;
        *right = -1.0;
        *top = -1.0;
        *bottom = -1.0;
    }
    else
    {
        *left = r.left;
        *top = r.top;
        *right = r.right;
        *bottom = r.bottom;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}
