/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2024 Dynarithmic Software.

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
#include "ctltwmgr.h"
#include "ctliface.h"
#include "cppfunc.h"

using namespace dynarithmic;

template <typename WrapperToUse, typename PointerType>
static HANDLE ConvertToAPIString_Internal(PointerType lpOrigString)
{
    return WrapperToUse::ConvertToAPIStringEx(lpOrigString);

}

HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIString(LPCTSTR lpOrigString)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString))
    auto retval = ConvertToAPIString_Internal<StringWrapper,LPCTSTR>(lpOrigString);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK((HANDLE)NULL)
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIStringA(LPCSTR lpOrigString)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString))
    auto retval = ConvertToAPIString_Internal<StringWrapperA,LPCSTR>(lpOrigString);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK((HANDLE)NULL)
}

HANDLE DLLENTRY_DEF DTWAIN_ConvertToAPIStringW(LPCWSTR lpOrigString)
{
    LOG_FUNC_ENTRY_PARAMS((lpOrigString))
    auto retval = ConvertToAPIString_Internal<StringWrapperW,LPCWSTR>(lpOrigString);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK((HANDLE)NULL)
}