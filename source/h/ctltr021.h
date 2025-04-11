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
#ifndef CTLTR021_H
#define CTLTR021_H

#include "ctltripletbase.h"
#include "ctltwainsession.h"
#include "ctltwainmanager.h"

namespace dynarithmic
{
    template <TW_UINT16 nMsg, typename ExecuteFn = CTL_DefaultTripletExecute>
    class CTL_UserInterfaceTripletImpl : public CTL_TwainTriplet
    {
        public:
            CTL_UserInterfaceTripletImpl(CTL_ITwainSession* pSession,
                                        CTL_ITwainSource* pSource,
                                        TW_USERINTERFACE* pTWUI,
                                        TW_BOOL bShowUI = TRUE)
                                        : CTL_TwainTriplet(), m_pUserInterface(pTWUI)
            {
                m_pUserInterface->ShowUI = bShowUI;
                m_pUserInterface->ModalUI = 0;
                const HWND* pWnd = pSession->GetWindowHandlePtr();

                m_pUserInterface->hParent = static_cast<TW_HANDLE>(*pWnd);
                InitGeneric(pSession, pSource, DG_CONTROL, DAT_USERINTERFACE, nMsg, m_pUserInterface);
            }

            bool    IsModal() const { return m_pUserInterface->ModalUI?true:false; }
            TW_USERINTERFACE *GetTWUserInterface() const { return m_pUserInterface; }

            virtual TW_UINT16 Execute()
            {
                return ExecuteFn::Execute(*this);
            }
        private:
            TW_USERINTERFACE    *m_pUserInterface;
    };

    struct CTL_ExecuteEnableUIFn
    {
        static TW_UINT16 Execute(CTL_TwainTriplet& pTrip)
        {
            CTL_ITwainSource* pSource = pTrip.GetSourcePtr();

            if (pSource->IsUIOpen())
                return TWRC_SUCCESS;

            const TW_UINT16 rc = CTL_DefaultTripletExecute::Execute(pTrip);
            if (rc == TWRC_SUCCESS)
                pSource->SetUIOpen(true);
            else
                pSource->SetUIOpen(false);
            return rc;
        }
    };

    struct CTL_ExecuteDisableUIFn
    {
        static TW_UINT16 Execute(CTL_TwainTriplet& pTrip)
        {
            const TW_UINT16 rc = CTL_DefaultTripletExecute::Execute(pTrip);
            if (rc == TWRC_SUCCESS)
                pTrip.GetSourcePtr()->SetUIOpen(FALSE);
            return rc;
        }
    };

    using CTL_EnableUserInterfaceTriplet = CTL_UserInterfaceTripletImpl<MSG_ENABLEDS, CTL_ExecuteEnableUIFn>;
    using CTL_DisableUserInterfaceTriplet = CTL_UserInterfaceTripletImpl<MSG_DISABLEDS, CTL_ExecuteDisableUIFn>;
    using CTL_DisplayUserInterfaceOnlyTriplet = CTL_UserInterfaceTripletImpl<MSG_ENABLEDSUIONLY, CTL_ExecuteEnableUIFn>;
}
#endif

