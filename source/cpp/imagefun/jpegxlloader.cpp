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

#include "ctliface.h"
#include "ctlstaticdata.h"
#include "ctlstringconversion.h"
#include "logwriterutils.h"
#include "dtwinverex.h"
#include "sharedlibraryutils.h"

#ifdef _MSC_VER
    #pragma warning (disable : 4244)
    #define STRINGER_2_(x) #x
    #define STRINGER_(x) STRINGER_2_(x)
    #pragma message ("Using libjxl version " STRINGER_(JPEGXL_MAJOR_VERSION) "." STRINGER_(JPEGXL_MINOR_VERSION) "." STRINGER_(JPEGXL_PATCH_VERSION))
#endif

using namespace dynarithmic;

JxlDynamicLoader::~JxlDynamicLoader()
{
    m_loader.Unload();
}

DynamicLibraryLoader::FunctionTable JxlDynamicLoader::CreateFunctionTable(Functions& funcs)
{
    using Loader = DynamicLibraryLoader;
    using F = Functions;

    return
    {
        // jxl.dll
        Loader::RegisterFunction("jxl.dll", "JxlEncoderCreate",
            funcs, &F::EncoderCreate),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderDestroy",
            funcs, &F::EncoderDestroy),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderSetParallelRunner",
            funcs, &F::EncoderSetParallelRunner),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderInitBasicInfo",
            funcs, &F::EncoderInitBasicInfo),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderSetBasicInfo",
            funcs, &F::EncoderSetBasicInfo),

        Loader::RegisterFunction("jxl.dll", "JxlColorEncodingSetToSRGB",
            funcs, &F::ColorEncodingSetToSRGB),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderSetColorEncoding",
            funcs, &F::EncoderSetColorEncoding),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderFrameSettingsCreate",
            funcs, &F::EncoderFrameSettingsCreate),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderSetFrameLossless",
            funcs, &F::EncoderSetFrameLossless),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderSetFrameDistance",
            funcs, &F::EncoderSetFrameDistance),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderFrameSettingsSetOption",
            funcs, &F::EncoderFrameSettingsSetOption),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderAddImageFrame",
            funcs, &F::EncoderAddImageFrame),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderCloseInput",
            funcs, &F::EncoderCloseInput),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderProcessOutput",
            funcs, &F::EncoderProcessOutput),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderDistanceFromQuality",
            funcs, &F::EncoderDistanceFromQuality),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderUseBoxes",
            funcs, &F::EncoderUseBoxes),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderAddBox",
            funcs, &F::EncoderAddBox),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderCloseBoxes",
            funcs, &F::EncoderCloseBoxes),

        Loader::RegisterFunction("jxl.dll", "JxlEncoderVersion",
            funcs, &F::EncoderVersion),

        // jxl_threads.dll
        Loader::RegisterFunction("jxl_threads.dll",
            "JxlThreadParallelRunnerCreate",
            funcs, &F::ThreadParallelRunnerCreate),

        Loader::RegisterFunction("jxl_threads.dll",
            "JxlThreadParallelRunnerDestroy",
            funcs, &F::ThreadParallelRunnerDestroy),

        Loader::RegisterFunction("jxl_threads.dll",
            "JxlThreadParallelRunnerDefaultNumWorkerThreads",
            funcs, &F::ThreadParallelRunnerDefaultNumWorkerThreads),

        Loader::RegisterFunction("jxl_threads.dll",
            "JxlThreadParallelRunner",
            funcs, &F::ThreadParallelRunner)
    };
}


JxlDynamicLoader& JxlDynamicLoader::operator=(JxlDynamicLoader&& other) noexcept
{
    if (this != &other)
    {
        m_loader.Unload();
        m_loader = std::exchange(other.m_loader, {});
        m_functions = std::exchange(other.m_functions, Functions{});
        m_loaded = std::exchange(other.m_loaded, false);
        m_lastError = std::exchange(other.m_lastError, ERROR_SUCCESS);
        m_failedFunction = std::move(other.m_failedFunction);
    }
    return *this;
}


std::pair<bool, int> JxlDynamicLoader::Load(const std::wstring& directory)
{
    const std::vector<std::string> dllNames =
    {
        "jxl.dll",
        "jxl_threads.dll"
    };

    if (!m_loader.Load(dllNames, directory))
    {
        m_lastError = m_loader.GetLastErrorCode();
        m_failedFunction = m_loader.GetFailedName();
        return { false, m_lastError };
    }

    Functions funcs{};
    const auto table = CreateFunctionTable(funcs);

    if (!m_loader.ResolveFunctions(table))
    {
        m_lastError = m_loader.GetLastErrorCode();
        m_failedFunction = m_loader.GetFailedName();

        m_loader.Unload();
        return { false, m_lastError };
    }

    m_functions = funcs;
    m_loaded = true;

    return { true, DTWAIN_NO_ERROR };
}
