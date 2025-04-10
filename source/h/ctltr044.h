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
#ifndef CTLTR044_H
#define CTLTR044_H

#include "ctltripletbase.h"
#include "ctltwainsession.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    template <TW_UINT16 nMsg = MSG_GET>
    class CTL_JPEGCompressionTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_JPEGCompressionTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource)
            {
                SetSessionPtr(nullptr);
                SetSourcePtr(nullptr);

                // Get the app manager's AppID
                const CTL_TwainAppMgrPtr pMgr = CTL_TwainAppMgr::GetInstance();

                if (pMgr && pMgr->IsValidTwainSession(pSession))
                {
                    SetSourcePtr(pSource);
                    SetSessionPtr(pSession);
                    if (pSource)
                    {
                        Init(pSession->GetAppIDPtr(), pSource->GetSourceIDPtr(), DG_IMAGE, DAT_JPEGCOMPRESSION, nMsg,
                            static_cast<TW_MEMREF>(&m_JPEGCompressionInfo));
                        SetAlive(true);
                    }
                }
            }

            TW_JPEGCOMPRESSION& GetJPEGCompressionInfo() { return m_JPEGCompressionInfo; }

        protected:
            TW_JPEGCOMPRESSION    m_JPEGCompressionInfo;
    };

    using CTL_GetJPEGCompressionTriplet = CTL_JPEGCompressionTriplet<MSG_GET>;
    using CTL_GetDefaultJPEGCompressionTriplet = CTL_JPEGCompressionTriplet<MSG_GETDEFAULT>;
    using CTL_SetJPEGCompressionTriplet = CTL_JPEGCompressionTriplet<MSG_SET>;
    using CTL_ResetJPEGCompressionTriplet = CTL_JPEGCompressionTriplet<MSG_RESET>;
}
#endif

