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
#ifndef CTLTR011_H
#define CTLTR011_H
#include "ctltr010.h"
namespace dynarithmic
{
    class CTL_CapabilityGetTriplet : public CTL_CapabilityTriplet
    {
        public:
            CTL_CapabilityGetTriplet(CTL_ITwainSession      *pSession,
                                     CTL_ITwainSource       *pSource,
                                     TW_UINT16  gType,
                                     TW_UINT16 gCap,
                                     TW_UINT16 TwainDataType=0xFFFF);

            virtual size_t          GetNumItems() {return m_nNumItems;}

            virtual bool            GetValue( void * /*pData*/, size_t /*nWhere*/ =0 ){ return false; }

            CTL_EnumContainer       GetSupportedContainer() const { return m_nContainerToUse; }
            TW_UINT16               Execute() override;


        protected:
            TW_UINT16         CapRetrievalType() const;
            TW_UINT16       CapToRetrieve()    const;

            virtual bool            EnumCapValues( void * /*pCapData*/) { return false; }

            bool GenericGetValue(void* pData, size_t nWhere = 0)
            {
                const CTL_TwainTypeArray* pArray = GetTwainTypeArray();

                if (nWhere >= m_nNumItems)
                    return false;
                const auto pOb = (*pArray)[nWhere].get();
                if (pOb)
                {
                    pOb->GetData(pData);
                    return true;
                }
                return false;
            }

            template <typename T>
            bool GenericEnumCapValues(void* pCapData)
            {
                size_t nNumItems = 1;

                // dereference to a pointer to the Twain container
                T* pTwainContainer = static_cast<T*>(pCapData);

                // Get # of items in Twain container if it is not one value
                if constexpr (!std::is_same_v<T, TW_ONEVALUE>)
                    nNumItems = static_cast<size_t>(pTwainContainer->NumItems);

                // Get item type
                TW_UINT16 nItemType = GetEffectiveItemType(pTwainContainer->ItemType);

                // Get sizeof each item in Twain container
                const int nItemSize = dynarithmic::GetTwainItemSize(nItemType);

                // Unknown item type.  Do error condition here??
                if (nItemSize == 0)
                    return false;

                // Set the item type for this object
                CTL_TwainTypeObPtr pOb;

                RemoveAllTypeObjects();

                CTL_TwainTypeArray* pArray = GetTwainTypeArray();
                auto sessionHandle = GetSessionPtr()->GetTwainDLLHandle();
                for (TW_UINT16 nIndex = 0; nIndex < nNumItems; nIndex++)
                {
                    if (nItemType == TWTY_FIX32)
                        pOb = std::make_shared<CTL_TwainTypeOb>(sessionHandle, static_cast<TW_UINT16>(sizeof(double)), false);
                    else
                        pOb = std::make_shared<CTL_TwainTypeOb>(sessionHandle, nItemType);

                    if (nItemType == TWTY_FIX32)
                    {
                        pTW_FIX32 p = {};
                        if constexpr (std::is_same_v<T, TW_ONEVALUE>)
                            p = reinterpret_cast<pTW_FIX32>(&pTwainContainer->Item);
                        else
                            p = reinterpret_cast<pTW_FIX32>(&pTwainContainer->ItemList[nIndex * nItemSize]);
                        double fFix = Fix32ToFloat(*p);
                        pOb->CopyData(&fFix);
                    }
                    else
                    {
                        // Copy Data to pOb
                        if constexpr (std::is_same_v <T, TW_ONEVALUE>)
                            pOb->CopyData(static_cast<void*>(&pTwainContainer->Item));
                        else
                            pOb->CopyData(static_cast<void*>(&pTwainContainer->ItemList[nIndex * nItemSize]));
                    }
                    // Store this object in object array
                    pArray->push_back(pOb);
                }
                this->m_nNumItems = nNumItems;
                return true;
            }

            void    Decode(void *p) override;

            TW_UINT16 GetEffectiveItemType(TW_UINT16 curDataType) const;

        protected:
            size_t m_nNumItems = 0;

        private:
            TW_UINT16               m_gCap;
            TW_UINT16         m_gType;
            CTL_EnumContainer       m_nContainerToUse;
    };
}
#endif
