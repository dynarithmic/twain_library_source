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
#ifndef DTWAIN_VERINFO_H
#define DTWAIN_VERINFO_H

#include "dtwain_version.h"

#ifdef _DEBUG
    #define DTWAIN_DEVELOP_SUFFIX "d"
#else
    #define DTWAIN_DEVELOP_SUFFIX ""
#endif

#ifdef _WIN32
#ifdef UNICODE
#define UNICODE_VERSION "(Unicode)"
#define UNICODE_SUFFIX  "u"
#else
#define UNICODE_VERSION
#define UNICODE_SUFFIX ""
#endif

#if defined (WIN64) || (_WIN64)
        #define DTWAIN_DLLNAME      "dtwain64" UNICODE_SUFFIX DTWAIN_DEVELOP_SUFFIX ".dll"
        #define DTWAIN_FILEDESCRIPTION  "dtwain64" UNICODE_SUFFIX DTWAIN_DEVELOP_SUFFIX
        #define DTWAIN_OSPLATFORM "64"
#else
        #define DTWAIN_DLLNAME      "dtwain32" UNICODE_SUFFIX DTWAIN_DEVELOP_SUFFIX ".dll"
        #define DTWAIN_FILEDESCRIPTION  "dtwain32" UNICODE_SUFFIX DTWAIN_DEVELOP_SUFFIX
        #define DTWAIN_OSPLATFORM "32"
#endif
#endif

#ifndef _WIN32
    #if defined (UNIX64)
        #define DTWAIN_DLLNAME      "libdtwain64" DTWAIN_DEVELOP_SUFFIX ".a"
        #define DTWAIN_FILEDESCRIPTION  "libdtwain64" DTWAIN_DEVELOP_SUFFIX
        #define DTWAIN_OSPLATFORM "64"
    #else
        #define DTWAIN_DLLNAME      "libdtwain32" DTWAIN_DEVELOP_SUFFIX ".a"
        #define DTWAIN_FILEDESCRIPTION  "libdtwain32" DTWAIN_DEVELOP_SUFFIX
        #define DTWAIN_OSPLATFORM "32"
    #endif
#endif


#define DTWAIN_VERINFO_COMMENTS             "Patch Level " DTWAIN_VERINFO_PATCHLEVEL_VERSION "\0"
#define DTWAIN_VERINFO_COMPANYNAME          "Dynarithmic Software\0"
#define DTWAIN_VERINFO_LEGALCOPYRIGHT       "Copyright © 2020-2022\0"
#define DTWAIN_VERINFO_PRODUCTNAME          "Dynarithmic Software Twain Library " UNICODE_VERSION "\0"
#define DTWAIN_VERINFO_INTERNALNAME         DTWAIN_DLLNAME
#define DTWAIN_VERINFO_ORIGINALFILENAME     DTWAIN_DLLNAME
#define DTWAIN_VERINFO_LEGALTRADEMARKS      "Dynarithmic Software\0"
#define DTWAIN_VERINFO_FILEDESCRIPTION      DTWAIN_FILEDESCRIPTION
#define DTWAIN_VERINFO_SPECIALBUILD         "Open Source Distribution (Special) " PDF_STRING_VERSION DEVELOP_DLL_VERSION "Version\0"
#define DTWAIN_VERINFO_PRIVATEBUILD         "Open Source Distribution (Special) " PDF_STRING_VERSION DEVELOP_DLL_VERSION "Version\0"

#define DTWAIN_NUMERIC_FILE_VERSION         DTWAIN_MAJOR_VERSION,DTWAIN_SUBVERSION_VERSION,DTWAIN_PLACEHOLDER_VERSION,DTWAIN_VERINFO_PATCHLEVEL_VERSION
#define DTWAIN_NUMERIC_PRODUCT_VERSION          DTWAIN_NUMERIC_FILE_VERSION

#define PDF_STRING_VERSION "PDF "
#define DEVELOP_DLL_VERSION ""

#define DTWAIN_VERINFO_PRODUCTVERSION       DTWAIN_VERINFO_SPECIALBUILD
#endif
