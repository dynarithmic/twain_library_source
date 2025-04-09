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
#ifndef CTLTR036_H
#define CTLTR036_H

#include "ctltr010.h"
#include "ctltwainmanager.h"
namespace dynarithmic
{
    template <TW_UINT16 nMsg>
    class CTL_CustomDSTripletImpl : public CTL_TwainTriplet
    {
        public:
            CTL_CustomDSTripletImpl(CTL_ITwainSession* pSession,
                CTL_ITwainSource* pSource) : CTL_TwainTriplet(), m_CustomDSData{}
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
                            DG_CONTROL,
                            DAT_CUSTOMDSDATA,
                            nMsg,
                            static_cast<TW_MEMREF>(&m_CustomDSData));
                        SetAlive(true);
                    }
                }
            }

            TW_UINT16 Execute()
            {
                const TW_UINT16 rc = CTL_TwainTriplet::Execute();
                return rc;
            }

            TW_UINT32 GetDataSize() const
            {
                return m_CustomDSData.InfoLength;
            }

            HANDLE GetData() const
            {
                return m_CustomDSData.hData;
            }

            void SetDataSize(TW_UINT32 nSize)
            {
                m_CustomDSData.InfoLength = nSize;
            }

            TW_UINT16 SetData(HANDLE hData, TW_UINT32 /*nSize*/)
            {
                m_CustomDSData.hData = hData;
                return Execute();
            }

        private:
            TW_CUSTOMDSDATA     m_CustomDSData;
    };

    using CTL_GetCustomDSTriplet = CTL_CustomDSTripletImpl<MSG_GET>;
    using CTL_SetCustomDSTriplet = CTL_CustomDSTripletImpl<MSG_SET>;
}
#endif
