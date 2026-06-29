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
#ifndef CTLTR001_H
#define CTLTR001_H
#include "ctltripletbase.h"
#include "ctltwainmanager.h"
namespace dynarithmic
{
    class CTL_ITwainSource;
    class CTL_ITwainSession;

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
                    pSession->AddTwainSource(pCurSource);
                    pSession->SetSelectedSource(pCurSource);
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
            CTL_SourceTripletUsingName(CTL_ITwainSession *pSession, LPCTSTR pProduct = nullptr) : m_bSourceCreated(false)
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

                    // We need to use the class TW_IDENTITY struct here, and not the
                    // source's version of TW_IDENTITY.  The reason is that some drivers
                    // corrupt the saved TW_IDENTITY of the source when selecting and/or closing
                    // the source
                    InitGeneric(pSession, nullptr, DG_CONTROL, DAT_IDENTITY, nMsg, &m_tempSourceID, { true, false });
                }
            }
            CTL_ITwainSource* GetSourceIDPtr() { return GetSourcePtr(); }

            TW_UINT16 Execute() override
            {
                auto retval = ExecuteFn::Execute(*this);
                if (retval == TWRC_SUCCESS)
                {
                    // We need to use the class TW_IDENTITY struct here, and not the
                    // source's version of TW_IDENTITY.  The reason is that some drivers
                    // corrupt the saved TW_IDENTITY of the source when selecting and/or closing
                    // the source
                    *(GetSourcePtr()->GetSourceIDPtr()) = m_tempSourceID;
                }
                return retval;
            }
        private:
            bool m_bSourceCreated;
            TW_IDENTITY m_tempSourceID = {};
    };

    template <TW_UINT16 nMsg>
    class CTL_SourceOpenCloseTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_SourceOpenCloseTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource) : m_bSourceCreated(false)
            {
                SetSourcePtr(pSource);
                // We need to use the class TW_IDENTITY struct here, and not the
                // source's version of TW_IDENTITY.  The reason is that some drivers
                // corrupt the saved TW_IDENTITY of the source when selecting and/or closing
                // the source
                m_tempSourceID = *(pSource->GetSourceIDPtr());
                InitGeneric(pSession, nullptr, DG_CONTROL, DAT_IDENTITY, nMsg, &m_tempSourceID, {true, false});
            }
            CTL_ITwainSource* GetSourceIDPtr() { return GetSourcePtr(); }

        private:
            TW_IDENTITY m_tempSourceID = {};
            bool m_bSourceCreated;
    };

    using CTL_GetFirstSourceTriplet = CTL_SourceTripletUsingName<MSG_GETFIRST, CTL_DefaultTripletExecute>;
    using CTL_GetNextSourceTriplet = CTL_SourceTripletUsingName<MSG_GETNEXT, CTL_DefaultTripletExecute>;
    using CTL_GetDefaultSourceTriplet = CTL_SourceTripletUsingName<MSG_GETDEFAULT, CTL_DefaultTripletExecute>;
    using CTL_SelectSourceDlgTriplet = CTL_SourceTripletUsingName<MSG_USERSELECT, CTL_SelectSourceExecute>;
    using CTL_OpenSourceTriplet = CTL_SourceOpenCloseTriplet<MSG_OPENDS>;
    using CTL_CloseSourceTriplet = CTL_SourceOpenCloseTriplet<MSG_CLOSEDS>;
    using CTL_SetDefaultSourceTriplet = CTL_SourceOpenCloseTriplet<MSG_SET>;
}
#endif
