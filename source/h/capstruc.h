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
#ifndef CAPSTRUCT_H
#define CAPSTRUCT_H

#include <boost/container/flat_map.hpp>
#include "ctlenum.h"
#include "twain.h"
#include "winconst.h"
#include "ctltmpl4.h"

namespace dynarithmic
{
    // Define the cap info structure used
    class CTL_CapStruct
    {
        public:
            CTL_CapStruct() = default;
            UINT       m_nDataType = 0;
            UINT       m_nGetContainer = 0;
            UINT       m_nGetCurrentContainer = 0;
            UINT       m_nGetDefaultContainer = 0;
            UINT       m_nSetContainer = 0;
            UINT       m_nSetConstraintContainer = 0;
            UINT       m_nResetContainer = 0;
            UINT       m_nQuerySupportContainer = 0;
            std::string m_strCapName;
            operator std::string() const;
    };

    typedef boost::container::flat_map<TW_UINT16, CTL_CapStruct> CTL_GeneralCapInfo;

    typedef CTL_ClassValues10<TW_UINT16 ,/* Capability*/
        UINT, /* Container for Get */
        UINT, /* Container for Set*/
        UINT,  /* Data Type */
        UINT,  /* Available cap support */
        UINT,  /* Container for Get Current */
        UINT,   /* Container for Get Default  */
        UINT,  /* Container for Set Constraint */
        UINT,  /* Container for Reset */
        UINT                 /* Container for Query Support*/
    > CTL_CapInfo;
}
#endif
