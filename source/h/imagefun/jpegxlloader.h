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
#ifndef JPEGXLLOADER_H
#define JPEGXLLOADER_H

#include <windows.h>

#include <jxl/encode.h>
#include <jxl/color_encoding.h>
#include <jxl/thread_parallel_runner.h>

#include <string>

namespace dynarithmic
{
    class JxlDynamicLoader
    {
    public:
        struct Functions
        {
            // jxl.dll
            decltype(&JxlEncoderCreate) EncoderCreate = nullptr;
            decltype(&JxlEncoderDestroy) EncoderDestroy = nullptr;
            decltype(&JxlEncoderSetParallelRunner) EncoderSetParallelRunner = nullptr;
            decltype(&JxlEncoderInitBasicInfo) EncoderInitBasicInfo = nullptr;
            decltype(&JxlEncoderSetBasicInfo) EncoderSetBasicInfo = nullptr;
            decltype(&JxlColorEncodingSetToSRGB) ColorEncodingSetToSRGB = nullptr;
            decltype(&JxlEncoderSetColorEncoding) EncoderSetColorEncoding = nullptr;
            decltype(&JxlEncoderFrameSettingsCreate) EncoderFrameSettingsCreate = nullptr;
            decltype(&JxlEncoderSetFrameLossless) EncoderSetFrameLossless = nullptr;
            decltype(&JxlEncoderSetFrameDistance) EncoderSetFrameDistance = nullptr;
            decltype(&JxlEncoderFrameSettingsSetOption) EncoderFrameSettingsSetOption = nullptr;
            decltype(&JxlEncoderAddImageFrame) EncoderAddImageFrame = nullptr;
            decltype(&JxlEncoderCloseInput) EncoderCloseInput = nullptr;
            decltype(&JxlEncoderProcessOutput) EncoderProcessOutput = nullptr;
            decltype(&JxlEncoderDistanceFromQuality) EncoderDistanceFromQuality = nullptr;
            decltype(&JxlEncoderUseBoxes) EncoderUseBoxes = nullptr;
            decltype(&JxlEncoderAddBox) EncoderAddBox = nullptr;
            decltype(&JxlEncoderCloseBoxes) EncoderCloseBoxes = nullptr;
            decltype(&JxlEncoderVersion) EncoderVersion = nullptr;

            // jxl_threads.dll
            decltype(&JxlThreadParallelRunnerCreate) ThreadParallelRunnerCreate = nullptr;
            decltype(&JxlThreadParallelRunnerDestroy) ThreadParallelRunnerDestroy = nullptr;
            decltype(&JxlThreadParallelRunnerDefaultNumWorkerThreads)
                ThreadParallelRunnerDefaultNumWorkerThreads = nullptr;
            decltype(&JxlThreadParallelRunner) ThreadParallelRunner = nullptr;
        };

        JxlDynamicLoader() = default;
        ~JxlDynamicLoader();

        JxlDynamicLoader(JxlDynamicLoader&& other) = delete;
        JxlDynamicLoader& operator=(JxlDynamicLoader&& other) noexcept;

        JxlDynamicLoader(const JxlDynamicLoader&) = delete;
        JxlDynamicLoader& operator=(const JxlDynamicLoader&) = delete;

        std::pair<bool, int> Load(const std::wstring& directory);
        void Unload() noexcept;
        void Reset() noexcept;
        bool IsLoaded() const noexcept
        {
            return m_loaded;
        }

        const Functions& GetFunctions() const noexcept
        {
            return m_functions;
        }

        DWORD GetLastErrorCode() const noexcept
        {
            return m_lastError;
        }

        const std::string& GetFailedFunction() const noexcept
        {
            return m_failedFunction;
        }

    private:
        HMODULE m_jxl = nullptr;
        HMODULE m_threads = nullptr;

        Functions m_functions{};
        bool m_loaded = false;

        DWORD m_lastError = ERROR_SUCCESS;
        std::string m_failedFunction;
    };
}
#endif
