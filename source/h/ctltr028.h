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
#ifndef CTLTR028_H
#define CTLTR028_H

#include <unordered_map>
#include <algorithm>
#include "capconst.h"
#include "ctltripletbase.h"
#include "ctltmpl5.h"
namespace dynarithmic
{
    template <TW_UINT16 GetSetType>
    class CTL_SetupFileXferTripletImpl : public CTL_TwainTriplet
    {
        public:
            CTL_SetupFileXferTripletImpl(CTL_ITwainSession* pSession,
                CTL_ITwainSource* pSource,
                CTL_TwainFileFormatEnum FileFormat,
                CTL_StringType strFileName
            ) : CTL_TwainTriplet(), m_SetupFileXfer()
            {
                SetSessionPtr(pSession);
                SetSourcePtr(pSource);
                StringWrapperA::SafeStrcpy(m_SetupFileXfer.FileName,
                    StringConversion::Convert_Native_To_Ansi(strFileName).c_str(),
                    sizeof m_SetupFileXfer.FileName - 1);

                m_SetupFileXfer.Format = static_cast<TW_UINT16>(FileFormat);

                InitGeneric(pSession, pSource, DG_CONTROL, DAT_SETUPFILEXFER, GetSetType, &m_SetupFileXfer);

                // Set the capability map if this is a set type
                const bool bIsSet = IsMSGSetType();
                if (bIsSet)
                {
                    m_capMap[DTWAIN_CV_ICAPJPEGPIXELTYPE] = TWPT_BW;
                    m_capMap[DTWAIN_CV_ICAPJPEGQUALITY] = TWJQ_MEDIUM;
                    m_capMap[DTWAIN_CV_ICAPCOMPRESSION] = TWCP_NONE;
                    m_capMap[DTWAIN_CV_ICAPJPEGSUBSAMPLING] = TWCP_NONE;
                }
            }
            typedef std::unordered_map<TW_UINT16, LONG> FileXferCapMap;

            struct CapGetter
            {
                CTL_ITwainSource* m_pSource;
                CapGetter(CTL_ITwainSource* pSource) : m_pSource(pSource) {}
                void operator()(CTL_SetupFileXferTripletImpl::FileXferCapMap::value_type& v) const
                {
                    CTL_TwainAppMgr::GetCurrentOneCapValue(m_pSource, &v.second, v.first, static_cast<TW_UINT16>(CTL_TwainAppMgr::GetGeneralCapInfo(v.first).m_nDataType));
                }
            };

            struct CapSetter
            {
                CTL_ITwainSource* m_pSource;
                CapSetter(CTL_ITwainSource* pSource) : m_pSource(pSource) {}
                void operator()(const CTL_SetupFileXferTripletImpl::FileXferCapMap::value_type& v) const
                {
                    SetOneCapValue(m_pSource, v.first, CTL_SetTypeSET, v.second, static_cast<TW_UINT16>(CTL_TwainAppMgr::GetGeneralCapInfo(v.first).m_nDataType));
                }
            };

            TW_UINT16 Execute() override
            {
                const bool bIsSet = IsMSGSetType();

                if (bIsSet)
                {
                    // Get the current cap values for each cap we need to know about before setting the file transfer
                    const CapGetter cg(GetSourcePtr());
                    std::for_each(m_capMap.begin(), m_capMap.end(), cg);
                }

                // set up the file xfer
                const TW_UINT16 rc = CTL_TwainTriplet::Execute();

                // if successful we now set the trailing capabilities we need for file xfer
                if (rc == TWRC_SUCCESS && bIsSet)
                {
                    const CapSetter cs(GetSourcePtr());
                    std::for_each(m_capMap.begin(), m_capMap.end(), cs);
                }
                return rc;
            }

            CTL_StringType GetFileName() const { return StringConversion::Convert_AnsiPtr_To_Native(m_SetupFileXfer.FileName); }
            CTL_TwainFileFormatEnum  GetFileFormat() const { return static_cast<CTL_TwainFileFormatEnum>(m_SetupFileXfer.Format); }

        private:
            FileXferCapMap          m_capMap;
            TW_SETUPFILEXFER        m_SetupFileXfer;
    };

    using CTL_GetSetupFileXferTriplet = CTL_SetupFileXferTripletImpl<MSG_GET>;
    using CTL_GetDefaultSetupFileXferTriplet = CTL_SetupFileXferTripletImpl<MSG_GETDEFAULT>;
    using CTL_SetSetupFileXferTriplet = CTL_SetupFileXferTripletImpl<MSG_SET>;
    using CTL_ResetSetupFileXferTriplet = CTL_SetupFileXferTripletImpl<MSG_RESET>;

    class CTL_AudioFileXferTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_AudioFileXferTriplet(CTL_ITwainSession* pSession, CTL_ITwainSource* pSource) : CTL_TwainTriplet()
            {
                InitGeneric(pSession, pSource, DG_CONTROL, DAT_AUDIOFILEXFER, MSG_GET, nullptr);
            }
    };
}
#endif
