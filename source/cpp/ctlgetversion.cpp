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
#include "dtwain.h"
#include "ctlstringutils.h"
#include "ctlstringutilsx.h"
#include "cppfunc.h"
#include "dtwinverex.h"
#include "ctlgetversion.h"
#include "ctldtwainhandle.h"
#include "ctltwaindllpath.h"
#include "ctlstaticdata.h"
#include "ctltwaindllhandle.h"
#include "ctlconstexprutils.h"

using namespace dynarithmic;
namespace stringutils = basicstringutils;

namespace
{
    CTL_StringType GetDTWAINDLLVersionInfoStr()
    {
        return _T(DTWAIN_SHORT_VERSION);
    }

    DTWAIN_BOOL DTWAIN_GetVersionInternal(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatch, LPLONG lBuildNumber = nullptr)
    {
        LOG_FUNC_ENTRY_PARAMS((lMajor, lMinor, lVersionType, lBuildNumber))
            constexpr LONG nDistr = DTWAIN_OPENSOURCE_VERSION;
        static constexpr auto modRet = GetDTWAINDLLVersionInfo();
        if (lMajor)
            *lMajor = modRet[0];
        if (lMinor)
            *lMinor = modRet[1];
        if (lPatch)
            *lPatch = modRet[2];
        if (lVersionType)
            *lVersionType = nDistr | GetDTWAINVersionType();
        if (lBuildNumber)
            *lBuildNumber = modRet[3];
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
            CATCH_BLOCK(false)
    }
}

namespace dynarithmic
{
    CTL_StringType GetVersionString()
    {
        auto& verString = CTL_StaticData::GetVersionString();
        if (!verString.empty())
            return verString;

        constexpr LONG lVersionType = DTWAIN_OPENSOURCE_VERSION | GetDTWAINVersionType();
        CTL_StringType s;
        const TCHAR* sBits = _T("[32-bit]");
        if (lVersionType & DTWAIN_64BIT_VERSION)
            sBits = _T("[64-bit]");

        s += CTL_StringType(_T(" ")) + _T(DTWAIN_VCRUNTIME_CHARSET);

        if (lVersionType & DTWAIN_DEVELOP_VERSION)
            s += _T(" [Debug]");
        else
            s += _T(" [Release]");

        s += _T(DTWAIN_BUILD_LOGGINGNAME);
        s += _T(DTWAIN_VCRUNTIME_BUILDNAME);
        s += _T(" ");
        s += sBits;

        auto appName = CTL_StaticData::GetApplicationName();
        CTL_StringType& sOut = verString;
        sOut += _T("Dynarithmic TWAIN Library, Version ");
        sOut += _T(DTWAIN_VERINFO_FILEVERSION);
        sOut += _T(' ') + s + _T('\n');
        sOut += _T("Shared Library path : ") + GetDTWAINDLLPath();
        sOut += _T("\nUsing Resource file (twaininfo.txt) version: ");
        sOut += _T(DTWAIN_TEXTRESOURCE_FILEVERSION);
        sOut += _T("\nResource file path: ") + CTL_StaticData::GetResourcePath();
        sOut += _T("\nText Resource Language: ") + CTL_StaticData::GetGeneralResourceInfo().sResourceName;
        if (CTL_StaticData::GetGeneralResourceInfo().bIsFromRC)
            sOut += _T(" (Text resources are directly from DTWAIN DLL and not from a text resource file)");
        sOut += _T("\nApplication Name: ") + appName;
        return sOut;
    }
}

namespace
{
    std::string GetStaticLibVer()
    {
        const LONG nVer = DTWAIN_GetStaticLibVersion();
        if (nVer != 0)
        {
            switch (nVer)
            {
              case 81:
                  return "Microsoft Visual Studio 2019";
              case 91:
                  return "Microsoft Visual Studio 2022";
            }
        }
        return {};
    }
}

extern "C"
{
    LONG DLLENTRY_DEF DTWAIN_GetStaticLibVersion()
    {
        LOG_FUNC_ENTRY_PARAMS(())
        #ifndef DTWAIN_LIB
            LOG_FUNC_EXIT_NONAME_PARAMS(0)
            #pragma message ("Compiling DLL -- no static library")
        #endif

        #ifdef _MSC_VER
            #if _MSC_VER < 1920
                #error("C++ Compiler must be Visual Studio 2019 or greater")
            #elif _MSC_VER >= 1920 && _MSC_VER < 1930
                #pragma message ("Microsoft Visual Studio 2019 compiler used to build library")
                LOG_FUNC_EXIT_NONAME_PARAMS(81)
            #elif _MSC_VER >= 1930
                #pragma message ("Microsoft Visual Studio 2022 compiler used to build library")
                LOG_FUNC_EXIT_NONAME_PARAMS(91)
            #endif
        #endif
        #ifndef _MSC_VER
            #pragma message("Unsupported compiler being used to compile DTWAIN")
        #endif

        CATCH_BLOCK(-1)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetVersion(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType)
    {
        LOG_FUNC_ENTRY_PARAMS((lMajor, lMinor, lVersionType))
        const bool bRetVal = DTWAIN_GetVersionInternal(lMajor, lMinor, lVersionType, nullptr) ? true : false;
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lMajor, lMinor, lVersionType))
        LOG_FUNC_EXIT_NONAME_PARAMS(bRetVal)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetVersionEx(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatchLevel)
    {
        LOG_FUNC_ENTRY_PARAMS((lMajor,lMinor,lVersionType, lPatchLevel))
        const bool bRetVal = DTWAIN_GetVersionInternal(lMajor, lMinor, lVersionType, lPatchLevel)?true:false;
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lMajor, lMinor, lVersionType, lPatchLevel))
        LOG_FUNC_EXIT_NONAME_PARAMS(bRetVal)
        CATCH_BLOCK(false)
    }

    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetVersionEx2(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatchLevel, 
                                                  LPLONG lBuildNumber)
    {
        LOG_FUNC_ENTRY_PARAMS((lMajor, lMinor, lVersionType, lPatchLevel, lBuildNumber))
        const bool bRetVal = DTWAIN_GetVersionInternal(lMajor, lMinor, lVersionType, lPatchLevel, lBuildNumber) ? true : false;
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lMajor, lMinor, lVersionType, lPatchLevel, lBuildNumber))
        LOG_FUNC_EXIT_NONAME_PARAMS(bRetVal)
        CATCH_BLOCK(false)
    }

    // Check the match type.  If version must be < than the passed-in values, return TRUE if version is <
    // If version must be equal to the passed-in values, return TRUE if version is equal
    // If version must be greater to the passed-in values, return TRUE if version is greater
    // Build number is ignored if passed-in build number is 0.
    DTWAIN_BOOL DLLENTRY_DEF DTWAIN_CheckDLLVersion(LONG lMajor, LONG lMinor, LONG lPatchLevel, LONG lBuildNumber,
                                                    LONG MatchType)
    {
        LOG_FUNC_ENTRY_PARAMS((lMajor, lMinor, lPatchLevel, lBuildNumber, MatchType))
        VersionNumbersSmall info;
        bool bMatchOk = false;
    
        auto versionVals = GetDTWAINDLLVersionInfo();

        if (lBuildNumber == 0)
            versionVals[3] = 0;

        std::array<int, 4> userVals = { lMajor, lMinor, lPatchLevel, lBuildNumber };

        int compareResults = -1;
        if (versionVals > userVals)
            compareResults = 1;
        else
        if (versionVals == userVals)
            compareResults = 0;

        switch ( MatchType )
        {
            case DTWAIN_CHECKDLLVERLESS:
                bMatchOk = (compareResults == -1);
            break;
            case DTWAIN_CHECKDLLVEREQUAL:
                bMatchOk = (compareResults == 0);
            break;
            case DTWAIN_CHECKDLLVERGREATER:
                bMatchOk = (compareResults == 1);
            break;
            case DTWAIN_CHECKDLLVERLESSEQ:
                bMatchOk = (compareResults == -1 || compareResults == 0);
            break;
            case DTWAIN_CHECKDLLVERGREATEREQ:
                bMatchOk = (compareResults == 1 || compareResults == 0);
            break;
            default:
                bMatchOk = (compareResults == 0);
            break;
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(bMatchOk)
        CATCH_BLOCK(false)
    }


    LONG DLLENTRY_DEF DTWAIN_GetVersionString(LPTSTR lpszVer, LONG nLength)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
        const LONG RetVal = CopyInfoToCString(GetVersionString(), lpszVer, nLength);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
        LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetLibraryPath(LPTSTR lpszVer, LONG nLength)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
        const LONG RetVal = CopyInfoToCString(GetDTWAINDLLPath(), lpszVer, nLength);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
        LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetShortVersionString(LPTSTR lpszVer, LONG nLength)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
        const LONG RetVal = CopyInfoToCString(GetDTWAINDLLVersionInfoStr(), lpszVer, nLength);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
        LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionInfo(LPTSTR lpszVer, LONG nLength)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
        const LONG RetVal = CopyInfoToCString(GetVersionInfo(), lpszVer, nLength);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
        LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetVersionCopyright(LPTSTR lpszVer, LONG nLength)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
        const LONG RetVal = static_cast<LONG>(GetResourceString(IDS_DTWAIN_APPTITLE, lpszVer, nLength));
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
        LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
        CATCH_BLOCK(-1)
    }

    LONG DLLENTRY_DEF DTWAIN_GetWindowsVersionInfo(LPTSTR lpszBuffer, LONG nMaxLen)
    {
        LOG_FUNC_ENTRY_PARAMS((lpszBuffer, nMaxLen))
        LONG RetVal = 0;
        auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
        if (pHandle)
        {
            if (pHandle->m_sWindowsVersionInfo.empty())
            {
                RetVal = CopyInfoToCString(GetWinVersion(), lpszBuffer, nMaxLen);
                if (lpszBuffer)
                    pHandle->m_sWindowsVersionInfo = lpszBuffer;
            }
            else
                RetVal = CopyInfoToCString(pHandle->m_sWindowsVersionInfo, lpszBuffer, nMaxLen);
        }
        else
            RetVal = CopyInfoToCString(GetWinVersion(), lpszBuffer, nMaxLen);
        LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
        LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
        CATCH_BLOCK(-1)
    }
    }