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
#ifndef CTLTR001_H
#define CTLTR001_H
#include "ctltripletbase.h"
#include "ctltwainmanager.h"
namespace dynarithmic
{
    class CTL_ITwainSource;
    class CTL_ITwainSession;

    struct CTL_DefaultExecute
    {
        static TW_UINT16 Execute(CTL_TwainTriplet& pTrip)
        {
            return pTrip.CTL_TwainTriplet::Execute();
        }
    };

    struct CTL_SelectSourceExecute
    {
        static TW_UINT16 Execute(CTL_TwainTriplet& pTrip)
        {
            const TW_UINT16 rc = pTrip.CTL_TwainTriplet::Execute();

            auto pSession = pTrip.GetSessionPtr();
            auto pCurSource = pTrip.GetSourcePtr();

            switch (rc)
            {
                case TWRC_SUCCESS:
                {
                    // Check if source exists
                    CTL_ITwainSource* pSource = pSession->Find(pCurSource);
                    if (!pSource)
                    {
                        pSession->AddTwainSource(pCurSource);
                        pSession->SetSelectedSource(pCurSource);
                    }
                    else
                    {
                        pSession->SetSelectedSource(pSource);
                        pCurSource->SetActive(FALSE);
                        CTL_ITwainSource::Destroy(pCurSource);
                    }
                }
                break;

                case TWRC_FAILURE:
                case TWRC_CANCEL:
                    CTL_ITwainSource::Destroy(pCurSource);
                    break;
            }
            return rc;
        }
    };

    template <TW_UINT16 nMsg, typename ExecuteFn>
    class CTL_SourceTripletUsingName : public CTL_TwainTriplet
    {
        public:
            CTL_SourceTripletUsingName(CTL_ITwainSession *pSession, LPCTSTR pProduct = nullptr)
            {
                SetSessionPtr(nullptr);
                SetSourcePtr(nullptr);

                // Get the app manager's AppID
                const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

                if (pMgr && pMgr->IsValidTwainSession(pSession))
                {
                    // Don't add this source to permanent list
                    CTL_ITwainSource* pSource = CTL_ITwainSource::Create(pSession, pProduct);
                    SetSourcePtr(pSource);
                    m_bSourceCreated = true;
                    SetSessionPtr(pSession);
                    if (pSource)
                    {
                        Init(pSession->GetAppIDPtr(), nullptr, DG_CONTROL, DAT_IDENTITY,
                            nMsg, static_cast<TW_MEMREF>(pSource->GetSourceIDPtr()));
                        SetAlive(true);
                    }
                }
            }
            CTL_ITwainSource* GetSourceIDPtr() { return GetSourcePtr(); }

            TW_UINT16 Execute() override
            {
                return ExecuteFn::Execute(*this);
            }
        private:
            bool m_bSourceCreated;
    };

    template <TW_UINT16 nMsg>
    class CTL_SourceOpenCloseTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_SourceOpenCloseTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource) : m_bSourceCreated(false)
            {
                SetSessionPtr(pSession);
                SetSourcePtr(pSource);

                // Get the app manager's AppID
                const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

                if (pMgr && pMgr->IsValidTwainSession(pSession))
                {
                    // Don't add this source to permanent list
                    Init(pSession->GetAppIDPtr(), nullptr, DG_CONTROL, DAT_IDENTITY,
                        nMsg, static_cast<TW_MEMREF>(pSource->GetSourceIDPtr()));
                    SetAlive(true);
                }
            }
            CTL_ITwainSource* GetSourceIDPtr() { return GetSourcePtr(); }

        private:
            bool m_bSourceCreated;
    };

    using CTL_GetFirstSourceTriplet = CTL_SourceTripletUsingName<MSG_GETFIRST, CTL_DefaultExecute>;
    using CTL_GetNextSourceTriplet = CTL_SourceTripletUsingName<MSG_GETNEXT, CTL_DefaultExecute>;
    using CTL_GetDefaultSourceTriplet = CTL_SourceTripletUsingName<MSG_GETDEFAULT, CTL_DefaultExecute>;
    using CTL_SelectSourceDlgTriplet = CTL_SourceTripletUsingName<MSG_USERSELECT, CTL_SelectSourceExecute>;
    using CTL_OpenSourceTriplet = CTL_SourceOpenCloseTriplet<MSG_OPENDS>;
    using CTL_CloseSourceTriplet = CTL_SourceOpenCloseTriplet<MSG_CLOSEDS>;
    using CTL_SetDefaultSourceTriplet = CTL_SourceOpenCloseTriplet<MSG_SET>;
}
#endif
