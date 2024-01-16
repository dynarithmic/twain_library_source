/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

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
#ifndef CTLTR038_H
#define CTLTR038_H

#include "ctltrp.h"

#include <vector>
namespace dynarithmic
{
    class CTL_ExtImageInfoTriplet : public CTL_TwainTriplet
    {
        public:
            // Only MSG_GET is supported
            CTL_ExtImageInfoTriplet() : m_pExtImageInfo(nullptr), m_memHandle{}, m_nNumInfo{} { }
            CTL_ExtImageInfoTriplet(CTL_ITwainSession *pSession,
                                   CTL_ITwainSource* pSource,
                                   int nInfo);
            CTL_ExtImageInfoTriplet(const CTL_ExtImageInfoTriplet&) = delete;
            CTL_ExtImageInfoTriplet& operator =(const CTL_ExtImageInfoTriplet&) = delete;
            CTL_ExtImageInfoTriplet(CTL_ExtImageInfoTriplet&& rhs) noexcept;
            CTL_ExtImageInfoTriplet& operator = (CTL_ExtImageInfoTriplet&& rhs) = delete;

            static void swap(CTL_ExtImageInfoTriplet& left, const CTL_ExtImageInfoTriplet& right) noexcept;

            void InitInfo(CTL_ITwainSession *pSession, CTL_ITwainSource* pSource, int nInfo);
            void DestroyInfo();

            TW_UINT16 Execute() override;

            // Get the number of information structures
            size_t GetNumInfo() const { return m_nNumInfo; }

            // Get the TW_INFO information
            TW_INFO GetInfo(size_t nWhich, int nSearch) const;

            // Set the information for item nWhich
            bool SetInfo(TW_INFO Info, size_t nWhich);

            bool AddInfo(TW_INFO Info);

            // Utility functions
            bool GetItemData(int nWhichItem, int nSearch, int nWhichValue, LPVOID Data, size_t *pItemSize= nullptr) const;

            bool IsItemHandle(size_t nWhich) const;

            ~CTL_ExtImageInfoTriplet() override;

            static bool EnumSupported(CTL_ITwainSource *pSource,
                                      CTL_ITwainSession *pSession,
                                      CTL_IntArray &rArray);

            bool RetrieveInfo(TWINFOVector &v) const;

        private:
            void ResolveTypes();
            bool CreateExtImageInfo();
            void CopyInfoToVector();

            TW_EXTIMAGEINFO *m_pExtImageInfo;
            TW_HANDLE m_memHandle;
            size_t m_nNumInfo;
            TWINFOVector m_vInfo;
            static TW_UINT16 s_AllAttr[80];
    };
}
#endif
