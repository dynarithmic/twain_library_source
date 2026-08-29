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
#ifndef ENUMERATORNODE_H
#define ENUMERATORNODE_H

#include <vector>

namespace dynarithmic
{
    template <typename T, int enumType = 0>
    struct CTL_EnumeratorNode
    {
        using container_base_type = std::vector<T>;
        using container_pointer_type = container_base_type*;
        using container_iterator_type = typename container_base_type::iterator;
        int m_EnumType;
        container_base_type m_Array;
        CTL_EnumeratorNode(int nSize) : m_EnumType(enumType), m_Array(nSize) {}
        int GetEnumType() const { return m_EnumType; }
        void SetEnumType(int EnumType) { m_EnumType = EnumType; }
        enum { ENUMTYPE = enumType };
    };
}
#endif
