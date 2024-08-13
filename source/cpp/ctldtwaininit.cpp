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
#include "ctltwmgr.h"
#include "ctlres.h"
#include "../dtwinver/dtwinverex.h"
#include "../simpleini/simpleini.h"
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

#ifdef DTWAIN_DEMO_VERSION
    #include "splash.h"
#endif

#ifdef USE_NAMESPACES

#endif

using namespace dynarithmic;

static DTWAIN_HANDLE SysInitializeHelper(bool noblock);
static LONG DTWAIN_CloseAllSources();
static void UnhookAllDisplays();
static bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId);
static bool RemoveThreadIdFromAssociation(unsigned long threadId);
static HWND CreateTwainWindow(CTL_TwainDLLHandle* pHandle,
                              HINSTANCE hInstance= nullptr,
                              HWND hWndParent= nullptr);
static void RegisterTwainWindowClass();
static void OpenLogFile(LPCTSTR pFileName, LONG logFlags, bool append=false);
static void WriteVersionToLog();
static bool SysDestroyHelper(CTL_TwainDLLHandle* pHandle, bool bCheck=true);
static void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL);
static void LoadTransferReadyOverrides();
static void LoadFlatbedOnlyOverrides();
static bool LoadGeneralResources(bool blockExecution);

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

static bool FindTask( DWORD hTask );
static HMODULE GetDLLInstance();

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
                                    _T(DTWAIN_PATCHLEVEL_VERSION);
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
    const bool modRet = GetDTWAINDLLVersionInfo(CTL_StaticData::s_DLLInstance, lMajor, lMinor, lPatch);
     if ( !modRet )
     {
         LOG_FUNC_EXIT_PARAMS(false)
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
            #pragma message ("Microsoft Visual Studio 2017 compiler used to build library")
            LOG_FUNC_EXIT_PARAMS(71)
        #elif _MSC_VER >= 1920
            #pragma message ("Microsoft Visual Studio 2019 compiler used to build library")
            LOG_FUNC_EXIT_PARAMS(81)
        #elif _MSC_VER >= 1930
            #pragma message ("Microsoft Visual Studio 2022 compiler used to build library")
            LOG_FUNC_EXIT_PARAMS(91)
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
    if (!IsDLLHandleValid(reinterpret_cast<CTL_TwainDLLHandle*>(pHandle), FALSE))
        LOG_FUNC_EXIT_PARAMS(retVal);
    retVal = IsDLLHandleValid(reinterpret_cast<CTL_TwainDLLHandle*>(pHandle), TRUE) ? DTWAIN_TWAINSESSIONOK : DTWAIN_APIHANDLEOK;
    LOG_FUNC_EXIT_PARAMS(retVal);
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
    LOG_FUNC_ENTRY_PARAMS(())
    // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if ( !IsDLLHandleValid( pHandle, FALSE ) )
    {
        LONG err = DTWAIN_ERR_BAD_HANDLE;
        if (!CTL_StaticData::s_ResourcesInitialized)
            err = DTWAIN_ERR_RESOURCES_NOT_FOUND;
        LOG_FUNC_EXIT_PARAMS(err)
    }
    else
    {
    }
    LOG_FUNC_EXIT_PARAMS(pHandle->m_lLastError)
    CATCH_BLOCK(DTWAIN_ERR_BAD_HANDLE)
}

static LONG GetResourceStringInternal(LONG resourceID, LPTSTR lpszBuffer, LONG nMaxLen)
{
    resourceID = std::abs(resourceID);
    const size_t nBytes = GetResourceStringA(static_cast<UINT>(resourceID), nullptr, 2048);
    if (nBytes == 0)
    {
        // Copy the error number to the buffer if we haven't been able to find the 
        // resource string
        const CTL_StringType sCopy = StringWrapper::ToString(resourceID);
        return StringWrapper::CopyInfoToCString(sCopy, lpszBuffer, nMaxLen);
    }

    size_t nAdditionalBytes = 0;
    auto iter = CTL_StaticData::s_mapExtraErrorInfo.find(resourceID);
    if (iter != CTL_StaticData::s_mapExtraErrorInfo.end())
        nAdditionalBytes += iter->second.size();

    std::vector<char> szTemp(nBytes);
    GetResourceStringA(static_cast<UINT>(resourceID), &szTemp[0], static_cast<LONG>(nBytes));
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
    const CTL_StringType sCopy = StringConversion::Convert_Ansi_To_Native(szTemp.data());

    return StringWrapper::CopyInfoToCString(sCopy, lpszBuffer, nMaxLen);
}

LONG DLLENTRY_DEF  DTWAIN_GetResourceString(LONG ResourceID, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((ResourceID, lpszBuffer, nMaxLen))
    auto nTotalBytes = GetResourceStringInternal(ResourceID, lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF  DTWAIN_GetErrorString(LONG lError, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((lError, lpszBuffer, nMaxLen))
    auto nTotalBytes = GetResourceStringInternal(lError, lpszBuffer, nMaxLen);
    LOG_FUNC_EXIT_PARAMS(nTotalBytes)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_SetLastError(LONG nError)
{
    LOG_FUNC_ENTRY_PARAMS(())
        // See if DLL Handle exists
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    if (!IsDLLHandleValid(pHandle, FALSE))
    {
        LONG err = DTWAIN_ERR_BAD_HANDLE;
        if (!CTL_StaticData::s_ResourcesInitialized)
            err = DTWAIN_ERR_RESOURCES_NOT_FOUND;
        LOG_FUNC_EXIT_PARAMS(err)
    }
    pHandle->m_lLastError = nError;
    LOG_FUNC_EXIT_PARAMS(DTWAIN_NO_ERROR)
    CATCH_BLOCK(-1)
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

LONG DLLENTRY_DEF DTWAIN_GetTwainNameFromConstant(LONG lConstantType, LONG lTwainConstant, LPTSTR lpszOut, LONG nSize)
{
    LOG_FUNC_ENTRY_PARAMS((lConstantType, lTwainConstant, lpszOut, nSize))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    auto& constantsmap = CTL_StaticData::GetTwainConstantsMap();
    auto iter1 = constantsmap.find(lConstantType);
    if (iter1 == constantsmap.end())
        LOG_FUNC_EXIT_PARAMS(0)
    auto iter2 = iter1->second.find(lTwainConstant);
    if (iter2 == iter1->second.end())
        LOG_FUNC_EXIT_PARAMS(0)
    LONG nActualCharactersCopied = 0;
    CTL_StringType constantName = StringConversion::Convert_Ansi_To_Native(iter2->second);
    nActualCharactersCopied = StringWrapper::CopyInfoToCString(constantName, lpszOut, nSize);
    LOG_FUNC_EXIT_PARAMS(nActualCharactersCopied)
    CATCH_BLOCK(-1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsInitialized()
{
    // Get the Current task
    LOG_FUNC_ENTRY_PARAMS(())
    const DWORD hTask = getThreadId();

#ifdef DTWAIN_LIB
    if ( CTL_StaticData::s_DLLInstance == NULL )
        LOG_FUNC_EXIT_PARAMS(false)
    if ( !CTL_StaticData::s_DLLHandles.empty() )
        LOG_FUNC_EXIT_PARAMS(true)
    LOG_FUNC_EXIT_PARAMS(false)
#else
    // Check if this task has already been hooked
    if (FindTask( hTask ) )
        // Already hooked.  No need to do this again
        LOG_FUNC_EXIT_PARAMS(true)
    LOG_FUNC_EXIT_PARAMS(false)
#endif
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartThread( DTWAIN_HANDLE DLLHandle )
{
    LOG_FUNC_ENTRY_PARAMS((DLLHandle))
    if (!CTL_StaticData::IsUsingMultipleThreads())
        LOG_FUNC_EXIT_PARAMS(FALSE)
    auto iter = std::find_if(CTL_StaticData::s_mapThreadToDLLHandle.begin(),
                             CTL_StaticData::s_mapThreadToDLLHandle.end(),
                             [&](const auto& pr) 
                                { return pr.second.get() == static_cast<CTL_TwainDLLHandle*>(DLLHandle); });
    if ( iter != CTL_StaticData::s_mapThreadToDLLHandle.end())
        AssociateThreadToTwainDLL(iter->second, getThreadId());
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndThread( DTWAIN_HANDLE DLLHandle )
{
    LOG_FUNC_ENTRY_PARAMS((DLLHandle))
    if ( !CTL_StaticData::IsUsingMultipleThreads())
        LOG_FUNC_EXIT_PARAMS(FALSE)
    if ( CTL_StaticData::s_mapThreadToDLLHandle.size() == 1)
        LOG_FUNC_EXIT_PARAMS(FALSE)

    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if (pHandle)
    {
        auto iter = std::find_if(CTL_StaticData::s_mapThreadToDLLHandle.begin(),
            CTL_StaticData::s_mapThreadToDLLHandle.end(),
            [&](const auto& pr)
            { return pr.second.get() == static_cast<CTL_TwainDLLHandle*>(DLLHandle); });

        if (iter != CTL_StaticData::s_mapThreadToDLLHandle.end() && 
            iter->first == getThreadId())
        {
            CTL_StaticData::s_mapThreadToDLLHandle.erase(iter);
            LOG_FUNC_EXIT_PARAMS(TRUE)
        }
    }
    LOG_FUNC_EXIT_PARAMS(FALSE)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_UseMultipleThreads(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    CTL_StaticData::s_multipleThreads = bSet ? true : false;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_HANDLE dynarithmic::GetDTWAINHandle_Internal()
{
    auto iter = CTL_StaticData::s_mapThreadToDLLHandle.find(getThreadId());
    if (iter != CTL_StaticData::s_mapThreadToDLLHandle.end())
        return iter->second.get();
    return nullptr;
}


DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_GetDTWAINHandle()
{
    LOG_FUNC_ENTRY_PARAMS(())
    LOG_FUNC_EXIT_PARAMS(::GetDTWAINHandle_Internal())
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}


DTWAIN_BOOL SetLangResourcePath(LPCTSTR szPath)
{
    LOG_FUNC_ENTRY_PARAMS((szPath))
    CTL_StaticData::s_strLangResourcePath = FixPathString(szPath);
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsTwainAvailable()
{
    LOG_FUNC_ENTRY_PARAMS(())

    struct SysInitializerRAII
    {
        bool bMustDestroy;
        SysInitializerRAII(bool mustDestroy) : bMustDestroy(mustDestroy) {}
        ~SysInitializerRAII() 
        {
            if ( bMustDestroy )
                DTWAIN_SysDestroy();
        }
    };

    // Check if TWAIN session started.  If so, they must have had TWAIN installed!
    // Save the filter flags
    DTWAINScopedLogController sLogContoller(0);
    bool bMustDestroy = false;
    CTL_TwainDLLHandle *pHandle = nullptr;
    try
    {
        // Check if DTWAIN already initialized
        pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
        if ( !pHandle )
        {
            // Temporarily set up a handle without
            pHandle = static_cast<CTL_TwainDLLHandle*> (DTWAIN_SysInitialize());
            if ( !pHandle )
                return DTWAIN_ERR_BAD_HANDLE;
            bMustDestroy = true;
        }
        else
        if (DTWAIN_IsSessionEnabled())
        {
            LOG_FUNC_EXIT_PARAMS(true)
        }
    }
    catch (...)
    {
        LOG_FUNC_EXIT_PARAMS(false)
    }

    SysInitializerRAII raii(bMustDestroy);
    try
    {
        // DTWAIN initialized, but a TWAIN Session was not started.
        // Do a minimal test here (start a session and unstart it)
        #ifdef WIN64
        DTWAIN_BOOL bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_2);
        #else
        #ifdef WIN32
        DTWAIN_BOOL bRet = FALSE;
        if (pHandle->m_SessionStruct.DSMName == TWAINDLLVERSION_2) 
            bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_2);
        else
            bRet = CTL_TwainAppMgr::CheckTwainExistence(TWAINDLLVERSION_1);
        #endif
        #endif
        LOG_FUNC_EXIT_PARAMS(bRet)
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
    CTL_StaticData::s_DLLInstance = hInstance;
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

    SetLangResourcePath(szLangResourcePath);

    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeLibEx(hInstance, szINIPath);
    LOG_FUNC_EXIT_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}

/////////////////////////////////////////////////////////////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeLibEx(HINSTANCE hInstance, LPCTSTR szINIPath)
{
    LOG_FUNC_ENTRY_PARAMS((hInstance, szINIPath))

    CTL_StaticData::s_sINIPath = FixPathString(szINIPath);

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
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitializeEx(szINIPath);
    LOG_FUNC_EXIT_PARAMS(Handle)
    CATCH_BLOCK(DTWAIN_HANDLE(0))
}
/////////////////////////////////////////////////////////////////////////////////////////////////
DTWAIN_HANDLE DLLENTRY_DEF DTWAIN_SysInitializeEx(LPCTSTR szINIPath)
{
    LOG_FUNC_ENTRY_PARAMS((szINIPath))
    CTL_StaticData::s_sINIPath = FixPathString(szINIPath);
    const DTWAIN_HANDLE Handle = DTWAIN_SysInitialize();
    LOG_FUNC_EXIT_PARAMS(Handle)
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

DTWAIN_HANDLE SysInitializeHelper(bool block)
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
    CTL_StaticData::s_lErrorFilterFlags = 0;
    CTL_StaticData::s_nRegisteredDTWAINMsg = ::RegisterWindowMessage(REGISTERED_DTWAIN_MSG);
#endif
    LOG_FUNC_ENTRY_PARAMS(())

    CTL_TwainDLLHandlePtr pHandlePtr;
    // Return handle if already registered
    const DTWAIN_HANDLE DLLHandle = GetDTWAINHandle_Internal();
    if (DLLHandle)
        LOG_FUNC_EXIT_PARAMS(DLLHandle)

    // This must be checked if this is used in a static library
    #ifdef DTWAIN_LIB
    if ( CTL_StaticData::s_DLLInstance == NULL )
        LOG_FUNC_EXIT_PARAMS(NULL)
    #endif

    // Load resources only if first time
    // Return a new DTWAIN_HANDLE
    try
    {
        pHandlePtr = std::make_shared<CTL_TwainDLLHandle>();
        auto threadId = getThreadId();
        AssociateThreadToTwainDLL(pHandlePtr, threadId);
        bool resourcesLoaded = LoadGeneralResources(block);
        if ( !resourcesLoaded )
        {
            RemoveThreadIdFromAssociation(threadId);
            LOG_FUNC_EXIT_PARAMS(NULL)
        }
        CTL_TwainDLLHandle* pHandle = pHandlePtr.get();
        #ifdef _WIN32
        pHandle->m_pSaveAsDlgProc = FileSaveAsHookProc;
        RegisterTwainWindowClass();
        #endif

        LoadStaticData(pHandle);

        if ( !CTL_StaticData::s_mapThreadToDLLHandle.empty() )
        {
            const CTL_StringType szLangDLL = _T("english");

            // Initialize the enumerator factory
            pHandlePtr->m_ArrayFactory = std::make_shared<CTL_ArrayFactory>();

            // Initialize the resource registry
            pHandle->InitializeResourceRegistry();

            // Load customized resources from INI
            LoadCustomResourcesFromIni(pHandle, szLangDLL.c_str());

            // Load DS overrides for transfer ready / close UI requests
            LoadTransferReadyOverrides();

            // Load flatbed only list of devices
            LoadFlatbedOnlyOverrides();

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
            LogToDebugMonitorA("DTWAIN Initialization Error\n");
            LogToDebugMonitorA(ex.what());
        }
        LOG_FUNC_EXIT_PARAMS(NULL)
    }
    catch(...)
    {
        LOG_FUNC_EXIT_PARAMS(NULL)
    }
}


void LoadCustomResourcesFromIni(CTL_TwainDLLHandle* pHandle, LPCTSTR szLangDLL)
{
    // Load the resources
    CSimpleIniA customProfile;
    auto err = customProfile.LoadFile(dynarithmic::GetDTWAININIPathA().c_str());
    if (err != SI_OK)
        return;

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
            CTL_StaticData::s_lErrorFilterFlags |= ps.orValue;
    });

    auto nVal = customProfile.GetLongValue("DSMErrorLogging", "EnableNone", 0);
    if (nVal == 1)
        CTL_StaticData::s_lErrorFilterFlags = 0;

    nVal = customProfile.GetLongValue("DSMErrorLogging", "EnableAll", 0);
    if (nVal != 0)
        CTL_StaticData::s_lErrorFilterFlags = 0xFFFFFFFFL & ~DTWAIN_LOG_USEFILE;

    szStr = customProfile.GetValue("DSMErrorLogging", "File", "");
    if (!szStr.empty())
    {
        CTL_StaticData::s_lErrorFilterFlags |= DTWAIN_LOG_USEFILE;
        OpenLogFile(StringConversion::Convert_Ansi_To_Native(szStr).c_str(), CTL_StaticData::s_lErrorFilterFlags);
        CTL_StaticData::s_appLog.StatusOutFast("In DTWAIN_SysInitialize()");
    }

    nVal = customProfile.GetLongValue("DSMErrorLogging", "BufferErrorThreshold", 50);
    if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_USEBUFFER)
        DTWAIN_SetErrorBufferThreshold(nVal);

    nVal = customProfile.GetLongValue("DSMErrorLogging", "AppHandlesExceptions", 0);
    CTL_StaticData::s_bThrowExceptions = nVal == 0 ? false : true;
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
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, FALSE, FUNC_MACRO);

    // If the log flags have not specified what to log
    // then log callstack and general TWAIN send/receive info.
    LONG allFlags = DTWAIN_LOG_ALL;
    if ( (LogFlags != 0) && (LogFlags & allFlags) == 0)  
        LogFlags |= (DTWAIN_LOG_CALLSTACK | DTWAIN_LOG_DECODE_SOURCE | DTWAIN_LOG_DECODE_DEST | DTWAIN_LOG_MISCELLANEOUS);

    CTL_StaticData::s_lErrorFilterFlags = LogFlags;
    if (AnyLoggerExists(pHandle) && LogFlags == 0)
        CTL_StaticData::s_appLog.PrintBanner(false);
    else
    {
        if ( LogFlags && !UserDefinedLoggerExists())
            CTL_StaticData::s_lErrorFilterFlags &= ~DTWAIN_LOG_USECALLBACK;

        const bool append = LogFlags & DTWAIN_LOG_FILEAPPEND?true:false;
        OpenLogFile(lpszLogFile, LogFlags, append);

        // Write the version info
        WriteVersionToLog();
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

bool dynarithmic::UserDefinedLoggerExists()
{
    auto dllHandle = static_cast<CTL_TwainDLLHandle*>(dynarithmic::GetDTWAINHandle_Internal());
    if ( dllHandle )
        return UserDefinedLoggerExists(dllHandle);
    return false;
}

bool dynarithmic::UserDefinedLoggerExists(CTL_TwainDLLHandle* pHandle)
{
    return pHandle->m_LoggerCallbackInfo.m_pLoggerCallback || 
           pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA || 
           pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW;
}

bool dynarithmic::AnyLoggerExists()
{
    auto dllHandle = static_cast<CTL_TwainDLLHandle*>(dynarithmic::GetDTWAINHandle_Internal());
    if (dllHandle)
        return AnyLoggerExists(dllHandle);
    return false;
}

bool dynarithmic::AnyLoggerExists(CTL_TwainDLLHandle* pHandle)
{
    return UserDefinedLoggerExists(pHandle) || CTL_StaticData::s_lErrorFilterFlags != 0;
}

void dynarithmic::WriteUserDefinedLogMsg(LPCTSTR sz)
{
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(dynarithmic::GetDTWAINHandle_Internal());
    if (!pHandle)
        return;
    if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallback)
        pHandle->m_LoggerCallbackInfo.m_pLoggerCallback(sz, pHandle->m_LoggerCallbackInfo.m_pLoggerCallback_UserData);
    if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackA)
#ifdef _UNICODE
        WriteUserDefinedLogMsgA(StringConversion::Convert_Native_To_Ansi(sz).c_str());
#else
        WriteUserDefinedLogMsgA(sz);
#endif
    if (pHandle->m_LoggerCallbackInfo.m_pLoggerCallbackW)
#ifdef _UNICODE
        WriteUserDefinedLogMsgW(sz);
#else
        WriteUserDefinedLogMsgW(StringConversion::Convert_NativePtr_To_Wide(sz).c_str());
#endif
}

void dynarithmic::WriteUserDefinedLogMsgA(LPCSTR sz)
{
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(dynarithmic::GetDTWAINHandle_Internal());
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

void dynarithmic::WriteUserDefinedLogMsgW(LPCWSTR sz)
{
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(dynarithmic::GetDTWAINHandle_Internal());
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

void OpenLogFile(LPCTSTR pFileName, LONG logFlags, bool bAppend)
{
    if ( pFileName && pFileName[0] )
        CTL_StaticData::s_appLog.InitFileLogging(pFileName, CTL_StaticData::s_DLLInstance, bAppend );
    if ( logFlags & DTWAIN_LOG_CONSOLE)
        CTL_StaticData::s_appLog.InitConsoleLogging(CTL_StaticData::s_DLLInstance);
    if ( logFlags & DTWAIN_LOG_DEBUGMONITOR)
        CTL_StaticData::s_appLog.InitDebugWindowLogging(CTL_StaticData::s_DLLInstance);
    if ( logFlags & DTWAIN_LOG_USECALLBACK )
        CTL_StaticData::s_appLog.InitCallbackLogging(CTL_StaticData::s_DLLInstance);
    CTL_StaticData::s_appLog.PrintTime(true);
    CTL_StaticData::s_appLog.PrintAppName(true);
    CTL_StaticData::s_appLog.PrintBanner();
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
        hInstance = CTL_StaticData::s_DLLInstance;

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
                                      CTL_StaticData::s_DLLInstance,
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
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

static DTWAIN_ARRAY GetFileTypes(int nType)
{
    constexpr const char *sNames[] = { "","-Single","-Multi" };
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if (!pHandle)
        return {};
    DTWAIN_ARRAY aFileTypes = CreateArrayFromFactory(DTWAIN_ARRAYLONG, 0);
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

bool AssociateThreadToTwainDLL(std::shared_ptr<CTL_TwainDLLHandle>& pHandle, unsigned long threadId)
{
    if ( !pHandle )
         return false;
    return CTL_StaticData::s_mapThreadToDLLHandle.insert({threadId, pHandle}).second;
}

bool RemoveThreadIdFromAssociation(unsigned long threadId)
{
    auto iter = CTL_StaticData::s_mapThreadToDLLHandle.find(threadId);
    if ( iter != CTL_StaticData::s_mapThreadToDLLHandle.end())
    {
        CTL_StaticData::s_mapThreadToDLLHandle.erase(iter);
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
    return CTL_StaticData::s_mapThreadToDLLHandle.find(hTask) != CTL_StaticData::s_mapThreadToDLLHandle.end();
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EndTwainSession()
{
    LOG_FUNC_ENTRY_PARAMS(())
    // Delete it


    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
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
        LOG_FUNC_EXIT_PARAMS(true)

    StringTraitsA::string_type sClosingDSM = dynarithmic::GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_DSM) + "\n";
    StringTraitsA::string_type sClosingTwainSession = dynarithmic::GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_TWAIN_SESSION) + "\n";

    try
    {
        CTL_TwainAppMgr::CloseSourceManager(pHandle->m_pTwainSession);
    }
    catch(...)
    {
        CTL_TwainAppMgr::WriteLogInfoA(sClosingDSM);
    }

    try
    {
        CTL_TwainAppMgr::DestroyTwainSession(pHandle->m_pTwainSession);
    }
    catch(...)
    {
        CTL_TwainAppMgr::WriteLogInfoA(sClosingTwainSession);
    }
    if ( CTL_StaticData::s_mapThreadToDLLHandle.size() == 1 )
    {
        try
        {
            CTL_TwainAppMgr::Destroy();
        }
        catch(...)
        {
            if (CTL_StaticData::s_lErrorFilterFlags)
            {
                StringTraitsA::string_type sClosingManager = GetResourceStringFromMap(IDS_DTWAIN_ERROR_CLOSING_DTWAIN_MANAGER) + "\n";
                CTL_TwainAppMgr::WriteLogInfoA(sClosingManager);
            }
        }
        if (CTL_StaticData::s_lErrorFilterFlags)
        {
            StringTraitsA::string_type sClosingDTWAIN = GetResourceStringFromMap(IDS_CLOSING_DTWAIN) + "\n";
            CTL_TwainAppMgr::WriteLogInfoA(sClosingDTWAIN);
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
            if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)
            {
                StringTraitsA::string_type sRemoveWindow = GetResourceStringFromMap(IDS_DTWAIN_ERROR_REMOVE_WINDOW) + "\n";
                CTL_TwainAppMgr::WriteLogInfoA(sRemoveWindow);
            }
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
    std::lock_guard<std::mutex> lg(CTL_StaticData::s_mutexInitDestroy);
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    if (!DTWAIN_EndTwainSession())
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = SysDestroyHelper(pHandle);

#ifdef DTWAIN_DEBUG_CALL_STACK
    if (CTL_StaticData::s_lErrorFilterFlags)
        CTL_LogFunctionCall(__FUNC__, 1);
#endif
    return bRet;
    CATCH_BLOCK(false)
}

static bool SysDestroyHelper(CTL_TwainDLLHandle* pHandle, bool bCheck)
{
    #ifdef _WIN32
    // Unload the OCR interfaces
    unsigned long threadId = getThreadId();

    UnloadOCRInterfaces(pHandle);
    #endif
    // Remove the handle for this thread
    auto it = CTL_StaticData::s_mapThreadToDLLHandle.find(threadId);
    if ( it == CTL_StaticData::s_mapThreadToDLLHandle.end() )
        return false;

    UnhookAllDisplays();
    pHandle->RemoveAllEnumerators();
    pHandle->RemoveAllSourceCapInfo();
    pHandle->RemoveAllSourceMaps();
    pHandle->m_CallbackMsg = nullptr;
    pHandle->m_CallbackError = nullptr;
    RemoveThreadIdFromAssociation(threadId);
    FreeImage_ClearPlugins();
    FreeImage_DeInitialise();
    return TRUE;
}

/* This function tests all open DLL handles to see if any source is acquiring */
DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsAcquiring()
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    const auto iter = std::find_if(pHandle->m_mapStringToSource.begin(),
                                   pHandle->m_mapStringToSource.end(), 
                                [&](const CTL_StringToSourcePtrMap::value_type& vt) {return vt.second->IsAcquireAttempt(); });
    if ( iter != pHandle->m_mapStringToSource.end())
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
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_TwainDSMSearchOrder = SearchOrder;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_GetDSMSearchOrder(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    const LONG SearchOrder = pHandle->m_TwainDSMSearchOrder;
    LOG_FUNC_EXIT_PARAMS(SearchOrder)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetDSMSearchOrderEx(LPCTSTR SearchOrder, LPCTSTR UserDirectory)
{
    LOG_FUNC_ENTRY_PARAMS((SearchOrder))
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    const std::string strValidString = CheckSearchOrderString(StringConversion::Convert_NativePtr_To_Ansi(SearchOrder));
    if ( !strValidString.empty() )
    {
        pHandle->m_TwainDSMSearchOrderStr = strValidString;
        pHandle->m_TwainDSMUserDirectory = UserDirectory?UserDirectory:StringWrapper::traits_type::GetEmptyString();
        pHandle->m_TwainDSMSearchOrder = -1;
        LOG_FUNC_EXIT_PARAMS(TRUE)
    }
    LOG_FUNC_EXIT_PARAMS(FALSE)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetResourcePath(LPCTSTR ResourcePath)
{
    LOG_FUNC_ENTRY_PARAMS((ResourcePath))
    CTL_StaticData::s_strResourcePath = ResourcePath;
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
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_CallCallback64(WPARAM wParam, LPARAM lParam, LONGLONG UserData)
{
    LOG_FUNC_ENTRY_PARAMS((wParam, lParam, UserData))
    auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, FUNC_MACRO);
    LONG RetVal = 1;
    if (pHandle->m_pCallbackFn64)
        RetVal = static_cast<LONG>((*pHandle->m_pCallbackFn64)(wParam, lParam, UserData));
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(0)
}

void UnhookAllDisplays()
{
#ifdef _WIN32
    if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_CONSOLE)
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
    if (!(CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_DTWAINERRORS) )
        return;
    static constexpr int MaxMessage=1024;
    char szBuf[MaxMessage+1];
    if ( !pHandle )
        DTWAIN_GetErrorStringA( DTWAIN_ERR_BAD_HANDLE, szBuf,MaxMessage);
    else
        DTWAIN_GetErrorStringA( pHandle->m_lLastError, szBuf, MaxMessage);
    std::string s(szBuf);
    s += "\n";
    if ( !pHandle )
        CTL_TwainAppMgr::WriteLogInfoA(s);

    if ( CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_ERRORMSGBOX && pHandle)
        LogDTWAINErrorToMsgBox(pHandle->m_lLastError, pFunc, s);
    else
    if ( !pHandle && CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_INITFAILURE)
        LogDTWAINErrorToMsgBox(DTWAIN_ERR_BAD_HANDLE, nullptr, s);
}



DTWAIN_BOOL DLLENTRY_DEF DTWAIN_AppHandlesExceptions(DTWAIN_BOOL bSet)
{
    CTL_StaticData::s_bThrowExceptions = bSet?true:false;
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
    const LONG RetVal = StringWrapper::CopyInfoToCString(GetVersionInfo(), lpszVer, nLength);
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetVersionCopyright(LPTSTR lpszVer, LONG nLength)
{
    LOG_FUNC_ENTRY_PARAMS((lpszVer, nLength))
    const LONG RetVal = GetResourceString(IDS_DTWAIN_APPTITLE, lpszVer, nLength);
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
            sValue = CTL_StaticData::GetTwainNameFromResource(CTL_StaticData::GetDGResourceID(), static_cast<int>(TwainID));
        break;
        case DTWAIN_DATNAME:
            sValue = CTL_StaticData::GetTwainNameFromResource(CTL_StaticData::GetDATResourceID(), static_cast<int>(TwainID));
        break;
        case DTWAIN_MSGNAME:
            sValue = CTL_StaticData::GetTwainNameFromResource(CTL_StaticData::GetMSGResourceID(), static_cast<int>(TwainID));
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
    const LONG RetVal = CTL_StaticData::GetIDFromTwainName(StringConversion::Convert_NativePtr_To_Ansi(lpszBuffer));
    LOG_FUNC_EXIT_PARAMS(RetVal)
    CATCH_BLOCK(-1)
}

LONG DLLENTRY_DEF DTWAIN_GetWindowsVersionInfo(LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((lpszBuffer, nMaxLen))
    LONG RetVal = 0;
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
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
    if ( !CTL_StaticData::s_DLLPath.empty())
        return CTL_StaticData::s_DLLPath;
    TCHAR buffer[1024];
    boost::winapi::GetModuleFileName(CTL_StaticData::s_DLLInstance, buffer, 1024);
    CTL_StaticData::s_DLLPath = buffer;
    return buffer;
}

CTL_StringType dynarithmic::GetVersionString()
{
    if (!CTL_StaticData::s_VersionString.empty())
        return CTL_StaticData::s_VersionString;

    LONG lMajor, lMinor, lVersionType, lPatch;
    // Write the version info
    if ( DTWAIN_GetVersionEx(&lMajor, &lMinor, &lVersionType, &lPatch) )
    {
        std::string s;
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
            << lPatch << ")\n" << "Shared Library path: " <<  StringConversion::Convert_Native_To_Ansi(GetDTWAINDLLPath());
        strm << "\nUsing Resource file (twaininfo.txt) version: " << StringConversion::Convert_Native_To_Ansi(CTL_StaticData::GetResourceVersion());
        strm << "\nResource file path: " << StringConversion::Convert_Native_To_Ansi(CTL_StaticData::GetResourcePath());
        CTL_StaticData::s_VersionString = StringConversion::Convert_Ansi_To_Native(strm.str());
        return CTL_StaticData::s_VersionString;
    }
    return {};
}

void WriteVersionToLog()
{
    std::string ansiVer;
    if (CTL_StaticData::s_lErrorFilterFlags)
    {
        auto sVer = GetVersionString();
        const auto sWinVer = GetWinVersion();
        sVer += _T("\n") + sWinVer;
        #ifdef _WIN32
        // All log messages must be ANSI
        ansiVer = StringConversion::Convert_Native_To_Ansi(sVer);
        if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_USEFILE)
        {
            if (!CTL_StaticData::s_appLog.StatusOutFast(ansiVer.c_str()))
            {
                ansiVer += "\n";
                LogToDebugMonitorA(ansiVer);
            }
        }
        else
        if (CTL_StaticData::s_lErrorFilterFlags & DTWAIN_LOG_DEBUGMONITOR)
        {
            sVer += _T("\n");
            LogToDebugMonitor(sVer);
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
        *lMinor = DTWAIN_MINOR_VERSION;
    if (pPatch)
        *pPatch = DTWAIN_PATCHLEVEL_VERSION;
    return true;
}

CTL_StringType GetDTWAINDLLVersionInfoStr()
{
    return StringConversion::Convert_AnsiPtr_To_Native(DTWAIN_VERINFO_FILEVERSION);
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

CTL_StringType dynarithmic::GetDTWAINTempFilePath()
{
    static CTL_StringType sDummy;
    auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if (!pHandle)
        return sDummy;
    if ( pHandle->m_sTempFilePath.empty())
    {
        const auto tempPath = temp_directory_path();
        if (tempPath.empty())
            CTL_TwainAppMgr::WriteLogInfoA(GetResourceStringFromMap(IDS_LOGMSG_ERRORTEXT) + ": " + GetResourceStringFromMap(IDS_LOGMSG_TEMPFILENOTEXISTTEXT) + "\n");
        else
            pHandle->m_sTempFilePath = tempPath;
    }
    CTL_TwainAppMgr::WriteLogInfoA("Temp path is " +  StringConversion::Convert_Native_To_Ansi(pHandle->m_sTempFilePath) + "\n");
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
    CSimpleIniA customProfile;
    auto err = customProfile.LoadFile(dynarithmic::GetDTWAININIPathA().c_str());
    if (err != SI_OK)
        return;
    CSimpleIniA::TNamesDepend keys;
    customProfile.GetAllKeys("SourceXferWaitInfo", keys);
    auto iter = keys.begin();
    while (iter != keys.end())
    {
        CSimpleIniA::TNamesDepend vals;
        customProfile.GetAllValues("SourceXferWaitInfo", iter->pItem, vals);
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

// This loads DTWAIN32.INI or DTWAIN64.INI, and checks the [FlatbedOnly]
// section for TWAIN sources that only work if the source is preset to use
// only the flatbed portion of the device.  
void LoadFlatbedOnlyOverrides()
{
    auto& flatbed_list = CTL_TwainAppMgr::GetSourceFlatbedOnlyList();
    flatbed_list.clear();

    // Get the section name
    CSimpleIniA customProfile;
    auto err = customProfile.LoadFile(dynarithmic::GetDTWAININIPathA().c_str());
    if (err != SI_OK)
        return;
    CSimpleIniA::TNamesDepend keys;
    customProfile.GetAllKeys("FlatbedOnly", keys);
    auto iter = keys.begin();
    while (iter != keys.end())
    {
        CSimpleIniA::TNamesDepend vals;
        customProfile.GetAllValues("FlatbedOnly", iter->pItem, vals);
        flatbed_list.insert(iter->pItem);
        ++iter;
    }
}

bool LoadGeneralResources(bool blockExecution)
{
    bool bResourcesLoaded = false;
    typedef std::function<bool(ResourceLoadingInfo&)> boolFuncs;
    boolFuncs bf[] = { &LoadTwainResources };
    for (auto& fnBool : bf)
    {
        ResourceLoadingInfo ret;
        fnBool(ret);
        if (!ret.errorValue[0] || !ret.errorValue[1] || !ret.errorValue[2] )
        {
#ifdef _WIN32
            if (blockExecution)
            {
                CTL_StringType errorMsg = _T("Error.  DTWAIN Resource file(s) not found or corrupted: ");
                std::vector<CTL_StringType> vErrors;
                if (!ret.errorValue[0])
                    vErrors.push_back(DTWAINRESOURCEINFOFILE);
                if (!ret.errorValue[1])
                    vErrors.push_back(DTWAIN_ININAME_NATIVE);
                if (!ret.errorValue[2])
                {
                    CTL_StringType versionErrorMessage = _T("Error.  Bad or outdated TWAIN version of resources used: (");
                    versionErrorMessage += ret.errorMessage;
                    versionErrorMessage += _T(").  Expected minimum version: ");
                    versionErrorMessage += _T(DTWAIN_TEXTRESOURCE_FILEVERSION);
                    versionErrorMessage += _T("\nPlease use the latest text resources found at \"https://github.com/dynarithmic/twain_library/tree/master/text_resources\"");
                    vErrors.push_back(versionErrorMessage);
                }
                CTL_StringType sAllErrors = errorMsg + StringWrapper::Join(vErrors, _T(",\n"));
                MessageBox(nullptr, sAllErrors.c_str(), _T("DTWAIN Resource Error"), MB_ICONERROR);
            }
#endif
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
