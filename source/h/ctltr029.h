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
#ifndef CTLTR029_H
#define CTLTR029_H

#include "ctltripletbase.h"
namespace dynarithmic
{
    class CTL_SetupMemXferTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_SetupMemXferTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource) : CTL_TwainTriplet(), m_SetupMemXfer{}
            {
                InitGeneric(pSession, pSource, DG_CONTROL, DAT_SETUPMEMXFER, MSG_GET, &m_SetupMemXfer);
            }

            TW_SETUPMEMXFER *  GetSetupMemXferBuffer() { return &m_SetupMemXfer; }

        private:
            TW_SETUPMEMXFER        m_SetupMemXfer;
    };
}
#endif

