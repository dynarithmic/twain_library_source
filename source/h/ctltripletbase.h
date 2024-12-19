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
#ifndef CTLTRIPLETBASE_H
#define CTLTRIPLETBASE_H

#include "ctltwainsession.h"
#include "ctlobstr.h"
#include <tuple>

namespace dynarithmic
{
    class CTL_TwainTriplet
    {
        public:
            enum {TRIPCOMPONENTPOS_= 2};
            enum {DGPOS_ = 0, DATPOS_ = 1, MSGPOS_ = 2, MEMREFPOS_ = 3};
            enum {ORIGINPOS_ = 0, DESTPOS_ =1};
            typedef std::tuple <TW_UINT32, TW_UINT16, TW_UINT16> TwainTripletComponents;

            CTL_TwainTriplet();
            CTL_TwainTriplet( pTW_IDENTITY pOrigin,
                              pTW_IDENTITY pDest,
                              TW_UINT32    DG,
                              TW_UINT16    DAT,
                              TW_UINT16    MSG,
                              TW_MEMREF    pData);

            void Init( const pTW_IDENTITY pOrigin,
                       const pTW_IDENTITY pDest,
                       TW_UINT32 nDG,
                       TW_UINT16 nDAT,
                       TW_UINT16 nMSG,
                       TW_MEMREF pData);

            typedef std::tuple<pTW_IDENTITY, pTW_IDENTITY, TwainTripletComponents, TW_MEMREF> TwainTripletArgs;

            const TwainTripletArgs& GetTripletArgs() const { return m_TwainTripletArg; }
            TwainTripletArgs& GetTripletArgs() { return m_TwainTripletArg; }
            const TwainTripletComponents& GetTripletComponents() const { return std::get<TRIPCOMPONENTPOS_>(m_TwainTripletArg); }
            TwainTripletComponents& GetTripletComponents() { return std::get<TRIPCOMPONENTPOS_>(m_TwainTripletArg); }

            virtual ~CTL_TwainTriplet() = default;
            virtual TW_UINT16 Execute();
            void    SetAlive( bool bSet );
            bool    IsAlive() const;
            bool    IsMSGGetType() const;
            bool    IsMSGSetType() const;
            bool    IsMSGResetType() const;

            pTW_IDENTITY GetOriginID() const { return std::get<ORIGINPOS_>(m_TwainTripletArg); }
            pTW_IDENTITY GetDestinationID() const { return std::get<DESTPOS_>(m_TwainTripletArg); }
            TW_UINT32 GetDG() const { return std::get<DGPOS_>(std::get<TRIPCOMPONENTPOS_>(m_TwainTripletArg)); }
            TW_UINT16 GetDAT() const { return std::get<DATPOS_>(std::get<TRIPCOMPONENTPOS_>(m_TwainTripletArg)); }
            TW_UINT16 GetMSG() const { return std::get<MSGPOS_>(std::get<TRIPCOMPONENTPOS_>(m_TwainTripletArg)); }
            TW_MEMREF GetMemRef() const { return std::get<MEMREFPOS_>(m_TwainTripletArg);}
            TW_MEMREF GetMemRef() { return std::get<MEMREFPOS_>(m_TwainTripletArg); }

            const CTL_ITwainSession* GetSessionPtr() const
            { return m_pSession; }

        protected:
            CTL_ITwainSession* GetSessionPtr()
            { return m_pSession; }

            void SetSessionPtr(CTL_ITwainSession* pSession)
            { m_pSession = pSession; }

            CTL_ITwainSource*  GetSourcePtr() const
            { return m_pSource; }

            void SetSourcePtr(CTL_ITwainSource* pSource)
            { m_pSource = pSource; }

        private:
            TwainTripletArgs m_TwainTripletArg;
            bool                    m_bInit;
            bool                    m_bAlive;
            CTL_ITwainSource*       m_pSource;
            CTL_ITwainSession*      m_pSession;
    };
}
#endif

