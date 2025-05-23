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
#ifndef CTLTR000_H
#define CTLTR000_H

#include "ctltripletbase.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    class CTL_ITwainSession;

    template <TW_UINT16 nMsg, int nErr>
    class CTL_TwainSMTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_TwainSMTriplet(CTL_ITwainSession* pSession) : m_nDSMVersion(DTWAIN_TWAINDSM_LEGACY)
            {
                // Get the app manager's AppID
                m_nErr = nErr;
                InitGeneric(pSession, nullptr, DG_CONTROL, DAT_PARENT, nMsg, pSession->GetWindowHandlePtr());
            }

            TW_UINT16 Execute() override
            {
                const TW_UINT16 rc = CTL_TwainTriplet::Execute();
                if (rc != TWRC_SUCCESS)
                {
                    DTWAIN_ERROR_CONDITION(m_nErr, TWRC_FAILURE, true)
                }
                return TWRC_SUCCESS;
            }

            int GetDSMVersion() const
            {
                if (std::get<0>(GetTripletArgs())->SupportedGroups & DF_DSM2)
                    return DTWAIN_TWAINDSM_VERSION2;
                return DTWAIN_TWAINDSM_LEGACY;
            }

        private:
            int     m_nErr;
            int     m_nDSMVersion;
    };

    using CTL_TwainCloseSMTriplet = CTL_TwainSMTriplet<MSG_CLOSEDSM, IDS_ErrSourceMgrClose>;
    using CTL_TwainOpenSMTriplet = CTL_TwainSMTriplet<MSG_OPENDSM, IDS_ErrSourceMgrOpen>;
}
#endif
