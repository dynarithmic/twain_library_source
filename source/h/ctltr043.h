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
#ifndef CTLTR043_H
#define CTLTR043_H

#include "ctltr026.h"
#include "ctliface.h"

namespace dynarithmic
{
    class CTL_ImageMemFileXferTriplet : public CTL_ImageXferTriplet
    {
        public:
            CTL_ImageMemFileXferTriplet(CTL_ITwainSession *pSession,
                                        CTL_ITwainSource* pSource,
                                        TW_UINT32 numBytes,
                                        bool bHandleMemory = false);

            TW_UINT16           Execute() override;
            ~CTL_ImageMemFileXferTriplet() override;
            CTL_ImageMemFileXferTriplet(const CTL_ImageMemFileXferTriplet&) = delete;
            CTL_ImageMemFileXferTriplet& operator=(const CTL_ImageMemFileXferTriplet&) = delete;

        protected:
            void InitXferBuffer();
            TW_IMAGEMEMXFER& GetMemXferBuffer() { return m_ImageMemXferBuffer; }
        private:
            TW_IMAGEMEMXFER m_ImageMemXferBuffer;
            TW_UINT32 m_nCompressPos;
            HANDLE hLocalHandle;
    };
}
#endif


