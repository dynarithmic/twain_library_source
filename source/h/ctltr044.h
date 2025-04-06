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
#ifndef CTLTR044_H
#define CTLTR044_H

#include "ctltripletbase.h"
#include "ctltwainsession.h"

namespace dynarithmic
{
    class CTL_JPEGCompressionTriplet : public CTL_TwainTriplet
    {
        public:
            CTL_JPEGCompressionTriplet(CTL_ITwainSession *pSession,
                                     CTL_ITwainSource *pSource,
                                     TW_UINT16 nMsg);

            TW_JPEGCOMPRESSION& GetJPEGCompressionInfo() { return m_JPEGCompressionInfo; }

        protected:
            TW_JPEGCOMPRESSION    m_JPEGCompressionInfo;
    };

    class CTL_JPEGCompressionGetTriplet : public CTL_JPEGCompressionTriplet
    {
        public:
            CTL_JPEGCompressionGetTriplet(CTL_ITwainSession *pSession,
                                          CTL_ITwainSource *pSource) : 
                        CTL_JPEGCompressionTriplet(pSession, pSource, MSG_GET) {}
    };

    class CTL_JPEGCompressionGetDefaultTriplet : public CTL_JPEGCompressionTriplet
    {
        public:
            CTL_JPEGCompressionGetDefaultTriplet(CTL_ITwainSession* pSession,
                CTL_ITwainSource* pSource) :
                CTL_JPEGCompressionTriplet(pSession, pSource, MSG_GETDEFAULT) {}
    };

    class CTL_JPEGCompressionSetTriplet : public CTL_JPEGCompressionTriplet
    {
        public:
            CTL_JPEGCompressionSetTriplet(CTL_ITwainSession *pSession,
                                          CTL_ITwainSource *pSource) : 
                        CTL_JPEGCompressionTriplet(pSession, pSource, MSG_SET) {}
    };

    class CTL_JPEGCompressionResetTriplet : public CTL_JPEGCompressionTriplet
    {
        public:
            CTL_JPEGCompressionResetTriplet(CTL_ITwainSession* pSession,
                CTL_ITwainSource* pSource) :
                CTL_JPEGCompressionTriplet(pSession, pSource, MSG_RESET) {}
    };
}
#endif

