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
#ifndef CTLTR016_INL
#define CTLTR016_INL

template <class T>
CTL_CapabilitySetTriplet<T>::CTL_CapabilitySetTriplet(CTL_ITwainSession *pSession,
                                                    CTL_ITwainSource* pSource,
                                                    CTL_EnumSetType sType,
                                                    TW_UINT16    sCap,
                                                    TW_UINT16 TwainType,
                                                    const std::vector<T> & rArray
                                                    ) : CTL_CapabilitySetTripletBase(pSession, pSource, sType, sCap, TwainType), m_Array(rArray)
{}

template <class T>
TW_UINT16 CTL_CapabilitySetTriplet<T>::Execute()
{
    TW_UINT16   rc = TWRC_FAILURE;
    void *pCapPtr = PreEncode();
    if ( pCapPtr )
    {
        if ( Encode(m_Array, pCapPtr) )
            // Call base class
            rc = CTL_TwainTriplet::Execute();
    }
    return PostEncode(rc);
}

#endif // CTLTR016_INL
