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
#ifndef DTWAIN_VERINFO_H
#define DTWAIN_VERINFO_H

#include "dtwain_config.h"
#include "dtwain_version.h"

#if DTWAIN_BUILD_LOGCALLSTACK == 1
    #if DTWAIN_BUILD_LOGPOINTERS == 1
        #define DTWAIN_BUILD_LOGGINGNAME  " [Logging+]\0"
    #else
        #define DTWAIN_BUILD_LOGGINGNAME  " [Logging-]\0"
    #endif
#else
    #define DTWAIN_BUILD_LOGGINGNAME  "\0"
#endif

#if DTWAIN_BUILD_NEEDVCRUNTIME == 1
    #define DTWAIN_VCRUNTIME_BUILDNAME " [VC++]\0"
#else
    #define DTWAIN_VCRUNTIME_BUILDNAME "\0"
#endif

#define DTWAIN_VERINFO_COMMENTS             DTWAIN_VERINFO_FILEVERSION " Build " DTWAIN_BUILDVERSION "\0"
#define DTWAIN_VERINFO_COMPANYNAME          "Dynarithmic Software\0"
#define DTWAIN_VERINFO_LEGALCOPYRIGHT       "Copyright © 2020-2025\0"
#define DTWAIN_VERINFO_PRODUCTNAME          "Dynarithmic Software Twain Library " UNICODE_VERSION "\0"
#define DTWAIN_VERINFO_INTERNALNAME         DTWAIN_DLLNAME
#define DTWAIN_VERINFO_ORIGINALFILENAME     DTWAIN_DLLNAME
#define DTWAIN_VERINFO_LEGALTRADEMARKS      "Dynarithmic Software\0"
#define DTWAIN_VERINFO_FILEDESCRIPTION      DTWAIN_FILEDESCRIPTION
#define DTWAIN_VERINFO_SPECIALBUILD         "Build " DTWAIN_BUILDVERSION "\0"
#define DTWAIN_VERINFO_PRIVATEBUILD         "Build " DTWAIN_BUILDVERSION "\0"

#define DTWAIN_NUMERIC_FILE_VERSION         DTWAIN_MAJOR_VERSION, DTWAIN_MINOR_VERSION, DTWAIN_PATCHLEVEL_VERSION
#define DTWAIN_NUMERIC_PRODUCT_VERSION      DTWAIN_NUMERIC_FILE_VERSION

#define PDF_STRING_VERSION "PDF "
#define DEVELOP_DLL_VERSION ""

#define DTWAIN_VERINFO_PRODUCTVERSION       DTWAIN_VERINFO_FILEVERSION " Build " DTWAIN_BUILDVERSION DTWAIN_BUILD_LOGGINGNAME DTWAIN_VCRUNTIME_BUILDNAME "\0"
#endif
