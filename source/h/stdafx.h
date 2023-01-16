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
 // stdafx.h : include file for standard system include files,
 // or project specific include files that are used frequently, but
 // are changed infrequently
 //

#pragma once
#include <stdio.h>
#include <tchar.h>

// TODO: reference additional headers your program requires here
#ifdef _WIN32
    #include <windows.h>
#endif
#include <algorithm>
#include <array>
#include <bitset>
#include <cstring>
#include <deque>
#include <list>
#include <queue>
#include <set>
#include <stack>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cstdlib>
#include <boost/functional/hash.hpp>
#include <boost/variant2/variant.hpp>
#include "../tsl/ordered_map.h"
#include "dtwain_filesystem.h"