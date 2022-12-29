/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#ifndef CTLTR039_H
#define CTLTR039_H

#include "ctltrp.h"
namespace dynarithmic
{
    class CTL_ITwainSession;

    class CTL_EntryPointTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_EntryPointTriplet(CTL_ITwainSession *pSession, TW_UINT16 msg);
            TW_UINT16 Execute() override;
            TW_ENTRYPOINT& getEntryPoint() { return m_EntryPoint; }
            bool isTripletFound() const { return m_bTripletFound; }

        private:
            TW_ENTRYPOINT m_EntryPoint{};
            bool m_bTripletFound;
    };

    class CTL_EntryPointTripletGet : public CTL_EntryPointTriplet
    {
        public:
            CTL_EntryPointTripletGet(CTL_ITwainSession *pSession) : CTL_EntryPointTriplet(pSession, MSG_GET){}
    };

    class CTL_EntryPointTripletSet : public CTL_EntryPointTriplet
    {
        public:
            CTL_EntryPointTripletSet(CTL_ITwainSession *pSession) : CTL_EntryPointTriplet(pSession, MSG_SET){}
    };
}
#endif
