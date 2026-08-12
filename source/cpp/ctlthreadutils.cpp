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
#include <thread>
#include <string>
#include <boost/lexical_cast.hpp>
#include "ctliface.h"
#include "ctlthreadutils.h"
#include "cppfunc.h"
#include "ctldtwainhandle.h"
#include "ctlstaticdata.h"

namespace dynarithmic
{
    CTL_ThreadMap::iterator getThreadIdIter()
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

using namespace dynarithmic;

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartThread( DTWAIN_HANDLE DLLHandle )
    {
        LOG_FUNC_ENTRY_PARAMS((DLLHandle))
        if (!CTL_StaticData::IsUsingMultipleThreads())
            LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        auto iter = std::find_if(threadMap.begin(),threadMap.end(), [&](const auto& pr) 
                                    { return pr.second.get() == static_cast<CTL_TwainDLLHandle*>(DLLHandle); });
        if ( iter != threadMap.end())
            AssociateThreadToTwainDLL(iter->second, getThreadId());
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndThread( DTWAIN_HANDLE DLLHandle )
    {
        LOG_FUNC_ENTRY_PARAMS((DLLHandle))
        if ( !CTL_StaticData::IsUsingMultipleThreads())
            LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
        if ( threadMap.size() == 1)
            LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto iter = std::find_if(threadMap.begin(), threadMap.end(),[&](const auto& pr)
            { return pr.second.get() == static_cast<CTL_TwainDLLHandle*>(DLLHandle); });

        if (iter != threadMap.end() && 
            iter->first == getThreadId())
        {
            threadMap.erase(iter);
            LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_UseMultipleThreads(DTWAIN_BOOL bSet)
    {
        LOG_FUNC_ENTRY_PARAMS((bSet))
        CTL_StaticData::SetUseMultipleThreads(bSet ? true : false);
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
    }
}