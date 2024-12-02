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
#include "ctltr022.h"

using namespace dynarithmic;

CTL_UserInterfaceUIOnlyTriplet::CTL_UserInterfaceUIOnlyTriplet(CTL_ITwainSession *pSession,
                                                               CTL_ITwainSource* pSource,
                                                               TW_USERINTERFACE *pUI
                                                               ) :

    CTL_UserInterfaceEnableTriplet(pSession, pSource, pUI, 1, MSG_ENABLEDSUIONLY)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
CTL_UserInterfaceEnableTriplet::CTL_UserInterfaceEnableTriplet(CTL_ITwainSession *pSession,
                                                               CTL_ITwainSource* pSource,
                                                               TW_USERINTERFACE *pUI,
                                                               TW_BOOL bShowUI/*=TRUE*/,
                                                               TW_UINT16 nMsg/*=MSG_ENABLEDS*/)
                       :  CTL_UserInterfaceTriplet(pSession, pSource, nMsg, pUI, bShowUI)
{
}


TW_UINT16 CTL_UserInterfaceEnableTriplet::Execute()
{
    CTL_ITwainSource *pSource = GetSourcePtr();

    if ( pSource->IsUIOpen() )
        return TWRC_SUCCESS;

    const TW_UINT16 rc = CTL_UserInterfaceTriplet::Execute();
    if ( rc == TWRC_SUCCESS )
        pSource->SetUIOpen(true);
    else
        pSource->SetUIOpen(false);
    return rc;
}

