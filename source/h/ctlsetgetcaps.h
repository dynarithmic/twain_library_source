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
#ifndef CTLSETGETCAPS_H
#define CTLSETGETCAPS_H
#include <algorithm>
#include "dtwain.h"
#include "ctliface.h"

///////////////////////////////////////////////////////////////////////////
namespace dynarithmic
{
    bool GetCapValuesEx2_Internal(CTL_ITwainSource* pSource,
                                  LONG lCap, LONG lGetType, LONG lContainerType, LONG nDataType, LPDTWAIN_ARRAY pArray);

    bool SetCapValuesEx2_Internal(CTL_ITwainSource* pSource, LONG lCap, LONG lSetType, LONG lContainerType,
                                  LONG nDataType, DTWAIN_ARRAY pArray);
}
#endif
