/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#ifndef CTLREG_H
#define CTLREG_H

#include <unordered_map>
#include "ctlarray.h"
#include "ctlobstr.h"

namespace dynarithmic
{
    typedef std::unordered_map<std::string, UINT> ContainerMap;
    BOOL SaveCapInfoToIni(const std::string& strSourceName, UINT nCap, const CTL_IntArray& rContainerTypes);
    bool GetCapInfoFromIni(const std::string& strCapName,
                           const std::string& strSourceName,
                           UINT nCap,
                           UINT &rGetValues,
                           UINT &rGetValuesCurrent,
                           UINT &rGetValuesDefault,
                           UINT &rSetValuesCurrent,
                           UINT &rSetValuesAvailable,
                           UINT &rQuerySupport,
                           UINT &rEOJValue,
                           TW_UINT16 &rStateInfo,
                           UINT &rDataType,
                           UINT &rEntryFound,
                           bool &bContainerInfoFound,
                           const ContainerMap &mapContainer
                           );
    }
#endif
