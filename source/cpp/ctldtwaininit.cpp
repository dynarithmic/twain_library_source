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
#include "ctltwmgr.h"
#include "ctlres.h"
#include "../dtwinver/dtwinverex.h"
#include "../simpleini/simpleini.h"
#include "dtwain_verinfo.h"
#include "dtwain_resource_constants.h"
#include "errorcheck.h"
#include "dtwstrfn.h"
#include "ctlfileutils.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#pragma comment (lib, "shlwapi")
#endif
#ifdef _WIN64
    #pragma message ("Compiling 64-bit DTWAIN")
#endif

#ifdef WIN32
    #pragma message ("Compiling 32-bit DTWAIN")
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

#ifdef DTWAIN_DEMO_VERSION
    #include "splash.h"
#endif

#ifdef USE_NAMESPACES

#endif

using namespace dynarithmic;

static DTWAIN_HANDLE SysInitializeHelper(bool noblock);
static LONG DTWAIN_CloseAllSources();
static void UnhookAllDisplays();
static bool HookTwainDLL(CTL_TwainDLLHandle* pHandle, bool bHook);
static HWND CreateTwainWindow(CTL_TwainDLLHandle* pHandle,
                              HINSTANCE hInstance= nullptr,
                              HWND hWndParent= nullptr);
static void RegisterTwainWindowClass();
static void OpenLogFile(LPCTSTR pFileName, LONG logFlags, bool append=false);
static void WriteVersionToLog();
static bool SysDestroyHelper(CTL_TwainDLLHandle* pHandle, bool bCheck=true);
static void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL, LPCTSTR szName);

#ifdef _WIN32
static UINT_PTR APIENTRY FileSaveAsHookProc(HWND hWnd, UINT msg, WPARAM w, LPARAM lparam);
static WNDPROC SubclassTwainMsgWindow(HWND hWnd, WNDPROC wProcIn = nullptr);
//LRESULT ProcessHook(int nCode, WPARAM wParam, LPARAM lParam, int nValue);
#define     TWSubclassWindow(hwnd, lpfn)  \
            (reinterpret_cast<WNDPROC>(SetWindowLongPtr((hwnd), GWLP_WNDPROC, (LONG_PTR)(WNDPROC)(lpfn))))
#endif

static void LogDTWAINErrorToMsgBox(int nError, LPCSTR pFunc, const std::string& s);
static std::string FixPathStringA( LPCSTR szINIPath );
static std::wstring FixPathStringW(LPCWSTR szINIPath);

/* Set the paths for image DLL's and language resource */
static DTWAIN_BOOL SetImageDLLPath(LPCTSTR szPath);
static DTWAIN_BOOL SetLangResourcePath(LPCTSTR szPath);
static std::string GetStaticLibVer();
static void LoadStaticData(CTL_TwainDLLHandle*);
static bool GetDTWAINDLLVersionInfo(HMODULE hMod, LONG* lMajor, LONG* lMinor, LONG *pPatch);
static CTL_StringType GetDTWAINDLLVersionInfoStr();
static DTWAIN_BOOL DTWAIN_GetVersionInternal(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatch);
static std::string CheckSearchOrderString(std::string);

#ifdef UNICODE
#define FixPathString FixPathStringW
#else
#define FixPathString FixPathStringA
#endif

#ifdef DTWAIN_LIB
    static void GetVersionFromResource(LPLONG lMajor, LPLONG lMinor, LPLONG patch);
#endif

static bool FindTask( DWORD hTask, int *pWhere= nullptr);
static HMODULE GetDLLInstance();

static unsigned long getThreadId()
{
    static std::unordered_map<std::string, unsigned long> s_thread_map;
    std::string threadId = boost::lexical_cast<std::string>(std::this_thread::get_id());
    auto iter = s_thread_map.find(threadId);
    if (iter == s_thread_map.end())
    {
    unsigned long threadNumber = 0;
    sscanf(threadId.c_str(), "%lx", &threadNumber);
        iter = s_thread_map.insert({ threadId, threadNumber }).first;
    }
    return iter->second;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetVersion(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType)
{
    LOG_FUNC_ENTRY_PARAMS((lMajor, lMinor, lVersionType))
    const bool bRetVal = DTWAIN_GetVersionInternal(lMajor, lMinor, lVersionType, nullptr) ? true : false;
    LOG_FUNC_EXIT_PARAMS(bRetVal)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetVersionEx(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatchLevel)
{
    LOG_FUNC_ENTRY_PARAMS((lMajor,lMinor,lVersionType, lPatchLevel))
    const bool bRetVal = DTWAIN_GetVersionInternal(lMajor, lMinor, lVersionType, lPatchLevel)?true:false;
    LOG_FUNC_EXIT_PARAMS(bRetVal)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DTWAIN_GetVersionInternal(LPLONG lMajor, LPLONG lMinor, LPLONG lVersionType, LPLONG lPatch)
{
    LOG_FUNC_ENTRY_PARAMS((lMajor,lMinor,lVersionType))
    LONG nDistr = 0;

    #ifdef DTWAIN_OPENSOURCE_DISTRIBUTION
        #pragma message ("Building Open Source Distribution version")
        nDistr |= DTWAIN_OPENSOURCE_VERSION;
    #endif

    #ifdef DTWAIN_LIB
            CTL_StringType strVer = _T(DTWAIN_VERINFO_BASEVERSION)
                                    _T(DTWAIN_VERINFO_PATCHLEVEL_VERSION);
            CTL_StringArrayType aInfo;
            StringWrapper::Tokenize(strVer, _T("."), aInfo);
            *lMajor = _ttol(aInfo[0].c_str());
            *lMinor = _ttol(aInfo[1].c_str());
            if ( lPatch )
                *lPatch = _ttol(aInfo[3].c_str());
    #endif
    #ifdef DTWAIN_LIB
    GetVersionFromResource(lMajor, lMinor, lPatch);
    #else
    const bool modRet = GetDTWAINDLLVersionInfo(CTL_TwainDLLHandle::s_DLLInstance, lMajor, lMinor, lPatch);
     if ( !modRet )
     {
         LOG_FUNC_EXIT_PARAMS(false)
         return false;
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

    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetStaticLibVersion()
{
    LOG_FUNC_ENTRY_PARAMS(())
    #ifndef DTWAIN_LIB
        LOG_FUNC_EXIT_PARAMS(0)
        #pragma message ("Compiling DLL -- no static library")
    #endif

    #ifdef _MSC_VER
        #if _MSC_VER < 1800
            #error("Compiler must be Visual Studio 2013 or greater")
        #elif _MSC_VER == 1800
            #pragma message ("Microsoft Visual Studio 2013 compiler used to build library")
            LOG_FUNC_EXIT_PARAMS(51)
        #elif _MSC_VER == 1900
            #pragma message ("Microsoft Visual Studio 2015 compiler used to build library")
            LOG_FUNC_EXIT_PARAMS(61)
        #elif _MSC_VER >= 1910 && _MSC_VER < 1920
            #pragma message ("Microsoft Visual Studio 2017 compiler to build library")
            LOG_FUNC_EXIT_PARAMS(71)
        #elif _MSC_VER >= 1920
            #pragma message ("Microsoft Visual Studio 2019 compiler or greater compiler used to build library")
            LOG_FUNC_EXIT_PARAMS(81)
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
    LOG_FUNC_ENTRY_PARAMS(())
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
    {
        LONG err = DTWAIN_ERR_BAD_HANDLE;
        if (!CTL_TwainDLLHandle::s_ResourcesInitialized)
            err = DTWAIN_ERR_RESOURCES_NOT_FOUND;
        LOG_FUNC_EXIT_PARAMS(err)
    }
    else
    {
    }
    LOG_FUNC_EXIT_PARAMS(pHandle->m_lLastError)
    CATCH_BLOCK(DTWAIN_ERR_BAD_HANDLE)
}

LONG DLLENTRY_DEF  DTWAIN_GetErrorString(LONG lError, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((lError, lpszBuffer, nMaxLen))
    const size_t nBytes = GetResourceStringA(static_cast<UINT>(-lError), nullptr, 2048);
    if ( nBytes == 0)
    {
        LOG_FUNC_EXIT_PARAMS(0)
    }

    std::vector<char> szTemp(nBytes);
    GetResourceStringA(static_cast<UINT>(-lError), &szTemp[0], static_cast<LONG>(nBytes));
    const CTL_StringType sCopy = StringConversion::Convert_Ansi_To_Native(szTemp.data());

    const LONG nTotalBytes = StringWrapper::CopyInfoToCString(sCopy.data(), lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetConditionCodeString(LONG CC, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((CC, lpszBuffer, nMaxLen))
    const LONG lError = -(IDS_TWCC_ERRORSTART + CC);
    const LONG Retval = DTWAIN_GetErrorString(lError, lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(Retval)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadLanguageResource(LONG nLanguage)
{
    LOG_FUNC_ENTRY_PARAMS((nLanguage))
    LPCTSTR pLangDLL;
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
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
            LOG_FUNC_EXIT_PARAMS(false)
    }

    // Add the resource to the registry.
    const DTWAIN_BOOL bRet = DTWAIN_LoadCustomStringResources(pLangDLL);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_LoadCustomStringResources(LPCTSTR sLangDLL)
{
    LOG_FUNC_ENTRY_PARAMS((sLangDLL))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    const std::string sLangDLLString = StringConversion::Convert_NativePtr_To_Ansi(sLangDLL);
    // Add the resource to the registry.
    const auto exists = pHandle->AddResourceToRegistry(sLangDLLString.c_str()).second;
    bool bRet = false;
    if (exists)
        bRet = LoadLanguageResourceA(sLangDLLString.c_str(), pHandle->GetResourceRegistry());
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !bRet;}, DTWAIN_ERR_FILEOPEN, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsInitialized()
{
    // Get the Current task
    LOG_FUNC_ENTRY_PARAMS(())
    const DWORD hTask = getThreadId();

#ifdef DTWAIN_LIB
    if ( CTL_TwainDLLHandle::s_DLLInstance == NULL )
        LOG_FUNC_EXIT_PARAMS(false)
    if ( CTL_TwainDLLHandle::s_DLLHandles.size() > 0 )
        LOG_FUNC_EXIT_PARAMS(true)
    LOG_FUNC_EXIT_PARAMS(false)
#else
    // Check if this task has already been hooked
    int nWhere;
    if ( ::FindTask( hTask, &nWhere ) )
        // Already hooked.  No need to do this again
        LOG_FUNC_EXIT_PARAMS(true)
    LOG_FUNC_EXIT_PARAMS(false)
#endif
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartThread( DTWAIN_HANDLE DLLHandle )
{
    LOG_FUNC_ENTRY_PARAMS((DLLHandle))
    HookTwainDLL(reinterpret_cast<CTL_TwainDLLHandle*>(DLLHandle), TRUE);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndThread( DTWAIN_HANDLE DLLHandle )
{
    LOG_FUNC_ENTRY_PARAMS((DLLHandle))
    int nWhere;
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    if ( pHandle )
    {
        const bool bOk = ::FindTask( getThreadId(), &nWhere );
        if ( bOk )
            pHandle->s_aHookInfo.erase(pHandle->s_aHookInfo.begin() +  nWhere );
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_UseMultipleThreads(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    CTL_TwainDLLHandle::s_multipleThreads = bSet ? true : false;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_HANDLE dynarithmic::GetDTWAINHandle_Internal()
{
    #ifdef DTWAIN_LIB
    if (CTL_TwainDLLHandle::s_DLLInstance == NULL)
        return nullptr;
    #endif

    if (!CTL_TwainDLLHandle::s_multipleThreads)
    {
        if (!CTL_TwainDLLHandle::s_DLLHandles.empty())
            return CTL_TwainDLLHandle::s_DLLHandles.back().get();
        return nullptr;
    }

    // Check if this task has already been hooked
    // Get the Current task
    const DWORD hTask = getThreadId();

    #ifdef DTWAIN_LIB
    if (!CTL_TwainDLLHandle::s_DLLHandles.empty())
    {
        return CTL_TwainDLLHandle::s_DLLHandles[0].get();
    }
    return NULL;
    #else
    int nWhere;
    if (::FindTask(hTask, &nWhere))
    {
        const CTL_HookInfo HookInfo = CTL_TwainDLLHandle::s_aHookInfo[nWhere];
        return std::get<3>(HookInfo);
    }
    return nullptr;
    #endif
}


DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_GetDTWAINHandle()
{
    LOG_FUNC_ENTRY_PARAMS(())
    LOG_FUNC_EXIT_PARAMS(::GetDTWAINHandle_Internal())
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}


/* static */ DTWAIN_BOOL SetImageDLLPath(LPCTSTR szPath)
{
    LOG_FUNC_ENTRY_PARAMS((szPath))
    CTL_TwainDLLHandle::s_ImageDLLFilePath = FixPathString(szPath);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


/* static */ DTWAIN_BOOL SetLangResourcePath(LPCTSTR szPath)
{
    LOG_FUNC_ENTRY_PARAMS((szPath))
    CTL_TwainDLLHandle::s_LangResourcePath = FixPathString(szPath);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTwainAvailable()
{
    LOG_FUNC_ENTRY_PARAMS(())

        // Check if TWAIN session started.  If so, they must have had TWAIN installed!
    // Save the filter flags
    DTWAINScopedLogController sLogContoller(0);
    try
    {
        // Check if DTWAIN already initialized
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
        if (pHandle && DTWAIN_IsSessionEnabled())
        {
            LOG_FUNC_EXIT_PARAMS(true)
        }
    }
    catch (...)
    {
        LOG_FUNC_EXIT_PARAMS(false)
    }

    try
    {
        // DTWAIN initialized, but a TWAIN Session was not started.
        // Do a minimal test here (start a session and unstart it)
        #ifdef WIN64
        DTWAIN_BOOL bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_2);
        #else
        #ifdef WIN32
        DTWAIN_BOOL bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_1);
        #endif
        #endif
        LOG_FUNC_EXIT_PARAMS(true)
    }
    catch(...)
    {
        LOG_FUNC_EXIT_PARAMS(false)
    }
    CATCH_BLOCK(false)
}

// Test which version of TWAIN is available.
LONG DLLENTRY_DEF DTWAIN_GetTwainAvailability()
{
    LOG_FUNC_ENTRY_PARAMS(())
    LONG availableFlag = 0;
    DTWAIN_BOOL bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_1);
    if ( bRet )
        availableFlag |= DTWAIN_TWAINDSM_LEGACY;
    bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_2);
    if ( bRet )
        availableFlag |= DTWAIN_TWAINDSM_VERSION2;
    LOG_FUNC_EXIT_PARAMS(availableFlag)
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
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

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
        LOG_FUNC_EXIT_PARAMS(0)
    LONG nTotalBytes;
    if ( !szDLLName )
    {
        nTotalBytes = static_cast<LONG>(strToSet->length() + 1);
        LOG_FUNC_EXIT_PARAMS(nTotalBytes)
    }

    CTL_StringType sDLLName;
    std::copy(strToSet->begin(), strToSet->end(), std::back_inserter(sDLLName));
    nTotalBytes = StringWrapper::CopyInfoToCString(sDLLName, szDLLName, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetActiveDSMPath(LPTSTR szDLLName, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((szDLLName, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    if (pHandle)
        return StringWrapper::CopyInfoToCString(CTL_TwainAppMgr::GetDSMPath(), szDLLName, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(-1)
    CATCH_BLOCK(-1)
}

DTWAIN_HANDLE DLLENTRY_DEF  DTWAIN_SysInitializeLib(HINSTANCE hInstance)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance))

#ifdef DTWAIN_LIB
    CTL_TwainDLLHandle::s_DLLInstance = hInstance;
#endif
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
    LOG_FUNC_EXIT_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

DTWAIN_HANDLE DLLENTRY_DEF  DTWAIN_SysInitializeLibEx2(HINSTANCE hInstance,
                                                       LPCTSTR szINIPath,
                                                       LPCTSTR szImageDLLPath,
                                                       LPCTSTR szLangResourcePath)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath, szImageDLLPath,szLangResourcePath))

    SetImageDLLPath(szImageDLLPath);
    SetLangResourcePath(szLangResourcePath);

    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLibEx(hInstance, szINIPath);
    LOG_FUNC_EXIT_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

/////////////////////////////////////////////////////////////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibEx(HINSTANCE hInstance, LPCTSTR szINIPath)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath))

    CTL_TwainDLLHandle::s_sINIPath = FixPathString(szINIPath);

    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLib(hInstance);
    LOG_FUNC_EXIT_PARAMS(Handle)
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
    SetImageDLLPath(szImageDLLPath);
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeEx(szINIPath);
    LOG_FUNC_EXIT_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}
/////////////////////////////////////////////////////////////////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx(LPCTSTR szINIPath)
{
    LOG_FUNC_ENTRY_PARAMS((szINIPath))
    CTL_TwainDLLHandle::s_sINIPath = FixPathString(szINIPath);
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
    LOG_FUNC_EXIT_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeNoBlocking()
{
    return SysInitializeHelper(false);
}

DTWAIN_HANDLE SysInitializeHelper(bool block)
{
#ifdef DTWAIN_LIB
    if ( CTL_TwainDLLHandle::s_DLLInstance == NULL )
    {
        // Get the instance handle of the application
        TCHAR szName[1024];
        ::GetModuleFileName(NULL, szName, 1023);

        CTL_TwainDLLHandle::s_DLLInstance = ::GetModuleHandle( szName );
    }
    CTL_TwainDLLHandle::s_lErrorFilterFlags = 0;
    CTL_TwainDLLHandle::s_nRegisteredDTWAINMsg = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
#endif

#if defined (WIN32) || defined(WIN64)
    CTL_TwainDLLHandle::s_bCritSectionCreated = TRUE;
    CTL_TwainDLLHandle::s_bFileCritSectionCreated = TRUE;
    CTL_TwainDLLHandle::s_bCritStaticCreated = TRUE;

#endif

    LOG_FUNC_ENTRY_PARAMS(())

    CTL_TwainDLLHandlePtr pHandlePtr;
    // Return handle if already registered
    const DTWAIN_HANDLE DLLHandle = GetDTWAINHandle_Internal();
    if (DLLHandle)
        LOG_FUNC_EXIT_PARAMS(DLLHandle)

    // This must be checked if this is used in a static library
    #ifdef DTWAIN_LIB
    if ( CTL_TwainDLLHandle::s_DLLInstance == NULL )
        LOG_FUNC_EXIT_PARAMS(NULL)
    #endif

    // Load resources only if first time
    if ( !CTL_TwainDLLHandle::s_ResourcesInitialized )
    {
        typedef std::function<bool()> boolFuncs;
        boolFuncs bf[] = { &LoadTwainResources };
        for (auto& i : bf)
        {
            const bool ret = i();
            if (!ret)
            {
            #ifdef _WIN32
                if ( block )
                    MessageBox(nullptr, _T("DTWAIN Resources not found"), _T("DTWAIN Resource Error"), MB_ICONERROR);
            #endif
                LOG_FUNC_EXIT_PARAMS(NULL)
            }
        }
        CTL_TwainDLLHandle::s_ResourcesInitialized = true;
    }
    // Return a new DTWAIN_HANDLE
    try
    {
        pHandlePtr = std::make_unique<CTL_TwainDLLHandle>();
        CTL_TwainDLLHandle* pHandle = pHandlePtr.get();
        CTL_TwainDLLHandle::s_DLLHandles.push_back(pHandlePtr);
        dynarithmic::CTL_TwainDLLHandle::s_pLoggerCallback  = nullptr;
        dynarithmic::CTL_TwainDLLHandle::s_pLoggerCallbackA = nullptr;
        dynarithmic::CTL_TwainDLLHandle::s_pLoggerCallbackW = nullptr;
        #ifdef _WIN32
        pHandle->m_pSaveAsDlgProc = FileSaveAsHookProc;
        RegisterTwainWindowClass();
        #endif

        // Load the static elements using a critical section
        LoadStaticData(pHandle);

        HookTwainDLL( pHandle, FALSE );
        if ( CTL_TwainDLLHandle::s_DLLHandles.size() == 1 )
        {
            const CTL_StringType iniName = _T(".ini");
            CTL_StringType szName =  StringConversion::Convert_AnsiPtr_To_Native("dtwain" DTWAIN_OSPLATFORM) + iniName;
            const CTL_StringType szLangDLL = _T("english");
            if (!CTL_TwainDLLHandle::s_sINIPath.empty())
            {
                szName = CTL_TwainDLLHandle::s_sINIPath + szName;
            }

            // Initialize the enumerator factory
            if ( !CTL_TwainDLLHandle::s_EnumeratorFactory )
                CTL_TwainDLLHandle::s_EnumeratorFactory = CTL_EnumeratorFactoryPtr(new CTL_EnumeratorFactory);

            // Initialize the resource registry
            pHandle->InitializeResourceRegistry();

            // Load customized resources from INI
            LoadCustomResourcesFromIni(pHandle, szLangDLL.c_str(), szName.c_str());

            // Initialize imaging code
            FreeImage_Initialise(true);

            WriteVersionToLog();
            pHandle->SetVersionString(GetVersionString());

            LOG_FUNC_ENTRY_PARAMS(())
            LOG_FUNC_EXIT_PARAMS(static_cast<DTWAIN_HANDLE>(pHandle))
            CATCH_BLOCK(DTWAIN_HANDLE(0))
        }
        CATCH_BLOCK(DTWAIN_HANDLE(0))
        LOG_FUNC_EXIT_PARAMS(NULL)
    }
    catch (std::exception& ex)
    {
        if ( block )
        {
            MessageBoxA(nullptr, "DTWAIN Initialization Error", ex.what(), MB_ICONERROR);
        }
        else
        {
            dynarithmic::LogToDebugMonitorA("DTWAIN Initialization Error\n");
            dynarithmic::LogToDebugMonitorA(ex.what());
        }
        LOG_FUNC_EXIT_PARAMS(NULL)
    }
    catch(...)
    {
        LOG_FUNC_EXIT_PARAMS(NULL)
    }
}

DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitialize()
{
    return SysInitializeHelper(true);
}

void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL, LPCTSTR szName)
{
    // Load the resources
    CSimpleIniA customProfile;
    customProfile.LoadFile(szName);
    std::string szStr = customProfile.GetValue("Language", "dll",
                                               StringConversion::Convert_NativePtr_To_Ansi(szLangDLL).c_str());
    if (!LoadLanguageResourceA(szStr, pHandle->GetResourceRegistry()))
    {
        // Use the English resources by default
        LoadLanguageResourceA("english", pHandle->GetResourceRegistry());
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
    std::for_each(allIntProfiles, std::end(allIntProfiles), [&](const ProfileSettingsInt& ps)
    {
        const auto nVal = customProfile.GetLongValue(ps.section, ps.name, 0);
        if (nVal != 0)
            CTL_TwainDLLHandle::s_lErrorFilterFlags |= ps.orValue;
    });

    auto nVal = customProfile.GetLongValue("DSMErrorLogging", "EnableNone", 0);
    if (nVal == 1)
        CTL_TwainDLLHandle::s_lErrorFilterFlags = 0;

    nVal = customProfile.GetLongValue("DSMErrorLogging", "EnableAll", 0);
    if (nVal != 0)
        CTL_TwainDLLHandle::s_lErrorFilterFlags = 0xFFFFFFFF & ~DTWAIN_LOG_USEFILE;

    szStr = customProfile.GetValue("DSMErrorLogging", "File", "");
    if (!szStr.empty())
    {
        CTL_TwainDLLHandle::s_lErrorFilterFlags |= DTWAIN_LOG_USEFILE;
        OpenLogFile(StringConversion::Convert_Ansi_To_Native(szStr).c_str(), CTL_TwainDLLHandle::s_lErrorFilterFlags);
        CTL_TwainDLLHandle::s_appLog.StatusOutFast("In DTWAIN_SysInitialize()");
    }

    nVal = customProfile.GetLongValue("DSMErrorLogging", "BufferErrorThreshold", 50);
    if (CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_USEBUFFER)
        DTWAIN_SetErrorBufferThreshold(nVal);

    nVal = customProfile.GetLongValue("DSMErrorLogging", "AppHandlesExceptions", 0);
    CTL_TwainDLLHandle::s_bThrowExceptions = (nVal == 0 ? false : true);
}


void LoadStaticData(CTL_TwainDLLHandle* pHandle)
{
    if ( pHandle->m_mapDTWAINArrayToTwainType.empty() )
    {
        int TwainIntTypes[] = {TWTY_INT8, TWTY_UINT8, TWTY_BOOL, TWTY_INT16, TWTY_INT32, TWTY_UINT16, TWTY_UINT32};
        int TwainStringTypes[] = {TWTY_STR32, TWTY_STR64, TWTY_STR128, TWTY_STR128, TWTY_STR255, TWTY_STR1024};
        int TwainFloatTypes[] = {TWTY_FIX32};
        int TwainFrameTypes[] = {TWTY_FRAME};
        int DTwainArrayTypes[] = {DTWAIN_ARRAYLONG, DTWAIN_ARRAYSTRING, DTWAIN_ARRAYFLOAT, DTWAIN_ARRAYFRAME};
        const int NumTwainTypes[] = {sizeof(TwainIntTypes) / sizeof(TwainIntTypes[0]),
                                     sizeof(TwainStringTypes) / sizeof(TwainStringTypes[0]),
                                     sizeof(TwainFloatTypes) / sizeof(TwainFloatTypes[0]),
                                     sizeof(TwainFrameTypes) / sizeof(TwainFrameTypes[0])
                                    };
        int *intArray[] = {TwainIntTypes, TwainStringTypes, TwainFloatTypes, TwainFrameTypes};

        const size_t nNumAllTypes = sizeof(NumTwainTypes) / sizeof(NumTwainTypes[0]);
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
    CTL_TwainDLLHandle::s_lErrorFilterFlags = LogFlags;
    if ((UserDefinedLoggerExists() || CTL_TwainDLLHandle::s_lErrorFilterFlags ) && (LogFlags == 0))
        CTL_TwainDLLHandle::s_appLog.PrintBanner(false);
    else
    {
        if ( LogFlags && !UserDefinedLoggerExists())
            CTL_TwainDLLHandle::s_lErrorFilterFlags |= DTWAIN_LOG_WRITE;

        const bool append = (LogFlags & DTWAIN_LOG_FILEAPPEND)?true:false;
        OpenLogFile(lpszLogFile, LogFlags, append);

        // Write the version info
        WriteVersionToLog();
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

/*DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainLogFilter(LPCTSTR szTripletFilter, BOOL bIncludeSet)
{
    LOG_FUNC_ENTRY_PARAMS((szTripletFilter, bIncludeSet))
    if ((UserDefinedLoggerExists() || CTL_TwainDLLHandle::s_lErrorFilterFlags) && (LogFlags == 0))
        CTL_TwainDLLHandle::s_appLog.PrintBanner(false);
*/
bool dynarithmic::UserDefinedLoggerExists()
{
    return CTL_TwainDLLHandle::s_pLoggerCallback || CTL_TwainDLLHandle::s_pLoggerCallbackA || CTL_TwainDLLHandle::s_pLoggerCallbackW;
}

void dynarithmic::WriteUserDefinedLogMsg(LPCTSTR sz)
{
    if (CTL_TwainDLLHandle::s_pLoggerCallback)
        CTL_TwainDLLHandle::s_pLoggerCallback(sz, CTL_TwainDLLHandle::s_pLoggerCallback_UserData);
    if (CTL_TwainDLLHandle::s_pLoggerCallbackA)
#ifdef _UNICODE
        WriteUserDefinedLogMsgA(StringConversion::Convert_Native_To_Ansi(sz).c_str());
#else
        WriteUserDefinedLogMsgA(sz);
#endif
    if (CTL_TwainDLLHandle::s_pLoggerCallbackW)
#ifdef _UNICODE
        WriteUserDefinedLogMsgW(sz);
#else
        WriteUserDefinedLogMsgW(StringConversion::Convert_NativePtr_To_Wide(sz).c_str());
#endif
}

void dynarithmic::WriteUserDefinedLogMsgA(LPCSTR sz)
{
    if (CTL_TwainDLLHandle::s_pLoggerCallbackA)
        CTL_TwainDLLHandle::s_pLoggerCallbackA(sz, CTL_TwainDLLHandle::s_pLoggerCallback_UserDataA);

    if (CTL_TwainDLLHandle::s_pLoggerCallback)
#ifdef _UNICODE
        CTL_TwainDLLHandle::s_pLoggerCallback(StringConversion::Convert_Ansi_To_Native(sz).c_str(),
                                              CTL_TwainDLLHandle::s_pLoggerCallback_UserData);
#else
        CTL_TwainDLLHandle::s_pLoggerCallback(sz, CTL_TwainDLLHandle::s_pLoggerCallback_UserData);
#endif
    if (CTL_TwainDLLHandle::s_pLoggerCallbackW)
#ifdef _UNICODE
        CTL_TwainDLLHandle::s_pLoggerCallbackW(StringConversion::Convert_Ansi_To_Native(sz).c_str(),
                                               CTL_TwainDLLHandle::s_pLoggerCallback_UserDataW);
#else
        CTL_TwainDLLHandle::s_pLoggerCallbackW(StringConversion::Convert_Native_To_Wide(sz).c_str(),
                                                CTL_TwainDLLHandle::s_pLoggerCallback_UserDataW);
#endif
}

void dynarithmic::WriteUserDefinedLogMsgW(LPCWSTR sz)
{
    if (CTL_TwainDLLHandle::s_pLoggerCallbackW)
        CTL_TwainDLLHandle::s_pLoggerCallbackW(sz, CTL_TwainDLLHandle::s_pLoggerCallback_UserDataW);

    if (CTL_TwainDLLHandle::s_pLoggerCallback)
#ifdef _UNICODE
        CTL_TwainDLLHandle::s_pLoggerCallback(sz, CTL_TwainDLLHandle::s_pLoggerCallback_UserData);
#else
        CTL_TwainDLLHandle::s_pLoggerCallback(StringConversion::Convert_Wide_To_Native(sz).c_str(),
                                              CTL_TwainDLLHandle::s_pLoggerCallback_UserData);
#endif
    if (CTL_TwainDLLHandle::s_pLoggerCallbackA)
#ifdef _UNICODE
        CTL_TwainDLLHandle::s_pLoggerCallbackA(StringConversion::Convert_Native_To_Ansi(sz).c_str(),
                                               CTL_TwainDLLHandle::s_pLoggerCallback_UserDataA);
#else
        CTL_TwainDLLHandle::s_pLoggerCallbackA(StringConversion::Convert_Wide_To_Native(sz).c_str(),
                                               CTL_TwainDLLHandle::s_pLoggerCallback_UserDataA);
#endif
}

void OpenLogFile(LPCTSTR pFileName, LONG logFlags, bool bAppend)
{
    if ( pFileName && pFileName[0] )
        CTL_TwainDLLHandle::s_appLog.InitFileLogging(pFileName, CTL_TwainDLLHandle::s_DLLInstance, bAppend );
    if ( logFlags & DTWAIN_LOG_CONSOLE)
        CTL_TwainDLLHandle::s_appLog.InitConsoleLogging(CTL_TwainDLLHandle::s_DLLInstance);
    if ( logFlags & DTWAIN_LOG_DEBUGMONITOR)
        CTL_TwainDLLHandle::s_appLog.InitDebugWindowLogging(CTL_TwainDLLHandle::s_DLLInstance);
    if ( !(logFlags & DTWAIN_LOG_NOCALLBACK) )
        CTL_TwainDLLHandle::s_appLog.InitCallbackLogging(CTL_TwainDLLHandle::s_DLLInstance);
    CTL_TwainDLLHandle::s_appLog.PrintTime(true);
    CTL_TwainDLLHandle::s_appLog.PrintAppName(true);
    CTL_TwainDLLHandle::s_appLog.PrintBanner();
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCountry(LONG nCountry)
{
    LOG_FUNC_ENTRY_PARAMS((nCountry))
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_SessionStruct.nCountry = static_cast<TW_UINT16>(nCountry);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetLanguage(LONG nLanguage)
{
    LOG_FUNC_ENTRY_PARAMS((nLanguage))
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_SessionStruct.nLanguage = static_cast<TW_UINT16>(nLanguage);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetCountry()
{
    LOG_FUNC_ENTRY_PARAMS(())
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, DTWAIN_FAILURE1, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(pHandle->m_SessionStruct.nCountry)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG DLLENTRY_DEF DTWAIN_GetLanguage()
{
    LOG_FUNC_ENTRY_PARAMS(())
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, DTWAIN_FAILURE1, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(pHandle->m_SessionStruct.nLanguage)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetAppInfo(LPCTSTR szVerStr, LPCTSTR szManu, LPCTSTR szProdFam, LPCTSTR szProdName)
{
    LOG_FUNC_ENTRY_PARAMS((szVerStr, szManu, szProdFam, szProdName))
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if ( szVerStr )
        pHandle->m_SessionStruct.szVersion = szVerStr;
    if ( szManu )
        pHandle->m_SessionStruct.szManufact = szManu;
    if ( szProdFam )
        pHandle->m_SessionStruct.szFamily = szProdFam;
    if ( szProdName )
        pHandle->m_SessionStruct.szProduct = szProdName;
    LOG_FUNC_EXIT_PARAMS(true)
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
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if ( szVerStr )
        std::copy(pHandle->m_SessionStruct.szVersion.begin(), pHandle->m_SessionStruct.szVersion.end(), szVerStr);
    if ( szManu )
        std::copy(pHandle->m_SessionStruct.szManufact.begin(), pHandle->m_SessionStruct.szManufact.end(), szManu);
    if ( szProdFam )
        std::copy(pHandle->m_SessionStruct.szFamily.begin(), pHandle->m_SessionStruct.szFamily.end(), szProdFam);
    if ( szProdName )
        std::copy(pHandle->m_SessionStruct.szProduct.begin(), pHandle->m_SessionStruct.szProduct.end(), szProdName);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DTWAIN_SetSourceCloseMode(LONG lCloseMode)
{
    LOG_FUNC_ENTRY_PARAMS((lCloseMode))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_nSourceCloseMode = lCloseMode?true:false;
    LOG_FUNC_EXIT_PARAMS(TRUE)
    CATCH_BLOCK(FALSE)
}

LONG DTWAIN_GetSourceCloseMode()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(pHandle->m_nSourceCloseMode)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSessionEnabled()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if ( pHandle->m_bSessionAllocated )
        LOG_FUNC_EXIT_PARAMS(true)
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartTwainSession(HWND hWndMsgNotify, LPCTSTR lpszDLLName)
{
    LOG_FUNC_ENTRY_PARAMS((hWndMsgNotify, lpszDLLName))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if ( pHandle->m_bSessionAllocated )
        LOG_FUNC_EXIT_PARAMS(true)

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
        hInstance = CTL_TwainDLLHandle::s_DLLInstance;

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
        hInstance = CTL_TwainDLLHandle::s_DLLInstance;
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
    hInstance = CTL_TwainDLLHandle::s_DLLInstance;
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
                                      CTL_TwainDLLHandle::s_DLLInstance,
                                      lpszDLLName?sDLLName.c_str():nullptr) )
        {
            if ( pHandle->m_SessionStruct.nSessionType == DTWAIN_TWAINDSM_LATESTVERSION )
               DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1;}, DTWAIN_ERR_TWAINOPENSOURCEDSMNOTFOUND, false, FUNC_MACRO);
            DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_TWAIN32DSMNOTFOUND, false, FUNC_MACRO);
        }
    }

    // Create a session
    auto Session =
        CTL_TwainAppMgr::CreateTwainSession( szName.c_str(), &hWndMsg,
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
    CTL_TwainAppMgr::SetDLLInstance( CTL_TwainDLLHandle::s_DLLInstance );
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
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

static DTWAIN_ARRAY GetFileTypes(int nType)
{
    const char *sNames[] = { "","-SinglePage","-Multipage" };
    DTWAIN_ARRAY aFileTypes = DTWAIN_ArrayCreate(DTWAIN_ARRAYLONG, 0);
    if (aFileTypes)
    {
        const auto& availableFileTypes = CTL_TwainDLLHandle::GetAvailableFileFormatsMap();
        for (auto& pr : availableFileTypes)
        {
            if (StringWrapperA::EndsWith(pr.second.m_formatName, sNames[nType]))
                DTWAIN_ArrayAddLong(aFileTypes, pr.first);
        }
    }
    return aFileTypes;
}

static std::string GetFileTypeExtensionsInternal(int nType)
{
    const auto& availableFileTypes = CTL_TwainDLLHandle::GetAvailableFileFormatsMap();
    const auto iter = availableFileTypes.find(nType);
    if ( iter != availableFileTypes.end())
        return StringWrapperA::Join(iter->second.m_vExtensions, "|");
    return {};
}

static std::string GetFileTypeNameInternal(int nType)
{
    const auto& availableFileTypes = CTL_TwainDLLHandle::GetAvailableFileFormatsMap();
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
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(GetFileTypeInfo(&GetFileTypeNameInternal, nType, lpszName, nMaxLen))
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetFileTypeExtensions(LONG nType, LPTSTR lpszName, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((nType, lpszName, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(GetFileTypeInfo(&GetFileTypeExtensionsInternal, nType, lpszName, nMaxLen))
    CATCH_BLOCK(-1)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedFileTypes(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, nullptr, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(GetFileTypes(0))
    CATCH_BLOCK(nullptr)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedMultiPageFileTypes(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, nullptr, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(GetFileTypes(2))
    CATCH_BLOCK(nullptr)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_EnumSupportedSinglePageFileTypes(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, nullptr, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(GetFileTypes(1))
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


static bool HookTwainDLL(CTL_TwainDLLHandle* pHandle, bool /*bHook*/)
{
    if ( !pHandle )
         return false;
    const DWORD hTask = getThreadId();

    // Get the pointers to the hook functions
    const HHOOK proc1 = nullptr;
    const HHOOK proc2 = nullptr;

    const CTL_HookInfo Info( hTask, proc1, proc2, pHandle, false,0,0 );
    pHandle->s_aHookInfo.push_back( Info );

    return true;
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

static bool FindTask( DWORD hTask, int *pWhere )
{
    if (pWhere)
        *pWhere = -1;

    const CTL_HookInfoArray::iterator it = std::find_if(CTL_TwainDLLHandle::s_aHookInfo.begin(), CTL_TwainDLLHandle::s_aHookInfo.end(), [&](const CTL_HookInfo& Info)
                                                        { return std::get<0>(Info) == hTask; });
    if ( it != CTL_TwainDLLHandle::s_aHookInfo.end() )
    {
        if ( pWhere )
            *pWhere = static_cast<int>(std::distance(CTL_TwainDLLHandle::s_aHookInfo.begin(), it));
        return true;
    }
    return false;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndTwainSession()
{
    LOG_FUNC_ENTRY_PARAMS(())
    // Delete it
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{return (DTWAIN_IsAcquiring()==1);}, DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

    // Check if any source is still acquiring
    if ( pHandle->m_nSourceCloseMode == DTWAIN_SourceCloseModeFORCE )
    {
        // Close any sources
        DTWAIN_CloseAllSources();
    }
    else
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return (DTWAIN_IsAcquiring()==1);}, DTWAIN_ERR_SOURCE_ACQUIRING, false, FUNC_MACRO);

    if ( !pHandle->m_bSessionAllocated )
        LOG_FUNC_EXIT_PARAMS(true)

    if ( 1 ) //CTL_TwainDLLHandle::s_DLLHandles.size() == 1 )
    {
        try
        {
            CTL_TwainAppMgr::CloseSourceManager(pHandle->m_pTwainSession);
        }
        catch(...)
        {
            CTL_TwainAppMgr::WriteLogInfoA("Error in closing Twain DSM\n");
        }

        try
        {
            CTL_TwainAppMgr::DestroyTwainSession(pHandle->m_pTwainSession);
        }
        catch(...)
        {
            CTL_TwainAppMgr::WriteLogInfoA("Error in closing Twain Session\n");
        }
        if ( CTL_TwainDLLHandle::s_DLLHandles.size() == 1 )
        {
            try
            {
                CTL_TwainAppMgr::Destroy();
            }
            catch(...)
            {
                CTL_TwainAppMgr::WriteLogInfoA("Error in closing DTWAIN Manager\n");
            }
        }
        CTL_TwainAppMgr::WriteLogInfoA("TWAIN Source Manager, Session, and DTWAIN closed successfully\n");
    }

    // Remove the hook
    // Close the window (Dummy window may have been created)
    pHandle->m_bSessionAllocated = false;
    #ifdef _WIN32
    if ( pHandle->m_bUseProxy ) //hWndTwain )
    {
        pHandle->m_CallbackMsg = nullptr;
        if ( ::IsWindow(pHandle->m_hWndTwain))
            try {
            ::DestroyWindow(pHandle->m_hWndTwain);
        }
        catch(...) {}
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
            CTL_TwainAppMgr::WriteLogInfoA("Could not remove subclass from user window\n");
        }
    }
    pHandle->m_hWndTwain = nullptr;
    pHandle->m_hOrigProc = nullptr;
    #endif
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SysDestroy()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if (!DTWAIN_EndTwainSession())
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = SysDestroyHelper(pHandle);

#ifdef DTWAIN_DEBUG_CALL_STACK
    if (CTL_TwainDLLHandle::s_lErrorFilterFlags)
        CTL_LogFunctionCall(__FUNC__, 1);
#endif

#if defined (WIN32) || (WIN64)
        CTL_TwainDLLHandle::s_bCritSectionCreated = FALSE;
        CTL_TwainDLLHandle::s_bFileCritSectionCreated = FALSE;
        CTL_TwainDLLHandle::s_bCritStaticCreated =FALSE;
#endif
    return bRet;
    CATCH_BLOCK(false)
}

static bool SysDestroyHelper(CTL_TwainDLLHandle* pHandle, bool bCheck)
{
    #ifdef _WIN32
    // Unload the OCR interfaces
    UnloadOCRInterfaces(pHandle);
    #endif
    // Remove the handle
    int nWhere;

    std::vector<CTL_TwainDLLHandlePtr>::iterator it = find_if(CTL_TwainDLLHandle::s_DLLHandles.begin(),
                                                         CTL_TwainDLLHandle::s_DLLHandles.end(),
                                                         SmartPointerFinder<CTL_TwainDLLHandlePtr>(pHandle));
    if ( it == CTL_TwainDLLHandle::s_DLLHandles.end() )
        return false;

    UnhookAllDisplays();
    // Remove the function hooks
    const DWORD hTask = getThreadId();

    bool bOk;
#ifdef DTWAIN_LIB
    bOk = true;
    nWhere = 0;
#else
    bOk = ::FindTask( hTask, &nWhere );
#endif
    if ( bOk || !bCheck )
        pHandle->s_aHookInfo.erase(pHandle->s_aHookInfo.begin() +  nWhere );

    // Remove the handle
    pHandle->m_CallbackMsg   = nullptr;
    pHandle->m_CallbackError = nullptr;
    CTL_TwainDLLHandle::s_DLLHandles.erase( it );

    #ifdef DTWAIN_LIB
    if ( CTL_TwainDLLHandle::s_DLLHandles.empty() )
    {
        // Only unload everything if no more instances of DTWAIN exists.
        CTL_TwainDLLHandle::s_aAcquireNum.clear();
        CTL_TwainDLLHandle::s_EnumeratorFactory.reset();
    }
    #else
    if ( CTL_TwainDLLHandle::s_DLLHandles.empty() )
    {
        CTL_TwainDLLHandle::s_aAcquireNum.clear();
        // remove enumerator factory
        CTL_TwainDLLHandle::s_EnumeratorFactory.reset();
    }
    #endif
    FreeImage_DeInitialise();

    return TRUE;
}

/* This function tests all open DLL handles to see if any source is acquiring */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsAcquiring()
{
    LOG_FUNC_ENTRY_PARAMS(())
    const CTL_HookInfoArray::iterator it = std::find_if(CTL_TwainDLLHandle::s_aHookInfo.begin(),
                                                            CTL_TwainDLLHandle::s_aHookInfo.end(), [&](const CTL_HookInfo& hookInfo)
                                                            {
                                                                CTL_TwainDLLHandle* pHandle = std::get<3>(hookInfo);
                                                                if (!pHandle)
                                                                    return false;

                                                                const CTL_StringToSourcePtrMap::iterator it = std::find_if(pHandle->m_mapStringToSource.begin(),
                                                                                                                           pHandle->m_mapStringToSource.end(), [&](const CTL_StringToSourcePtrMap::value_type& vt) {return (vt.second)->IsAcquireAttempt(); });

                                                                return (it != pHandle->m_mapStringToSource.end());
                                                            });
    if ( it != CTL_TwainDLLHandle::s_aHookInfo.end() )
         LOG_FUNC_EXIT_PARAMS(true)
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

HWND  DLLENTRY_DEF  DTWAIN_GetTwainHwnd()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(pHandle->m_hWndTwain)
    CATCH_BLOCK(HWND(0))
}

LONG DTWAIN_CloseAllSources()
{
    LOG_FUNC_ENTRY_PARAMS(())

    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, -1L, FUNC_MACRO);
    if ( pHandle->m_mapStringToSource.empty() )
        LOG_FUNC_EXIT_PARAMS(0)
    CTL_StringToSourcePtrMap m_mapTemp = pHandle->m_mapStringToSource;
    std::for_each(m_mapTemp.begin(), m_mapTemp.end(), [&](CTL_StringToSourcePtrMap::value_type& vt)
    {
        CTL_ITwainSource *pSource = vt.second;
        if (pSource->IsAcquireAttempt())
            CTL_TwainAppMgr::DisableUserInterface(pSource);
        DTWAIN_CloseSource(pSource);
    });

    LOG_FUNC_EXIT_PARAMS(0)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainMode()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, -1L, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(pHandle->m_lAcquireMode)
    CATCH_BLOCK(-1L)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainMode(LONG lMode)
{
    LOG_FUNC_ENTRY_PARAMS((lMode))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);

    switch (lMode)
    {
        case DTWAIN_MODELESS:
        case DTWAIN_MODAL:
            pHandle->m_lAcquireMode = lMode;
            LOG_FUNC_EXIT_PARAMS(true)
        break;
    }
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return 0;}, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTIFFSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPDFSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsPNGSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsJPEGSupported(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetTwainDSM(LONG DSMType)
{
    LOG_FUNC_ENTRY_PARAMS((DSMType))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex( pHandle, false, FUNC_MACRO);
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
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrder(LONG SearchOrder)
{
    LOG_FUNC_ENTRY_PARAMS((SearchOrder))
    CTL_TwainDLLHandle::s_TwainDSMSearchOrder = SearchOrder;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetDSMSearchOrder(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const LONG SearchOrder = CTL_TwainDLLHandle::s_TwainDSMSearchOrder;
    LOG_FUNC_EXIT_PARAMS(SearchOrder)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderEx(LPCTSTR SearchOrder, LPCTSTR UserDirectory)
{
    LOG_FUNC_ENTRY_PARAMS((SearchOrder))
    const std::string strValidString = CheckSearchOrderString(StringConversion::Convert_NativePtr_To_Ansi(SearchOrder));
    if ( !strValidString.empty() )
    {
        CTL_TwainDLLHandle::s_TwainDSMSearchOrderStr = strValidString;
        CTL_TwainDLLHandle::s_TwainDSMUserDirectory = UserDirectory?UserDirectory:StringWrapper::traits_type::GetEmptyString();
        CTL_TwainDLLHandle::s_TwainDSMSearchOrder = -1;
        LOG_FUNC_EXIT_PARAMS(TRUE)
    }
    LOG_FUNC_EXIT_PARAMS(FALSE)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePath(LPCTSTR ResourcePath)
{
    LOG_FUNC_ENTRY_PARAMS((ResourcePath))
    CTL_TwainDLLHandle::s_strResourcePath = ResourcePath;
    LOG_FUNC_EXIT_PARAMS(TRUE)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_CallCallback(WPARAM wParam, LPARAM lParam, LONG UserData)
{
    LOG_FUNC_ENTRY_PARAMS((wParam, lParam, UserData))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    LONG RetVal = 1;
    if (pHandle->m_pCallbackFn)
        RetVal = static_cast<LONG>((*pHandle->m_pCallbackFn)(wParam, lParam, UserData));
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(false)
}

void UnhookAllDisplays()
{
#ifdef _WIN32
    if (CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_CONSOLE)
        FreeConsole();
#endif
}

void dynarithmic::OutputDTWAINErrorW(CTL_TwainDLLHandle* pHandle, LPCWSTR pFunc)
{
    if ( pFunc )
        OutputDTWAINError(pHandle, StringConversion::Convert_Wide_To_Ansi(pFunc).c_str());
    else
        OutputDTWAINError(pHandle);
}

void dynarithmic::OutputDTWAINErrorA(CTL_TwainDLLHandle* pHandle, LPCSTR pFunc)
{
    OutputDTWAINError(pHandle, pFunc);
}

void dynarithmic::OutputDTWAINError(CTL_TwainDLLHandle* pHandle, LPCSTR pFunc)
{
    if (!CTL_TwainDLLHandle::s_lErrorFilterFlags )
        return;
    static constexpr int MaxMessage=1024;
    char szBuf[MaxMessage+1];
    if ( !pHandle )
        DTWAIN_GetErrorStringA( DTWAIN_ERR_BAD_HANDLE, szBuf,MaxMessage);
    else
        DTWAIN_GetErrorStringA( pHandle->m_lLastError, szBuf, MaxMessage);
    std::string s(szBuf);
    s += "\n";
    if ( !pHandle || pHandle->s_bProcessError)
        CTL_TwainAppMgr::WriteLogInfoA(s);

    if ( CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_ERRORMSGBOX && pHandle)
        LogDTWAINErrorToMsgBox(pHandle->m_lLastError, pFunc, s);
    else
    if ( !pHandle && (CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_INITFAILURE ))
        LogDTWAINErrorToMsgBox(DTWAIN_ERR_BAD_HANDLE, nullptr, s);
}



DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AppHandlesExceptions(DTWAIN_BOOL bSet)
{
    CTL_TwainDLLHandle::s_bThrowExceptions = (bSet?true:false);
    return TRUE;
}


std::string GetStaticLibVer()
{
    const LONG nVer = DTWAIN_GetStaticLibVersion();
    if ( nVer != 0 )
    {
        switch (nVer)
        {
            case 10:
                return "Microsoft Visual C++ 5.0";
            case 11:
                return "Microsoft Visual C++ 6.0";
            case 12:
                return "Microsoft Visual C++ 7.0";
            case 18:
                return "Microsoft Visual Studio 2003";
            case 20:
                return "Microsoft Visual Studio 2005";
            case 21:
                return "Microsoft Visual Studio 2008";
            case 31:
                return "Microsoft Visual Studio 2010";
            case 41:
                return "Microsoft Visual Studio 2012";
            case 51:
                return "Microsoft Visual Studio 2013";
            case 22:
                return "gcc/MingW";
            case 24:
                return "Metrowerks CodeWarrior";
            case 26:
                return "Borland C++ Builder";
            case 28:
                return "Borland C++ 5.0";
            case 30:
                return "Digital Mars";
        }
    }
    return {};
}

LONG DLLENTRY_DEF DTWAIN_GetVersionString(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetVersionString(), lpszVer, nLength);
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetLibraryPath(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetDTWAINDLLPath(), lpszVer, nLength);
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetShortVersionString(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetDTWAINDLLVersionInfoStr(), lpszVer, nLength);
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetVersionInfo(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = StringWrapper::CopyInfoToCString(dynarithmic::GetVersionInfo(), lpszVer, nLength);
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainStringName(LONG category, LONG TwainID, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((category, TwainID, lpszBuffer, nMaxLen))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    std::string sValue;
    switch (category)
    {
        case DTWAIN_DGNAME:
            sValue = CTL_TwainDLLHandle::GetTwainNameFromResource(CTL_TwainDLLHandle::GetDGResourceID(), static_cast<int>(TwainID));
        break;
        case DTWAIN_DATNAME:
            sValue = CTL_TwainDLLHandle::GetTwainNameFromResource(CTL_TwainDLLHandle::GetDATResourceID(), static_cast<int>(TwainID));
        break;
        case DTWAIN_MSGNAME:
            sValue = CTL_TwainDLLHandle::GetTwainNameFromResource(CTL_TwainDLLHandle::GetMSGResourceID(), static_cast<int>(TwainID));
        break;
    }
    const LONG RetVal = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(sValue), lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetTwainIDFromName(LPCTSTR lpszBuffer)
{
    LOG_FUNC_ENTRY_PARAMS((lpszBuffer))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    const LONG RetVal = pHandle->GetIDFromTwainName(StringConversion::Convert_NativePtr_To_Ansi(lpszBuffer));
    LOG_FUNC_EXIT_PARAMS(RetVal)
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
    if ( !CTL_TwainDLLHandle::s_DLLPath.empty())
        return CTL_TwainDLLHandle::s_DLLPath;
    TCHAR buffer[1024];
    boost::winapi::GetModuleFileName(CTL_TwainDLLHandle::s_DLLInstance, buffer, 1024);
    return buffer;
}

CTL_StringType dynarithmic::GetVersionString()
{
    if (!CTL_TwainDLLHandle::s_VersionString.empty())
        return CTL_TwainDLLHandle::s_VersionString;

    LONG lMajor, lMinor, lVersionType, lPatch;
    // Write the version info
    if ( DTWAIN_GetVersionEx(&lMajor, &lMinor, &lVersionType, &lPatch) )
    {
        std::string s = "";
        std::string sBits = "(32-bit)";
        if ( lVersionType & DTWAIN_64BIT_VERSION )
            sBits = "(64-bit)";

        if (lVersionType & DTWAIN_UNICODE_VERSION)
            s += " [Unicode Version]";

        if (lVersionType & DTWAIN_DEVELOP_VERSION)
            s += " [Debug]";

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
            << lPatch << ")\n" << "Shared Library Path: " <<  StringConversion::Convert_Native_To_Ansi(GetDTWAINDLLPath());
        CTL_TwainDLLHandle::s_VersionString = StringConversion::Convert_Ansi_To_Native(strm.str());
        return CTL_TwainDLLHandle::s_VersionString;
    }
    return {};
}

void WriteVersionToLog()
{
    std::string ansiVer;
    if (CTL_TwainDLLHandle::s_lErrorFilterFlags)
    {
        auto sVer = GetVersionString();
        const auto sWinVer = GetWinVersion();
        sVer += _T("\n") + sWinVer;
        #ifdef _WIN32
        // All log messages must be ANSI
        ansiVer = StringConversion::Convert_Native_To_Ansi(sVer);
        if (CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_USEFILE)
        {
            if (!CTL_TwainDLLHandle::s_appLog.StatusOutFast(ansiVer.c_str()))
            {
                ansiVer += "\n";
                dynarithmic::LogToDebugMonitorA(ansiVer.c_str());
            }
        }
        else
        if (CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_DEBUGMONITOR)
        {
            sVer += _T("\n");
            dynarithmic::LogToDebugMonitor(sVer.c_str());
        }
        if ( UserDefinedLoggerExists() )
            WriteUserDefinedLogMsgA(ansiVer.c_str());
        #endif
    }
}

void dynarithmic::DTWAIN_InternalThrowException() THIS_FUNCTION_THROWS
{
    throw;
}

/////////////////////////////// Describe Path functions
struct ANSIPathTraits
{
    static LPSTR PathAddBackslashFn(LPSTR path) { return ANSIStringTraits::AddBackslashImpl(path); }
};

struct UNICODEPathTraits
{
    static LPWSTR PathAddBackslashFn(LPWSTR path) { return UnicodeStringTraits::AddBackslashImpl(path); }
};

template <typename CHARTYPE,
          typename STRINGTYPE,
          typename POINTERTYPE,
          typename FN>
STRINGTYPE FixPathString_Impl(POINTERTYPE szINIPath, size_t numChars)
{
    if ( szINIPath )
    {
        std::vector<CHARTYPE> vec(FILENAME_MAX,0);
        size_t minCopy = (std::min)(static_cast<int>(numChars), FILENAME_MAX);
        std::copy(szINIPath, szINIPath + minCopy, vec.begin());
        FN::PathAddBackslashFn(&vec[0]);
        return &vec[0];
    }
    return {};
}

std::string FixPathStringA(LPCSTR szINIPath)
{
    if ( szINIPath)
    {
        const size_t numChars = strlen(szINIPath);
        return FixPathString_Impl<char, std::string, LPCSTR, ANSIPathTraits>(szINIPath, numChars);
    }
    return "";
}

std::wstring FixPathStringW(LPCWSTR szINIPath)
{
    if ( szINIPath)
    {
        const size_t numChars = wcslen(szINIPath);
        return FixPathString_Impl<wchar_t, std::wstring, LPCWSTR, UNICODEPathTraits>(szINIPath, numChars);
    }
    return L"";
}

bool GetDTWAINDLLVersionInfo(HMODULE hMod, LONG* lMajor, LONG* lMinor, LONG *pPatch)
{
    if ( lMajor )
        *lMajor = DTWAIN_MAJOR_VERSION;
    if (lMinor)
        *lMinor = DTWAIN_SUBVERSION_VERSION;
    if (pPatch)
        *pPatch = DTWAIN_VERINFO_PATCHLEVEL_VERSION;
    return true;
}

CTL_StringType GetDTWAINDLLVersionInfoStr()
{
    std::ostringstream strm;
    strm << DTWAIN_MAJOR_VERSION << "." << DTWAIN_SUBVERSION_VERSION << "." << DTWAIN_PLACEHOLDER_VERSION << "." << DTWAIN_VERINFO_PATCHLEVEL_VERSION;
    return StringConversion::Convert_Ansi_To_Native(strm.str());
}

CTL_StringType& dynarithmic::GetDTWAINTempFilePath()
{
    if ( CTL_TwainDLLHandle::s_TempFilePath.empty())
    {
        const auto tempPath = dynarithmic::temp_directory_path();
        if (tempPath.empty())
            CTL_TwainAppMgr::WriteLogInfoA("Error: Temp directory does not exist\n");
        else
            CTL_TwainDLLHandle::s_TempFilePath = tempPath;
    }
    CTL_TwainAppMgr::WriteLogInfoA("Temp path is " +  StringConversion::Convert_Native_To_Ansi(CTL_TwainDLLHandle::s_TempFilePath) + "\n");
    return CTL_TwainDLLHandle::s_TempFilePath;
}


std::string CheckSearchOrderString(std::string str)
{
    static std::set<char> setValidChars = {'C','W','O','U','S'};
    std::string strOut;
    StringWrapperA::MakeUpperCase(str);
    std::copy_if(str.begin(), str.end(), std::back_inserter(strOut), [&](char ch) { return setValidChars.count(ch); });
    return strOut;
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
    ver += _T(DTWAIN_VERINFO_PATCHLEVEL_VERSION);
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
