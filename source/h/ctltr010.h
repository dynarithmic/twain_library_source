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
#ifndef CTLTR010_H
#define CTLTR010_H

#include "ctltripletbase.h"
#include "ctltwainsession.h"
#include "ctlobtyp.h"
namespace dynarithmic
{
    class CTL_CapabilityTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_CapabilityTriplet(CTL_ITwainSession *pSession,
                                   CTL_ITwainSource *pSource,
                                   TW_UINT16 nMsg,
                                   TW_UINT16 nItemType,
                                   bool     bReading);
            ~CTL_CapabilityTriplet() override;
            TW_UINT16       Execute() override;
            void            SetTestMode( bool bSet = true );
            bool            IsTesting() const;
            bool            IsFindDefaultItemType() const { return m_bGetDefaultType; }
            void            SetFindDefaultItemType(bool bSet) { m_bGetDefaultType = bSet; }
            bool            IsCapabilitySupported();
            virtual bool    IsSupported();
            TW_UINT16       GetItemType() const { return m_nItemType; }
            static constexpr bool IsCapOperationReset(LONG lCapOp) { return lCapOp == DTWAIN_CAPRESET || lCapOp == DTWAIN_CAPRESETALL; }
            static constexpr bool IsCapOperationSet(LONG lCapOp) { return !IsCapOperationReset(lCapOp); }

        protected:
            virtual void        Decode(void *pData);
            bool                IsReading() const;
            TW_CAPABILITY*      GetCapabilityBuffer();
            CTL_TwainTypeArray* GetTwainTypeArray();
            void                SetItemType(TW_UINT16 nItemType) { m_nItemType = nItemType; }
            void                RemoveAllTypeObjects();

        private:
            TW_CAPABILITY       m_Capability;
            bool                m_bReading;
            bool                m_bTesting;
            CTL_TwainTypeArray  m_ObArray;
            bool                m_bSupported;
            TW_UINT16           m_nItemType;
            bool                m_bGetDefaultType;
    };
}
#endif
