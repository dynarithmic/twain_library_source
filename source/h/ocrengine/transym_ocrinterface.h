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
#ifndef TRANSYM_OCRINTERFACE_H
#define TRANSYM_OCRINTERFACE_H
#include <unordered_map>
#include <boost/dll/shared_library.hpp>

#include "ocrinterface.h"
#include "../ocr/transym/vc++/tocrdll.h"
#include "../ocr/transym/vc++/TOCRuser.h"
#include "../ocr/transym/vc++/TOCRerrs.h"
#include "ctliface.h"
#ifndef _WIN32
#define __stdcall
#endif

namespace dynarithmic
{
    struct TOCRSDK
    {
        struct Functions
        {
            decltype(&TOCRInitialise) Initialise = nullptr;
            decltype(&TOCRShutdown) Shutdown = nullptr;
            decltype(&TOCRGetErrorMode) GetErrorMode = nullptr;
            decltype(&TOCRSetErrorMode) SetErrorMode = nullptr;
            decltype(&TOCRDoJob) DoJob = nullptr;
            decltype(&TOCRWaitForJob) WaitForJob = nullptr;
            decltype(&TOCRWaitForAnyJob) WaitForAnyJob = nullptr;
            decltype(&TOCRGetJobDBInfo) GetJobDBInfo = nullptr;
            decltype(&TOCRGetJobStatus) GetJobStatus = nullptr;
            decltype(&TOCRGetJobStatusEx) GetJobStatusEx = nullptr;
            decltype(&TOCRGetJobStatusMsg) GetJobStatusMsg = nullptr;
            decltype(&TOCRGetNumPages) GetNumPages = nullptr;
            decltype(&TOCRGetJobResults) GetJobResults = nullptr;
            decltype(&TOCRGetJobResultsEx) GetJobResultsEx = nullptr;
            decltype(&TOCRGetLicenceInfo) GetLicenseInfo = nullptr;
            decltype(&TOCRConvertTIFFtoDIB) ConvertTIFFtoDIB = nullptr;
            decltype(&TOCRRotateMonoBitmap) RotateMonoBitmap = nullptr;
            decltype(&TOCRConvertFormat) ConvertFormat = nullptr;
            decltype(&TOCRGetLicenceInfoEx) GetLicenseInfoEx = nullptr;
        };

        TOCRSDK() = default;
        ~TOCRSDK(); 
        boost::dll::shared_library InitTOCR();
        bool IsInitialized() const { return m_hMod.is_loaded(); }
        boost::dll::shared_library GetModuleHandle() const { return m_hMod; }
        boost::dll::shared_library m_hMod = {};
        Functions m_functions{};
    };

    using LongCapMap = BASIC_UNORDERED_MAPTYPE_<LONG,LONG>;
    using StringCapMap = BASIC_UNORDERED_MAPTYPE_<std::string,std::string>;
    using TOCRErrorCodeMap = BASIC_UNORDERED_MAPTYPE_<LONG, std::string>;
    using TOCRBitDepthMap = BASIC_UNORDERED_MAPTYPE_<LONG, std::vector<LONG> >;

    class TransymOCR : public OCREngine
    {
    #ifdef _WIN32
    public:
        TransymOCR(CTL_TwainDLLHandle* pHandle);
        ~TransymOCR() override;
        bool IsInitialized() const override;
        bool SetOptions(OCRJobOptions& options) override;
        LONG StartOCR(CTL_StringType filename) override;
        bool SetFileType() override { return true; }
        std::string GetOCRVersionInfo() override;
        bool SetOCRVersionIdentity() override;
        bool IsReturnCodeOk(LONG returnCode) override;
        std::string GetErrorString(LONG returnCode) override;
        int GetNumPagesInFile(CTL_StringType szFileName, int& errCode) override;
        bool ShutdownOCR(int& status) override;
        LONG StartupOCREngine() override;

    protected:
        bool ProcessGetCapValues(LONG nOCRCap, LONG CapType, OCRLongArrayValues& vals) override;
        bool ProcessGetCapValues(LONG nOCRCap, LONG CapType, OCRStringArrayValues& vals) override;
        bool ProcessSetCapValues(LONG, LONG, const OCRLongArrayValues&) override;
        bool ProcessSetCapValues(LONG nOCRCap, LONG CapType, const OCRStringArrayValues& vals) override;

    private:
        void SetAvailableCaps();
        TOCRSDK m_SDK;
        TOCRJOBINFO m_JobInfo;
        UINT m_nJobRetrieveType;
        LONG m_JobHandle;
        std::string m_sOCRResults;
        LONG ProcessTOCRJob();
        LongCapMap m_OrientationMap;
        TOCRRESULTS m_JobResults;
        TOCRErrorCodeMap m_ErrorCode;
        TOCRBitDepthMap m_BitDepths;
        LongCapMap m_BitDepthsCurrent;
        LongCapMap m_BitDepthsDefault;
    #endif
    };
}
#endif
