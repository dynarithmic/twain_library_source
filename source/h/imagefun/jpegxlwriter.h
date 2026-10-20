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

#ifndef JPEGXLWRITER_H
#define JPEGXLWRITER_H
#include <memory>
#include <optional>
#include <string>
#include "imagefilewriterbase.h"

namespace dynarithmic 
{
    struct JxlSessionOptions
    {
        bool lossless = true; 
        float distance = 1.0f; 
        int qualitySetting = 100;
        int effort = 7; 
        int numThreads = 0;
        std::string copyright;
    };

    struct PreparedJxlDibPage { DibPageView view{}; };

    class JxlSessionWriter 
    {
        public:
            JxlSessionWriter() = default;
            ~JxlSessionWriter();
            JxlSessionWriter(const JxlSessionWriter&) = delete;
            JxlSessionWriter& operator=(const JxlSessionWriter&) = delete;
            bool Open(const std::wstring& filename, const JxlSessionOptions& options);
            bool SetPageInfo(const PreparedJxlDibPage& page);
            bool WriteCurrentPage();
            void Close();
            bool IsOpen() const noexcept { return file_ != nullptr; }
            static std::optional<PreparedJxlDibPage> MakePreparedJxlPage(const DibPageView& view);

        private:
            FILE* file_ = nullptr;
            JxlSessionOptions options_{};
            PreparedJxlDibPage page_{};
            bool hasPage_ = false;
    };

    class DTWAINJxlOutput 
    {
        public:
            bool OnFirstPage(const std::wstring& filename, const JxlSessionOptions& options, const PreparedJxlDibPage& page);
            bool OnLastPage();
        private:
            std::unique_ptr<JxlSessionWriter> writer_;
    };
}
#endif
