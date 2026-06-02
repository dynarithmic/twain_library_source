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
#ifndef CTLBIMAP_H
#define CTLBIMAP_H

#include <map>
#include <stdexcept>

namespace dynarithmic
{
    template<typename Left, typename Right>
    class BidirectionMap
    {
    public:
        using left_type = Left;
        using right_type = Right;

        bool Insert(const Left& l, const Right& r)
        {
            if (left_to_right.count(l) || right_to_left.count(r))
                return false;

            left_to_right[l] = r;
            right_to_left[r] = l;
            return true;
        }

        bool EraseLeft(const Left& l)
        {
            auto it = left_to_right.find(l);
            if (it == left_to_right.end())
                return false;

            right_to_left.erase(it->second);
            left_to_right.erase(it);
            return true;
        }

        bool EraseRight(const Right& r)
        {
            auto it = right_to_left.find(r);
            if (it == right_to_left.end())
                return false;

            left_to_right.erase(it->second);
            right_to_left.erase(it);
            return true;
        }

        const Right& AtLeft(const Left& l) const
        {
            return left_to_right.at(l);
        }

        const Left& AtRight(const Right& r) const
        {
            return right_to_left.at(r);
        }

        bool ContainsLeft(const Left& l) const
        {
            return left_to_right.count(l) != 0;
        }

        bool ContainsRight(const Right& r) const
        {
            return right_to_left.count(r) != 0;
        }

        void Clear()
        {
            left_to_right.clear();
            right_to_left.clear();
        }

        std::size_t Size() const
        {
            return left_to_right.size();
        }

        const auto& GetRightToLeftMap() const { return right_to_left; }
        const auto& GetLeftToRightMap() const { return left_to_right; }
    private:
        std::map<Left, Right> left_to_right;
        std::map<Right, Left> right_to_left;
    };
}
#endif
