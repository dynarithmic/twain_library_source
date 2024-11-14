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
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <sstream>
#include <boost/format.hpp>
#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/lexical_cast.hpp>
#include <thread>
#include <set>
#include <vector>

#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "ctlloadresources.h"
#include "../dtwinver/dtwinverex.h"
#include "dtwain_verinfo.h"
#include "dtwain_resource_constants.h"
#include "errorcheck.h"
#include "dtwstrfn.h"
#include "ctlfileutils.h"
#include "ctlthreadutils.h"
#include "arrayfactory.h"
#include "dtwain_library_selector.h"
#include "ctltwainmsgloop.h"
#include "ctldefsource.h"

#ifdef _MSC_VER
    #pragma warning (disable:4702)
    #pragma comment (lib, "shlwapi")
#endif
#ifdef _WIN64
    #pragma message ("Compiling 64-bit DTWAIN")
#else
#ifdef WIN32
    #pragma message ("Compiling 32-bit DTWAIN")
#endif
#endif

#ifdef WIN64
    #ifndef UNICODE
        #define DTWAINSTUB _T("dtwain_stub64")
    #else
        #define DTWAINSTUB _T("dtwainu_stub64")
    #endif
#else
    #ifndef UNICODE
        #define DTWAINSTUB _T("dtwain_stub")
    #else
        #define DTWAINSTUB _T("dtwainu_stub")
    #endif
#endif

using namespace dynarithmic;

static DTWAIN_HANDLE SysInitializeHelper(bool noblock, bool bMinimalSetup = false);
static LONG DTWAIN_CloseAllSources();
static void UnhookAllDisplays();
static bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId);
static bool RemoveThreadIdFromAssociation(unsigned long threadId);
static HWND CreateTwainWindow(CTL_TwainDLLHandle* pHandle,
                              HINSTANCE hInstance= nullptr,
                              HWND hWndParent= nullptr);
static void RegisterTwainWindowClass();
static std::pair<bool, std::vector<uint16_t>> OpenLogging(LPCTSTR pFileName, LONG logFlags, const LoggingTraits& fTraits = {});
static void WriteVersionToLog(CTL_TwainDLLHandle* pHandle);
static bool SysDestroyHelper(const char* pParentFunc, CTL_TwainDLLHandle* pHandle, bool bCheck=true);
static void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL, bool bClear);
static void LoadTransferReadyOverrides();
static void LoadFlatbedOnlyOverrides();
static void LoadOnSourceOpenProperties(CTL_TwainDLLHandle* pHandle);
static bool LoadGeneralResources(bool blockExecution);
static void LoadImageFileOptions(CTL_TwainDLLHandle* pHandle);


#ifdef _WIN32
static UINT_PTR APIENTRY FileSaveAsHookProc(HWND hWnd, UINT msg, WPARAM w, LPARAM lparam);
static WNDPROC SubclassTwainMsgWindow(HWND hWnd, WNDPROC wProcIn = nullptr);
#define     TWSubclassWindow(hwnd, lpfn)  \
            (reinterpret_cast<WNDPROC>(SetWindowLongPtr((hwnd), GWLP_WNDPROC, (LONG_PTR)(WNDPROC)(lpfn))))
#endif

static void LogDTWAINErrorToMsgBox(int nError, LPCSTR pFunc, const std::string& s);

/* Set the paths for image DLL's and language resource */
static DTWAIN_BOOL SetLangResourcePath(LPCTSTR szPath);
static std::string GetStaticLibVer();
static void LoadStaticData(CTL_TwainDLLHandle*);
static bool GetDTWAINDLLVersionInfo(HMODULE hMod, LONG* lMajor, LONG* lMinor, LONG *pPatch);
static CTL_StringType GetDTWAINDLLVersionInfoStr();
static CTL_StringType GetDTWAINInternalBuildNumber();
static DTWAIN_BOOL DTWAIN_GetVersionInternal(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatch);
static std::string CheckSearchOrderString(std::string);

#ifdef DTWAIN_LIB
    static void GetVersionFromResource(LPLONG lMajor, LPLONG lMinor, LPLONG patch);
#endif

static bool FindTask( DWORD hTask );
static HMODULE GetDLLInstance();

static const int numLoggingOptions = 4;

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

DTWAIN_BOOL DTWAIN_GetVersionInternal(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatch)
{
    LOG_FUNC_ENTRY_PARAMS((lMajor, lMinor, lVersionType))
    LONG nDistr = 0;

#ifdef DTWAIN_OPENSOURCE_DISTRIBUTION
#pragma message ("Building Open Source Distribution version")
    nDistr |= DTWAIN_OPENSOURCE_VERSION;
#endif

#ifdef DTWAIN_LIB
    CTL_StringType strVer = _T(DTWAIN_VERINFO_BASEVERSION)
        _T(DTWAIN_PATCHLEVEL_VERSION);
    CTL_StringArrayType aInfo;
    StringWrapper::Tokenize(strVer, _T("."), aInfo);
    *lMajor = _ttol(aInfo[0].c_str());
    *lMinor = _ttol(aInfo[1].c_str());
    if (lPatch)
        *lPatch = _ttol(aInfo[3].c_str());
#endif
#ifdef DTWAIN_LIB
    GetVersionFromResource(lMajor, lMinor, lPatch);
#else
    const bool modRet = GetDTWAINDLLVersionInfo(CTL_StaticData::GetDLLInstanceHandle(), lMajor, lMinor, lPatch);
    if (!modRet)
    {
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
#endif
    *lVersionType = nDistr;
#ifdef UNICODE
    *lVersionType |= DTWAIN_UNICODE_VERSION;
#endif

#ifdef DTWAIN_DEBUG
    *lVersionType |= DTWAIN_DEVELOP_VERSION;
#endif

#if defined (WIN64) || defined(_WIN64)
    *lVersionType |= DTWAIN_64BIT_VERSION;
#else
#if defined (WIN32) || defined(_WIN32)
    *lVersionType |= DTWAIN_32BIT_VERSION;
#endif
#endif

#ifdef DTWAIN_LIB
    #ifndef DTWAIN_STDCALL
        *lVersionType |= DTWAIN_STATICLIB_VERSION;
    #else
        *lVersionType |= DTWAIN_STATICLIB_STDCALL_VERSION;
    #endif
    #ifdef DTWAIN_ACTIVEX
        *lVersionType |= DTWAIN_ACTIVEX_VERSION;
    #endif
#endif

#ifndef DTWAIN_LIMITED_VERSION
    #ifdef PDFLIB_INTERNAL
        #ifdef DTWAIN_LIB
            #ifndef DTWAIN_STDCALL
                *lVersionType |= DTWAIN_PDF_VERSION;
            #else
                *lVersionType |= DTWAIN_PDF_VERSION;
            #endif
        #else
            #ifndef DTWAIN_DEMO_VERSION
                *lVersionType |= DTWAIN_PDF_VERSION;
            #else
                *lVersionType |= DTWAIN_PDF_VERSION;
            #endif
        #endif
    #endif
#endif

#ifndef DTWAIN_LIMITED_VERSION
    #ifdef TWAINSAVE_INTERNAL
        #ifdef DTWAIN_LIB
            #ifndef DTWAIN_STDCALL
                *lVersionType |= DTWAIN_TWAINSAVE_VERSION;
            #else
                *lVersionType |= DTWAIN_TWAINSAVE_VERSION;
            #endif
            #else
            #ifndef DTWAIN_DEMO_VERSION
                *lVersionType |= DTWAIN_TWAINSAVE_VERSION;
            #else
                *lVersionType |= DTWAIN_TWAINSAVE_VERSION;
            #endif
        #endif
    #endif
#endif

#ifdef DTWAIN_DEVELOP_DLL
    *lVersionType |= DTWAIN_DEVELOP_VERSION;
#endif

#if DTWAIN_BUILD_LOGCALLSTACK == 1
    *lVersionType |= DTWAIN_CALLSTACK_LOGGING;
#endif

#if DTWAIN_BUILD_LOGCALLSTACK == 1 && DTWAIN_BUILD_LOGPOINTERS == 1
    *lVersionType |= DTWAIN_CALLSTACK_LOGGING_PLUS;
#endif

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetStaticLibVersion()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
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

void DLLENTRY_DEF DTWAIN_X(LPCTSTR)
{}

//////////////////////////////// Initialize DLL /////////////////////////////
LONG DLLENTRY_DEF DTWAIN_GetAPIHandleStatus(DTWAIN_HANDLE pHandle)
{
    LOG_FUNC_ENTRY_PARAMS((pHandle))
    LONG retVal = 0;
    if (!IsDLLHandleValid(static_cast<CTL_TwainDLLHandle*>(pHandle), FALSE))
        LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    retVal = IsDLLHandleValid(static_cast<CTL_TwainDLLHandle*>(pHandle), TRUE) ? DTWAIN_TWAINSESSIONOK : DTWAIN_APIHANDLEOK;
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTwainMsg(MSG *pMsg)
{
    LOG_FUNC_ENTRY_PARAMS_ISTWAINMSG((pMsg))
    if (!TwainMessageLoopV2::s_MessageQueue.empty())
    {
        MSG msg = TwainMessageLoopV2::s_MessageQueue.front();
        TwainMessageLoopV2::s_MessageQueue.pop();
        CTL_TwainAppMgr::IsTwainMsg(&msg, true);  // make sure we perform what we need to do for TWAIN 2.x.
    }
    // make sure we perform any default message handling here.
    const DTWAIN_BOOL bRet = CTL_TwainAppMgr::IsTwainMsg( pMsg, false );
    LOG_FUNC_EXIT_PARAMS_ISTWAINMSG(bRet)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetLastError()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
    if ( !pHandle )
    {
        LONG err = DTWAIN_ERR_BAD_HANDLE;
        if (!CTL_StaticData::ResourcesLoaded())
            err = CTL_StaticData::GetResourceLoadError();
        LOG_FUNC_EXIT_NONAME_PARAMS(err)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_lLastError)
    CATCH_BLOCK(DTWAIN_ERR_BAD_HANDLE)
}

static LONG GetResourceStringInternal(LONG resourceID, LPTSTR lpszBuffer, LONG nMaxLen)
{
    auto actualResourceID = std::abs(resourceID);
    CTL_StringType sCopy;
    size_t nBytes = GetResourceStringA(static_cast<UINT>(actualResourceID), nullptr, DTWAIN_USERRES_MAXSIZE);
    if (nBytes == 0)
    {
        // Copy the error number to the buffer if we haven't been able to find the 
        // resource string
        sCopy = StringWrapper::ToString(resourceID);
        if (resourceID != DTWAIN_ERR_WIN32_ERROR)
            return StringWrapper::CopyInfoToCString(sCopy, lpszBuffer, nMaxLen);
    }
    nBytes = DTWAIN_USERRES_MAXSIZE;
    resourceID = actualResourceID;

    size_t nAdditionalBytes = 0;
    auto& extraInfoMap = CTL_StaticData::GetExtraErrorInfoMap();
    auto iter = extraInfoMap.find(resourceID);
    if (iter != extraInfoMap.end())
        nAdditionalBytes += iter->second.size();

    std::vector<char> szTemp(nBytes, 0);
    GetResourceStringA(static_cast<UINT>(resourceID), szTemp.data(), static_cast<LONG>(nBytes));
    if (nAdditionalBytes > 0)
    {
        while (!szTemp.empty() && szTemp.back() == 0)
            szTemp.pop_back();
        szTemp.push_back(' ');
        szTemp.push_back('-');
        szTemp.push_back(' ');
        szTemp.insert(szTemp.end(), iter->second.begin(), iter->second.end());
        szTemp.push_back(0);
    }
    sCopy += StringConversion::Convert_Ansi_To_Native(szTemp.data());
    return StringWrapper::CopyInfoToCString(sCopy, lpszBuffer, nMaxLen);
}

LONG DLLENTRY_DEF  DTWAIN_GetResourceString(LONG ResourceID, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((ResourceID, lpszBuffer, nMaxLen))
    auto nTotalBytes = GetResourceStringInternal(ResourceID, lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
    LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF  DTWAIN_GetErrorString(LONG lError, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((lError, lpszBuffer, nMaxLen))
    auto nTotalBytes = GetResourceStringInternal(lError, lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
    LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_SetLastError(LONG nError)
{
    LOG_FUNC_ENTRY_PARAMS((nError))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);
    if (!pHandle)
    {
        LONG err = DTWAIN_ERR_BAD_HANDLE;
        if (!CTL_StaticData::ResourcesLoaded())
            err = DTWAIN_ERR_RESOURCES_NOT_FOUND;
        LOG_FUNC_EXIT_NONAME_PARAMS(err)
    }
    pHandle->m_lLastError = nError;
    LOG_FUNC_EXIT_NONAME_PARAMS(DTWAIN_NO_ERROR)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetConditionCodeString(LONG CC, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((CC, lpszBuffer, nMaxLen))
    const LONG lError = -(IDS_TWCC_ERRORSTART + CC);
    const LONG Retval = DTWAIN_GetErrorString(lError, lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
    LOG_FUNC_EXIT_NONAME_PARAMS(Retval)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadLanguageResource(LONG nLanguage)
{
    LOG_FUNC_ENTRY_PARAMS((nLanguage))
    LPCTSTR pLangDLL;
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    switch(nLanguage)
    {
        case DTWAIN_RES_ENGLISH:
            pLangDLL = _T("english");
            break;
        case DTWAIN_RES_FRENCH:
            pLangDLL = _T("french");
            break;
        case DTWAIN_RES_SPANISH:
            pLangDLL = _T("spanish");
            break;
        case DTWAIN_RES_GERMAN:
            pLangDLL = _T("german");
            break;
        case DTWAIN_RES_DUTCH:
            pLangDLL = _T("dutch");
            break;
        case DTWAIN_RES_ITALIAN:
            pLangDLL = _T("italian");
            break;
        default:
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }

    // Add the resource to the registry.
    const DTWAIN_BOOL bRet = DTWAIN_LoadCustomStringResources(pLangDLL);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

static bool GenericResourceLoader(CTL_TwainDLLHandle* pHandle, LPCTSTR sLangDLL, bool bClear)
{
    const std::string sLangDLLString = StringConversion::Convert_NativePtr_To_Ansi(sLangDLL);
    // Add the resource to the registry.
    const auto exists = pHandle->AddResourceToRegistry(sLangDLLString.c_str(), bClear).second;
    bool bRet = false;
    if (exists)
        bRet = LoadLanguageResourceA(sLangDLLString.c_str(), pHandle->GetResourceRegistry(), bClear);
    return bRet;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResources(LPCTSTR sLangDLL)
{
    LOG_FUNC_ENTRY_PARAMS((sLangDLL))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    bool bRet = GenericResourceLoader(pHandle, sLangDLL, false);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !bRet;}, DTWAIN_ERR_FILEOPEN, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResourcesEx(LPCTSTR sLangDLL, DTWAIN_BOOL bClear)
{
    LOG_FUNC_ENTRY_PARAMS((sLangDLL, bClear))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    bool bRet = GenericResourceLoader(pHandle, sLangDLL, bClear);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] {return !bRet; }, DTWAIN_ERR_FILEOPEN, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstant(LONG lConstantType, LONG lTwainConstant, LPTSTR lpszOut, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((lConstantType, lTwainConstant, lpszOut, nSize))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto ret = CTL_StaticData::GetTwainNameFromConstant(lConstantType, lTwainConstant);
    auto numChars = StringWrapper::CopyInfoToCString(ret, lpszOut, nSize);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszOut))
    LOG_FUNC_EXIT_NONAME_PARAMS(numChars)
    CATCH_BLOCK(-1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsInitialized()
{
    // Get the Current task
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    const DWORD hTask = getThreadId();

#ifdef DTWAIN_LIB
    if ( CTL_StaticData::s_DLLInstance == NULL )
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    if ( !CTL_StaticData::s_DLLHandles.empty() )
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
#else
    // Check if this task has already been hooked
    if (FindTask( hTask ) )
        // Already hooked.  No need to do this again
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
#endif
    CATCH_BLOCK(false)
}

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
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
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

DTWAIN_HANDLE dynarithmic::GetDTWAINHandle_Internal()
{
    auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
    auto iter = threadMap.find(getThreadId());
    if (iter != threadMap.end())
        return iter->second.get();
    return nullptr;
}


DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_GetDTWAINHandle()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    LOG_FUNC_EXIT_NONAME_PARAMS(::GetDTWAINHandle_Internal())
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}


DTWAIN_BOOL SetLangResourcePath(LPCTSTR szPath)
{
    LOG_FUNC_ENTRY_PARAMS((szPath))
    CTL_StaticData::GetLanguageResourcePath() = StringWrapper::AddBackslashToDirectory(szPath);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

// Determines if any TWAIN subsystem is available (TWAIN version 1.x and 2.x)
static std::pair<LONG, std::array<CTL_StringType, 2>> GetTwainAvailablityInternal()
{
    std::pair<LONG, std::array<CTL_StringType, 2>> retVal;
    LONG availableFlag = 0;

    // Check TWAIN version 1.x
    auto existCheck = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_1);
    bool bRet = existCheck.first;
    if (bRet)
    {
        availableFlag |= DTWAIN_TWAINDSM_LEGACY;
        // Store the path found
        retVal.second[0] = existCheck.second;
    }

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

static LONG IsTwainAvailableHelper(LPTSTR directories, LONG nMaxLen)
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
            pHandle = static_cast<CTL_TwainDLLHandle*> (SysInitializeHelper(false, true));
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

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTwainAvailable()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto retVal = IsTwainAvailableHelper(nullptr, 0);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal > 0 ? true : false)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_IsTwainAvailableEx(LPTSTR directories, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto retVal = IsTwainAvailableHelper(directories, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((directories))
    LOG_FUNC_EXIT_NONAME_PARAMS((std::max)(retVal, 0L))
    CATCH_BLOCK(0)
}

// Test which version of TWAIN is available.
LONG DLLENTRY_DEF DTWAIN_GetTwainAvailability()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto availability = GetTwainAvailablityInternal();
    LOG_FUNC_EXIT_NONAME_PARAMS(availability.first)
    CATCH_BLOCK(0)
}

// Test which version of TWAIN is available.
LONG DLLENTRY_DEF DTWAIN_GetTwainAvailabilityEx(LPTSTR directories, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto availability = GetTwainAvailablityInternal();

    // If not available set the error and exit
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return availability.first == 0; },
                                        DTWAIN_ERR_TWAIN_NOT_INSTALLED, 0, FUNC_MACRO);

    // Provide "<null>" for either TWAIN 1 or TWAIN 2 directories in the
    // returned array of directories if TWAIN could not be found
    for (auto& s : availability.second)
    {
        if (s.empty())
            s = _T("<null>");
    }


    CTL_StringType sDirs;
    auto joinedString = StringWrapper::Join(availability.second, _T("|"));
    StringWrapper::CopyInfoToCString(joinedString, directories, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((directories))
    LOG_FUNC_EXIT_NONAME_PARAMS(static_cast<LONG>(joinedString.length()))
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetDSMFullName(LONG DSMType, LPTSTR szDLLName, LONG nMaxLen, LPLONG pWhichSearch)
{
    LOG_FUNC_ENTRY_PARAMS((DSMType, szDLLName, nMaxLen, pWhichSearch))
    CTL_StringType sName;
    if ( DSMType == DTWAIN_TWAINDSM_LEGACY )
        sName = TWAINDLLVERSION_1;
    else
        sName = TWAINDLLVERSION_2;

    CTL_StringType sPath;
    CTL_StringType* strToSet = &sPath;
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE | DTWAIN_TEST_NOTHROW);

    if (pHandle)
    {
        if (DSMType == DTWAIN_TWAINDSM_LEGACY)
            strToSet = &pHandle->m_strTWAINPath;
        else
            strToSet = &pHandle->m_strTWAINPath2;
    }
    if ( strToSet->empty() )
        *strToSet = CTL_TwainAppMgr::GetTwainDirFullName(sName.c_str(), pWhichSearch);

    const size_t nBytes = strToSet->length();
    if ( nBytes == 0)
        LOG_FUNC_EXIT_NONAME_PARAMS(0)
    LONG nTotalBytes;
    if ( !szDLLName )
    {
        nTotalBytes = static_cast<LONG>(strToSet->length() + 1);
        LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
    }

    CTL_StringType sDLLName;
    std::copy(strToSet->begin(), strToSet->end(), std::back_inserter(sDLLName));
    nTotalBytes = StringWrapper::CopyInfoToCString(sDLLName, szDLLName, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szDLLName, pWhichSearch))
    LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPath(LPTSTR szDLLName, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((szDLLName, nMaxLen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto retVal = StringWrapper::CopyInfoToCString(CTL_TwainAppMgr::GetDSMPath(), szDLLName, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szDLLName))
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetActiveDSMVersionInfo(LPTSTR szDLLInfo, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((szDLLInfo, nMaxLen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto retVal = StringWrapper::CopyInfoToCString(CTL_TwainAppMgr::GetDSMVersionInfo(), szDLLInfo, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szDLLInfo))
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(-1)
}

DTWAIN_HANDLE DLLENTRY_DEF  DTWAIN_SysInitializeLib(HINSTANCE hInstance)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance))

#ifdef DTWAIN_LIB
    CTL_StaticData::s_DLLInstance = hInstance;
#endif
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
    LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

DTWAIN_HANDLE DLLENTRY_DEF  DTWAIN_SysInitializeLibEx2(HINSTANCE hInstance,
                                                       LPCTSTR szINIPath,
                                                       LPCTSTR szImageDLLPath,
                                                       LPCTSTR szLangResourcePath)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath, szImageDLLPath,szLangResourcePath))

    SetLangResourcePath(szLangResourcePath);

    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLibEx(hInstance, szINIPath);
    LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

/////////////////////////////////////////////////////////////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibEx(HINSTANCE hInstance, LPCTSTR szINIPath)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath))

    CTL_StaticData::GetINIPath() = StringWrapper::AddBackslashToDirectory(szINIPath);

    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLib(hInstance);
    LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}
/////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////// Initialize Library EX2 code //////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx2(LPCTSTR szINIPath,
                                                   LPCTSTR szImageDLLPath,
                                                   LPCTSTR szLangResourcePath)
{
    LOG_FUNC_ENTRY_PARAMS((szINIPath, szImageDLLPath, szLangResourcePath))

    SetLangResourcePath(szLangResourcePath);
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeEx(szINIPath);
    LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}
/////////////////////////////////////////////////////////////////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx(LPCTSTR szINIPath)
{
    LOG_FUNC_ENTRY_PARAMS((szINIPath))
    CTL_StaticData::GetINIPath() = StringWrapper::AddBackslashToDirectory(szINIPath);
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
    LOG_FUNC_EXIT_NONAME_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeNoBlocking()
{
    return SysInitializeHelper(false);
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitialize()
{
    return SysInitializeHelper(true);
}

DTWAIN_HANDLE SysInitializeHelper(bool block, bool bMinimalSetup)
{
    std::lock_guard<std::mutex> lg(CTL_StaticData::s_mutexInitDestroy);
#ifdef DTWAIN_LIB
    if ( CTL_StaticData::s_DLLInstance == NULL )
    {
        // Get the instance handle of the application
        TCHAR szName[1024];
        ::GetModuleFileName(NULL, szName, 1023);

        CTL_StaticData::s_DLLInstance = ::GetModuleHandle( szName );
    }
    CTL_StaticData::GetLogFilterFlags() = 0;
    CTL_StaticData::s_nRegisteredDTWAINMsg = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
#endif
    LOG_FUNC_ENTRY_NONAME_PARAMS()

    CTL_TwainDLLHandlePtr pHandlePtr;
    // Return handle if already registered
    const DTWAIN_HANDLE DLLHandle = GetDTWAINHandle_Internal();
    if (DLLHandle)
        LOG_FUNC_EXIT_NONAME_PARAMS(DLLHandle)

    // This must be checked if this is used in a static library
    #ifdef DTWAIN_LIB
    if ( CTL_StaticData::s_DLLInstance == NULL )
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    #endif

    // Load resources only if first time
    // Return a new DTWAIN_HANDLE
    try
    {
        pHandlePtr = std::make_shared<CTL_TwainDLLHandle>();
        auto threadId = getThreadId();
        AssociateThreadToTwainDLL(pHandlePtr, threadId);
        CTL_TwainDLLHandle* pHandle = pHandlePtr.get();
        if (!bMinimalSetup)
        {
            // Open dtwain32.ini or dtwain64.ini
            if ( !CTL_StaticData::s_iniInterface )
                CTL_StaticData::s_iniInterface = std::make_unique<CSimpleIniA>();
            auto* ptrIni = CTL_StaticData::GetINIInterface();
            if (!CTL_StaticData::IsINIFileLoaded())
            {
                auto err = ptrIni->LoadFile(dynarithmic::GetDTWAININIPathA().c_str());
                CTL_StaticData::SetINIFileLoaded(err == SI_OK);
                CTL_StaticData::GetINIPath() = GetDTWAININIPath();
            }

            bool resourcesLoaded = LoadGeneralResources(block);
            if (!resourcesLoaded)
            {
                RemoveThreadIdFromAssociation(threadId);
                LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
            }
#ifdef _WIN32
            pHandle->m_pSaveAsDlgProc = FileSaveAsHookProc;
            RegisterTwainWindowClass();
#endif

            LoadStaticData(pHandle);
            auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
            if (!threadMap.empty())
            {
                const CTL_StringType szLangDLL = _T("english");

                // Initialize the enumerator factory
                pHandlePtr->m_ArrayFactory = std::make_shared<CTL_ArrayFactory>();

                // Initialize the resource registry
                pHandle->InitializeResourceRegistry();

                // Load customized resources from INI
                LoadCustomResourcesFromIni(pHandle, szLangDLL.c_str(), true);

                // Load DS overrides for transfer ready / close UI requests
                LoadTransferReadyOverrides();

                // Load flatbed only list of devices
                LoadFlatbedOnlyOverrides();

                // Load check feeder on open status
                LoadOnSourceOpenProperties(pHandle);

                // Load image file related options
                LoadImageFileOptions(pHandle);

                // Initialize imaging code
                FreeImage_Initialise(true);

                WriteVersionToLog(pHandle);
                pHandle->SetVersionString(GetVersionString());
            }
            LOG_FUNC_ENTRY_NONAME_PARAMS()
            LOG_FUNC_EXIT_NONAME_PARAMS(static_cast<DTWAIN_HANDLE>(pHandle))
            CATCH_BLOCK(DTWAIN_HANDLE(0))
        }
        else
        {
            LOG_FUNC_ENTRY_NONAME_PARAMS()
            LOG_FUNC_EXIT_NONAME_PARAMS(static_cast<DTWAIN_HANDLE>(pHandle))
            CATCH_BLOCK(DTWAIN_HANDLE(0))
        }
        CATCH_BLOCK(DTWAIN_HANDLE(0))
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }
    catch (std::exception& ex)
    {
        if (block)
        {
            MessageBoxA(nullptr, "DTWAIN Initialization Error", ex.what(), MB_ICONERROR);
        }
        else
        {
            LogToDebugMonitorA("DTWAIN Initialization Error\n");
            LogToDebugMonitorA(ex.what());
        }
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }
    catch (...)
    {
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    }
}


void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL, bool bClear)
{
    // Load the resources
    auto* customProfile = CTL_StaticData::GetINIInterface();
    if ( !customProfile )
        return;

    std::string szStr = customProfile->GetValue("Language", "default",
                                               StringConversion::Convert_NativePtr_To_Ansi(szLangDLL).c_str());
    if (!LoadLanguageResourceA(szStr, pHandle->GetResourceRegistry(), bClear))
    {
        // Use the English resources by default
        if (!LoadLanguageResourceA("english", pHandle->GetResourceRegistry(), bClear))
        {
            // Too bad.  Last chance -- load english resources directly from internal rc.
            // Note that unlike the text resources that should have been loaded, 
            // these internal resources should not be able to be edited without
            // corrupting the DLL in terms of checking the hash value of the DLL file.
            LoadLanguageResourceFromRC();
        }
    }
    // Load error flags if defined
    struct ProfileSettingsInt
    {
        LPCSTR section;
        LPCSTR name;
        long orValue;
    };
    static ProfileSettingsInt allIntProfiles[] = {
        { "DSMErrorLogging", "Decode_Identity1", DTWAIN_LOG_DECODE_SOURCE },
        { "DSMErrorLogging", "Decode_Identity2", DTWAIN_LOG_DECODE_DEST },
        { "DSMErrorLogging", "Decode_Data", DTWAIN_LOG_DECODE_TWMEMREF },
        { "DSMErrorLogging", "Decode_Events", DTWAIN_LOG_DECODE_TWEVENT },
        { "DSMErrorLogging", "CallStack", DTWAIN_LOG_CALLSTACK },
        { "DSMErrorLogging", "ShowExceptions", DTWAIN_LOG_SHOWEXCEPTIONS },
        { "DSMErrorLogging", "DisplayLibErrors", DTWAIN_LOG_ERRORMSGBOX },
        { "DSMErrorLogging", "DisplayInitFailErrors", DTWAIN_LOG_INITFAILURE },
        { "DSMErrorLogging", "BufferErrors", DTWAIN_LOG_USEBUFFER } };

    auto& logFilterFlags = CTL_StaticData::GetLogFilterFlags();
    std::for_each(allIntProfiles, std::end(allIntProfiles), [&](const ProfileSettingsInt& ps)
    {
        const auto nVal = customProfile->GetLongValue(ps.section, ps.name, 0);
        if (nVal != 0)
            logFilterFlags |= ps.orValue;
    });

    auto nVal = customProfile->GetLongValue("DSMErrorLogging", "EnableNone", 0);
    if (nVal == 1)
        logFilterFlags = 0;

    nVal = customProfile->GetLongValue("DSMErrorLogging", "EnableAll", 0);
    if (nVal != 0)
        logFilterFlags = 0xFFFFFFFFL & ~DTWAIN_LOG_USEFILE;

    szStr = customProfile->GetValue("DSMErrorLogging", "File", "");
    if (!szStr.empty())
    {
        logFilterFlags |= DTWAIN_LOG_USEFILE;
        OpenLogging(StringConversion::Convert_Ansi_To_Native(szStr).c_str(), logFilterFlags);
        CTL_StaticData::GetLogger().StatusOutFast("In DTWAIN_SysInitialize()");
    }

    nVal = customProfile->GetLongValue("DSMErrorLogging", "BufferErrorThreshold", 50);
    if (logFilterFlags & DTWAIN_LOG_USEBUFFER)
        DTWAIN_SetErrorBufferThreshold(nVal);

    nVal = customProfile->GetLongValue("DSMErrorLogging", "AppHandlesExceptions", 0);
    logFilterFlags = nVal == 0 ? false : true;
}

void LoadStaticData(CTL_TwainDLLHandle* pHandle)
{
    if ( pHandle->m_mapDTWAINArrayToTwainType.empty() )
    {
        static constexpr int TwainIntTypes[] = {TWTY_INT8, TWTY_UINT8, TWTY_BOOL, TWTY_INT16, TWTY_INT32, TWTY_UINT16, TWTY_UINT32};
        static constexpr int TwainStringTypes[] = {TWTY_STR32, TWTY_STR64, TWTY_STR128, TWTY_STR128, TWTY_STR255, TWTY_STR1024};
        static constexpr int TwainFloatTypes[] = {TWTY_FIX32};
        static constexpr int TwainFrameTypes[] = {TWTY_FRAME};
        static constexpr int DTwainArrayTypes[] = {DTWAIN_ARRAYLONG, DTWAIN_ARRAYANSISTRING, DTWAIN_ARRAYFLOAT, DTWAIN_ARRAYFRAME};
        static constexpr size_t NumTwainTypes[] = {std::size(TwainIntTypes),
                                         std::size(TwainStringTypes),
                                         std::size(TwainFloatTypes),
                                         std::size(TwainFrameTypes)};
        static constexpr std::array<const int*, 4> intArray = {TwainIntTypes, TwainStringTypes, TwainFloatTypes, TwainFrameTypes};

        static constexpr size_t nNumAllTypes = std::size(NumTwainTypes);
        for (size_t i = 0; i < nNumAllTypes; ++i)
        {
            std::vector<LONG> TwainType(intArray[i], intArray[i] + NumTwainTypes[i]);
            pHandle->m_mapDTWAINArrayToTwainType.insert(make_pair(DTwainArrayTypes[i], TwainType));
        }
    }
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLog(LONG LogFlags, LPCTSTR lpszLogFile)
{
    LOG_FUNC_ENTRY_PARAMS((LogFlags, lpszLogFile))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    CTL_StaticData::GetLogger().SetDLLHandle(pHandle);

    // If the log flags have not specified what to log
    // then log callstack and general TWAIN send/receive info.
    LONG allFlags = DTWAIN_LOG_ALL;
    if ( (LogFlags != 0) && (LogFlags & allFlags) == 0)  
        LogFlags |= (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_DECODE_SOURCE | DTWAIN_LOG_DECODE_DEST | DTWAIN_LOG_MISCELLANEOUS);

    bool bLoggerExists = AnyLoggerExists(pHandle);
    auto& logFilterFlags = CTL_StaticData::GetLogFilterFlags();
    if (LogFlags == 0 && bLoggerExists)
    {
        CTL_StaticData::GetLogger().PrintBanner(false);
        CTL_StaticData::GetLogger().DisableAllLoggers();
        logFilterFlags = LogFlags;
    }
    else
    {
        logFilterFlags = LogFlags;
        if (LogFlags && !UserDefinedLoggerExists(pHandle))
            logFilterFlags &= ~DTWAIN_LOG_USECALLBACK;

        LoggingTraits fTraits;
        fTraits.m_bAppend = LogFlags & DTWAIN_LOG_FILEAPPEND?true:false;
        fTraits.m_bCreateDirectory = LogFlags & DTWAIN_LOG_CREATEDIRECTORY ? true : false;
        fTraits.m_filename = lpszLogFile;
        fTraits.m_bSetConsoleHandler = LogFlags & DTWAIN_LOG_CONSOLEWITHHANDLER ? true : false;
        auto isLogOpen = OpenLogging(lpszLogFile, LogFlags, fTraits);

        // Write the version info
        // Write to all the loggers that were created
        if ( LogFlags > 0)
            WriteVersionToLog(pHandle);
        if (LogFlags > 0 && !isLogOpen.first)
        {
            // Indicate that there is at least one logger that failed
            DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_LOG_CREATE_ERROR, false, FUNC_MACRO);
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

bool dynarithmic::UserDefinedLoggerExists(CTL_TwainDLLHandle* pHandle)
{
    return pHandle->m_LoggerCallbackInfo.m_pLoggerCallback || 
           pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA || 
           pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW;
}

bool dynarithmic::AnyLoggerExists(CTL_TwainDLLHandle* pHandle)
{
    return UserDefinedLoggerExists(pHandle) || CTL_StaticData::GetLogFilterFlags() != 0;
}

void dynarithmic::WriteUserDefinedLogMsg(CTL_TwainDLLHandle* pHandle, LPCTSTR sz)
{
    if (!pHandle)
        return;
    if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallback)
        pHandle->m_LoggerCallbackInfo.m_pLoggerCallback(sz, pHandle->m_LoggerCallbackInfo.m_pLoggerCallback_UserData);
    if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA)
#ifdef _UNICODE
        WriteUserDefinedLogMsgA(pHandle, StringConversion::Convert_Native_To_Ansi(sz).c_str());
#else
        WriteUserDefinedLogMsgA(pHandle, sz);
#endif
    if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW)
#ifdef _UNICODE
        WriteUserDefinedLogMsgW(pHandle, sz);
#else
        WriteUserDefinedLogMsgW(pHandle, StringConversion::Convert_NativePtr_To_Wide(sz).c_str());
#endif
}

void dynarithmic::WriteUserDefinedLogMsgA(CTL_TwainDLLHandle* pHandle, LPCSTR sz)
{
    if (!pHandle)
        return;
    auto& loggerRef = pHandle->m_LoggerCallbackInfo;
    if (loggerRef.m_pLoggerCallbackA)
        loggerRef.m_pLoggerCallbackA(sz, loggerRef.m_pLoggerCallback_UserDataA);

    if (loggerRef.m_pLoggerCallback)
#ifdef _UNICODE
        loggerRef.m_pLoggerCallback(StringConversion::Convert_Ansi_To_Native(sz).c_str(),
                                    loggerRef.m_pLoggerCallback_UserData);
#else
        loggerRef.m_pLoggerCallback(sz, loggerRef.m_pLoggerCallback_UserData);
#endif
    if (loggerRef.m_pLoggerCallbackW)
#ifdef _UNICODE
        loggerRef.m_pLoggerCallbackW(StringConversion::Convert_Ansi_To_Native(sz).c_str(),
                                    loggerRef.m_pLoggerCallback_UserDataW);
#else
        loggerRef.m_pLoggerCallbackW(StringConversion::Convert_Native_To_Wide(sz).c_str(),
                                     loggerRef.m_pLoggerCallback_UserDataW);
#endif
}

void dynarithmic::WriteUserDefinedLogMsgW(CTL_TwainDLLHandle* pHandle, LPCWSTR sz)
{
    if (!pHandle)
        return;
    auto& loggerRef = pHandle->m_LoggerCallbackInfo;
    if (loggerRef.m_pLoggerCallbackW)
        loggerRef.m_pLoggerCallbackW(sz, loggerRef.m_pLoggerCallback_UserDataW);

    if (loggerRef.m_pLoggerCallback)
#ifdef _UNICODE
        loggerRef.m_pLoggerCallback(sz, loggerRef.m_pLoggerCallback_UserData);
#else
        loggerRef.m_pLoggerCallback(StringConversion::Convert_Wide_To_Native(sz).c_str(),
                                    loggerRef.m_pLoggerCallback_UserData);
#endif
    if (loggerRef.m_pLoggerCallbackA)
#ifdef _UNICODE
        loggerRef.m_pLoggerCallbackA(StringConversion::Convert_Native_To_Ansi(sz).c_str(),
                                     loggerRef.m_pLoggerCallback_UserDataA);
#else
        loggerRef.m_pLoggerCallbackA(StringConversion::Convert_Wide_To_Native(sz).c_str(),
                                     loggerRef.m_pLoggerCallback_UserDataA);
#endif
}

std::pair<bool, std::vector<uint16_t>> OpenLogging(LPCTSTR pFileName, LONG logFlags, const LoggingTraits& lTraits)
{
    uint16_t nWhichLogging = 0;
    uint16_t totalLoggingOptions = 0;
    std::vector<uint16_t> vBadLogs;
    if (pFileName && pFileName[0])
    {
        bool bLogOpen = false;
        bLogOpen = CTL_StaticData::GetLogger().InitFileLogging(pFileName, CTL_StaticData::GetDLLInstanceHandle(), lTraits);
        if (!bLogOpen)
            vBadLogs.push_back(nWhichLogging);
        ++totalLoggingOptions;
    }
    ++nWhichLogging;
    std::array<std::function<bool(HINSTANCE, const LoggingTraits&)>, 3> vLoggingFuncs = {
                            [&](HINSTANCE hinst, const LoggingTraits& theTraits) { return CTL_StaticData::GetLogger().InitConsoleLogging(hinst, theTraits); },
                            [&](HINSTANCE hinst, const LoggingTraits&) { return CTL_StaticData::GetLogger().InitDebugWindowLogging(hinst); },
                            [&](HINSTANCE hinst, const LoggingTraits&) { return CTL_StaticData::GetLogger().InitCallbackLogging(hinst); }};
    static constexpr std::array<long, 4> aLogFlags = { 0, DTWAIN_LOG_CONSOLE, DTWAIN_LOG_DEBUGMONITOR, DTWAIN_LOG_USECALLBACK };
    for (auto& fn : vLoggingFuncs)
    {
        if (logFlags & aLogFlags[nWhichLogging])
        {
            bool bRet = true;
            ++totalLoggingOptions;
            bRet = fn(CTL_StaticData::GetDLLInstanceHandle(), lTraits);
            if (!bRet)
                vBadLogs.push_back(nWhichLogging);
        }
        ++nWhichLogging;
    }
    bool bAnyLogsOpen = vBadLogs.size() < totalLoggingOptions;
    if (bAnyLogsOpen && logFlags)
    {
        CTL_StaticData::GetLogger().PrintTime(true);
        CTL_StaticData::GetLogger().PrintAppName(true);
        CTL_StaticData::GetLogger().PrintBanner();
    }
    return { vBadLogs.empty(), vBadLogs };
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCountry(LONG nCountry)
{
    LOG_FUNC_ENTRY_PARAMS((nCountry))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    pHandle->m_SessionStruct.nCountry = static_cast<TW_UINT16>(nCountry);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLanguage(LONG nLanguage)
{
    LOG_FUNC_ENTRY_PARAMS((nLanguage))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    pHandle->m_SessionStruct.nLanguage = static_cast<TW_UINT16>(nLanguage);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetCountry()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_SessionStruct.nCountry)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_GetLanguage()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_SessionStruct.nLanguage)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfo(LPCTSTR szVerStr, LPCTSTR szManu, LPCTSTR szProdFam, LPCTSTR szProdName)
{
    LOG_FUNC_ENTRY_PARAMS((szVerStr, szManu, szProdFam, szProdName))
    // See if DLL Handle exists
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if ( szVerStr )
        pHandle->m_SessionStruct.szVersion = szVerStr;
    if ( szManu )
        pHandle->m_SessionStruct.szManufact = szManu;
    if ( szProdFam )
        pHandle->m_SessionStruct.szFamily = szProdFam;
    if ( szProdName )
        pHandle->m_SessionStruct.szProduct = szProdName;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAppInfo(LPTSTR szVerStr, LPTSTR szManu, LPTSTR szProdFam, LPTSTR szProdName)
{
    if (szVerStr)
        szVerStr[0] = 0;

    if (szManu)
        szManu[0] = 0;

    if (szProdFam)
        szProdFam[0] = 0;

    if (szProdName)
        szProdName[0] = 0;

    LOG_FUNC_ENTRY_PARAMS((szVerStr, szManu, szProdFam, szProdName))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if ( szVerStr )
        std::copy(pHandle->m_SessionStruct.szVersion.begin(), pHandle->m_SessionStruct.szVersion.end(), szVerStr);
    if ( szManu )
        std::copy(pHandle->m_SessionStruct.szManufact.begin(), pHandle->m_SessionStruct.szManufact.end(), szManu);
    if ( szProdFam )
        std::copy(pHandle->m_SessionStruct.szFamily.begin(), pHandle->m_SessionStruct.szFamily.end(), szProdFam);
    if ( szProdName )
        std::copy(pHandle->m_SessionStruct.szProduct.begin(), pHandle->m_SessionStruct.szProduct.end(), szProdName);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((szVerStr, szManu, szProdFam, szProdName))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DTWAIN_SetSourceCloseMode(LONG lCloseMode)
{
    LOG_FUNC_ENTRY_PARAMS((lCloseMode))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    pHandle->m_nSourceCloseMode = lCloseMode?true:false;
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(FALSE)
}

LONG DTWAIN_GetSourceCloseMode()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_nSourceCloseMode)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSessionEnabled()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if ( pHandle->m_bSessionAllocated )
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSession(HWND hWndMsgNotify, LPCTSTR lpszDLLName)
{
    LOG_FUNC_ENTRY_PARAMS((hWndMsgNotify, lpszDLLName))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if ( pHandle->m_bSessionAllocated )
        LOG_FUNC_EXIT_NONAME_PARAMS(true)

    const CTL_StringType sDLLName = lpszDLLName?lpszDLLName:StringWrapper::traits_type::GetEmptyString();
#ifdef _WIN32
    HWND hWndMsg;
    HINSTANCE hInstance;

    // Create a proxy if no window handle is given
    if ( !hWndMsgNotify )
    {
        // Create the window
        hWndMsg = CreateTwainWindow(pHandle,nullptr,hWndMsgNotify);

        // This is the window's instance handle
        hInstance = CTL_StaticData::GetDLLInstanceHandle();

        pHandle->m_bUseProxy = true;
    }
    else
    {
        // Set the TWAIN window to the user's window
        hWndMsg = hWndMsgNotify;

        // Record that we need to subclass this guy
        pHandle->m_bUseProxy = false;

        // Get the instance handle of the user's window
        #ifdef DTWAIN_LIB
        hInstance = CTL_StaticData::s_DLLInstance;
        #else
        hInstance = reinterpret_cast<HINSTANCE>(GetWindowLongPtr(hWndMsg, GWLP_HINSTANCE));
        #endif
    }
    // Error if the window does not exist
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{ return !hWndMsg;}, DTWAIN_ERR_NULL_WINDOW, false, FUNC_MACRO);
    // Set the callback window
    pHandle->m_hWndTwain = hWndMsg;
#else
    HINSTANCE hInstance;
    // This is the window's instance handle
    hInstance = CTL_StaticData::s_DLLInstance;
    HWND hWndMsg;
#endif

#ifdef _UNICODE
    const CTL_StringType szName = boost::dll::symbol_location(DTWAIN_DLLNAME).wstring();
#else
    CTL_StringType szName = boost::dll::symbol_location(DTWAIN_DLLNAME).string();
#endif

    // See if we need to create a TWAIN application manager
    if ( !CTL_TwainAppMgr::GetInstance() )
    {
        // Create it with the parameters shown
        if ( !CTL_TwainAppMgr::Create(pHandle,
                                      hInstance,
                                      CTL_StaticData::GetDLLInstanceHandle(),
                                      lpszDLLName?sDLLName.c_str():nullptr) )
        {
            if ( pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_LATESTVERSION ||
                 pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_VERSION2 )
               DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1;}, DTWAIN_ERR_TWAINOPENSOURCEDSMNOTFOUND, false, FUNC_MACRO);
            else
               DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_TWAIN32DSMNOTFOUND, false, FUNC_MACRO);
        }
    }

    // Create a session
    auto Session =
        CTL_TwainAppMgr::CreateTwainSession( pHandle, szName.c_str(), &hWndMsg,
                                             pHandle->m_SessionStruct.nMajorNum,
                                             pHandle->m_SessionStruct.nMinorNum,
                                             static_cast<CTL_TwainLanguageEnum>(pHandle->m_SessionStruct.nLanguage),
                                             static_cast<CTL_TwainCountryEnum>(pHandle->m_SessionStruct.nCountry),
                                             pHandle->m_SessionStruct.szVersion.c_str(),
                                             pHandle->m_SessionStruct.szManufact.c_str(),
                                             pHandle->m_SessionStruct.szFamily.c_str(),
                                             pHandle->m_SessionStruct.szProduct.c_str());

    if ( Session == nullptr)
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, []{return 1;}, DTWAIN_ERR_TWAIN, false, FUNC_MACRO);
    }
    #ifdef DTWAIN_LIB
    CTL_TwainAppMgr::SetDLLInstance( CTL_StaticData::s_DLLInstance );
    #else
    CTL_TwainAppMgr::SetDLLInstance( static_cast<HINSTANCE>(GetDLLInstance()) );
    #endif

    pHandle->m_pAppMgr     = CTL_TwainAppMgr::GetInstance().get();
    pHandle->m_pTwainSession = Session;
    pHandle->m_hInstance   = hInstance;

    // We may need to route the messages to the user.  The user window
    // must be aware that if it is subclassed again, the message will
    // appear twice.
    pHandle->m_hNotifyWnd = hWndMsgNotify;

    // Destroy the pointer
    pHandle->m_bSessionAllocated = true;

    // Subclass the user's window (if necessary)
    #ifdef _WIN32
    if ( !pHandle->m_bUseProxy )
        pHandle->m_hOrigProc = SubclassTwainMsgWindow(hWndMsgNotify);
    else
        pHandle->m_hOrigProc = nullptr;
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

static DTWAIN_ARRAY GetFileTypes(CTL_TwainDLLHandle* pHandle, int nType)
{
    constexpr const char *sNames[] = { "","-Single","-Multi" };
    DTWAIN_ARRAY aFileTypes = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYLONG, 0);
    if (aFileTypes)
    {
        auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
        auto& factory = pHandle->m_ArrayFactory;
        for (auto& pr : availableFileTypes)
        {
            auto val = pr.first;
            if (StringWrapperA::EndsWith(pr.second.m_formatName, sNames[nType]))
                factory->add_to_back(aFileTypes, &val, 1);
        }
    }
    return aFileTypes;
}

static std::string GetFileTypeExtensionsInternal(int nType)
{
    const auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
    const auto iter = availableFileTypes.find(nType);
    if ( iter != availableFileTypes.end())
        return StringWrapperA::Join(iter->second.m_vExtensions, "|");
    return {};
}

static std::string GetFileTypeNameInternal(int nType)
{
    const auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
    const auto iter = availableFileTypes.find(nType);
    if (iter != availableFileTypes.end())
        return iter->second.m_formatName;
    return{};
}

template <typename Fn>
static LONG GetFileTypeInfo(Fn infoFn, int nType, LPTSTR lpszName, LONG nMaxLen)
{
    LONG realLen = -1;
    std::string str = infoFn(nType);
    if (!str.empty())
    {
        const CTL_StringType str2 = StringConversion::Convert_Ansi_To_Native(str);
        realLen = StringWrapper::CopyInfoToCString(str2, lpszName, nMaxLen);
    }
    return realLen;
}

LONG DLLENTRY_DEF DTWAIN_GetFileTypeName(LONG nType, LPTSTR lpszName, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((nType, lpszName, nMaxLen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszName))
    LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypeInfo(&GetFileTypeNameInternal, nType, lpszName, nMaxLen))
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensions(LONG nType, LPTSTR lpszName, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((nType, lpszName, nMaxLen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszName))
    LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypeInfo(&GetFileTypeExtensionsInternal, nType, lpszName, nMaxLen))
    CATCH_BLOCK(-1)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedFileTypes(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypes(pHandle, 0))
    CATCH_BLOCK(nullptr)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedMultiPageFileTypes(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypes(pHandle, 2))
    CATCH_BLOCK(nullptr)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedSinglePageFileTypes(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(GetFileTypes(pHandle, 1))
    CATCH_BLOCK(nullptr)
}

static HMODULE GetDLLInstance()
{
    #ifdef _WIN32
    return GetModuleHandleA(DTWAIN_DLLNAME);
    #else
    return 0;
    #endif
}

bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId)
{
    if ( !pHandle )
         return false;
    return CTL_StaticData::GetThreadToDLLHandleMap().insert({threadId, pHandle}).second;
}

bool RemoveThreadIdFromAssociation(unsigned long threadId)
{
    auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
    auto iter = threadMap.find(threadId);
    if ( iter != threadMap.end())
    {
        threadMap.erase(iter);
        return true;
    }
    return false;
}


template <class TypeInfo, class TypeArray>
bool FindFirstValue( TypeInfo SearchVal,
                    std::vector<TypeArray> *pSearchArray,
                    int *pWhere/*=NULL*/ )
{
    if ( pWhere )
        *pWhere = -1;
    auto it = std::find_if(pSearchArray->begin(), pSearchArray->end(), [&](const TypeArray& val) { return val.GetValue1() == SearchVal;}); //Searcher(SearchVal));
    if ( it != pSearchArray->end() )
    {
        if (pWhere)
            *pWhere = static_cast<int>(std::distance(pSearchArray->begin(), it));
        return true;
    }
    return false;
}

static bool FindTask( DWORD hTask )
{
    auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
    return threadMap.find(hTask) != threadMap.end();
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndTwainSession()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    // Delete it
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{return DTWAIN_IsAcquiring()==1;}, DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

    // Check if any source is still acquiring
    if ( pHandle->m_nSourceCloseMode == DTWAIN_SourceCloseModeFORCE )
    {
        // Close any sources
        DTWAIN_CloseAllSources();
    }
    else
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return DTWAIN_IsAcquiring()==1;}, DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

    if ( !pHandle->m_bSessionAllocated )
        LOG_FUNC_EXIT_NONAME_PARAMS(true)

    StringTraitsA::string_type sClosingDSM = dynarithmic::GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_DSM) + "\n";
    StringTraitsA::string_type sClosingTwainSession = dynarithmic::GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_TWAIN_SESSION) + "\n";

    try
    {
        CTL_TwainAppMgr::CloseSourceManager(pHandle->m_pTwainSession);
    }
    catch(...)
    {
        LogWriterUtils::WriteLogInfoIndentedA(sClosingDSM);
    }

    try
    {
        CTL_TwainAppMgr::DestroyTwainSession(pHandle->m_pTwainSession);
    }
    catch(...)
    {
        LogWriterUtils::WriteLogInfoIndentedA(sClosingTwainSession);
    }
    if ( CTL_StaticData::GetThreadToDLLHandleMap().size() == 1 )
    {
        auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
        try
        {
            CTL_TwainAppMgr::Destroy();
        }
        catch(...)
        {
            if (logFilterFlags)
            {
                StringTraitsA::string_type sClosingManager = GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_DTWAIN_MANAGER);
                LogWriterUtils::WriteLogInfoIndentedA(sClosingManager);
            }
        }
        if (logFilterFlags)
        {
            LogWriterUtils::WriteLogInfoIndentedA(GetResourceStringFromMap(IDS_CLOSING_DTWAIN));
        }
    }

    // Close the window (Dummy window may have been created)
    pHandle->m_bSessionAllocated = false;
    #ifdef _WIN32
    if ( pHandle->m_bUseProxy )
    {
        pHandle->m_CallbackMsg = nullptr;
        if (IsWindow(pHandle->m_hWndTwain))
        {
            try
            {
                DestroyWindow(pHandle->m_hWndTwain);
            }
            catch (...) {}
        }
    }
    else
    {
        try
        {
            // Remove subclass from the user's window
            if ( pHandle->m_hOrigProc && pHandle->m_hWndTwain )
                SubclassTwainMsgWindow(pHandle->m_hWndTwain, pHandle->m_hOrigProc);
        }
        catch(...)
        {
            if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            {
                LogWriterUtils::WriteLogInfoIndentedA(GetResourceStringFromMap(IDS_DTWAIN_ERROR_REMOVE_WINDOW));
            }
        }
    }
    pHandle->m_hWndTwain = nullptr;
    pHandle->m_hOrigProc = nullptr;
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

void dynarithmic::SysDestroyNoCheck()
{

}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SysDestroy()
{
    std::lock_guard<std::mutex> lg(CTL_StaticData::s_mutexInitDestroy);
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if (!DTWAIN_EndTwainSession())
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    const DTWAIN_BOOL bRet = SysDestroyHelper(FUNC_MACRO, pHandle);
    if (!bRet)
    {
        #if DTWAIN_BUILD_LOGCALLSTACK == 1
        LogValue(FUNC_MACRO, false, false);
        #endif
    }
    #if DTWAIN_BUILD_LOGCALLSTACK == 1
    if (CTL_StaticData::GetLogFilterFlags())
        CTL_LogFunctionCall(__FUNC__, 1);
    #endif
    return bRet;
    CATCH_BLOCK(false)
}

static bool SysDestroyHelper(const char* pParentFunc, CTL_TwainDLLHandle* pHandle, bool bCheck)
{
    #ifdef _WIN32
    // Unload the OCR interfaces
    unsigned long threadId = getThreadId();

    UnloadOCRInterfaces(pHandle);
    #endif

    // Close out any INI changes
    auto* customProfile = CTL_StaticData::GetINIInterface();
    if (customProfile)
    {
        customProfile->SaveFile(CTL_StaticData::GetINIPath().c_str());
        CTL_StaticData::s_iniInterface.reset();
        CTL_StaticData::SetINIFileLoaded(false);
    }

    // Remove the handle for this thread
    auto& threadMap = CTL_StaticData::GetThreadToDLLHandleMap();
    auto it = threadMap.find(threadId);
    if ( it == threadMap.end() )
        return false;
    #if DTWAIN_BUILD_LOGCALLSTACK
    // Log the parent function, and the assumption that the 
    // return value from this point on is "true". 
    LogValue(pParentFunc, false, true);
    #endif
    try
    {
        // Note that the assumption is that no exception is thrown from
        // this point on.
        UnhookAllDisplays();
        pHandle->RemoveAllEnumerators();
        pHandle->RemoveAllSourceCapInfo();
        pHandle->RemoveAllSourceMaps();
        pHandle->m_CallbackMsg = nullptr;
        pHandle->m_CallbackError = nullptr;
        RemoveThreadIdFromAssociation(threadId);
        CTL_StaticData::Reset();
        FreeImage_ClearPlugins();
        FreeImage_DeInitialise();
        return true;
    }
    catch (...)
    {
        OutputDebugString(_T("Exception error in DTWAIN_SysDestroy()"));
    }
    return false;
}

/* This function tests all open DLL handles to see if any source is acquiring */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsAcquiring()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const auto iter = std::find_if(pHandle->m_mapStringToSource.begin(),
                                   pHandle->m_mapStringToSource.end(), 
                                [&](const CTL_StringToSourcePtrMap::value_type& vt) {return vt.second->IsAcquireAttempt(); });
    if ( iter != pHandle->m_mapStringToSource.end())
         LOG_FUNC_EXIT_NONAME_PARAMS(true)
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

HWND  DLLENTRY_DEF  DTWAIN_GetTwainHwnd()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_hWndTwain)
    CATCH_BLOCK(HWND(0))
}

LONG DTWAIN_CloseAllSources()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()

    // See if DLL Handle exists
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    if ( pHandle->m_mapStringToSource.empty() )
        LOG_FUNC_EXIT_NONAME_PARAMS(0)
    CTL_StringToSourcePtrMap m_mapTemp = pHandle->m_mapStringToSource;
    std::for_each(m_mapTemp.begin(), m_mapTemp.end(), [&](CTL_StringToSourcePtrMap::value_type& vt)
    {
        CTL_ITwainSource *pTheSource = vt.second;
        if (pTheSource->IsAcquireAttempt())
            CTL_TwainAppMgr::DisableUserInterface(pTheSource);
        DTWAIN_CloseSource(pTheSource);
    });

    LOG_FUNC_EXIT_NONAME_PARAMS(0)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainMode()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(pHandle->m_lAcquireMode)
    CATCH_BLOCK(-1L)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainMode(LONG lMode)
{
    LOG_FUNC_ENTRY_PARAMS((lMode))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    switch (lMode)
    {
        case DTWAIN_MODELESS:
        case DTWAIN_MODAL:
            pHandle->m_lAcquireMode = lMode;
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        break;
    }
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return 0;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTIFFSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPDFSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPNGSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsJPEGSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainDSM(LONG DSMType)
{
    LOG_FUNC_ENTRY_PARAMS((DSMType))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    #ifndef WIN64
    if ( DSMType == DTWAIN_TWAINDSM_LEGACY || DSMType == DTWAIN_TWAINDSM_LATESTVERSION)
    {
        pHandle->m_SessionStruct.nSessionType = DSMType;
        pHandle->m_SessionStruct.DSMName = TWAINDLLVERSION_1;
    }
    else
    if ( DSMType == DTWAIN_TWAINDSM_VERSION2)
    {
        pHandle->m_SessionStruct.nSessionType = DTWAIN_TWAINDSM_VERSION2;
        pHandle->m_SessionStruct.DSMName = TWAINDLLVERSION_2;
    }
    #else
    pHandle->m_SessionStruct.nSessionType = DTWAIN_TWAINDSM_VERSION2;
    pHandle->m_SessionStruct.DSMName = TWAINDLLVERSION_2;
    #endif
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrder(LONG SearchOrder)
{
    LOG_FUNC_ENTRY_PARAMS((SearchOrder))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    pHandle->m_TwainDSMSearchOrder = SearchOrder;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetDSMSearchOrder(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const LONG SearchOrder = pHandle->m_TwainDSMSearchOrder;
    LOG_FUNC_EXIT_NONAME_PARAMS(SearchOrder)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderEx(LPCTSTR SearchOrder, LPCTSTR UserDirectory)
{
    LOG_FUNC_ENTRY_PARAMS((SearchOrder))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const std::string strValidString = CheckSearchOrderString(StringConversion::Convert_NativePtr_To_Ansi(SearchOrder));
    if ( !strValidString.empty() )
    {
        pHandle->m_TwainDSMSearchOrderStr = strValidString;
        pHandle->m_TwainDSMUserDirectory = UserDirectory?UserDirectory:StringWrapper::traits_type::GetEmptyString();
        pHandle->m_TwainDSMSearchOrder = -1;
        LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePath(LPCTSTR ResourcePath)
{
    LOG_FUNC_ENTRY_PARAMS((ResourcePath))
    CTL_StaticData::GetResourcePath() = ResourcePath;
    LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_CallCallback(WPARAM wParam, LPARAM lParam, LONG UserData)
{
    LOG_FUNC_ENTRY_PARAMS((wParam, lParam, UserData))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG RetVal = 1;
    if (pHandle->m_pCallbackFn)
        RetVal = static_cast<LONG>((*pHandle->m_pCallbackFn)(wParam, lParam, UserData));
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_CallCallback64(WPARAM wParam, LPARAM lParam, LONGLONG UserData)
{
    LOG_FUNC_ENTRY_PARAMS((wParam, lParam, UserData))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    LONG RetVal = 1;
    if (pHandle->m_pCallbackFn64)
        RetVal = static_cast<LONG>((*pHandle->m_pCallbackFn64)(wParam, lParam, UserData));
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(0)
}

void UnhookAllDisplays()
{
#ifdef _WIN32
    if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_CONSOLE)
        FreeConsole();
#endif
}

void dynarithmic::OutputDTWAINErrorW(const CTL_TwainDLLHandle* pHandle, LPCWSTR pFunc)
{
    if ( pFunc )
        OutputDTWAINError(pHandle, StringConversion::Convert_Wide_To_Ansi(pFunc).c_str());
    else
        OutputDTWAINError(pHandle);
}

void dynarithmic::OutputDTWAINErrorA(const CTL_TwainDLLHandle* pHandle, LPCSTR pFunc)
{
    OutputDTWAINError(pHandle, pFunc);
}

void dynarithmic::OutputDTWAINError(const CTL_TwainDLLHandle* pHandle, LPCSTR pFunc)
{
    auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
    if (!(logFilterFlags & DTWAIN_LOG_DTWAINERRORS) )
        return;
    static constexpr int MaxMessage = DTWAIN_USERRES_MAXSIZE;
    char szBuf[MaxMessage+1];
    if ( !pHandle )
        DTWAIN_GetErrorStringA( DTWAIN_ERR_BAD_HANDLE, szBuf,MaxMessage);
    else
        DTWAIN_GetErrorStringA( pHandle->m_lLastError, szBuf, MaxMessage);
    std::string s(szBuf);
    if ( !pHandle )
        LogWriterUtils::WriteLogInfoIndentedA(s);

    if (logFilterFlags & DTWAIN_LOG_ERRORMSGBOX && pHandle)
        LogDTWAINErrorToMsgBox(pHandle->m_lLastError, pFunc, s);
    else
    if ( !pHandle && logFilterFlags & DTWAIN_LOG_INITFAILURE)
        LogDTWAINErrorToMsgBox(DTWAIN_ERR_BAD_HANDLE, nullptr, s);
}



DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AppHandlesExceptions(DTWAIN_BOOL bSet)
{
    CTL_StaticData::SetThrowExceptions(bSet?true:false);
    return TRUE;
}


std::string GetStaticLibVer()
{
    const LONG nVer = DTWAIN_GetStaticLibVersion();
    if ( nVer != 0 )
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

LONG DLLENTRY_DEF DTWAIN_GetVersionString(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetVersionString(), lpszVer, nLength);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetLibraryPath(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetDTWAINDLLPath(), lpszVer, nLength);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetShortVersionString(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetDTWAINDLLVersionInfoStr(), lpszVer, nLength);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszVer))
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetVersionInfo(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetVersionInfo(), lpszVer, nLength);
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

LONG DLLENTRY_DEF DTWAIN_GetTwainStringName(LONG category, LONG TwainID, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((category, TwainID, lpszBuffer, nMaxLen))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    std::string sValue;
    switch (category)
    {
        case DTWAIN_DGNAME:
            sValue = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DG, TwainID);
        break;
        case DTWAIN_DATNAME:
            sValue = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_DAT, TwainID);
        break;
        case DTWAIN_MSGNAME:
            sValue = CTL_StaticData::GetTwainNameFromConstantA(DTWAIN_CONSTANT_MSG, TwainID);
        break;
    }
    const LONG RetVal = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(sValue), lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainIDFromName(LPCTSTR lpszBuffer)
{
    LOG_FUNC_ENTRY_PARAMS((lpszBuffer))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    auto retVal = CTL_StaticData::GetIDFromTwainName(StringConversion::Convert_NativePtr_To_Ansi(lpszBuffer));
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !retVal.first; }, DTWAIN_ERR_STRINGID_NOTFOUND, retVal.second, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second)
    CATCH_BLOCK(std::numeric_limits<int32_t>::min())
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
            RetVal = StringWrapper::CopyInfoToCString(GetWinVersion(), lpszBuffer, nMaxLen);
            if (lpszBuffer)
                pHandle->m_sWindowsVersionInfo = lpszBuffer;
        }
        else
            RetVal = StringWrapper::CopyInfoToCString(pHandle->m_sWindowsVersionInfo, lpszBuffer, nMaxLen);
    }
    else
        RetVal = StringWrapper::CopyInfoToCString(GetWinVersion(), lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((lpszBuffer))
    LOG_FUNC_EXIT_NONAME_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

CTL_StringType dynarithmic::GetDTWAINExecutionPath()
{
    const auto symlocation = boost::dll::symbol_location(DTWAIN_DLLNAME);
    #ifdef _UNICODE
    CTL_StringType szName = symlocation.parent_path().wstring();
    #else
    CTL_StringType szName = symlocation.parent_path().string();
    #endif
    return szName;
}

CTL_StringType dynarithmic::GetDTWAINDLLPath()
{
    auto& dllPath = CTL_StaticData::GetDLLPath();
    if ( !dllPath.empty())
        return dllPath;
    TCHAR buffer[1024];
    boost::winapi::GetModuleFileName(CTL_StaticData::GetDLLInstanceHandle(), buffer, 1024);
    dllPath = buffer;
    return buffer;
}

CTL_StringType dynarithmic::GetVersionString()
{
    auto& verString = CTL_StaticData::GetVersionString();
    if (!verString.empty())
        return verString;

    LONG lMajor, lMinor, lVersionType, lPatch;
    // Write the version info
    if (DTWAIN_GetVersionInternal(&lMajor, &lMinor, &lVersionType, &lPatch) )
    {
        std::string s;
        std::string sBits = "(32-bit)";
        if ( lVersionType & DTWAIN_64BIT_VERSION )
            sBits = "(64-bit)";

        if (lVersionType & DTWAIN_UNICODE_VERSION)
            s += " [Unicode Version]";

        if (lVersionType & DTWAIN_DEVELOP_VERSION)
            s += " [Debug]";

        if (lVersionType & DTWAIN_CALLSTACK_LOGGING_PLUS)
            s += " [Log Call Stack+]";
        else
        if (lVersionType & DTWAIN_CALLSTACK_LOGGING)
            s += " [Log Call Stack]";
        else
        if (!(lVersionType & DTWAIN_CALLSTACK_LOGGING))
            s += " [Minimal Logging]";

        s += " ";
        s += sBits;
        StringStreamA strm;
        std::string sStatic;
        if (DTWAIN_GetStaticLibVersion() != 0 )
        {
            sStatic += "Compiler used: " + GetStaticLibVer();
            sStatic += "\n";
        }

        strm << sStatic << "Dynarithmic TWAIN Library, Version " << lMajor << "." << lMinor << " - " << s << " Version (Patch Level "
            << lPatch << ") Internal Build Number: " << StringConversion::Convert_Native_To_Ansi(GetDTWAINInternalBuildNumber()) << "\n" << 
            "Shared Library path : " <<  StringConversion::Convert_Native_To_Ansi(GetDTWAINDLLPath());

        strm << "\nUsing Resource file (twaininfo.txt) version: " << StringConversion::Convert_Native_To_Ansi(CTL_StaticData::GetResourceVersion());
        strm << "\nResource file path: " << StringConversion::Convert_Native_To_Ansi(CTL_StaticData::GetResourcePath());
        strm << "\nText Resource Language: " << StringConversion::Convert_Native_To_Ansi(CTL_StaticData::GetGeneralResourceInfo().sResourceName);
        if (CTL_StaticData::GetGeneralResourceInfo().bIsFromRC)
            strm << " (Text resources are directly from DTWAIN DLL and not from a text resource file)";
        verString = StringConversion::Convert_Ansi_To_Native(strm.str());
        return verString;
    }
    return {};
}

void WriteVersionToLog(CTL_TwainDLLHandle *pHandle)
{
    std::string ansiVer;
    if (CTL_StaticData::GetLogFilterFlags())
    {
        auto sVer = GetVersionString();
        const auto sWinVer = GetWinVersion();
        auto sDSMPath = CTL_TwainAppMgr::GetDSMPath();
        CTL_StringType sDSMVersionInfo;
        if (sDSMPath.empty())
        {
            sDSMPath = _T("(unknown or not queried)");
            sDSMVersionInfo.clear();
            sDSMVersionInfo = _T("\nDSM Version Information: ");
            sDSMVersionInfo += _T("(unknown or not queried)");
        }
        else
        {
            sDSMVersionInfo = _T("\nDSM Version Information:\n");
            if ( CTL_TwainAppMgr::GetInstance() )
                sDSMVersionInfo += dynarithmic::GetVersionInfo(CTL_TwainAppMgr::GetInstance()->GetDSMModuleHandle(), 4, _T("\n"));
            else
                sDSMVersionInfo += _T("(unknown or not queried)");
        }
        sDSMPath = _T("Active DSM Path: ") + sDSMPath;
        sVer += _T("\n") + sWinVer + sDSMPath + sDSMVersionInfo + _T("\n");
        #ifdef _WIN32
        // All log messages must be ANSI
        ansiVer = StringConversion::Convert_Native_To_Ansi(sVer);
        auto logFilterFlags = CTL_StaticData::GetLogFilterFlags();
        if (logFilterFlags & (DTWAIN_LOG_USEFILE | DTWAIN_LOG_CONSOLE))
        {
            if (!CTL_StaticData::GetLogger().StatusOutFast(ansiVer.c_str()))
            {
                ansiVer += "\n";
                LogToDebugMonitorA(ansiVer);
            }
        }
        if (logFilterFlags & DTWAIN_LOG_DEBUGMONITOR)
        {
            sVer += _T("\n");
            LogToDebugMonitor(sVer);
        }
        if ( UserDefinedLoggerExists(pHandle) )
            WriteUserDefinedLogMsgA(pHandle, ansiVer.c_str());
        #endif
    }
}

void dynarithmic::DTWAIN_InternalThrowException() THIS_FUNCTION_THROWS
{
    throw;
}


bool GetDTWAINDLLVersionInfo(HMODULE hMod, LONG* lMajor, LONG* lMinor, LONG *pPatch)
{
    if ( lMajor )
        *lMajor = DTWAIN_MAJOR_VERSION;
    if (lMinor)
        *lMinor = DTWAIN_MINOR_VERSION;
    if (pPatch)
        *pPatch = DTWAIN_PATCHLEVEL_VERSION;
    return true;
}

CTL_StringType GetDTWAINDLLVersionInfoStr()
{
    return StringConversion::Convert_AnsiPtr_To_Native(DTWAIN_VERINFO_FILEVERSION);
}

CTL_StringType GetDTWAINInternalBuildNumber()
{
    return StringConversion::Convert_AnsiPtr_To_Native(DTWAIN_BUILDVERSION);
}

CTL_StringType dynarithmic::GetDTWAININIPath()
{
    CTL_StringType szName = DTWAIN_ININAME_NATIVE; 
    return get_parent_directory(GetDTWAINDLLPath().c_str()) + szName;
}

std::string dynarithmic::GetDTWAININIPathA()
{
    return StringConversion::Convert_Native_To_Ansi(GetDTWAININIPath());
}

CTL_StringType dynarithmic::GetDTWAINTempFilePath(CTL_TwainDLLHandle* pHandle)
{
    static CTL_StringType sDummy;
    if (!pHandle)
        return sDummy;
    if ( pHandle->m_sTempFilePath.empty())
    {
        const auto tempPath = temp_directory_path();
        if (tempPath.empty())
        {
            std::string msg = GetResourceStringFromMap(IDS_LOGMSG_ERRORTEXT) + ": " + GetResourceStringFromMap(IDS_LOGMSG_TEMPFILENOTEXISTTEXT);
            LogWriterUtils::WriteLogInfoIndentedA(msg);
        }
        else
            pHandle->m_sTempFilePath = tempPath;
    }
    std::string msg = "Temp path is " + StringConversion::Convert_Native_To_Ansi(pHandle->m_sTempFilePath);
    LogWriterUtils::WriteLogInfoIndentedA(msg);
    return pHandle->m_sTempFilePath;
}


std::string CheckSearchOrderString(std::string str)
{
    static std::set<char> setValidChars = {'C','W','O','U','S'};
    std::string strOut;
    StringWrapperA::MakeUpperCase(str);
    std::copy_if(str.begin(), str.end(), std::back_inserter(strOut), [&](char ch) { return setValidChars.count(ch); });
    return strOut;
}

// This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [SourceXferWaitInfo]
// section for TWAIN sources that may potentially send "close source" requests
// before sending the "start transfer" request when acquiring images.
void LoadTransferReadyOverrides()
{
    auto& xfer_map = CTL_TwainAppMgr::GetSourceToXferReadyMap();
    xfer_map.clear();
    auto& xfer_list = CTL_TwainAppMgr::GetSourceToXferReadyList();
    xfer_list.clear();

    // Get the section name
    auto *customProfile = CTL_StaticData::GetINIInterface();
    if (!customProfile)
        return;
    CSimpleIniA::TNamesDepend keys;
    customProfile->GetAllKeys("SourceXferWaitInfo", keys);
    auto iter = keys.begin();
    while (iter != keys.end())
    {
        CSimpleIniA::TNamesDepend vals;
        customProfile->GetAllValues("SourceXferWaitInfo", iter->pItem, vals);
        if (!vals.empty())
        {
                auto iter2 = vals.begin();
                if ( !vals.empty())
            {
                try
                {
                    uint32_t valueToUse = std::stoi(iter2->pItem);
                    xfer_list.push_back({ iter->pItem, valueToUse });
                }
                catch (const std::invalid_argument& /*ex*/)
                {
                    // We can get here if std::stoi detects that the value is not 
                    // a valid integer. 
                    xfer_list.push_back({ iter->pItem, 0 });
                }
                catch (const std::out_of_range& /*ex*/)
                {
                    // We can get here if std::stoi detects that the value is not 
                    // a valid integer. 
                    xfer_list.push_back({ iter->pItem, 0 });
                }
            }
        }
        ++iter;
    }
}

// This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [SourceOpenProps]
// section.  This section determines the activities to perform after sucessfully
// opening a TWAIN Source
void LoadOnSourceOpenProperties(CTL_TwainDLLHandle* pHandle)
{
    // Get the section name
    auto* feederProfile = CTL_StaticData::GetINIInterface();
    if (!feederProfile)
        return;

    pHandle->m_OnSourceOpenProperties.m_bCheckFeederStatusOnOpen = feederProfile->GetBoolValue("SourceOpenProps", "CheckFeederStatus", true);
    pHandle->m_OnSourceOpenProperties.m_bQueryBestCapContainer = feederProfile->GetBoolValue("SourceOpenProps", "QueryBestCapContainer", true);
    pHandle->m_OnSourceOpenProperties.m_bQueryCapOperations = feederProfile->GetBoolValue("SourceOpenProps", "QueryCapOperations", true);
}

void LoadImageFileOptions(CTL_TwainDLLHandle* pHandle)
{
    auto *customProfile = CTL_StaticData::GetINIInterface();
    if (!customProfile)
        return;
    CTL_StaticData::SetResamplingDone(customProfile->GetBoolValue("ImageFile", "resample", true));
    return;
}

// This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [FlatbedOnly]
// section for TWAIN sources that only work if the source is preset to use
// only the flatbed portion of the device.  
void LoadFlatbedOnlyOverrides()
{
    auto& flatbed_list = CTL_TwainAppMgr::GetSourceFlatbedOnlyList();
    flatbed_list.clear();

    // Get the section name
    auto* customProfile = CTL_StaticData::GetINIInterface();
    if (!customProfile)
        return;
    CSimpleIniA::TNamesDepend keys;
    customProfile->GetAllKeys("FlatbedOnly", keys);
    auto iter = keys.begin();
    while (iter != keys.end())
    {
        CSimpleIniA::TNamesDepend vals;
        customProfile->GetAllValues("FlatbedOnly", iter->pItem, vals);
        flatbed_list.insert(iter->pItem);
        ++iter;
    }
}

bool LoadGeneralResources(bool blockExecution)
{
    bool bResourcesLoaded = false;
    CTL_StaticData::SetResourceLoadError(DTWAIN_NO_ERROR);
    typedef std::function<bool(ResourceLoadingInfo&)> boolFuncs;
    boolFuncs bf[] = { &LoadTwainResources };
    for (auto& fnBool : bf)
    {
        ResourceLoadingInfo ret;
        fnBool(ret);

        // If there are any errors loading the twaininfo.txt or INI files, report them here.
        if (std::any_of(ret.errorValue.begin(), ret.errorValue.end(), [](bool b) { return b == false; }))
        {
            CTL_StringType versionErrorMessage;
            CTL_StringType errorMsg = _T("Error.  DTWAIN Resource file(s) not found or corrupted:\r\n");
            std::vector<CTL_StringType> vErrors;
            if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_LOADED])
            {
                CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_NOT_FOUND);
            }
            if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INIFILE_LOADED])
            {
                #if defined (WIN64) || defined(_WIN64)
                CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_INI64_NOT_FOUND);
                #else
                CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_INI32_NOT_FOUND);
                #endif
            }
            if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_INFOFILE_VERSION_READ])
            {
                CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_BAD_VERSION);
                versionErrorMessage = _T("\r\nBad or outdated TWAIN version of resources used: (");
                versionErrorMessage += ret.errorMessage;
                versionErrorMessage += _T(").  Expected minimum version: ");
                versionErrorMessage += _T(DTWAIN_TEXTRESOURCE_FILEVERSION);
                versionErrorMessage += _T("\r\nPlease use the latest text resources found at \"https://github.com/dynarithmic/twain_library/tree/master/text_resources\"");
            }
            if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_CRC_CHECK])
                CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_CRC_CHECK);
            if (!ret.errorValue[ResourceLoadingInfo::DTWAIN_RESLOAD_NODUPLICATE_ID])
            {
                CTL_StringStreamType strm;
                strm << _T("\r\n\r\nDuplicate ID Information:\r\n") << _T("Line number: ") << ret.m_dupInfo.lineNumber;
                strm << _T("\r\nLine: ") << StringConversion::Convert_Ansi_To_Native(ret.m_dupInfo.line);
                strm << _T("\r\nID: ") << ret.m_dupInfo.duplicateID;
                versionErrorMessage = strm.str();
                CTL_StaticData::SetResourceLoadError(DTWAIN_ERR_RESOURCES_DUPLICATEID_FOUND);
            }
            // Only display the error message box if DTWAIN_SysInitialize() was called
            // instead of DTWAIN_SysInitialNoBlocking()
            if (blockExecution)
            {
                #ifdef _WIN32
                TCHAR szBuf[DTWAIN_USERRES_MAXSIZE + 1] = {};
                DTWAIN_GetErrorString(CTL_StaticData::GetResourceLoadError(), szBuf, DTWAIN_USERRES_MAXSIZE);
                CTL_StringType sAllErrors = _T("Error in reading resource file:\r\n") + ret.resourcePath + _T("\r\n") + 
                    errorMsg + szBuf + versionErrorMessage;
                MessageBox(nullptr, sAllErrors.c_str(), _T("DTWAIN Resource Error"), MB_ICONERROR);
                #endif
            }
        }
        else
            bResourcesLoaded = true;
    }
    return bResourcesLoaded;
}

#undef min
#undef max
#include <sstream>

#ifdef DTWAIN_LIB
void GetVersionFromResource(LPLONG lMajor, LPLONG lMinor, LPLONG patch)
{
    // split on the "."
    CTL_StringArrayType aStr;
    CTL_StringType ver = _T(DTWAIN_VERINFO_BASEVERSION);
    ver += _T(DTWAIN_PATCHLEVEL_VERSION);
    StringWrapper::Tokenize(ver, _T("."), aStr, true);
    if ( aStr.size() == 4 )
    {
        if ( lMajor )
            *lMajor = _ttoi(aStr[0].c_str());
        if ( lMinor )
            *lMinor = _ttoi(aStr[1].c_str());
        if ( patch )
            *patch = _ttoi(aStr[3].c_str());
    }
    else
    {
        if ( lMajor )
            *lMajor = 0;
        if ( lMinor )
            *lMinor = 0;
        if ( patch )
            *patch = 0;
    }
}
#endif

#ifdef _WIN32
#include "windowsinit_impl.inl"
#else
#include "linuxinit_impl.inl"
#endif
