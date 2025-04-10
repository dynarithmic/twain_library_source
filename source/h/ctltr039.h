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
#ifndef CTLTR039_H
#define CTLTR039_H

#include "ctltripletbase.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    class CTL_ITwainSession;

    template <TW_UINT16 msg>
    class CTL_EntryPointTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_EntryPointTriplet(CTL_ITwainSession* pSession) :
                CTL_TwainTriplet(), m_bTripletFound(false)
            {
                SetSessionPtr(pSession);
                const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
                if (pMgr && pMgr->IsValidTwainSession(pSession))
                {
                    m_EntryPoint.Size = sizeof(TW_ENTRYPOINT);
                    Init(pSession->GetAppIDPtr(), nullptr, DG_CONTROL, DAT_ENTRYPOINT, msg, &m_EntryPoint);
                    SetAlive(true);
                }
            }
            TW_UINT16 Execute() override
            {
                const TW_UINT16 retVal = CTL_TwainTriplet::Execute();
                if (retVal == TWRC_SUCCESS)
                    m_bTripletFound = true;
                return retVal;
            }

            TW_ENTRYPOINT& getEntryPoint() { return m_EntryPoint; }

            bool isTripletFound() const { return m_bTripletFound; }

        private:
            TW_ENTRYPOINT m_EntryPoint{};
            bool m_bTripletFound;
    };
    using CTL_GetEntryPointTriplet = CTL_EntryPointTriplet<MSG_GET>;
    using CTL_SetEntryPointTriplet = CTL_EntryPointTriplet<MSG_SET>; 
}
#endif
