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
#ifndef CTLSUPPORT_H
#define CTLSUPPORT_H
#include <algorithm>
#include "dtwain.h"
#include "ctliface.h"

///////////////////////////////////////////////////////////////////////////
namespace dynarithmic
{
    template <typename T>
    bool IsSupported(DTWAIN_SOURCE Source, bool getAnySupport, T SupportVal, LONG Cap)
    {
        if (!DTWAIN_IsCapSupported(Source, Cap))
            return false;
        if (getAnySupport)
            return true;
        DTWAIN_ARRAY Array = 0;
        if (DTWAIN_GetCapValues(Source, Cap, DTWAIN_CAPGET, &Array))
        {
            auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
            DTWAINArrayLowLevel_RAII raii(pHandle, Array);
            DTWAIN_ARRAY tempArray = 0;
            DTWAIN_ARRAY arrayToUse = Array;
            DTWAINArrayLowLevel_RAII raii2(pHandle, nullptr); 
            if (DTWAIN_GetCapContainer(Source, Cap, DTWAIN_CAPGET) == DTWAIN_CONTRANGE)
            {
                // expand range if we find that the underlying values are in a range
                try
                {
                    DTWAIN_RangeExpand(Array, &tempArray);
                    raii2.SetArray(tempArray);
                    arrayToUse = tempArray;
                }
                catch(...)
                {
                    return false;
                }
            }
            // get underlying vector and search it for the value
            auto& vData = pHandle->m_ArrayFactory->underlying_container_t<T>(arrayToUse);
            return std::find(vData.begin(), vData.end(), SupportVal) != vData.end();
        }
        return false;
    }


    template <typename T>
    bool SetSupport(DTWAIN_SOURCE Source, T* SupportVal, LONG Cap, bool bSetCurrent)
    {
        if (DTWAIN_IsCapSupported(Source, Cap))
        {
            DTWAIN_ARRAY Array = DTWAIN_ArrayCreateFromCap(Source, Cap, 1);
            if (!Array)
                return false;
            const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
            DTWAINArrayLowLevel_RAII a(pHandle, Array);

            LONG SetType = DTWAIN_CAPSET;
            if (!bSetCurrent)
                SetType = DTWAIN_CAPRESET;

            // See if the container is a range, if so, get the nearest range value.
            if (SetType == DTWAIN_CAPSET)
            {
                LONG nContainer = DTWAIN_GetCapContainer(Source, Cap, DTWAIN_CAPGET);
                if (nContainer == DTWAIN_CONTRANGE)
                {
                    DTWAIN_ARRAY Array2 = 0;
                    DTWAIN_BOOL bRet = DTWAIN_GetCapValues(Source, Cap, DTWAIN_CAPGET, &Array2);
                    DTWAINArrayLowLevel_RAII a2(pHandle, Array2);
                    if (bRet)
                    {
                        LONG nSize = static_cast<LONG>(pHandle->m_ArrayFactory->size(Array2));
                        if (nSize > 0)
                            DTWAIN_RangeGetNearestValue(Source, SupportVal, SupportVal, DTWAIN_ROUNDNEAREST);
                    }
                }
            }

            DTWAIN_ArraySetAt(Array, 0, SupportVal);
            BOOL bRet = DTWAIN_SetCapValues(Source, Cap, SetType, Array);
            return bRet ? true : false;
        }
        return false;
    }

    template <typename T>
    int GetSupport(DTWAIN_SOURCE Source, typename T::value_type* lpSupport, LONG Cap, LONG CapOp=DTWAIN_CAPGET)
    {
        const auto pHandle = static_cast<CTL_ITwainSource*>(Source)->GetDTWAINHandle();
        if (lpSupport == NULL)
        {
            pHandle->m_lLastError = DTWAIN_ERR_INVALID_PARAM;
            return -1;
        }
        DTWAIN_ARRAY Array = 0;
        BOOL isSupported = DTWAIN_GetCapValues(Source, Cap, CapOp, &Array);
        DTWAINArrayLowLevel_RAII raii(pHandle, Array);
        if ( isSupported )
        {
            // get underlying vector and search it for the value
            auto& vData = pHandle->m_ArrayFactory->underlying_container<T>(Array);
            if (!vData.empty())
            {
                *lpSupport = vData.front();
                return 1;
            }
        }
        return 0;
    }
}
#endif
