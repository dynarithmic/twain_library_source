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
#ifndef CTLTR032_H
#define CTLTR032_H

#include "ctltr010.h"
namespace dynarithmic
{
    template <TW_UINT16 GetSetType>
    class CTL_ImageLayoutTripletImpl: public CTL_TwainTriplet
    {
        public:
            CTL_ImageLayoutTripletImpl(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource,
                                       const CTL_RealArray* rArray = nullptr) : CTL_TwainTriplet(), m_ImageLayout{}
            {
                InitGeneric(pSession, pSource, DG_IMAGE, DAT_IMAGELAYOUT, GetSetType, &m_ImageLayout);

                if (::IsMSGSetOrResetType(GetSetType))
                {
                    TW_IMAGELAYOUT* pLayout = GetImageLayout();
                    if (GetSetType != MSG_RESET && rArray && rArray->size() >= 4)
                    {
                        pLayout->Frame.Left = FloatToFix32(static_cast<float>((*rArray)[0]));
                        pLayout->Frame.Top = FloatToFix32(static_cast<float>((*rArray)[1]));
                        pLayout->Frame.Right = FloatToFix32(static_cast<float>((*rArray)[2]));
                        pLayout->Frame.Bottom = FloatToFix32(static_cast<float>((*rArray)[3]));
                    }
                    pLayout->DocumentNumber = static_cast<TW_UINT32>(-1);
                    pLayout->PageNumber = static_cast<TW_UINT32>(-1);
                    pLayout->FrameNumber = static_cast<TW_UINT32>(-1);
                }
            }

            double GetLeft() const
            {
                return Fix32ToFloat(m_ImageLayout.Frame.Left);
            }

            double GetRight() const
            {
                return Fix32ToFloat(m_ImageLayout.Frame.Right);
            }

            double GetTop() const
            {
                return Fix32ToFloat(m_ImageLayout.Frame.Top);
            }

            double GetBottom() const
            {
                return Fix32ToFloat(m_ImageLayout.Frame.Bottom);
            }

            TW_UINT32 GetDocumentNumber() const
            {
                return m_ImageLayout.DocumentNumber;
            }

            TW_UINT32 GetPageNumber() const
            {
                return m_ImageLayout.PageNumber;
            }

            TW_UINT32 GetFrameNumber() const
            {
                return m_ImageLayout.FrameNumber;
            }

            TW_FRAME GetFrame() const
            {
                return m_ImageLayout.Frame;
            }

            TW_IMAGELAYOUT* GetImageLayout() { return &m_ImageLayout; }

        private:
            TW_IMAGELAYOUT  m_ImageLayout;
    };

    using CTL_GetImageLayoutTriplet = CTL_ImageLayoutTripletImpl<MSG_GET>;
    using CTL_GetDefaultImageLayoutTriplet = CTL_ImageLayoutTripletImpl<MSG_GETDEFAULT>;
    using CTL_SetImageLayoutTriplet = CTL_ImageLayoutTripletImpl<MSG_SET>;
    using CTL_ResetImageLayoutTriplet = CTL_ImageLayoutTripletImpl<MSG_RESET>;
}
#endif


