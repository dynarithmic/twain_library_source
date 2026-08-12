/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2026 Dynarithmic Software.

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
#ifndef CTLINTERNALCONSTANTS_H
#define CTLINTERNALCONSTANTS_H

#define DTWAIN_INTERNAL_NOTIFICATION   10000
/* DTWAIN Source UI Close Modes */
#define DTWAIN_SourceCloseModeFORCE           0
#define DTWAIN_SourceCloseModeBYPASS          1

#define DSM_STATE_NONE      1
#define DSM_STATE_LOADED    2
#define DSM_STATE_OPENED    3

// Select source wParam's 
#define  DTWAIN_SelectSourceFailed             (DTWAIN_INTERNAL_NOTIFICATION + 1)
#define  DTWAIN_AcquireSourceClosed            (DTWAIN_INTERNAL_NOTIFICATION + 2)
#define  DTWAIN_TN_ACQUIRECANCELLED_EX         (DTWAIN_INTERNAL_NOTIFICATION + 3)
#define  DTWAIN_TN_ACQUIREDONE_EX              (DTWAIN_INTERNAL_NOTIFICATION + 4)
#define  DTWAIN_RETRY_EX                       (DTWAIN_INTERNAL_NOTIFICATION + 5)

// modal processing messages
#define DTWAIN_TN_MESSAGELOOPERROR             (DTWAIN_INTERNAL_NOTIFICATION + 6)
#define REGISTERED_DTWAIN_MSG _T("DTWAIN_NOTIFY-{37AE5C3E-34B6-472f-A0BC-74F3CB199F2B}")

/* Transfer started */
/* Scanner already has physically scanned a page.
    This is sent only once (when TWAIN actually does the transformation of the
    scanned image to the DIB) */
#define  DTWAIN_TWAINAcquireStarted          (DTWAIN_INTERNAL_NOTIFICATION + 7)

/* Sent when DTWAIN_Acquire...() functions are about to return */
#define  DTWAIN_AcquireTerminated            (DTWAIN_INTERNAL_NOTIFICATION + 8)
#ifdef _WIN32
    #define  TWAINDLLVERSION_1    _T("TWAIN_32.DLL")
    #define  TWAINDLLVERSION_2    _T("TWAINDSM.DLL")
#else
    #define  TWAINDLLVERSION_1    ""
    #define  TWAINDLLVERSION_2    "/usr/local/lib/libtwaindsm.so"
#endif

#define THIS_FUNCTION_PROTO_THROWS  ;
#define THIS_FUNCTION_THROWS

#define IDS_DTWAIN_APPTITLE       9700
#define IDS_DTWAIN_APPTITLE_HTML  9701

#endif