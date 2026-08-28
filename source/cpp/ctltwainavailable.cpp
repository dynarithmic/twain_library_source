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

#include <algorithm>
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include <ctlstringutilsx.h>
#include <errorcheck.h>
#include "ctldtwainhandle.h"
#include "ctltwainlogging.h"
#include "dtwainx.h"

using namespace dynarithmic;

static const constexpr TCHAR* s_NullEntry = _T("<null>");

namespace
{
    // Determines if any TWAIN subsystem is available (TWAIN version 1.x and 2.x)
    std::pair<LONG, std::array<CTL_StringType, 2>> GetTwainAvailablityInternal()
    {
        std::pair<LONG, std::array<CTL_StringType, 2>> retVal;
        LONG availableFlag = 0;
        bool bRet = false;

    #ifndef _WIN64
        // Check TWAIN version 1.x
        auto existCheck = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_1);
	    bRet = existCheck.first;
        if (bRet)
        {
            availableFlag |= DTWAIN_TWAINDSM_LEGACY;
            // Store the path found
            retVal.second[0] = existCheck.second;
        }
    #else
        retVal.second[0] = {};
    #endif
        // Check TWAIN version 2.x
        auto existCheck2 = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_2);
        bRet = existCheck2.first;
        if (bRet)
        {
            availableFlag |= DTWAIN_TWAINDSM_VERSION2;
            // Store the path found
            retVal.second[1] = existCheck2.second;
        }
        retVal.first = availableFlag;
        return retVal;
    }

    LONG IsTwainAvailableHelper(LPTSTR directories, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((directories, nMaxLen))
        struct SysInitializerRAII
        {
            bool bMustDestroy;
            SysInitializerRAII(bool mustDestroy) : bMustDestroy(mustDestroy) {}
            ~SysInitializerRAII()
            {
                if (bMustDestroy)
                    DTWAIN_SysDestroy();
            }
        };

        // Check if TWAIN session started.  If so, they must have had TWAIN installed!
        // Save the filter flags
        DTWAINScopedLogController sLogContoller(0);
        bool bMustDestroy = false;
        try
        {
            CTL_TwainDLLHandle* pHandle = nullptr;
            // Check if DTWAIN already initialized
            pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            if (!pHandle)
            {
                // Temporarily set up a handle without loading everything
                pHandle = static_cast<CTL_TwainDLLHandle*> (SysInitializeImpl({ false, false , true }));
                if (!pHandle)
                    LOG_FUNC_EXIT_NONAME_PARAMS(DTWAIN_ERR_BAD_HANDLE)
                bMustDestroy = true;
            }
        }
        catch (...)
        {
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
        }

        SysInitializerRAII raii(bMustDestroy);
        try
        {
            // Test for availability
            auto infoLength = DTWAIN_GetTwainAvailabilityEx(directories, nMaxLen);
            LOG_FUNC_EXIT_NONAME_PARAMS(infoLength)
        }
        catch (...)
        {
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
        }
        CATCH_BLOCK(0)
    }

    bool CheckTwainAvailability(LPTSTR directories, LONG nMaxLen, LONG* maxCharsCopied)
    {
        TCHAR szTemp[100] = {};
        LPTSTR dirsToUse = directories;
        LONG maxLenToUse = nMaxLen;
        if (!dirsToUse)
        {
            dirsToUse = szTemp;
            maxLenToUse = 100;
        }
        CTL_StringArrayType arr;
        auto retVal = IsTwainAvailableHelper(dirsToUse, maxLenToUse);
        if (maxCharsCopied)
            *maxCharsCopied = retVal;
        basicstringutils::Tokenize(dirsToUse, _T("|"), arr);
        return std::any_of(arr.begin(), arr.end(), 
                        [&](const auto& s) { return s != s_NullEntry; });
    }
}

extern "C"
{
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTwainAvailable()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto retVal = CheckTwainAvailability(nullptr, 0, nullptr);
        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
        CATCH_BLOCK(0)
    }

    LONG DLLENTRY_DEF DTWAIN_IsTwainAvailableEx(LPTSTR directories, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        LONG maxCharsCopied = 0;
        CheckTwainAvailability(directories, nMaxLen, &maxCharsCopied);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((directories))
        LOG_FUNC_EXIT_NONAME_PARAMS(maxCharsCopied)
        CATCH_BLOCK(0)
    }

    // Test which version of TWAIN is available.
    LONG DLLENTRY_DEF DTWAIN_GetTwainAvailability()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto availability = GetTwainAvailablityInternal();
        LOG_FUNC_EXIT_NONAME_PARAMS(availability.first)
        CATCH_BLOCK(0)
    }

    // Test which version of TWAIN is available.
    LONG DLLENTRY_DEF DTWAIN_GetTwainAvailabilityEx(LPTSTR directories, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS(())
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
        auto availability = GetTwainAvailablityInternal();

        // If not available set the error and exit
        DTWAIN_Check_Error_Condition_WithThrow_Ex(pHandle, [&] { return availability.first == 0; },
                                            DTWAIN_ERR_TWAIN_NOT_INSTALLED, 0, FUNC_MACRO);

        // Provide "<null>" for either TWAIN 1 or TWAIN 2 directories in the
        // returned array of directories if TWAIN could not be found
        for (auto& s : availability.second)
        {
            if (s.empty())
                s = _T("<null>");
        }

        CTL_StringType sDirs;
        auto joinedString = basicstringutils::Join(availability.second, _T("|"));
        auto actualLengthCopied = CopyInfoToCString(joinedString, directories, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((directories))
        LOG_FUNC_EXIT_NONAME_PARAMS(actualLengthCopied)
        CATCH_BLOCK(0)
    }
}