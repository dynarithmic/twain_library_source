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
#ifndef DTWAIN_TWAIN_HPP
#define DTWAIN_TWAIN_HPP
#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning (disable:4996)
#endif
#ifdef  DTWAIN_CPP_NOIMPORTLIB
    #define API_INSTANCE dynarithmic::twain::RuntimeDLL::DTWAIN_API__.
    #include "dtwainx2.h"
#else
    #define API_INSTANCE
    #include <dtwain.h>
#endif

namespace dynarithmic
{
    namespace twain
    {
        #ifdef  DTWAIN_CPP_NOIMPORTLIB
        struct RuntimeDLL
        {
            static DYNDTWAIN_API DTWAIN_API__;
        };
        #endif
    };
#ifdef _MSC_VER
    #pragma warning( pop )
#endif
}

#endif
