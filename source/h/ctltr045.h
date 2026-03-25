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
#ifndef CTLTR045_H
#define CTLTR045_H

#include "ctltr013.h"
#include "ctltr014.h"
#include "ctltmpl5.h"

namespace dynarithmic
{
    template <TW_UINT16 msgType>
    class CTL_CapabilityLabelHelpTriplet : public CTL_CapabilityGetOneValTriplet
    {
        public:
            CTL_CapabilityLabelHelpTriplet(CTL_ITwainSession *pSession,
                                           TW_UINT16 gCap,
                                           TW_UINT16 TwainType=0xFFFF) :
			CTL_CapabilityGetOneValTriplet(pSession, nullptr, msgType, gCap, TwainType)
			{
				TW_CAPABILITY* pCap = GetCapabilityBuffer();
				pCap->Cap = static_cast<TW_UINT16>(gCap);
				pCap->ConType = TWON_ONEVALUE;
				pCap->hContainer = nullptr;
				m_bIsSupported = false;
			}

            TW_UINT16  Execute() override
            {
				const TW_UINT16 rc = CTL_CapabilityGetOneValTriplet::Execute();
				if (rc == TWRC_SUCCESS)
				{
					m_bIsSupported = true;
					GetValue(nullptr);
				}
				return rc;
            }

            bool            IsSupported() const noexcept { return m_bIsSupported; }
            std::string     GetString() const { return m_strLabel; }

        protected:
            bool  GetValue(void* pData, size_t nWhere = 0) override {return true;}
            bool  EnumCapValues(void* pCapData) override
            {
				const bool bRetVal = CTL_CapabilityGetOneValTriplet::EnumCapValues(pCapData);
				if (bRetVal)
				{
					// dereference to a TW_ONEVALUE structure
					pTW_ONEVALUE pValOne = static_cast<pTW_ONEVALUE>(pCapData);
					HANDLE hStr = (HANDLE)(uintptr_t)pValOne->Item;

					// Get the string 
					LPSTR label = (LPSTR)GlobalLock(hStr);
					if (label)
					{
						std::string s = label;
						GlobalUnlock(hStr);
					}

					if (GlobalFlags(hStr) != GMEM_INVALID_HANDLE)
						GlobalFree(hStr);
				}
				return bRetVal;
            }

        private:
            bool m_bIsSupported;
            std::string m_strLabel;
    };

    class CTL_CapabilityLabelEnumTriplet : public CTL_CapabilityTriplet
    {
        public:
            CTL_CapabilityLabelEnumTriplet(CTL_ITwainSession* pSession, TW_UINT16 gCap) :
                CTL_CapabilityTriplet(pSession, nullptr, MSG_GETLABELENUM, TWTY_STR255, true) {}
            bool IsSupported() const noexcept { return m_bIsSupported; }
            const std::vector<std::string>&  GetStrings() const noexcept { return m_aStrings; }
            TW_UINT16 Execute() override
            {
                auto retValue = dynarithmic::GetCapabilityValues(nullptr, GetCapabilityBuffer()->Cap, MSG_GETLABELENUM,
                    DTWAIN_CONTARRAY, 1, TWTY_STR255, m_aStrings);
                if (retValue.second == TWRC_SUCCESS)
                    m_bIsSupported = true;
                return retValue.second;
            }

        private:
            bool m_bIsSupported;
            std::vector<std::string> m_aStrings;
    };

    using CTL_CapabilityLabelTriplet = CTL_CapabilityLabelHelpTriplet<MSG_GETLABEL>;
	using CTL_CapabilityHelpTriplet = CTL_CapabilityLabelHelpTriplet<MSG_GETHELP>;
}
#endif
