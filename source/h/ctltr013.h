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
#ifndef CTLTR013_H
#define CTLTR013_H

#include "ctltr011.h"
namespace dynarithmic
{
    class CTL_CapabilityGetOneValTriplet : public CTL_CapabilityGetTriplet
    {
        public:
            CTL_CapabilityGetOneValTriplet(CTL_ITwainSession *pSession,
                                           CTL_ITwainSource *pSource,
                                           TW_UINT16   gType,
                                           TW_UINT16 gCap,
                                           TW_UINT16    TwainDataType)
                : CTL_CapabilityGetTriplet(pSession,
                    pSource,
                    gType,
                    gCap,
                    TwainDataType) {}

            bool   GetValue(void* pData, size_t nWhere = 0) override
            {
                return GenericGetValue(pData, 0);
            }

        protected:
            bool EnumCapValues(void* pCapData) override
            {
                return GenericEnumCapValues<TW_ONEVALUE>(pCapData);
            }
    };
}
#endif
