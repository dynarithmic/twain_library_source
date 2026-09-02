/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

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
#ifndef CTLARRAY_H
#define CTLARRAY_H

#include <vector>
#include <algorithm>
#include <limits>
#include <memory>
#include "dtwtype.h"
#include "twain.h"
#include "dtwaindefs.h"
#include "arrayfactory.h"
#include "ctlarraydefs.h"

#define DTWAIN_ARRAY_TO_VOID(p)   ((void*)(p))
#define VOID_TO_DTWAIN_ARRAY(p)   ((DTWAIN_ARRAY)(p))

namespace dynarithmic
{
    class CTL_TwainDLLHandle;
    void ArrayCopyWideToNative(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ArraySource, DTWAIN_ARRAY ArrayDest);
    void ArrayCopyAnsiToNative(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ArraySource, DTWAIN_ARRAY ArrayDest);
    std::shared_ptr<CTL_ArrayFactory>& GetArrayFactoryFromHandle(CTL_TwainDLLHandle* pHandle);
    std::pair<int, DTWAIN_ARRAY> CreateArrayFromFactory(CTL_TwainDLLHandle* pHandle, LONG nEnumType, LONG nInitialSize);
    void DestroyArrayFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray);

    template <typename Container, typename DTWAINArrayType>
    static void CopyContainer(CTL_TwainDLLHandle* pHandle, const Container& theContainer, DTWAIN_ARRAY theArray)
    {
        auto& vect = GetArrayFactoryFromHandle(pHandle)->underlying_container_t<DTWAINArrayType>(theArray);
        std::copy(theContainer.begin(), theContainer.end(), vect.begin());
    }
    
    template <typename Container>
    DTWAIN_ARRAY CreateArrayFromContainer(CTL_TwainDLLHandle* pHandle, const Container& theContainer)
    {
        DTWAIN_ARRAY theArray = {};
        LONG nSize = static_cast<LONG>(theContainer.size());
        if constexpr (std::is_integral_v<typename Container::value_type>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, nSize).second;
            if ( theArray )
                CopyContainer<Container, LONG>(pHandle, theContainer, theArray);
        }
        else
        if constexpr (std::is_floating_point_v<typename Container::value_type>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYFLOAT, nSize).second;
            if ( theArray )
                CopyContainer<Container, double>(pHandle, theContainer, theArray);
        }
        else
        if constexpr (std::is_same_v<typename Container::value_type, std::string>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYANSISTRING, nSize).second;
            if ( theArray )
                CopyContainer<Container, std::string>(pHandle, theContainer, theArray);
        }
        else 
        if constexpr (std::is_same_v<typename Container::value_type, TwainFrameInternal>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYFRAME, nSize).second;
            if ( theArray )
                CopyContainer<Container, TwainFrameInternal>(pHandle, theContainer, theArray);
        }
        return theArray;
    }

    template <typename Container, typename ArrayType>
    Container CreateContainerHelper(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY theArray, size_t maxElements)
    {
        Container theContainer;
        auto& pVector = GetArrayFactoryFromHandle(pHandle)->underlying_container_t<ArrayType>(theArray);
        if ( maxElements == (std::numeric_limits<size_t>::max)())
            std::copy(pVector.begin(), pVector.end(), std::back_inserter(theContainer));
        else
        {
            size_t minToCopy = (std::min)(maxElements, pVector.size());
            std::copy(pVector.begin(), pVector.begin() + minToCopy, std::back_inserter(theContainer));
        }
        return theContainer;
    }
    
    template <typename Container>
    Container CreateContainerFromArray(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY theArray, 
                                        size_t maxElements = (std::numeric_limits<size_t>::max)())
    {
        if (!theArray)
            return {};
        if constexpr (std::is_integral_v<typename Container::value_type>)
            return CreateContainerHelper<Container, LONG>(pHandle, theArray, maxElements);
        else
        if constexpr (std::is_same_v<typename Container::value_type, double>)
            return CreateContainerHelper<Container, double>(pHandle, theArray, maxElements);
        else
        if constexpr (std::is_same_v<typename Container::value_type, std::string>)
            return CreateContainerHelper<Container, std::string>(pHandle, theArray, maxElements);
        return {};
    }

    template <typename ArrayType, typename TwainTypeOut>
    void ConvertArrayInPlace(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY theArray)
    {
        auto& vect = GetArrayFactoryFromHandle(pHandle)->underlying_container_t<ArrayType>(theArray);
        std::transform(vect.begin(), vect.end(), vect.begin(), [&](ArrayType value) { return static_cast<TwainTypeOut>(value); });
    }

    template <typename ArrayType>
    struct DTWAINArrayLowLevel_RAII_Impl
    {
        CTL_TwainDLLHandle* m_pHandle;
        ArrayType m_Array;
        bool m_bDestroy;
        DTWAINArrayLowLevel_RAII_Impl() : m_pHandle{}, m_Array{}, m_bDestroy(true) {}
        DTWAINArrayLowLevel_RAII_Impl(CTL_TwainDLLHandle* pHandle, ArrayType a) : m_pHandle(pHandle), m_Array(a), m_bDestroy(true) {}
        DTWAINArrayLowLevel_RAII_Impl(const DTWAINArrayLowLevel_RAII_Impl&) = delete;
        DTWAINArrayLowLevel_RAII_Impl& operator=(const DTWAINArrayLowLevel_RAII_Impl&) = delete;

        void SetDestroy(bool bSet) { m_bDestroy = bSet; }
        void SetArray(ArrayType arr) { m_Array = arr; }
        void SetHandle(CTL_TwainDLLHandle* pHandle) { m_pHandle = pHandle; }
        void Destroy()
        {
            if (m_pHandle && m_bDestroy && m_Array)
            {
                if constexpr (std::is_same_v<ArrayType, DTWAIN_ARRAY*>)
                {
                    if (*m_Array)
                        DestroyArrayFromFactory(m_pHandle, *m_Array);
                }
                else
                {
                    DestroyArrayFromFactory(m_pHandle, m_Array);
                }
                m_Array = {};
            }
        }
        ~DTWAINArrayLowLevel_RAII_Impl()
        {
            Destroy();
        }
    };

    using DTWAINArrayLowLevel_RAII = DTWAINArrayLowLevel_RAII_Impl<DTWAIN_ARRAY>;
    using DTWAINArrayLowLevelPtr_RAII = DTWAINArrayLowLevel_RAII_Impl<DTWAIN_ARRAY*>;
    using DTWAINArrayPtr_RAII = DTWAINArrayLowLevelPtr_RAII;

    void SetAcquiredImage(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY aAcq, LONG nWhichAcq, LONG nWhichDib, HANDLE theDib);
    void DestroyFrameFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_FRAME Frame);
    std::pair<int, DTWAIN_ARRAY> CreateArrayFromCap(CTL_TwainDLLHandle* pHandle, CTL_ITwainSource* pSource, LONG lCapType, LONG lSize);
    DTWAIN_ARRAY CreateArrayCopyFromFactory(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY Source);
    DTWAIN_FRAME CreateFrameArray(const CTL_TwainDLLHandle* pHandle, double Left, double Top, double Right, double Bottom);
    void SetArrayValueFromFactory(const CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray, size_t lPos, LPVOID pVariant);
    bool MoveArray(const CTL_TwainDLLHandle* pHandle, LPDTWAIN_ARRAY aDestination, LPDTWAIN_ARRAY aSource);
    LONG DTWAIN_ArrayType(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY pArray);
    bool DTWAINFRAMEToTWFRAME(DTWAIN_FRAME pDdtwil, pTW_FRAME pTwain);
    bool TWFRAMEToDTWAINFRAME(TW_FRAME pTwain, DTWAIN_FRAME pDdtwil);
}
#endif
