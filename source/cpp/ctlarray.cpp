/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

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
//#define MC_NO_CPP
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <boost/format.hpp>

#include "cppfunc.h"
#include "ctltwmgr.h"
#include "ctltrall.h"
#include "ctltmpl5.h"
#include "errorcheck.h"
#include "dtwainx.h"
#include "dtwstrfn.h"
#include "dtwain_float_utils.h"
#include "arrayfactory.h"

typedef LPVOID* LPLPVOID;

using namespace dynarithmic;

struct ArrayChecker
{
    static constexpr int CHECK_ARRAY_BOUNDS = 1;
    static constexpr int CHECK_ARRAY_EXISTS = 2;
    static constexpr int CHECK_ARRAY_ANSI_TYPE = 4;
    static constexpr int CHECK_ARRAY_WIDE_TYPE = 8;
    static constexpr int CHECK_ARRAY_SAME_TYPE = 16;
    static constexpr int CHECK_ARRAY_EXPLICIT_TYPE = 32;
    static constexpr int CHECK_ARRAY_STRING_TYPE = (CHECK_ARRAY_ANSI_TYPE | CHECK_ARRAY_WIDE_TYPE);

    private:
        std::array<DTWAIN_ARRAY, 2> m_Array;
        std::array<LONG, 2> m_ArrayPos;
        int m_checkFlag = CHECK_ARRAY_BOUNDS | CHECK_ARRAY_EXISTS;
        int m_explicitTypeCheck = -1;
        mutable bool m_bAnsiArray = false;

    public:
        ArrayChecker() : m_Array{}, m_ArrayPos{} {}
        ArrayChecker& SetArray1(DTWAIN_ARRAY a1) { m_Array[0] = a1; return *this; }
        ArrayChecker& SetArray2(DTWAIN_ARRAY a2) { m_Array[1] = a2; return *this; }
        ArrayChecker& SetArrayPos(LONG arrayPos) { m_ArrayPos[0] = arrayPos; return *this; }
        ArrayChecker& SetCheckType(LONG checkType) { m_checkFlag = checkType; return *this; }
        ArrayChecker& SetExplicitTypeCheck(int checkType) { m_explicitTypeCheck = checkType; return *this;}

        bool IsAnsiArray() const { return m_bAnsiArray; }

        int Check() const
        {
            const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            auto retVal = CheckImpl(pHandle);
            if (retVal != DTWAIN_NO_ERROR)
                pHandle->m_lLastError = retVal;
            return retVal;
        }

        int CheckImpl(CTL_TwainDLLHandle* pHandle) const
        {
            const auto& factory = pHandle->m_ArrayFactory;

            m_bAnsiArray = false;

            // Check if array exists
            if (m_checkFlag & CHECK_ARRAY_EXISTS)
            {
                if (!m_Array[0] || !factory->is_valid(m_Array[0]))
                    return DTWAIN_ERR_BAD_ARRAY;
            }

            if (m_checkFlag & CHECK_ARRAY_SAME_TYPE)
            {
                if (!(m_checkFlag & CHECK_ARRAY_EXISTS))
                {
                    if (!m_Array[0] || !factory->is_valid(m_Array[0]))
                        return DTWAIN_ERR_BAD_ARRAY;
                    if (!m_Array[1] || !factory->is_valid(m_Array[1]))
                        return DTWAIN_ERR_BAD_ARRAY;
                }

                const auto tagType1 = factory->tag_type(m_Array[0]);
                const auto tagType2 = factory->tag_type(m_Array[1]);
                if (tagType1 != tagType2)
                    return DTWAIN_ERR_ARRAYTYPE_MISMATCH;
            }

            if (m_checkFlag & CHECK_ARRAY_EXPLICIT_TYPE)
            {
                if (!(m_checkFlag & CHECK_ARRAY_EXISTS))
                {
                    if (!m_Array[0] || !factory->is_valid(m_Array[0]))
                        return DTWAIN_ERR_BAD_ARRAY;
                }
                const auto tagType1 = factory->tag_type(m_Array[0]);
                if (tagType1 != m_explicitTypeCheck)
                    return DTWAIN_ERR_ARRAYTYPE_MISMATCH;
            }

            if ( m_checkFlag & CHECK_ARRAY_STRING_TYPE)
            {
                if (!(m_checkFlag & CHECK_ARRAY_EXISTS))
                {
                    if (!m_Array[0] || !factory->is_valid(m_Array[0]))
                        return DTWAIN_ERR_BAD_ARRAY;
                }
                bool bIsValid = true;

                if ((m_checkFlag & CHECK_ARRAY_STRING_TYPE) == CHECK_ARRAY_STRING_TYPE)
                {
                    bIsValid = factory->is_valid(m_Array[0], CTL_ArrayFactory::arrayTag::StringType);
                    if ( !bIsValid)
                        bIsValid = factory->is_valid(m_Array[0], CTL_ArrayFactory::arrayTag::WStringType);
                    else
                        m_bAnsiArray = true;
                }
                else
                if (m_checkFlag & CHECK_ARRAY_ANSI_TYPE)
                {
                    bIsValid = factory->is_valid(m_Array[0], CTL_ArrayFactory::arrayTag::StringType);
                    m_bAnsiArray = true;
                }
                else
                if (m_checkFlag & CHECK_ARRAY_WIDE_TYPE)
                    bIsValid = factory->is_valid(m_Array[0], CTL_ArrayFactory::arrayTag::WStringType);
                if (!bIsValid)
                    return DTWAIN_ERR_STRINGTYPE_MISMATCH;
            }

            if (m_checkFlag & CHECK_ARRAY_BOUNDS)
            {
                if (!(m_checkFlag & CHECK_ARRAY_EXISTS))
                {
                    if (!m_Array[0] || !factory->is_valid(m_Array[0]))
                        return DTWAIN_ERR_BAD_ARRAY;
                }
                const LONG Count = static_cast<LONG>(factory->size(m_Array[0]));
                if (Count <= 0 || static_cast<LONG>(m_ArrayPos[0]) >= Count)
                    return DTWAIN_ERR_INDEX_BOUNDS;
            }
            return DTWAIN_NO_ERROR;
        }
};

static LONG IsValidRangeArray( DTWAIN_ARRAY pArray );
static LONG IsValidAcqArray( DTWAIN_ARRAY pArray );

void dynarithmic::DestroyArrayFromFactory(DTWAIN_ARRAY pArray)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    pHandle->m_ArrayFactory->destroy(pArray);
}

void dynarithmic::DestroyFrameFromFactory(DTWAIN_FRAME Frame)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    pHandle->m_ArrayFactory->destroy_frame(Frame);
}

DTWAIN_ARRAY dynarithmic::CreateArrayFromFactory(LONG nEnumType, LONG nInitialSize)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if (!pHandle)
        return nullptr;
    switch (nEnumType)
    {
        case DTWAIN_ARRAYINT16:
        case DTWAIN_ARRAYINT32:
        case DTWAIN_ARRAYUINT16:
        case DTWAIN_ARRAYUINT32:
            nEnumType = DTWAIN_ARRAYLONG;
            break;
        default:;
    }
    int dummy = 0;
    return pHandle->m_ArrayFactory->create_array(static_cast<CTL_ArrayType>(nEnumType), &dummy, nInitialSize);
}

DTWAIN_ARRAY dynarithmic::CreateArrayCopyFromFactory(DTWAIN_ARRAY Source)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if (!pHandle)
        return nullptr;
    const LONG lType = CTL_ArrayFactory::tagtype_to_arraytype(pHandle->m_ArrayFactory->tag_type(Source));
    DTWAIN_ARRAY Dest = CreateArrayFromFactory(lType, 0);
    if (Dest)
        pHandle->m_ArrayFactory->copy(Dest, Source);
    return Dest;
}

void dynarithmic::SetArrayValueFromFactory(DTWAIN_ARRAY pArray, size_t lPos, LPVOID pVariant)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

    const auto& factory = pHandle->m_ArrayFactory;
    const int enumType = factory->tag_type(pArray);
    switch (enumType)
    {
        // Do something special for strings
        case CTL_ArrayFactory::arrayTag::WStringType:
        {
            std::wstring sVal = static_cast<LPCWSTR>(pVariant);
            factory->set_value(pArray, lPos, &sVal);
        }
        break;
        case CTL_ArrayFactory::arrayTag::StringType:
        {
            std::string sVal = static_cast<LPCSTR>(pVariant);
            factory->set_value(pArray, lPos, &sVal);
        }
        break;
        default:
            factory->set_value(pArray, lPos, pVariant);
    }
}

DTWAIN_FRAME dynarithmic::CreateFrameArray(CTL_TwainDLLHandle* pHandle, double Left, double Top, double Right, double Bottom)
{
    return pHandle->m_ArrayFactory->create_frame(Left, Top, Right, Bottom);
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayInit()
{
    LOG_FUNC_ENTRY_PARAMS(())
    LOG_FUNC_EXIT_PARAMS(NULL)
    CATCH_BLOCK(DTWAIN_ARRAY())
}


DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayCreate( LONG nEnumType, LONG nInitialSize )
{
    LOG_FUNC_ENTRY_PARAMS((nEnumType, nInitialSize))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);
    const DTWAIN_ARRAY Array = CreateArrayFromFactory(nEnumType, nInitialSize);
    auto checkStatus = ArrayChecker().SetArray1(Array).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if ( checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(DTWAIN_ARRAY())
    LOG_FUNC_EXIT_PARAMS(Array)
    CATCH_BLOCK(DTWAIN_ARRAY())
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayCreateFromCap(DTWAIN_SOURCE Source, LONG lCapType, LONG lSize)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCapType, lSize))
    const LONG lType = DTWAIN_GetCapArrayType(Source, lCapType);
    if ( lType == DTWAIN_FAILURE1 )
        LOG_FUNC_EXIT_PARAMS(NULL)
    const DTWAIN_ARRAY Array = CreateArrayFromFactory(lType, lSize);
    LOG_FUNC_EXIT_PARAMS(Array)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

LONG DLLENTRY_DEF DTWAIN_ArrayGetType(DTWAIN_ARRAY pArray)
{
    LOG_FUNC_ENTRY_PARAMS((pArray))
    const LONG Ret = DTWAIN_ArrayType(pArray, FALSE);
    LOG_FUNC_EXIT_PARAMS(Ret)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG dynarithmic::DTWAIN_ArrayType(DTWAIN_ARRAY pArray, bool bGetReal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, bGetReal))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, DTWAIN_FAILURE1, FUNC_MACRO);

    // Check if array exists
    const auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(-1)
    const LONG Ret = CTL_ArrayFactory::tagtype_to_arraytype(pHandle->m_ArrayFactory->tag_type(pArray));

    LOG_FUNC_EXIT_PARAMS(Ret)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayCreateCopy(DTWAIN_ARRAY Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const LONG lType = DTWAIN_ArrayGetType(Source);
    if (lType == DTWAIN_FAILURE1)
        LOG_FUNC_EXIT_PARAMS(NULL)
    DTWAIN_ARRAY Dest = CreateArrayCopyFromFactory(Source);
    if ( !Dest )
        LOG_FUNC_EXIT_PARAMS(NULL)
    LOG_FUNC_EXIT_PARAMS(Dest)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayCopy(DTWAIN_ARRAY Source, DTWAIN_ARRAY Dest)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Dest))
        // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    auto& factory = pHandle->m_ArrayFactory;
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // Check if array exists
    auto checkStatus = ArrayChecker().SetArray1(Source).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS );
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    checkStatus.SetArray1(Dest);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    checkStatus.SetArray1(Source).SetArray2(Dest).SetCheckType(ArrayChecker::CHECK_ARRAY_SAME_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    // Copy the arrays
    factory->copy(Dest, Source);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

// Only use the template if T is a known type (not void*)
template <typename T, bool checkType=true>
static bool ArrayAddN(DTWAIN_ARRAY pArray, T pVariant, LONG num = 1)
{
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    if (checkType)
    {
        auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
        if (checkStatus.Check() != DTWAIN_NO_ERROR)
            LOG_FUNC_EXIT_PARAMS(false)
    }
    auto& factory = pHandle->m_ArrayFactory;
    factory->add_to_back(pArray, &pVariant, num);
    return true;
}

DTWAIN_BOOL  DLLENTRY_DEF DTWAIN_ArrayAddN( DTWAIN_ARRAY pArray, LPVOID pVariant, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariant, num))

    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // Check if array exists
    auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    auto& factory = pHandle->m_ArrayFactory;
    const auto tag_type = factory->tag_type(pArray);
    bool bRet = true;
    switch (tag_type)
    {
        case CTL_ArrayFactory::arrayTag::StringType:
            bRet = ArrayAddN<std::string, false>(pArray, std::string(static_cast<const char *>(pVariant)), num);
        break;
        case CTL_ArrayFactory::arrayTag::WStringType:
            bRet = ArrayAddN<std::wstring, false>(pArray, std::wstring(static_cast<wchar_t*>(pVariant)), num);
        break;
        default:
            factory->add_to_back(pArray, pVariant, num);
        break;
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddLongN( DTWAIN_ARRAY pArray, LONG Val, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, num))
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, Val, num );
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddLong64N( DTWAIN_ARRAY pArray, LONG64 Val, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, num))
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, Val, num );
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddFloatN( DTWAIN_ARRAY pArray, DTWAIN_FLOAT Val, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, num))
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, Val,num );
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddStringN( DTWAIN_ARRAY pArray, LPCTSTR Val, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, num))
#ifdef _UNICODE
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, std::wstring(Val), num );
#else
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, std::string(Val), num);
#endif
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_ArrayAddWideStringN(DTWAIN_ARRAY pArray, LPCWSTR Val, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, num))
    const auto checkStatus = ArrayChecker().SetArray1(pArray).
                                                SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_WIDE_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = ArrayAddN<std::wstring, false>(pArray, std::wstring(Val), num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddANSIStringN(DTWAIN_ARRAY pArray, LPCSTR Val, LONG num )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, num))

    const auto checkStatus = ArrayChecker().SetArray1(pArray).
                                                SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_ANSI_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    const DTWAIN_BOOL bRet = ArrayAddN<std::string, false>(pArray, std::string(Val) ,num );
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL  DLLENTRY_DEF DTWAIN_ArrayAdd( DTWAIN_ARRAY pArray, LPVOID pVariant )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariant))
    const DTWAIN_BOOL bAdded = DTWAIN_ArrayAddN(pArray, pVariant, 1);
    LOG_FUNC_EXIT_PARAMS(bAdded)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddLong( DTWAIN_ARRAY pArray, LONG Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddLong64( DTWAIN_ARRAY pArray, LONG64 Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddFloat(DTWAIN_ARRAY pArray, DTWAIN_FLOAT Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const DTWAIN_BOOL bRet = ArrayAddN(pArray, Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddString(DTWAIN_ARRAY pArray, LPCTSTR Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayAddStringN(pArray, Val, 1);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddWideString(DTWAIN_ARRAY pArray, LPCWSTR Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayAddWideStringN(pArray, Val, 1);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayAddANSIString(DTWAIN_ARRAY pArray, LPCSTR Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayAddANSIStringN(pArray, Val, 1);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_ARRAY  DLLENTRY_DEF  DTWAIN_ArrayCreateFromLongs(LPLONG pCArray, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((pCArray, nSize))
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_ARRAY Dest = CreateArrayFromFactory(DTWAIN_ARRAYLONG, 0);
    if ( !Dest )
        LOG_FUNC_EXIT_PARAMS(NULL)
    auto& vect = pHandle->m_ArrayFactory->underlying_container_t<LONG>(Dest);
    vect.insert(vect.end(), pCArray, pCArray + nSize);
    LOG_FUNC_EXIT_PARAMS(Dest)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}
DTWAIN_ARRAY  DLLENTRY_DEF  DTWAIN_ArrayCreateFromLong64s(LPLONG64 pCArray, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((pCArray, nSize))
    const DTWAIN_ARRAY Dest = CreateArrayFromFactory(DTWAIN_ARRAYLONG64, nSize);
    if ( !Dest )
        LOG_FUNC_EXIT_PARAMS(NULL)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vect = pHandle->m_ArrayFactory->underlying_container_t<LONG64>(Dest);
    vect.insert(vect.end(), pCArray, pCArray + nSize);
    LOG_FUNC_EXIT_PARAMS(Dest)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_ARRAY  DLLENTRY_DEF  DTWAIN_ArrayCreateFromReals(double* pCArray, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((pCArray, nSize))
    const DTWAIN_ARRAY Dest = CreateArrayFromFactory(DTWAIN_ARRAYFLOAT, nSize);
    if ( !Dest )
        LOG_FUNC_EXIT_PARAMS(NULL)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vect = pHandle->m_ArrayFactory->underlying_container_t<double>(Dest);
    vect.insert(vect.end(), pCArray, pCArray + nSize);
    LOG_FUNC_EXIT_PARAMS(Dest)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayCreateFromStrings(LPCTSTR* pCArray, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((pCArray, nSize))
    const DTWAIN_ARRAY Dest = CreateArrayFromFactory(DTWAIN_ARRAYSTRING, nSize);
    if ( !Dest )
        LOG_FUNC_EXIT_PARAMS(NULL)
    CTL_StringArrayType tempArray;
    for (LONG i = 0; i < nSize; ++i)
    {
        tempArray.push_back(*pCArray);
        ++pCArray;
    }
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
#ifdef _UNICODE
    auto& vect = pHandle->m_ArrayFactory->underlying_container_t<std::wstring>(Dest);
#else
    auto& vect = pHandle->m_ArrayFactory->underlying_container_t<std::string>(Dest);
#endif
    vect.insert(vect.end(), tempArray.begin(), tempArray.end());
    LOG_FUNC_EXIT_PARAMS(Dest)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayRemoveAll( DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((pArray))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory;

    // Check if array exists
    const auto checkStatus = ArrayChecker().SetArray1(pArray).
                                            SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    factory->clear(pArray);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayRemoveAt(DTWAIN_ARRAY pArray, LONG nWhere)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere))
    DTWAIN_ArrayRemoveAtN(pArray, nWhere, 1);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayRemoveAtN(  DTWAIN_ARRAY pArray, LONG nWhere, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, num))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory;

    // Check if array exists
    DTWAIN_Check_Error_Condition_0_Ex(pHandle,
                                      [&] { return !factory->is_valid(pArray); },
                                      DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
    const size_t Count = factory->size(pArray);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return Count == 0 || nWhere < 0 
														|| static_cast<size_t>(nWhere) >= Count; },
                                      DTWAIN_ERR_INDEX_BOUNDS, false, FUNC_MACRO);
    factory->remove(pArray, nWhere, num);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAt( DTWAIN_ARRAY pArray, LONG nWhere, LPVOID pVariant)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVariant))
    DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray, nWhere, pVariant, 1);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtLong(DTWAIN_ARRAY pArray, LONG nWhere, LONG Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAt(pArray, nWhere, &Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtLong64(DTWAIN_ARRAY pArray, LONG nWhere, LONG64 Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAt(pArray, nWhere, &Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtFloat(DTWAIN_ARRAY pArray, LONG nWhere,  DTWAIN_FLOAT Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAt(pArray, nWhere, &Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtString(DTWAIN_ARRAY pArray, LONG nWhere, LPCTSTR pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVal))
#ifdef _UNICODE
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtWideString(pArray, nWhere, pVal);
#else
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtANSIString(pArray, nWhere, pVal);
#endif
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtWideString(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVal))
    auto checkStatus = ArrayChecker().SetArray1(pArray).SetArrayPos(nWhere).
            SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_WIDE_TYPE | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAt(pArray, nWhere, (LPVOID)pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_ArrayInsertAtANSIString(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVal))
    auto checkStatus = ArrayChecker().SetArray1(pArray).SetArrayPos(nWhere).
    SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_ANSI_TYPE | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAt(pArray, nWhere, (LPVOID)pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayDestroy( DTWAIN_ARRAY pArray)
{
    LOG_FUNC_ENTRY_PARAMS((pArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // Make it legal to delete a null array
    if ( !pArray )
        LOG_FUNC_EXIT_PARAMS(true)

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // Check if this is a valid DWAIN_ARRAY
    auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    DestroyArrayFromFactory(pArray);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_ArrayGetCount( DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((pArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, DTWAIN_FAILURE1, FUNC_MACRO);

    // Check if array exists
    auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(DTWAIN_FAILURE1)

    const auto& factory = pHandle->m_ArrayFactory; 

    const LONG Ret = static_cast<LONG>(factory->size(pArray));
    LOG_FUNC_EXIT_PARAMS(Ret)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayResize(DTWAIN_ARRAY Array, LONG NewSize)
{
    LOG_FUNC_ENTRY_PARAMS((Array, NewSize))

    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory; 

    // Check if array exists
    auto checkStatus = ArrayChecker().SetArray1(Array).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    factory->resize(Array, NewSize);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetSourceAt(DTWAIN_ARRAY pArray, LONG nWhere, DTWAIN_SOURCE* ppSource)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, ppSource))
    DTWAIN_BOOL ret = DTWAIN_ArrayGetAt(pArray, nWhere, ppSource);
    LOG_FUNC_EXIT_PARAMS(ret)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAt( DTWAIN_ARRAY pArray, LONG nWhere, LPVOID pVariant)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVariant))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    const auto checkStatus= ArrayChecker().
                            SetArray1(pArray).
                            SetArrayPos(nWhere).
                            SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if ( checkStatus.Check() != DTWAIN_NO_ERROR )
        LOG_FUNC_EXIT_PARAMS(FALSE)

    const auto& factory = pHandle->m_ArrayFactory; 

    // Do something special for strings
    DTWAIN_BOOL bRet = FALSE;
    switch (factory->tag_type(pArray))
    {
        case CTL_ArrayFactory::arrayTag::WStringType:
            bRet = DTWAIN_ArrayGetAtWideString(pArray, nWhere, static_cast<LPWSTR>(pVariant));
        break;

        case CTL_ArrayFactory::arrayTag::StringType:
            bRet = DTWAIN_ArrayGetAtANSIString(pArray, nWhere, static_cast<LPSTR>(pVariant));
        break;

        case CTL_ArrayFactory::arrayTag::SourceType:
        {
            auto pSource = static_cast<CTL_ITwainSource **>(pVariant);
            *pSource = static_cast<CTL_ITwainSource*>(factory->get_value(pArray, nWhere, pVariant));
        }
        break;
        default:
            factory->get_value(pArray, nWhere, pVariant);
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtLong(DTWAIN_ARRAY pArray, LONG nWhere, LPLONG pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayGetAt(pArray, nWhere, pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtLong64(DTWAIN_ARRAY pArray, LONG nWhere, LPLONG64 pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayGetAt(pArray, nWhere, pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtFloat(DTWAIN_ARRAY pArray, LONG nWhere, LPDTWAIN_FLOAT pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayGetAt(pArray, nWhere, pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

template <typename T, typename U>
static DTWAIN_BOOL StringGetter(DTWAIN_ARRAY pArray, LONG nWhere, T& sVal, U pStr, bool bNullTerminate=true)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& factory = pHandle->m_ArrayFactory;
    factory->get_value(pArray, nWhere, &sVal); 
    std::copy(sVal.begin(), sVal.end(), pStr);
    if (bNullTerminate)
        pStr[sVal.length()] = typename T::value_type('\0');
    return TRUE;
}


template <typename T, typename U>
static T StringPtrGetter(DTWAIN_ARRAY pArray, LONG nWhere)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<typename U::value_type>(pArray);
    if (!vValues.empty())
    {
        if ( nWhere >= 0 && nWhere < static_cast<LONG>(vValues.size()) )
            return vValues[nWhere].c_str();
    }
    return nullptr;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtString(DTWAIN_ARRAY pArray, LONG nWhere, LPTSTR pStr)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pStr))
#ifdef _UNICODE
    std::wstring sVal;
#else
    std::string sVal;
#endif
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().
        SetArray1(pArray).
        SetArrayPos(nWhere).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(FALSE)

    const DTWAIN_BOOL bRet = StringGetter(pArray, nWhere, sVal, pStr);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

LPCTSTR DLLENTRY_DEF  DTWAIN_ArrayGetAtStringPtr(DTWAIN_ARRAY pArray, LONG nWhere)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere))
#ifdef _UNICODE
    const auto bRet = DTWAIN_ArrayGetAtWideStringPtr(pArray, nWhere);
#else
    const auto bRet = DTWAIN_ArrayGetAtANSIStringPtr(pArray, nWhere);
#endif
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(LPCTSTR(0))
}

LPCWSTR DLLENTRY_DEF  DTWAIN_ArrayGetAtWideStringPtr(DTWAIN_ARRAY pArray, LONG nWhere)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().
        SetArray1(pArray).
        SetArrayPos(nWhere).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(FALSE)

    const auto bRet = StringPtrGetter<LPCWSTR, CTL_ArrayFactory::tagged_array_wstring>(pArray, nWhere);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(LPCWSTR(0))
}

LPCSTR DLLENTRY_DEF  DTWAIN_ArrayGetAtANSIStringPtr(DTWAIN_ARRAY pArray, LONG nWhere)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().
        SetArray1(pArray).
        SetArrayPos(nWhere).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(FALSE)

    const auto bRet = StringPtrGetter<LPCSTR, CTL_ArrayFactory::tagged_array_string>(pArray, nWhere);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(LPCSTR(0))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayGetAtWideString(DTWAIN_ARRAY pArray, LONG nWhere, LPWSTR pStr)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pStr))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().
                                        SetArray1(pArray).
                                        SetArrayPos(nWhere).
                                        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | 
                                                     ArrayChecker::CHECK_ARRAY_BOUNDS | 
                                                     ArrayChecker::CHECK_ARRAY_WIDE_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(FALSE)
    std::wstring sVal;
    const DTWAIN_BOOL bRet = StringGetter(pArray, nWhere, sVal, pStr);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF   DTWAIN_ArrayGetAtANSIString(DTWAIN_ARRAY pArray, LONG nWhere, LPSTR pStr)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pStr))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().
        SetArray1(pArray).
        SetArrayPos(nWhere).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS |
                     ArrayChecker::CHECK_ARRAY_BOUNDS |
                     ArrayChecker::CHECK_ARRAY_ANSI_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(FALSE)

    std::string sVal;
    const DTWAIN_BOOL bRet = StringGetter(pArray, nWhere, sVal, pStr);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

static LONG ArrayFindInternal(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray, LPVOID pVariant, DTWAIN_FLOAT Tolerance, bool UseTolerance = false)
{
    const auto& factory = pHandle->m_ArrayFactory;

    // Get correct array type
    size_t pos = 0;
    if (!UseTolerance)
        pos = factory->find(pArray, pVariant);
    else
        pos = factory->find(pArray, pVariant, Tolerance);
    if (pos != (std::numeric_limits<size_t>::max)())
        return static_cast<LONG>(pos);
    return -1;
}

LONG  DLLENTRY_DEF DTWAIN_ArrayFind( DTWAIN_ARRAY pArray, LPVOID pVariant )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariant))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, -1L, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(-1)

    auto FoundPos = ArrayFindInternal(pHandle, pArray, pVariant, 0.0, false);
    LOG_FUNC_EXIT_PARAMS(FoundPos)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_ArrayFindLong( DTWAIN_ARRAY pArray, LONG Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const LONG lRet = DTWAIN_ArrayFind(pArray, &Val);
    LOG_FUNC_EXIT_PARAMS(lRet)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}


LONG DLLENTRY_DEF DTWAIN_ArrayFindLong64( DTWAIN_ARRAY pArray, LONG64 Val )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val))
    const LONG lRet = DTWAIN_ArrayFind(pArray, &Val);
    LOG_FUNC_EXIT_PARAMS(lRet)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}


LONG DLLENTRY_DEF DTWAIN_ArrayFindFloat( DTWAIN_ARRAY pArray, DTWAIN_FLOAT Val, DTWAIN_FLOAT Tolerance )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, Tolerance))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(-1)

    LONG FoundPos = ArrayFindInternal(pHandle, pArray, &Val, Tolerance, true);
    LOG_FUNC_EXIT_PARAMS(FoundPos)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtN( DTWAIN_ARRAY pArray, LONG nWhere, LPVOID pVariant, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pVariant, num))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory; 
    const auto checkStatus = ArrayChecker().SetArray1(pArray).
                                        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    factory->insert(pArray, pVariant, nWhere, num);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_ArrayInsertAtLongN(DTWAIN_ARRAY pArray, LONG nWhere, LONG Val, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val, num))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray,nWhere,&Val,num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtLong64N(DTWAIN_ARRAY pArray, LONG nWhere, LONG64 Val, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val, num))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray,nWhere,&Val,num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtFloatN(DTWAIN_ARRAY pArray, LONG nWhere, DTWAIN_FLOAT Val, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val, num))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray,nWhere,&Val,num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtStringN(DTWAIN_ARRAY pArray, LONG nWhere, LPCTSTR Val, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val, num))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray,nWhere,(LPVOID)Val,num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtWideStringN(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR Val, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val, num))
    const auto checkStatus = ArrayChecker().SetArray1(pArray).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS | ArrayChecker::CHECK_ARRAY_WIDE_TYPE);
    if ( checkStatus.Check() != DTWAIN_NO_ERROR )
        LOG_FUNC_EXIT_PARAMS(false)

    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray,nWhere, (LPVOID)Val, num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayInsertAtANSIStringN(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR Val, LONG num)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val, num))
    const auto checkStatus = ArrayChecker().SetArray1(pArray).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS | ArrayChecker::CHECK_ARRAY_ANSI_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = DTWAIN_ArrayInsertAtN(pArray, nWhere, (LPVOID)Val, num);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


LONG DLLENTRY_DEF DTWAIN_ArrayFindString( DTWAIN_ARRAY pArray, LPCTSTR pString )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pString))
    const LONG lRet = DTWAIN_ArrayFind(pArray, (LPVOID)pString);
    LOG_FUNC_EXIT_PARAMS(lRet)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_ArrayFindWideString( DTWAIN_ARRAY pArray, LPCWSTR pString )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pString))
    const auto checkStatus =  ArrayChecker().
                                        SetArray1(pArray).
                                        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_WIDE_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(-1)
    const LONG lRet = DTWAIN_ArrayFind(pArray, (LPVOID)pString);
    LOG_FUNC_EXIT_PARAMS(lRet)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_ArrayFindANSIString( DTWAIN_ARRAY pArray, LPCSTR pString )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pString))
    const auto checkStatus = ArrayChecker().
                                    SetArray1(pArray).
                                    SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_ANSI_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(-1)
    const LONG lRet = DTWAIN_ArrayFind(pArray, (LPVOID)pString);
    LOG_FUNC_EXIT_PARAMS(lRet)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_BOOL  DLLENTRY_DEF DTWAIN_ArraySetAt( DTWAIN_ARRAY pArray, LONG lPos, LPVOID pVariant )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lPos, pVariant))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // Check if array exists
    const auto checkStatus = ArrayChecker().
        SetArray1(pArray).
        SetArrayPos(lPos).
        SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false )

    SetArrayValueFromFactory(pArray, lPos, pVariant);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtLong(DTWAIN_ARRAY pArray, LONG nWhere, LONG Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(pArray,nWhere,&Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtLong64(DTWAIN_ARRAY pArray, LONG nWhere, LONG64 Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(pArray,nWhere,&Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtFloat(DTWAIN_ARRAY pArray, LONG nWhere, DTWAIN_FLOAT Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, Val))
    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(pArray,nWhere,&Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtString(DTWAIN_ARRAY pArray, LONG nWhere, LPCTSTR pStr)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pStr))
    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(pArray,nWhere,(LPVOID)pStr);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArraySetAtWideString(DTWAIN_ARRAY pArray, LONG nWhere, LPCWSTR pStr)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pStr))
    const auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_WIDE_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)

    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(pArray, nWhere, (LPVOID)pStr);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF  DTWAIN_ArraySetAtANSIString(DTWAIN_ARRAY pArray, LONG nWhere, LPCSTR pStr)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhere, pStr))
    const auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_ANSI_TYPE);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(pArray, nWhere, (LPVOID)pStr);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

LPVOID DLLENTRY_DEF DTWAIN_ArrayGetBuffer( DTWAIN_ARRAY pArray, LONG nOffset )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nOffset))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, NULL, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory; 

    // Check if array exists
    const auto checkStatus = ArrayChecker().SetArray1(pArray).SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS);
    if (checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(DTWAIN_ARRAY(NULL))

    // Get correct array type
    const LPVOID v = factory->get_buffer(pArray, nOffset);
    LOG_FUNC_EXIT_PARAMS(v)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

/* Range functions */
DTWAIN_RANGE DLLENTRY_DEF DTWAIN_RangeCreate(LONG nEnumType)
{
    LOG_FUNC_ENTRY_PARAMS((nEnumType))
    const auto Array = static_cast<DTWAIN_RANGE>(CreateArrayFromFactory(nEnumType, 5));
    if ( !Array )
        LOG_FUNC_EXIT_PARAMS(NULL)
    LOG_FUNC_EXIT_PARAMS(Array)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

DTWAIN_RANGE DLLENTRY_DEF DTWAIN_RangeCreateFromCap(DTWAIN_SOURCE Source, LONG lCapType)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lCapType))
    const auto Array = static_cast<DTWAIN_RANGE>(DTWAIN_ArrayCreateFromCap(Source, lCapType, 5));
    if ( !Array )
        LOG_FUNC_EXIT_PARAMS(NULL)
    LOG_FUNC_EXIT_PARAMS(Array)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeDestroy(DTWAIN_RANGE Range)
{
    LOG_FUNC_ENTRY_PARAMS((Range))
    const DTWAIN_BOOL bRet = DTWAIN_ArrayDestroy(static_cast<DTWAIN_ARRAY>(Range));
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeIsValid(DTWAIN_RANGE Range, LPLONG pStatus )
{
    LOG_FUNC_ENTRY_PARAMS((Range, pStatus))
    LONG lCurStatus;
    if ( (lCurStatus = IsValidRangeArray(Range)) != 1 )
    {
        if ( pStatus )
            *pStatus = lCurStatus;
        LOG_FUNC_EXIT_PARAMS(false)
    }
    if (pStatus)
        *pStatus = 1;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

static LONG IsValidRangeArray( DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((pArray))

    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, DTWAIN_ERR_BAD_HANDLE, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory; 

    // Check if array is a valid type for ranges
    auto enumType = factory->tag_type(pArray);
    if ( enumType != CTL_ArrayFactory::arrayTag::LongType && enumType != CTL_ArrayFactory::arrayTag::DoubleType)
        LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_WRONG_ARRAY_TYPE)

    // Check if (low < high) and 0 < step < (high - low)
    if ( enumType == CTL_ArrayFactory::arrayTag::LongType)
    {
        auto& pVals = factory->underlying_container_t<LONG>(pArray);

        // Check if the array has 5 elements
        if ( pVals.size() != 5 )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_BAD_ARRAY)

        const LONG lLow = pVals[0];
        const LONG lUp = pVals[1];
        const LONG lStep = pVals[2];

        LOG_FUNC_VALUES_EX((lLow, lUp, lStep))

        if ( lLow > lUp )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_INVALID_RANGE)
        if ( lStep < 0 )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_INVALID_RANGE)
         if ( lStep == 0 && lLow < lUp )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_INVALID_RANGE)
    }
    else
    {
        auto& pVals = factory->underlying_container_t<double>(pArray);

        // Check if the array has 5 elements
        if (pVals.size() != 5)
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_BAD_ARRAY)
        const double dLow = pVals[0];
        const double dUp = pVals[1];
        const double dStep = pVals[2];
        LOG_FUNC_VALUES_EX((dLow, dUp, dStep))
        if ( dLow > dUp )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_INVALID_RANGE)
        if ( dStep < 0 )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_INVALID_RANGE)
        if ( float_equal(dStep,0.0) && dLow < dUp )
            LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_INVALID_RANGE)
    }
    LOG_FUNC_EXIT_PARAMS(1)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetValue( DTWAIN_RANGE pArray, LONG nWhich, LPVOID pVariant )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, pVariant))
    const LONG lResult = IsValidRangeArray( pArray );
    if ( lResult != 1 && lResult != DTWAIN_ERR_INVALID_RANGE)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = DTWAIN_ArraySetAt(static_cast<DTWAIN_ARRAY>(pArray), nWhich, pVariant);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeSetValueLong( DTWAIN_RANGE pArray, LONG nWhich,LONG Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, Val))
    const DTWAIN_BOOL bRet = DTWAIN_RangeSetValue(pArray,nWhich,&Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeSetValueFloat( DTWAIN_RANGE pArray, LONG nWhich,DTWAIN_FLOAT Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, Val))
    const DTWAIN_BOOL bRet = DTWAIN_RangeSetValue(pArray,nWhich,&Val);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeSetValueFloatString( DTWAIN_RANGE pArray, LONG nWhich, LPCTSTR Val)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, Val))
    const double d = StringWrapper::ToDouble(Val);
    const DTWAIN_BOOL bRet = DTWAIN_RangeSetValueFloat(pArray,nWhich, d);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetValue( DTWAIN_RANGE pArray, LONG nWhich, LPVOID pVariant )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, pVariant))
    const LONG lResult = IsValidRangeArray( pArray );
    if ( lResult != 1 && lResult != DTWAIN_ERR_INVALID_RANGE)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = DTWAIN_ArrayGetAt(static_cast<DTWAIN_ARRAY>(pArray), nWhich, pVariant);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetValueLong( DTWAIN_RANGE pArray, LONG nWhich,LPLONG pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetValue(pArray,nWhich,pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetValueFloat( DTWAIN_RANGE pArray, LONG nWhich, LPDTWAIN_FLOAT pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetValue(pArray,nWhich,pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetValueFloatString( DTWAIN_RANGE pArray, LONG nWhich, LPTSTR pVal)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, nWhich, pVal))
    double d;
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetValueFloat(pArray, nWhich, &d);
    if ( bRet )
    {
        StringStreamA strm;
        strm << boost::format("%1%") % d;
        StringWrapper::SafeStrcpy(pVal, StringConversion::Convert_Ansi_To_Native(strm.str()).c_str());
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

template <typename Traits>
void DTWAIN_RangeSetter(DTWAIN_ARRAY a,
                        LPVOID valueLow,
                        LPVOID valueUp,
                        LPVOID valueStep,
                        LPVOID valueCurrent,
                        LPVOID pDefault)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    Traits* pBuffer = static_cast<Traits*>(pHandle->m_ArrayFactory->get_buffer(a, 0));
    pBuffer[DTWAIN_RANGEMIN] = *static_cast<typename Traits*>(valueLow);
    pBuffer[DTWAIN_RANGEMAX] = *static_cast<typename Traits*>(valueUp);
    pBuffer[DTWAIN_RANGESTEP] = *static_cast<typename Traits*>(valueStep);

    if ( pDefault )
        pBuffer[DTWAIN_RANGEDEFAULT] = *static_cast<Traits*>(pDefault);
    else
        pBuffer[DTWAIN_RANGEDEFAULT] = {};

    if (valueCurrent)
        pBuffer[DTWAIN_RANGECURRENT] = *static_cast<Traits*>(valueCurrent);
    else
        pBuffer[DTWAIN_RANGECURRENT] = {};
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeSetAll( DTWAIN_RANGE pArray, LPVOID pVariantLow,
                                            LPVOID pVariantUp, LPVOID pVariantStep,
                                            LPVOID pDefault, LPVOID pCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariantLow,pVariantUp, pVariantStep,pDefault, pCurrent ))

    const LONG lResult = IsValidRangeArray( pArray );
    if ( lResult != 1 && lResult != DTWAIN_ERR_INVALID_RANGE)
        LOG_FUNC_EXIT_PARAMS(false)

    if ( DTWAIN_ArrayGetType(pArray) == DTWAIN_ARRAYLONG )
        DTWAIN_RangeSetter<LONG>(pArray, pVariantLow, pVariantUp, pVariantStep, pCurrent, pDefault);
    else
        DTWAIN_RangeSetter<double>(pArray, pVariantLow, pVariantUp, pVariantStep, pCurrent, pDefault);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeSetAllLong( DTWAIN_RANGE pArray, LONG lLow,
                                                        LONG lUp, LONG lStep,
                                                        LONG lDefault,
                                                        LONG lCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lLow,lUp,lStep,lDefault,lCurrent ))
    const DTWAIN_BOOL bRet = DTWAIN_RangeSetAll(pArray,&lLow,&lUp,&lStep,&lDefault,&lCurrent);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeSetAllFloat( DTWAIN_RANGE pArray, DTWAIN_FLOAT dLow,
                                                         DTWAIN_FLOAT dUp, DTWAIN_FLOAT dStep,
                                                         DTWAIN_FLOAT dDefault,
                                                         DTWAIN_FLOAT dCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, dLow, dUp,dStep,dDefault,dCurrent ))
    const DTWAIN_BOOL bRet = DTWAIN_RangeSetAll(pArray,&dLow,&dUp,&dStep,&dDefault,&dCurrent);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeSetAllFloatString( DTWAIN_RANGE pArray, LPCTSTR dLow,
                                                                LPCTSTR dUp, LPCTSTR dStep, LPCTSTR dDefault,LPCTSTR dCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, dLow, dUp,dStep,dDefault,dCurrent ))
    double d[5];
    LPCTSTR* vals[] = {&dLow, &dUp, &dStep, &dDefault, &dCurrent};
    for (int i = 0; i < 5; ++i )
      d[i] = StringWrapper::ToDouble(*vals[i]);
    const DTWAIN_BOOL bRet = DTWAIN_RangeSetAllFloat(pArray, d[0], d[1], d[2], d[3], d[4]);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetAll( DTWAIN_RANGE pArray, LPVOID pVariantLow,
                                            LPVOID pVariantUp, LPVOID pVariantStep,
                                            LPVOID pDefault, LPVOID pCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariantLow,pVariantUp, pVariantStep,pDefault, pCurrent ))
    const LONG lResult = IsValidRangeArray( pArray );
    if ( lResult != 1 && lResult != DTWAIN_ERR_INVALID_RANGE)
        LOG_FUNC_EXIT_PARAMS(false)
    
    if ( pVariantLow )
        DTWAIN_ArrayGetAt(pArray, DTWAIN_RANGEMIN, pVariantLow);
    if ( pVariantUp )
        DTWAIN_ArrayGetAt(pArray, DTWAIN_RANGEMAX, pVariantUp);
    if ( pVariantStep )
        DTWAIN_ArrayGetAt(pArray, DTWAIN_RANGESTEP, pVariantStep);
    if ( pDefault )
        DTWAIN_ArrayGetAt(pArray, DTWAIN_RANGEDEFAULT, pDefault);
    if ( pCurrent )
        DTWAIN_ArrayGetAt(pArray, DTWAIN_RANGECURRENT, pCurrent);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetAllLong( DTWAIN_RANGE pArray, LPLONG lLow,
                                                        LPLONG lUp, LPLONG lStep,
                                                        LPLONG lDefault,
                                                        LPLONG lCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lLow, lUp, lStep, lDefault, lCurrent ))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetAll(pArray,lLow,lUp,lStep,lDefault,lCurrent);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetAllFloat( DTWAIN_RANGE pArray, LPDTWAIN_FLOAT dLow,
                                                         LPDTWAIN_FLOAT dUp, LPDTWAIN_FLOAT dStep,
                                                         LPDTWAIN_FLOAT dDefault,
                                                         LPDTWAIN_FLOAT dCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, dLow, dUp, dStep, dDefault, dCurrent ))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetAll(pArray,dLow,dUp,dStep,dDefault,dCurrent);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetAllFloatString( DTWAIN_RANGE pArray, LPTSTR dLow,
                                                         LPTSTR dUp, LPTSTR dStep,
                                                         LPTSTR dDefault,
                                                         LPTSTR dCurrent )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, dLow, dUp, dStep, dDefault, dCurrent ))
    double d[5];
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetAllFloat(pArray, &d[0], &d[1], &d[2], &d[3], &d[4]);
    if ( bRet )
    {
        LPTSTR* vals[] = { &dLow, &dUp, &dStep, &dDefault, &dCurrent };
        StringStreamA strm;
        for (int i = 0; i < 5; ++i )
        {
            strm << boost::format("%1%") % d[i];
            StringWrapper::SafeStrcpy(*vals[i], StringConversion::Convert_Ansi_To_Native(strm.str()).c_str());
            strm.str("");
        }
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_RangeGetCount( DTWAIN_RANGE pArray )
{
    LOG_FUNC_ENTRY_PARAMS((pArray))
    if (IsValidRangeArray( static_cast<DTWAIN_ARRAY>(pArray) ) < 0 )
        LOG_FUNC_EXIT_PARAMS(DTWAIN_FAILURE1)

    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

    // Check if (low < high) and 0 < step < (high - low)
    LONG nNumItems;
    auto eType = pHandle->m_ArrayFactory->tag_type(pArray);
    if ( eType == CTL_ArrayFactory::arrayTag::LongType)
    {
        LONG* pBuffer = static_cast<LONG*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));
        nNumItems = abs(pBuffer[DTWAIN_RANGEMAX] - pBuffer[DTWAIN_RANGEMIN]) / pBuffer[DTWAIN_RANGESTEP] + 1;
    }
    else
    {
        double * pBuffer = static_cast<double*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));
        nNumItems = static_cast<LONG>(fabs(pBuffer[DTWAIN_RANGEMAX] - pBuffer[DTWAIN_RANGEMIN]) / pBuffer[DTWAIN_RANGESTEP] + 1);
    }
    LOG_FUNC_EXIT_PARAMS(nNumItems)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetExpValue( DTWAIN_RANGE pArray, LONG lPos, LPVOID pVariant )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lPos, pVariant))
    if (IsValidRangeArray( static_cast<DTWAIN_ARRAY>(pArray) ) < 0 )
        LOG_FUNC_EXIT_PARAMS(false)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return lPos < 0;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    // Check if (low < high) and 0 < step < (high - low)
    auto eType = pHandle->m_ArrayFactory->tag_type(pArray);
    if ( eType == CTL_ArrayFactory::arrayTag::LongType)
    {
        LONG* pBuffer = static_cast<LONG*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));
        LONG* pLong = static_cast<LONG*>(pVariant);
        *pLong = pBuffer[DTWAIN_RANGEMIN] + pBuffer[DTWAIN_RANGESTEP] * lPos;
    }
    else
    {
        const auto pFloat = static_cast<double*>(pVariant);
        double* pBuffer = static_cast<double*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));
        *pFloat = pBuffer[DTWAIN_RANGEMIN] + pBuffer[DTWAIN_RANGESTEP] * lPos; 
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetExpValueLong( DTWAIN_RANGE pArray, LONG lPos, LPLONG pVal )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lPos, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetExpValue(pArray,lPos,pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetExpValueFloat( DTWAIN_RANGE pArray, LONG lPos, LPDTWAIN_FLOAT pVal )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lPos, pVal))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetExpValue(pArray,lPos,pVal);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL    DLLENTRY_DEF      DTWAIN_RangeGetExpValueFloatString( DTWAIN_RANGE pArray, LONG lPos, LPTSTR pVal )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lPos, pVal))
    double d;
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetExpValueFloat(pArray,lPos,&d);
    if ( bRet )
    {
        StringStreamA strm;
        strm << boost::format("%1%") % d;
        StringWrapper::SafeStrcpy(pVal, StringConversion::Convert_Ansi_To_Native(strm.str()).c_str());
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPos( DTWAIN_RANGE pArray, LPVOID pVariant, LPLONG pPos )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariant, pPos))
    if (IsValidRangeArray( static_cast<DTWAIN_ARRAY>(pArray) ) < 0 )
        LOG_FUNC_EXIT_PARAMS(false)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    auto eType = pHandle->m_ArrayFactory->tag_type(pArray);
    if ( eType == CTL_ArrayFactory::arrayTag::LongType )
    {
        auto pLong = static_cast<LONG*>(pVariant);
        LONG* pBuffer = static_cast<LONG*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));
        if (pBuffer[DTWAIN_RANGESTEP] == 0 )
            LOG_FUNC_EXIT_PARAMS(false)
        *pPos = (*pLong - pBuffer[DTWAIN_RANGEMIN]) / pBuffer[DTWAIN_RANGESTEP];
    }
    else
    {
        const double *pFloat = static_cast<double*>(pVariant);
        double* pBuffer = static_cast<double*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));
        if (float_equal(0.0, pBuffer[DTWAIN_RANGESTEP]))
            LOG_FUNC_EXIT_PARAMS(false)
        *pPos = static_cast<LONG>((*pFloat - pBuffer[DTWAIN_RANGEMIN]) / pBuffer[DTWAIN_RANGESTEP]);
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPosFloat( DTWAIN_RANGE pArray, DTWAIN_FLOAT pVariant, LPLONG pPos )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariant, pPos))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetPos(pArray, &pVariant, pPos);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPosFloatString( DTWAIN_RANGE pArray, LPCTSTR Val, LPLONG pPos )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Val, pPos))
    const double d = StringWrapper::ToDouble(Val);
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetPosFloat(pArray, d, pPos);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetPosLong( DTWAIN_RANGE pArray, LONG Value, LPLONG pPos )
{
    LOG_FUNC_ENTRY_PARAMS((pArray, Value, pPos))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetPos(pArray, &Value, pPos);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeExpand(DTWAIN_RANGE Range, LPDTWAIN_ARRAY Array)
{
    LOG_FUNC_ENTRY_PARAMS((Range, Array))
    if (IsValidRangeArray( static_cast<DTWAIN_ARRAY>(Range) ) < 0 )
        LOG_FUNC_EXIT_PARAMS(false)

    // Check if array exists
    // Instance of class that takes an array that takes a handle (pointer)
    const LONG nArrayType = DTWAIN_ArrayGetType(Range);
    const LONG lCount = DTWAIN_RangeGetCount( Range );

    const DTWAIN_ARRAY pDest = CreateArrayFromFactory( nArrayType, lCount );
    if ( !pDest )
        LOG_FUNC_EXIT_PARAMS(false)

    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto eType = pHandle->m_ArrayFactory->tag_type(Range);

    if ( eType == CTL_ArrayFactory::arrayTag::LongType )
    {
        LONG* pBuffer = static_cast<LONG*>(pHandle->m_ArrayFactory->get_buffer(Range, 0));
        LONG* pArrayBuf = static_cast<LONG*>(pHandle->m_ArrayFactory->get_buffer(pDest, 0));
        LONG i = 0;
        std::transform(pArrayBuf, pArrayBuf + lCount, pArrayBuf, [&](int /*n*/)
        {
            const int retVal = static_cast<int>(pBuffer[DTWAIN_RANGEMIN] + pBuffer[DTWAIN_RANGESTEP] * i);
            ++i;
            return retVal;
        });
    }
    else
    {
        LONG i = 0;
        double* pBuffer = static_cast<double*>(pHandle->m_ArrayFactory->get_buffer(Range, 0));
        double *pArrayBuf =  static_cast<double*>(pHandle->m_ArrayFactory->get_buffer(pDest, 0));
        std::transform(pArrayBuf, pArrayBuf + lCount, pArrayBuf, [&](double /*d*/)
        {
            const double dValue = pBuffer[DTWAIN_RANGEMIN] + pBuffer[DTWAIN_RANGESTEP] * i;
            ++i;
            return dValue;
        });
    }
    // Destroy the user array if one is supplied
    if (pHandle->m_ArrayFactory->is_valid(*Array))
        pHandle->m_ArrayFactory->destroy(*Array);

    *Array = pDest;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeGetNearestValue( DTWAIN_RANGE pArray, LPVOID pVariantIn,
                                                     LPVOID pVariantOut, LONG RoundType)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, pVariantIn, pVariantOut, RoundType))
    if (IsValidRangeArray( static_cast<DTWAIN_ARRAY>(pArray) ) < 0 )
        LOG_FUNC_EXIT_PARAMS(false)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

    auto eType = pHandle->m_ArrayFactory->tag_type(pArray);

    if ( eType == CTL_ArrayFactory::arrayTag::LongType )
    {
        // Get the values
        LONG* pBuffer = static_cast<LONG*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));

        // Get the value passed in
        LONG lInVal = *static_cast<LONG*>(pVariantIn);
        LONG *pOutVal = static_cast<LONG*>(pVariantOut);

        // return immediately if step is 0
        if ( pBuffer[DTWAIN_RANGESTEP] == 0 )
        {
            *pOutVal = pBuffer[DTWAIN_RANGEMIN];
            LOG_FUNC_EXIT_PARAMS(true)
        }

        // Check if value passed in is out of bounds
        if ( lInVal < pBuffer[DTWAIN_RANGEMIN])
        {
            *pOutVal = pBuffer[DTWAIN_RANGEMIN];
            LOG_FUNC_EXIT_PARAMS(true)
        }
        else
        if ( lInVal > pBuffer[DTWAIN_RANGEMAX])
        {
            *pOutVal = pBuffer[DTWAIN_RANGEMAX];
            LOG_FUNC_EXIT_PARAMS(true)
        }

        // Get the nearest value to *pVariantIn;
        // First get the bias value from 0
        LONG lBias = 0;
        if (pBuffer[DTWAIN_RANGEMIN] != 0 )
            lBias = -pBuffer[DTWAIN_RANGEMIN];

        lInVal += lBias;

        const LONG Remainder = abs(lInVal % pBuffer[DTWAIN_RANGESTEP]);
        const LONG Dividend = lInVal / pBuffer[DTWAIN_RANGESTEP];

        if ( Remainder == 0 )
        {
            *pOutVal = lInVal - lBias;
            LOG_FUNC_EXIT_PARAMS(true)
        }

        // Check if round to lowest or highest valid value
        if ( RoundType == DTWAIN_ROUNDNEAREST )
        {
            if ( Remainder >= abs(pBuffer[DTWAIN_RANGESTEP]) / 2 )
                RoundType = DTWAIN_ROUNDUP;
            else
                RoundType = DTWAIN_ROUNDDOWN;
        }

        if ( RoundType == DTWAIN_ROUNDDOWN )
            *pOutVal = Dividend * pBuffer[DTWAIN_RANGESTEP]- lBias;
        else
            if ( RoundType == DTWAIN_ROUNDUP )
                *pOutVal = (Dividend + 1) * pBuffer[DTWAIN_RANGESTEP] - lBias;
        LOG_FUNC_EXIT_PARAMS(true)
    }
    else
    if ( eType == CTL_ArrayDoubleType )
    {
        // Get the values
        double* pBuffer = static_cast<double*>(pHandle->m_ArrayFactory->get_buffer(pArray, 0));

        // Get the value passed in
        double dInVal = *static_cast<double*>(pVariantIn);
        const auto pOutVal = static_cast<double*>(pVariantOut);

        // Check if value passed in is out of bounds
        if (float_equal(pBuffer[DTWAIN_RANGEMIN], dInVal) ||
            float_equal(0.0, pBuffer[DTWAIN_RANGESTEP]) ||
            dInVal < pBuffer[DTWAIN_RANGEMIN])
        {
            *pOutVal = pBuffer[DTWAIN_RANGEMIN];
            LOG_FUNC_EXIT_PARAMS(true)
        }
        else
        if (float_equal(pBuffer[DTWAIN_RANGEMAX], dInVal) || dInVal > pBuffer[DTWAIN_RANGEMAX])
        {
            *pOutVal = pBuffer[DTWAIN_RANGEMAX];
            LOG_FUNC_EXIT_PARAMS(true)
        }

        // Get the nearest value to *pVariantIn;
        // First get the bias value from 0
        double dBias = 0;
        if (pBuffer[DTWAIN_RANGEMIN] != 0 )
            dBias = -pBuffer[DTWAIN_RANGEMIN];

        dInVal += dBias;
        const double Remainder = fabs(fmod(dInVal, pBuffer[DTWAIN_RANGESTEP]));
        const double Dividend = static_cast<double>(static_cast<LONG>(dInVal / pBuffer[DTWAIN_RANGESTEP]));

        if ( float_equal(Remainder,0.0 ))
        {
            *pOutVal = dInVal - dBias;
            LOG_FUNC_EXIT_PARAMS(true)
        }

        // Check if round to lowest or highest valid value
        if ( RoundType == DTWAIN_ROUNDNEAREST )
        {
            if ( Remainder >= fabs(pBuffer[DTWAIN_RANGESTEP]) / 2.0 )
                RoundType = DTWAIN_ROUNDUP;
            else
                RoundType = DTWAIN_ROUNDDOWN;
        }
        if ( RoundType == DTWAIN_ROUNDDOWN )
            *pOutVal = Dividend * pBuffer[DTWAIN_RANGESTEP]- dBias;
        else
            if ( RoundType == DTWAIN_ROUNDUP )
                *pOutVal = (Dividend + 1.0) * pBuffer[DTWAIN_RANGESTEP]- dBias;
        LOG_FUNC_EXIT_PARAMS(true)
    }
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeNearestValueLong( DTWAIN_RANGE pArray, LONG lIn,
                                                              LPLONG pOut, LONG RoundType)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, lIn, pOut, RoundType))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetNearestValue(pArray,&lIn,pOut,RoundType);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeNearestValueFloat( DTWAIN_RANGE pArray, DTWAIN_FLOAT dIn,
                                                             LPDTWAIN_FLOAT pOut, LONG RoundType)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, dIn, pOut, RoundType))
    const DTWAIN_BOOL bRet = DTWAIN_RangeGetNearestValue(pArray,&dIn,pOut,RoundType);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_RangeNearestValueFloatString( DTWAIN_RANGE pArray, LPCTSTR dIn,
                                                                      LPTSTR pOut, LONG RoundType)
{
    LOG_FUNC_ENTRY_PARAMS((pArray, dIn, pOut, RoundType))
    const double d = StringWrapper::ToDouble(dIn);
    double dOut;
    const DTWAIN_BOOL bRet = DTWAIN_RangeNearestValueFloat(pArray, d, &dOut,RoundType);
    if ( bRet )
    {
        StringStreamA strm;
        strm << boost::format("%1%") % dOut;
        StringWrapper::SafeStrcpy(pOut, StringConversion::Convert_Ansi_To_Native(strm.str()).c_str());
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

///////////////////////////////////////////////// Frame functions //////////////////////////////////////////
DTWAIN_FRAME DLLENTRY_DEF DTWAIN_FrameCreate(DTWAIN_FLOAT Left, DTWAIN_FLOAT Top, DTWAIN_FLOAT Right, DTWAIN_FLOAT Bottom)
{
    LOG_FUNC_ENTRY_PARAMS((Left, Top, Right, Bottom))
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);
    const auto newFrame = dynarithmic::CreateFrameArray(pHandle, Left, Top, Right, Bottom);
    LOG_FUNC_EXIT_PARAMS(newFrame)
    CATCH_BLOCK(nullptr)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayFrameGetAt(DTWAIN_ARRAY FrameArray, LONG nWhere,
                                                LPDTWAIN_FLOAT pleft, LPDTWAIN_FLOAT ptop,
                                                LPDTWAIN_FLOAT pright, LPDTWAIN_FLOAT pbottom )
{
    LOG_FUNC_ENTRY_PARAMS((FrameArray, nWhere, pleft, ptop, pright, pbottom))
    DTWAIN_BOOL bRet = FALSE;
    DTWAIN_FRAME Frame = DTWAIN_ArrayFrameGetFrameAt(FrameArray, nWhere);
    if ( Frame )
    {
        bRet = TRUE;
        const TwainFrameInternal& theFrame = *static_cast<TwainFrameInternal*>(Frame);
        if ( pleft )
            *pleft = theFrame.Left();
        if ( ptop )
            *ptop = theFrame.Top();
        if ( pright )
            *pright = theFrame.Right();
        if ( pbottom )
            *pbottom = theFrame.Bottom();
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(FALSE)
}

DTWAIN_FRAME DLLENTRY_DEF DTWAIN_ArrayFrameGetFrameAt(DTWAIN_ARRAY FrameArray, LONG nWhere )
{
    LOG_FUNC_ENTRY_PARAMS((FrameArray, nWhere))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto checkStatus = ArrayChecker().SetArray1(FrameArray).SetArrayPos(nWhere).
                    SetExplicitTypeCheck(CTL_ArrayFactory::arrayTag::FrameType).
                    SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_BOUNDS | ArrayChecker::CHECK_ARRAY_EXPLICIT_TYPE);
    if ( checkStatus.Check() != DTWAIN_NO_ERROR )
        LOG_FUNC_EXIT_PARAMS(DTWAIN_ARRAY(NULL))

    const auto& factory = pHandle->m_ArrayFactory;
    auto buffer = static_cast<TwainFrameInternal*>(factory->get_buffer(FrameArray, nWhere));
    LOG_FUNC_EXIT_PARAMS(buffer)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayFrameSetAt(DTWAIN_ARRAY FrameArray, LONG nWhere, DTWAIN_FLOAT left, DTWAIN_FLOAT top, DTWAIN_FLOAT right, DTWAIN_FLOAT bottom )
{
    LOG_FUNC_ENTRY_PARAMS((FrameArray, nWhere, left, top, right, bottom))
    DTWAIN_BOOL bRet = FALSE;
    DTWAIN_FRAME Frame = DTWAIN_ArrayFrameGetFrameAt(FrameArray, nWhere);
    if (Frame)
    {
        bRet = TRUE;
        TwainFrameInternal& theFrame = *static_cast<TwainFrameInternal*>(Frame);
        theFrame = TwainFrameInternal(left, top, right, bottom);
    }
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayFrameSetFrameAt(DTWAIN_ARRAY FrameArray, LONG nWhere, DTWAIN_FRAME theFrame)
{
    LOG_FUNC_ENTRY_PARAMS((FrameArray, nWhere, theFrame))
    DTWAIN_BOOL bRet = FALSE;
    if ( !DTWAIN_FrameIsValid(theFrame))
        LOG_FUNC_EXIT_PARAMS(false)
    double left, top, right, bottom;
    DTWAIN_FrameGetAll(theFrame, &left, &top, &right, &bottom);
    bRet = DTWAIN_ArrayFrameSetAt(FrameArray, nWhere, left, top, right, bottom);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameIsValid(DTWAIN_FRAME Frame)
{
    LOG_FUNC_ENTRY_PARAMS((Frame))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    const auto checkStatus = ArrayChecker().SetArray1(Frame).
                SetExplicitTypeCheck(CTL_ArrayFactory::arrayTag::FrameSingleType).
                SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_EXPLICIT_TYPE);
    if ( checkStatus.Check() != DTWAIN_NO_ERROR)
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return false; },
                                          DTWAIN_ERR_INVALID_DTWAIN_FRAME, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameDestroy(DTWAIN_FRAME Frame)
{
    LOG_FUNC_ENTRY_PARAMS((Frame))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !Frame )
        LOG_FUNC_EXIT_PARAMS(true)
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    bool isValid = DTWAIN_FrameIsValid(Frame);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !isValid;},
                                      DTWAIN_ERR_INVALID_DTWAIN_FRAME, false, FUNC_MACRO);
    DestroyFrameFromFactory(Frame);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetAll(DTWAIN_FRAME Frame,DTWAIN_FLOAT Left,
                                            DTWAIN_FLOAT Top, DTWAIN_FLOAT Right,
                                            DTWAIN_FLOAT Bottom)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, Left, Top, Right, Bottom))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    bool frameValid = DTWAIN_FrameIsValid(Frame);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !frameValid; },
                                      DTWAIN_ERR_INVALID_DTWAIN_FRAME, false, FUNC_MACRO);
    auto& pPtr = *static_cast<TwainFrameInternal*>(Frame);
    pPtr = TwainFrameInternal(Left, Top, Right, Bottom);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetAll(DTWAIN_FRAME Frame, LPDTWAIN_FLOAT Left,
                                            LPDTWAIN_FLOAT Top, LPDTWAIN_FLOAT Right,
                                            LPDTWAIN_FLOAT Bottom)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, Left, Top, Right, Bottom))

    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    bool frameValid = DTWAIN_FrameIsValid(Frame);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !frameValid; },
                                      DTWAIN_ERR_INVALID_DTWAIN_FRAME, false, FUNC_MACRO);
    auto& vOne = pHandle->m_ArrayFactory->underlying_container_t<TwainFrameInternal>(Frame);
    auto& pPtr = vOne.front();

    *Left =   pPtr.Left();
    *Top =    pPtr.Top();
    *Right =  pPtr.Right();
    *Bottom = pPtr.Bottom();
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetValue(DTWAIN_FRAME Frame, LONG nWhich, LPDTWAIN_FLOAT Value)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, nWhich, Value))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    bool frameValid = DTWAIN_FrameIsValid(Frame);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !frameValid; },
                                      DTWAIN_ERR_INVALID_DTWAIN_FRAME, false, FUNC_MACRO);
    const bool bCheck = TwainFrameInternal::IsValidComponent(nWhich);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !bCheck;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    auto pPtr = static_cast<TwainFrameInternal*>(Frame);
    *Value = pPtr->m_FrameComponent[nWhich];
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetValue(DTWAIN_FRAME Frame, LONG nWhich, DTWAIN_FLOAT Value)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, nWhich, Value))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    bool frameValid = DTWAIN_FrameIsValid(Frame);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !frameValid; },
                                      DTWAIN_ERR_INVALID_DTWAIN_FRAME, false, FUNC_MACRO);
    const bool bCheck = TwainFrameInternal::IsValidComponent(nWhich);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !bCheck;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    auto pPtr = static_cast<TwainFrameInternal*>(Frame);
    pPtr->m_FrameComponent[nWhich] = Value;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_FRAME DLLENTRY_DEF DTWAIN_FrameCreateString(LPCTSTR Left, LPCTSTR Top, LPCTSTR Right, LPCTSTR Bottom)
{
    LOG_FUNC_ENTRY_PARAMS((Left, Top, Right, Bottom))
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);
    const double leftD    = StringWrapper::ToDouble(Left);
    const double topD     = StringWrapper::ToDouble(Top);
    const double rightD   = StringWrapper::ToDouble(Right);
    const double bottomD  = StringWrapper::ToDouble(Bottom);
    const DTWAIN_FRAME newFrame = CreateFrameArray(pHandle, leftD, topD, rightD, bottomD);
    LOG_FUNC_EXIT_PARAMS(newFrame)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetAllString(DTWAIN_FRAME Frame, LPCTSTR Left, LPCTSTR Top, LPCTSTR Right, LPCTSTR Bottom)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, Left, Top, Right, Bottom))
    const double leftD    = StringWrapper::ToDouble(Left);
    const double topD     = StringWrapper::ToDouble(Top);
    const double rightD   = StringWrapper::ToDouble(Right);
    const double bottomD  = StringWrapper::ToDouble(Bottom);
    const DTWAIN_BOOL bRet = DTWAIN_FrameSetAll(Frame, leftD, topD, rightD, bottomD);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetAllString(DTWAIN_FRAME Frame, LPTSTR Left, LPTSTR Top, LPTSTR Right, LPTSTR Bottom)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, Left, Top, Right, Bottom))
    std::array<double, 4> aFrameComponent;
    const DTWAIN_BOOL bRet = DTWAIN_FrameGetAll(Frame, &aFrameComponent[0], &aFrameComponent[1], &aFrameComponent[2], &aFrameComponent[3]);
    if ( !bRet )
        LOG_FUNC_EXIT_PARAMS(bRet)
    std::array<LPTSTR*, 4> vals= {&Left, &Top, &Right, &Bottom};
    StringStreamA strm;
    for (size_t i = 0; i < aFrameComponent.size(); ++i )
    {
        strm << boost::format("%1%") % aFrameComponent[i];
        StringWrapper::SafeStrcpy(*vals[i], StringConversion::Convert_Ansi_To_Native(strm.str()).c_str());
        strm.str("");
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameGetValueString(DTWAIN_FRAME Frame, LONG nWhich, LPTSTR Value)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, nWhich, Value))
    double d;
    const DTWAIN_BOOL bRet = DTWAIN_FrameGetValue(Frame, nWhich, &d);
    if ( !bRet )
        LOG_FUNC_EXIT_PARAMS(bRet)
    StringStreamA strm;
    strm << boost::format("%1%") % d;
    StringWrapper::SafeStrcpy(Value, StringConversion::Convert_Ansi_To_Native(strm.str()).c_str());
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FrameSetValueString(DTWAIN_FRAME Frame, LONG nWhich, LPCTSTR Value)
{
    LOG_FUNC_ENTRY_PARAMS((Frame, nWhich, Value))
    const double dFrameComponent = StringWrapper::ToDouble(Value);
    const DTWAIN_BOOL bRet = DTWAIN_FrameSetValue(Frame, nWhich, dFrameComponent);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayDestroyFrames(DTWAIN_ARRAY FrameArray)
{
    LOG_FUNC_ENTRY_PARAMS((FrameArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    auto pVariant = FrameArray;
    const auto& factory = pHandle->m_ArrayFactory;

    auto checkStatus = ArrayChecker().SetArray1(FrameArray).SetExplicitTypeCheck(CTL_ArrayFactory::arrayTag::FrameType).
    SetCheckType(ArrayChecker::CHECK_ARRAY_EXISTS | ArrayChecker::CHECK_ARRAY_EXPLICIT_TYPE);
    // Check if array is of the correct type
    if ( checkStatus.Check() != DTWAIN_NO_ERROR)
        LOG_FUNC_EXIT_PARAMS(true)

    // Now destroy the enumerator
    factory->destroy(pVariant);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

////////////////////////////// TW_FIX32 functions //////////////////////////////////////////////
static bool CheckFix32(DTWAIN_ARRAY aFix32, DTWAIN_LONG lPos)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory; 
    // Check if array exists
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !factory->is_valid(aFix32); },
                                      DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);

    // check for out of bounds size
    const size_t Count = factory->size(aFix32);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return Count == 0 || 
														 lPos < 0 || 
														 static_cast<size_t>(lPos) >= Count; }, 
														 DTWAIN_ERR_INDEX_BOUNDS, false, FUNC_MACRO);
    return true;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayFix32SetAt(DTWAIN_ARRAY aFix32, DTWAIN_LONG lPos, 
                                                DTWAIN_LONG Whole, DTWAIN_LONG Frac)
{
    LOG_FUNC_ENTRY_PARAMS((aFix32, lPos, Whole, Frac))
    const bool bOk = CheckFix32(aFix32, lPos);
    if ( bOk )
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        const auto& factory = pHandle->m_ArrayFactory;
        TW_FIX32 temp = {static_cast<TW_INT16>(Whole), static_cast<TW_UINT16>(Frac)};
        factory->set_value(aFix32, lPos, &temp);
    }
    LOG_FUNC_EXIT_PARAMS(bOk)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ArrayFix32GetAt(DTWAIN_ARRAY aFix32, DTWAIN_LONG lPos, LPLONG Whole, LPLONG Frac)
{
    LOG_FUNC_ENTRY_PARAMS((aFix32, lPos, Whole, Frac))
    const bool bOk = CheckFix32(aFix32, lPos);
    if (bOk)
    {
        const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
        auto& factory = pHandle->m_ArrayFactory;
        TW_FIX32 tempF;
        factory->get_value(aFix32, lPos, &tempF);
        if ( Whole )
            *Whole = tempF.Whole;
        if ( Frac )
            *Frac = tempF.Frac;
    }
    LOG_FUNC_EXIT_PARAMS(bOk)
    CATCH_BLOCK(false)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayConvertFloatToFix32(DTWAIN_ARRAY FloatArray)
{
    LOG_FUNC_ENTRY_PARAMS((FloatArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory;
    const auto FloatArrayV = FloatArray;

    // Check if array exists
    const bool bIsValid = factory->is_valid(FloatArrayV);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return !bIsValid; }, DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);

    auto bIsFloat = factory->tag_type(FloatArrayV);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return bIsFloat != CTL_ArrayFactory::arrayTag::DoubleType; }, DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);

    int nStatus;
    // get count
    const size_t Count = factory->size(FloatArrayV);

    // create a new enumerator
    auto aFix32 = factory->create_array(CTL_ArrayTWFIX32Type, &nStatus, Count);

    // get the underlying vectors
    auto vIn = static_cast<double*>(factory->get_buffer(FloatArrayV, 0));
    auto vOut = static_cast<TW_FIX32Ex*>(factory->get_buffer(aFix32, 0));

    // call transform to create array of TW_FIX32 values
    std::transform(vIn, vIn + Count, vOut, [&](double d) 
					{ return FloatToFix32(static_cast<float>(d)); });

    // remove the old array
    factory->destroy(FloatArrayV);

    LOG_FUNC_EXIT_PARAMS(aFix32)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_ArrayConvertFix32ToFloat(DTWAIN_ARRAY Fix32Array)
{
    LOG_FUNC_ENTRY_PARAMS((Fix32Array))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const auto& factory = pHandle->m_ArrayFactory;

    // Check if array exists
    DTWAIN_Check_Error_Condition_0_Ex(pHandle,
        [&]{return !factory->is_valid(Fix32Array, CTL_ArrayTWFIX32Type );},
                          DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);

    int nStatus;
    const size_t Count = factory->size(Fix32Array);
    const auto aFloat = factory->create_array(CTL_ArrayDoubleType, &nStatus, Count);
    auto& vIn = factory->underlying_container_t<TW_FIX32Ex>(Fix32Array);
    auto& vOut = factory->underlying_container_t<double>(aFloat);
    std::transform(vIn.begin(), vIn.end(), vOut.begin(), Fix32ToFloat);

    // remove the old array
    factory->destroy(Fix32Array);
    LOG_FUNC_EXIT_PARAMS(aFloat)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

bool dynarithmic::DTWAINFRAMEToTWFRAME(DTWAIN_FRAME pDdtwil, pTW_FRAME pTwain)
{
    double Val[4];
    pTW_FIX32 pVal[4];
    pVal[0] = &pTwain->Left;
    pVal[1] = &pTwain->Top;
    pVal[2] = &pTwain->Right;
    pVal[3] = &pTwain->Bottom;

    if ( !DTWAIN_FrameGetAll(pDdtwil, static_cast<LPDTWAIN_FLOAT>(&Val[0]), static_cast<LPDTWAIN_FLOAT>(&Val[1]), static_cast<LPDTWAIN_FLOAT>(&Val[2]), static_cast<LPDTWAIN_FLOAT>(&Val[3])))
        return false;
    for ( int i = 0; i < 4; i++ )
        *pVal[i] = dynarithmic::FloatToFix32( static_cast<float>(Val[i]) );
    return true;
}

bool dynarithmic::TWFRAMEToDTWAINFRAME(TW_FRAME pTwain, DTWAIN_FRAME pDdtwil)
{
    double ValOut[4];
    TW_FIX32 ValIn[4];
    ValIn[0] = pTwain.Left;
    ValIn[1] = pTwain.Top;
    ValIn[2] = pTwain.Right;
    ValIn[3] = pTwain.Bottom;

    std::transform(ValIn, ValIn + 4, ValOut, dynarithmic::Fix32ToFloat);
    if ( !DTWAIN_FrameSetAll(pDdtwil, ValOut[0], ValOut[1], ValOut[2], ValOut[3]) )
        return false;
    return true;
}

static LONG IsValidAcqArray( DTWAIN_ARRAY pArray )
{
    LOG_FUNC_ENTRY_PARAMS((pArray))
    // Instance of class that takes an array that takes a handle (pointer)
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, DTWAIN_ERR_BAD_HANDLE, nullptr);

    // Check if array is a valid type for ranges
    if ( !pHandle->m_ArrayFactory->is_valid(pArray, CTL_ArrayFactory::arrayTag::ArrayOfArrayOfVoidPtrType) )
       LOG_FUNC_EXIT_PARAMS(DTWAIN_ERR_WRONG_ARRAY_TYPE)
    LOG_FUNC_EXIT_PARAMS(1)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetNumAcquisitions(DTWAIN_ARRAY aAcq)
{
    LOG_FUNC_ENTRY_PARAMS((aAcq))
    LONG lError;
    if (( lError = IsValidAcqArray( aAcq )) < 0 )
        LOG_FUNC_EXIT_PARAMS(lError)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const LONG Ret = static_cast<LONG>(pHandle->m_ArrayFactory->size(aAcq));
    LOG_FUNC_EXIT_PARAMS(Ret)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_GetNumAcquiredImages( DTWAIN_ARRAY aAcq, LONG nWhich )
{
    LOG_FUNC_ENTRY_PARAMS((aAcq, nWhich))
    LONG lError;
    if (( lError = IsValidAcqArray( aAcq )) < 0 )
        LOG_FUNC_EXIT_PARAMS(lError)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const auto& factory = pHandle->m_ArrayFactory;
    LONG lCount = static_cast<LONG>(factory->size(aAcq));
    if ( nWhich >= lCount )
        LOG_FUNC_EXIT_PARAMS(DTWAIN_FAILURE1)

    DTWAIN_ARRAY aDib = nullptr;
    aDib = factory->get_value(aAcq, nWhich, nullptr);
    lCount = static_cast<LONG>(factory->size( aDib ));
    LOG_FUNC_EXIT_PARAMS(lCount)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

HANDLE DLLENTRY_DEF DTWAIN_GetAcquiredImage( DTWAIN_ARRAY aAcq, LONG nWhichAcq, LONG nWhichDib )
{
    LOG_FUNC_ENTRY_PARAMS((aAcq, nWhichAcq, nWhichDib))
    if ( nWhichDib < 0 )
        LOG_FUNC_EXIT_PARAMS(NULL)
    const int nDibs = DTWAIN_GetNumAcquiredImages( aAcq, nWhichAcq );
    if ( nDibs == DTWAIN_FAILURE1 || nWhichDib >= nDibs )
        LOG_FUNC_EXIT_PARAMS(NULL)
    DTWAIN_ARRAY aDib = nullptr;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& factory = pHandle->m_ArrayFactory;
    aDib = factory->get_value(aAcq, nWhichAcq, nullptr);
    HANDLE hDib = nullptr;
    hDib = factory->get_value(aDib, nWhichDib, nullptr);
    LOG_FUNC_EXIT_PARAMS(hDib)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_GetAcquiredImageArray(DTWAIN_ARRAY aAcq, LONG nWhichAcq)
{
    LOG_FUNC_ENTRY_PARAMS((aAcq, nWhichAcq))
    const int nDibs = DTWAIN_GetNumAcquiredImages( aAcq, nWhichAcq );
    if ( nDibs == DTWAIN_FAILURE1 )
        LOG_FUNC_EXIT_PARAMS(NULL)

    //use a copy
    DTWAIN_ARRAY aDib = nullptr;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& factory = pHandle->m_ArrayFactory;
    aDib = factory->get_value(aAcq, nWhichAcq, nullptr);
    const DTWAIN_ARRAY aCopy = CreateArrayCopyFromFactory(aDib);
    LOG_FUNC_EXIT_PARAMS(aCopy)
    CATCH_BLOCK(DTWAIN_ARRAY(NULL))
}

template <typename T>
static LONG ArrayStringLength_Internal(DTWAIN_ARRAY theArray, LONG nWhichString)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<typename T::value_type>(theArray);
    const size_t nCount = vValues.size();
    if ( nWhichString < 0 || static_cast<size_t>(nWhichString) > nCount )
        return DTWAIN_ERR_INDEX_BOUNDS;
    return static_cast<LONG>(vValues[nWhichString].size());
}

template <typename T>
struct stringLengthComparer
{
  bool operator()(const T& s1, const T& s2) const
  { return s1.size() < s2.size(); }
};

template <typename T>
static LONG ArrayMaxStringLength_Internal(DTWAIN_ARRAY theArray)
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<typename T::value_type>(theArray);
    auto it = std::max_element(vValues.begin(), vValues.end(), stringLengthComparer<typename T::value_type>());
    return static_cast<LONG>((*it).size());
}

static bool IsValidStringArray(DTWAIN_ARRAY pVariant, int &nWhich)
{
    nWhich = 0;
    auto checkStatus = ArrayChecker().SetArray1(pVariant).SetCheckType(ArrayChecker::CHECK_ARRAY_STRING_TYPE);
    if ( checkStatus.Check() != DTWAIN_NO_ERROR )
        return false;
    nWhich = checkStatus.IsAnsiArray()?2:3;
    return true;
}

LONG DLLENTRY_DEF DTWAIN_ArrayGetStringLength(DTWAIN_ARRAY theArray, LONG nWhichString)
{
    LOG_FUNC_ENTRY_PARAMS((theArray, nWhichString))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // check if array is a string array

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    int nWhich = 0;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !IsValidStringArray(theArray, nWhich); },
                                      DTWAIN_ERR_WRONG_ARRAY_TYPE, DTWAIN_ERR_WRONG_ARRAY_TYPE, FUNC_MACRO);

    LONG retValue;
    if ( nWhich == 2 )
        retValue = ArrayStringLength_Internal<CTL_ArrayFactory::tagged_array_wstring>(theArray, nWhichString);
    else
        retValue = ArrayStringLength_Internal<CTL_ArrayFactory::tagged_array_string>(theArray, nWhichString);

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return retValue == DTWAIN_ERR_INDEX_BOUNDS; },
        DTWAIN_ERR_INDEX_BOUNDS, DTWAIN_ERR_INDEX_BOUNDS, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(retValue)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_ArrayGetMaxStringLength(DTWAIN_ARRAY theArray)
{
    LOG_FUNC_ENTRY_PARAMS((theArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // check if array is a string array

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    int nWhich = 0;
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !IsValidStringArray(theArray, nWhich); },
                                      DTWAIN_ERR_WRONG_ARRAY_TYPE, DTWAIN_ERR_WRONG_ARRAY_TYPE, FUNC_MACRO);
    LONG retValue;
    if ( nWhich == 2 )
        retValue = ArrayMaxStringLength_Internal<CTL_ArrayFactory::tagged_array_wstring>(theArray);
    else
        retValue = ArrayMaxStringLength_Internal<CTL_ArrayFactory::tagged_array_string>(theArray);

    // Check if array exists
    LOG_FUNC_EXIT_PARAMS(retValue)
    CATCH_BLOCK(0)
}

void CTL_TwainDLLHandle::RemoveAllEnumerators()
{
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if ( pHandle )
        pHandle->m_ArrayFactory.reset();
}

