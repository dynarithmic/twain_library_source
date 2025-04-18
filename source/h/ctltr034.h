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
#ifndef CTLTR034_H
#define CTLTR034_H

#include "ctltr010.h"
namespace dynarithmic
{
    class CTL_DeviceEventTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_DeviceEventTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource) : CTL_TwainTriplet()
            {
                InitGeneric(pSession, pSource, DG_CONTROL, DAT_DEVICEEVENT, MSG_GET, &m_DeviceEvent);
                m_bPassed = false;
            }

            TW_UINT16 Execute() override
            {
                m_bPassed = false;
                const TW_UINT16 rc = CTL_TwainTriplet::Execute();
                if (rc != TWRC_SUCCESS)
                    return rc;
                m_bPassed = true;
                return rc;
            }

            CTL_DeviceEvent GetDeviceEvent() const
            {
                return m_DeviceEvent;
            }

            bool IsSuccessful() const
            {
                return m_bPassed;
            }
    
    private:
            CTL_DeviceEvent     m_DeviceEvent;
            bool                m_bPassed;
    };
}
#endif
