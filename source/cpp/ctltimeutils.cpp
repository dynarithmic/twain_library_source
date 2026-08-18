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

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/thread/thread_time.hpp>
#include "ctltimeutils.h"

namespace dynarithmic
{
    namespace timeutils
    {
        std::string GetSystemTimeInMilliseconds()
        {
            constexpr boost::posix_time::ptime time_t_epoch(boost::gregorian::date(1601, 1, 1));
            const auto systimex = boost::get_system_time();
            const boost::posix_time::time_duration diff = systimex - time_t_epoch;
            const auto mill = diff.total_milliseconds() * 10000LL;
            std::ostringstream strm;
            strm << mill;
            return strm.str();
        }
    }
}