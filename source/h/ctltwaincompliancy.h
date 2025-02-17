/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2025 Dynarithmic Software.

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
#ifndef CTLTWAINCOMPLIANCY_H
#define CTLTWAINCOMPLIANCY_H

#include <array>
#include <string_view>
#include <boost/logic/tribool.hpp>
#include <utility>
#include <algorithm>
#include "dtwaindefs.h"

namespace dynarithmic
{
    class CTL_ITwainSource;
    class TWAINCompliancyTester
    {
        public:
            enum
            {
                TWAIN_PIXELTYPE_TESTS,
                TWAIN_XFERMECH_TESTS,
                TWAIN_CAPABILITY_TESTS,
                VENDOR_CUSTOM_CAPABILITY_TESTS,
                STATUS_RETURN_TESTS,
                STRESS_TESTS,
                NOUI_IMAGE_TRANSFER_TESTS,
                UI_IMAGE_TRANSFER_TESTS,
                ICAP_XFERMECH_TESTS,
                VERSION_TESTS,
                NUM_COMPLIANCY_TESTS
            };

        private:
            CTL_ITwainSource* m_pSource = nullptr;
            std::array<boost::tribool, NUM_COMPLIANCY_TESTS> m_TestResults;
            std::array<int, NUM_COMPLIANCY_TESTS> m_ResultCode;

        public:
            static constexpr std::array<std::string_view, 8> MainTests = {
                "TWAIN Standard Capability Tests",
                "Vendor Custom Capability Tests",
                "Status Return Tests",
                "Stress Tests",
                "Non-UI Image Transfer Tests",
                "UI Image Transfer Tests",
                "ICAP_XFERMECH Tests",
                "Version Tests" };

            TWAINCompliancyTester(CTL_ITwainSource* p = nullptr) : m_pSource(p)
            {
                std::fill_n(m_TestResults.begin(), NUM_COMPLIANCY_TESTS, boost::logic::indeterminate);
                std::fill_n(m_ResultCode.begin(), NUM_COMPLIANCY_TESTS, DTWAIN_NO_ERROR);
            }

            void SetSource(CTL_ITwainSource* p) noexcept { m_pSource = p;  }
            const CTL_ITwainSource* GetSource() noexcept { return m_pSource; }
            std::array<boost::tribool, NUM_COMPLIANCY_TESTS>& GetCompliancyResults() noexcept { return m_TestResults; }
            std::array<int, NUM_COMPLIANCY_TESTS>& GetResultCodes() noexcept { return m_ResultCode; }

            std::pair<bool, int> TestStandardCapabilitiesCompliancy();
            std::pair<bool, int> TestVendorCustomCapabilities();
            std::pair<bool, int> TestPixelTypeCompliancy();
            std::pair<bool, int> TestXfermechCompliancy();
    };
}
#endif