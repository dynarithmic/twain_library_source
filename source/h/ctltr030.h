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
#ifndef CTLTR030_H
#define CTLTR030_H

#include "ctltripletbase.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    template <TW_UINT16 MsgType>
    class CTL_Palette8Triplet : public CTL_TwainTriplet
    {
        public:
            CTL_Palette8Triplet(CTL_ITwainSession* pSession,
                                CTL_ITwainSource* pSource) : CTL_TwainTriplet(), m_Palette8()
            {
                SetSessionPtr(pSession);
                SetSourcePtr(pSource);

                // Get the app manager's AppID
                const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

                if (pMgr && pMgr->IsValidTwainSession(pSession))
                {
                    if (pSource)
                    {
                        Init(pSession->GetAppIDPtr(),
                            pSource->GetSourceIDPtr(),
                            DG_IMAGE,
                            DAT_PALETTE8,
                            MsgType,
                            static_cast<TW_MEMREF>(&m_Palette8));

                        SetAlive(true);
                    }
                }
            }


            TW_PALETTE8* GetPalette8Buffer()
            {
                return &m_Palette8;
            }

        private:
            TW_PALETTE8         m_Palette8;
    };

    using CTL_GetPalette8Triplet = CTL_Palette8Triplet<MSG_GET>;
    using CTL_GetDefaultPalette8Triplet = CTL_Palette8Triplet<MSG_GETDEFAULT>;
    using CTL_SetPalette8Triplet = CTL_Palette8Triplet<MSG_SET>;
    using CTL_ResetPalette8Triplet = CTL_Palette8Triplet<MSG_RESET>;
}
#endif

