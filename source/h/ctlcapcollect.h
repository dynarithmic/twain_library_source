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
#ifndef CTLCAPCOLLECT_H
#define CTLCAPCOLLECT_H

#include <twain.h>
#include "dtwtype.h"
#include "enumeratornode.h"

namespace dynarithmic
{
    class CTL_ITwainSource;
    class CTL_TwainDLLHandle;

    DTWAIN_BOOL DTWAIN_CacheCapabilityInfo(CTL_ITwainSource* pSource, CTL_TwainDLLHandle* pHandle, CTL_EnumeratorNode<LONG>::container_pointer_type vCaps);
    DTWAIN_BOOL DTWAIN_CacheCapabilityInfo(CTL_ITwainSource* p, CTL_TwainDLLHandle* pHandle, TW_UINT16 nCapToCache);
}
#endif
