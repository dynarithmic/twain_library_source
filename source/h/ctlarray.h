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
#ifndef CTLARRAY_H
#define CTLARRAY_H

#include <vector>
#include <algorithm>
#include <limits>
#include "dtwtype.h"
#include "twain.h"

typedef std::vector<int>            CTL_IntArray;
typedef std::vector<TW_UINT16>      CTL_TwainCapArray;
typedef std::vector<double>         CTL_RealArray;
typedef std::vector<HANDLE>         CTL_HDIBArray;

namespace dynarithmic
{
    class CTL_TwainDLLHandle;
    void ArrayCopyWideToNative(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ArraySource, DTWAIN_ARRAY ArrayDest);
    void ArrayCopyAnsiToNative(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY ArraySource, DTWAIN_ARRAY ArrayDest);

    template <typename Container, typename DTWAINArrayType>
    static void CopyContainer(CTL_TwainDLLHandle* pHandle, const Container& theContainer, DTWAIN_ARRAY theArray)
    {
        auto& vect = pHandle->m_ArrayFactory->underlying_container_t<DTWAINArrayType>(theArray);
        std::copy(theContainer.begin(), theContainer.end(), vect.begin());
    }

    template <typename Container>
    DTWAIN_ARRAY CreateArrayFromContainer(CTL_TwainDLLHandle* pHandle, const Container& theContainer)
    {
        DTWAIN_ARRAY theArray = {};
        LONG nSize = static_cast<LONG>(theContainer.size());
        if constexpr (std::is_integral_v<Container::value_type>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, nSize);
            CopyContainer<Container, LONG>(pHandle, theContainer, theArray);
        }
        else
        if constexpr (std::is_floating_point_v<Container::value_type>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYFLOAT, nSize);
            CopyContainer<Container, double>(pHandle, theContainer, theArray);
        }
        else
        if constexpr (std::is_same_v<Container::value_type, std::string>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYANSISTRING, nSize);
            CopyContainer<Container, std::string>(pHandle, theContainer, theArray);
        }
        else 
        if constexpr (std::is_same_v<Container::value_type, TwainFrameInternal>)
        {
            theArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYFRAME, nSize);
            CopyContainer<Container, TwainFrameInternal>(pHandle, theContainer, theArray);
        }
        return theArray;
    }

    template <typename Container, typename ArrayType>
    Container CreateContainerHelper(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY theArray, size_t maxElements)
    {
        Container theContainer;
        auto& pVector = pHandle->m_ArrayFactory->underlying_container_t<ArrayType>(theArray);
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
        if constexpr (std::is_integral_v<Container::value_type>)
            return CreateContainerHelper<Container, LONG>(pHandle, theArray, maxElements);
        else
        if constexpr (std::is_same_v<Container::value_type, double>)
            return CreateContainerHelper<Container, double>(pHandle, theArray, maxElements);
        else
        if constexpr (std::is_same_v<Container::value_type, std::string>)
            return CreateContainerHelper<Container, std::string>(pHandle, theArray, maxElements);
        return {};
    }
}
#endif
