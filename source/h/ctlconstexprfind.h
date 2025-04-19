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
#ifndef CTLCONSTEXPRFIND_H
#define CTLCONSTEXPRFIND_H
#include <utility>
#include <algorithm>

namespace dynarithmic
{
    template <typename Arr, typename Fn>
    static constexpr std::pair<bool, unsigned> generic_array_finder_if(const Arr& theArray, Fn fn)
    {
        for (auto iter = theArray.begin(); iter != theArray.end(); ++iter)
        {
            if (fn(*iter))
                return { true, static_cast<unsigned>(std::distance(theArray.begin(), iter)) };
        }
        return { false,0 };
    }

    template <typename Arr, typename Val>
    static constexpr std::pair<bool, unsigned> generic_array_finder(const Arr& theArray, const Val& value)
    {
        return generic_array_finder_if(theArray, [&](int val) { return val == value; });
    }

}
#endif