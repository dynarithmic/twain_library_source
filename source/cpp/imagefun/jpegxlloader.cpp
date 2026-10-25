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
#include "jpegxlloader.h"

#include <filesystem>
#include <tchar.h>
#include <utility>

#include "ctlstaticdata.h"
#include "ctlstringconversion.h"
#include "ctlstringdefs.h"
#include "logwriterutils.h"
#include "dtwinverex.h"

using namespace dynarithmic;

namespace
{
    template <typename T>
    bool ResolveFunction(HMODULE module,const char* name,T& function)
    {
        function = reinterpret_cast<T>(GetProcAddress(module, name));
        return function != nullptr;
    }
}

JxlDynamicLoader::~JxlDynamicLoader()
{
    Unload();
}

JxlDynamicLoader& JxlDynamicLoader::operator=(JxlDynamicLoader&& other) noexcept
{
    if (this != &other)
    {
        Unload();
        m_jxl = std::exchange(other.m_jxl, nullptr);
        m_threads = std::exchange(other.m_threads, nullptr);
        m_functions = std::exchange(other.m_functions, Functions{});
        m_loaded = std::exchange(other.m_loaded, false);
        m_lastError = std::exchange(other.m_lastError, ERROR_SUCCESS);
        m_failedFunction = std::move(other.m_failedFunction);
    }
    return *this;
}

std::pair<bool, int> JxlDynamicLoader::Load(const std::wstring& directory)
{
    struct LoadLibraryRAII
    {
        JxlDynamicLoader* m_pThis{};
        bool bUnload = true;
        LoadLibraryRAII(JxlDynamicLoader* thisLoader) : m_pThis(thisLoader) {}
        ~LoadLibraryRAII()
        {
            try
            {
                if (bUnload && m_pThis)
                {
                    m_pThis->Unload();
                }
            }
            catch (...)
            {}
        }
    };

    struct FunctionResolverRAII
    {
        std::string m_functionName;
        bool bDisplayError = true;
        FunctionResolverRAII() = default;
        ~FunctionResolverRAII()
        {
            try
            {
                if (bDisplayError)
                {
                    if (CTL_StaticData::GetLogFilterFlags())
                    {
                        CTL_StringType sJXLNative = L"Could not load JXL exported function " + 
                            stringconversion::Convert_Ansi_To_Wide(m_functionName);
                        LogWriterUtils::WriteLogInfoIndented(sJXLNative);
                    }
                }
            }
            catch (...)
            {}
        }
    };

    if (m_loaded)
        return { true, 0 };

    // Remove any previous partial state.
    Unload();

    m_lastError = ERROR_SUCCESS;
    m_failedFunction.clear();

    namespace fs = std::filesystem;

    HMODULE hJxl = nullptr;
    HMODULE hThreads = nullptr;
    std::wstring jxlPath = L"jxl.dll";
    std::wstring threadsPath = L"jxl_threads.dll";
    LoadLibraryRAII raii(this);

    if (directory.empty())
    {
        // Use standard Windows DLL search behavior.
        hJxl = LoadLibraryW(L"jxl.dll");

        if (!hJxl)
        {
            m_lastError = GetLastError();
            return { false, m_lastError };
        }
        hThreads = LoadLibraryW(L"jxl_threads.dll");
    }
    else
    {
        const fs::path base(directory);

        if (!base.is_absolute())
        {
            m_lastError = ERROR_INVALID_PARAMETER;
            return { false, m_lastError };
        }

        constexpr DWORD flags =
            LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR |
            LOAD_LIBRARY_SEARCH_DEFAULT_DIRS;

        jxlPath = (base / L"jxl.dll").wstring();
        threadsPath = (base / L"jxl_threads.dll").wstring();

        hJxl = LoadLibraryExW(jxlPath.c_str(), nullptr, flags);

        if (!hJxl)
        {
            m_lastError = GetLastError();
            return { false, m_lastError };
        }

        hThreads = LoadLibraryExW(threadsPath.c_str(), nullptr, flags);
    }

    m_jxl = hJxl;

    if (!hThreads)
    {
        m_lastError = GetLastError();
        return { false, m_lastError };
    }

    m_threads = hThreads;

    if (CTL_StaticData::GetLogFilterFlags())
    {
        CTL_StringTypeW sJXLMVersionInfo = basicstringutils::QuoteString<std::wstring>( jxlPath) + L" loaded successfully.  Version Information:\n";
        CTL_StringType sJXLNative = stringconversion::Convert_Wide_To_Native(sJXLMVersionInfo);
        try
        {
            sJXLNative += GetVersionInfo(hJxl, 4, _T("\n"));
        }
        catch (...)
        {
            sJXLNative += _T("No version info available for jxl.dll\n");
        }
        sJXLMVersionInfo = basicstringutils::QuoteString<std::wstring>(threadsPath) + L" loaded successfully.  Version Information:\n";
        sJXLNative += stringconversion::Convert_Wide_To_Native(sJXLMVersionInfo);
        try
        {
            sJXLNative += GetVersionInfo(hThreads, 4, _T("\n"));
        }
        catch (...)
        {
            sJXLNative += _T("No version info available for jxl_threads.dll");
        }
        LogWriterUtils::WriteMultiLineInfoIndented(sJXLNative, _T("\n"));
    }

    Functions f{};

    FunctionResolverRAII funcResolver;

    // Resolve each required function.
    // Preserve the error before unloading modules.
    #define RESOLVE_JXL(module, member, exportedName)  \
    do {   \
        if (!ResolveFunction(module, exportedName, f.member))  \
        {   \
            m_lastError = ::GetLastError();  \
            m_failedFunction = exportedName; \
            funcResolver.m_functionName = exportedName; \
            return { false, m_lastError };   \
        }                                    \
    } while (false)

    RESOLVE_JXL(m_jxl, EncoderCreate, "JxlEncoderCreate");
    RESOLVE_JXL(m_jxl, EncoderDestroy, "JxlEncoderDestroy");
    RESOLVE_JXL(m_jxl, EncoderSetParallelRunner, "JxlEncoderSetParallelRunner");
    RESOLVE_JXL(m_jxl, EncoderInitBasicInfo, "JxlEncoderInitBasicInfo");
    RESOLVE_JXL(m_jxl, EncoderSetBasicInfo, "JxlEncoderSetBasicInfo");
    RESOLVE_JXL(m_jxl, ColorEncodingSetToSRGB, "JxlColorEncodingSetToSRGB");
    RESOLVE_JXL(m_jxl, EncoderSetColorEncoding, "JxlEncoderSetColorEncoding");
    RESOLVE_JXL(m_jxl, EncoderFrameSettingsCreate, "JxlEncoderFrameSettingsCreate");
    RESOLVE_JXL(m_jxl, EncoderSetFrameLossless, "JxlEncoderSetFrameLossless");
    RESOLVE_JXL(m_jxl, EncoderSetFrameDistance, "JxlEncoderSetFrameDistance");
    RESOLVE_JXL(m_jxl, EncoderFrameSettingsSetOption, "JxlEncoderFrameSettingsSetOption");
    RESOLVE_JXL(m_jxl, EncoderAddImageFrame, "JxlEncoderAddImageFrame");
    RESOLVE_JXL(m_jxl, EncoderCloseInput, "JxlEncoderCloseInput");
    RESOLVE_JXL(m_jxl, EncoderProcessOutput, "JxlEncoderProcessOutput");
    RESOLVE_JXL(m_jxl, EncoderDistanceFromQuality, "JxlEncoderDistanceFromQuality");
    RESOLVE_JXL(m_jxl, EncoderUseBoxes, "JxlEncoderUseBoxes");
    RESOLVE_JXL(m_jxl, EncoderAddBox, "JxlEncoderAddBox");
    RESOLVE_JXL(m_jxl, EncoderCloseBoxes, "JxlEncoderCloseBoxes");

    RESOLVE_JXL(m_threads, ThreadParallelRunnerCreate, "JxlThreadParallelRunnerCreate");
    RESOLVE_JXL(m_threads, ThreadParallelRunnerDestroy, "JxlThreadParallelRunnerDestroy");
    RESOLVE_JXL(m_threads, ThreadParallelRunnerDefaultNumWorkerThreads,
                "JxlThreadParallelRunnerDefaultNumWorkerThreads");
    RESOLVE_JXL(m_threads, ThreadParallelRunner, "JxlThreadParallelRunner");

#undef RESOLVE_JXL

    // Publish only a fully resolved table.
    m_functions = f;
    m_loaded = true;
    raii.bUnload = false;
    funcResolver.bDisplayError = false;
    return { true, 0 };
}

void JxlDynamicLoader::Unload() noexcept
{
    // Must not be called while encoder/runner objects
    // are still alive or encoding is in progress.

    m_loaded = false;
    m_functions = {};

    if (m_threads)
    {
        FreeLibrary(m_threads);
        m_threads = nullptr;
    }

    if (m_jxl)
    {
        FreeLibrary(m_jxl);
        m_jxl = nullptr;
    }
}

void JxlDynamicLoader::Reset() noexcept
{
    Unload();
    m_lastError = ERROR_SUCCESS;
    m_failedFunction.clear();
}

