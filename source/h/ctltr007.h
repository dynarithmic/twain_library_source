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
#ifndef CTLTR007_H
#define CTLTR007_H

#include "ctltripletbase.h"
#include "ctltwainsession.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    class CTL_ConditionCodeTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_ConditionCodeTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource/* = nullptr*/) :
                CTL_TwainTriplet(), m_Status{}
            {
                SetSourcePtr(const_cast<CTL_ITwainSource*>(pSource));
                SetSessionPtr(pSession);
                const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();
                if (pMgr && pMgr->IsValidTwainSession(pSession))
                {
                    if (pSource)
                        SetValues(pSession->GetAppIDPtr(), pSource->GetSourceIDPtr());
                    else
                        SetValues(pSession->GetAppIDPtr(), nullptr);
                }
            }

            CTL_ConditionCodeTriplet(TW_IDENTITY* pSession, TW_IDENTITY* pSourceID)
            {
                SetValues(pSession, pSourceID);
            }

            TW_UINT16 CTL_ConditionCodeTriplet::GetConditionCode() const
            {
                return m_Status.ConditionCode;
            }

            TW_UINT16 CTL_ConditionCodeTriplet::GetData() const
            {
                return m_Status.Data;
            }

        private:
            void SetValues(TW_IDENTITY* pSession, TW_IDENTITY* pSourceID)
            {
                Init(pSession, pSourceID, DG_CONTROL, DAT_STATUS, MSG_GET, static_cast<TW_MEMREF>(&m_Status));
                SetAlive(true);
            }

            TW_STATUS   m_Status;

    };
}
#endif

