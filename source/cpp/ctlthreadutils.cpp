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
#include <thread>
#include <string>
#include <cstdio>
#include <boost/lexical_cast.hpp>
#include "ctliface.h"
#include "ctlthreadutils.h"

namespace dynarithmic
{
    static CTL_ThreadMap::iterator getThreadIdIter()
    {
        auto& threadMap = CTL_StaticData::GetThreadMap();
        std::string threadId = boost::lexical_cast<std::string>(std::this_thread::get_id());
        auto iter = threadMap.find(threadId);
        if (iter == threadMap.end())
        {
            unsigned long threadNumber = 0;
            sscanf(threadId.c_str(), "%lx", &threadNumber);
            iter = threadMap.insert({ threadId, threadNumber }).first;
        }
        return iter;
    }

    unsigned long getThreadId()
    {
        auto iter = getThreadIdIter();
        return iter->second;
    }

    std::string getThreadIdAsString()
    {
        auto iter = getThreadIdIter();
        return iter->first;
    }
}
