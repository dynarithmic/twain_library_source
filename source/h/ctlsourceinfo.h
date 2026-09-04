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
#ifndef CTLSOURCEINFO_H
#define CTLSOURCEINFO_H

#include <bitset>
#include <map>
#include <vector>
#include <string>
#include "ctlstringutilsx.h"

namespace dynarithmic
{
    class CTL_ITwainSource;

    struct SourceXferReadyOverride
    {
        uint32_t m_MaxThreshold = 0;
        uint32_t m_CurrentCount = 0;
        bool m_bSeenUIClose = false;
        bool m_bSeenXferReady = false;
    };

    struct CTL_OnSourceOpenProperties
    {
        bool m_bCheckFeederStatusOnOpen;
        bool m_bQueryBestCapContainer;
        bool m_bQueryCapOperations;
        bool m_bSaveDefaultToINI;
    };

    using SourceToXferReadyMap = std::map<std::string, SourceXferReadyOverride>;

    struct SourceStatus
    {
        enum { SOURCE_STATUS_OPEN, SOURCE_STATUS_SELECECTED, SOURCE_STATUS_UNKNOWN };

        std::bitset<3> m_Status;
        std::string m_ThreadId;
        CTL_ITwainSource* m_pSource;
        SourceStatus() : m_Status(), m_ThreadId{}, m_pSource{} {}
        SourceStatus& SetStatus(int Status, bool bSet) { m_Status[Status] = bSet; return *this; }
        bool GetStatus(int Status) const { return m_Status[Status]; }
        bool IsSelected() const { return m_Status[SOURCE_STATUS_SELECECTED]; }
        bool IsOpen() const { return m_Status[SOURCE_STATUS_OPEN]; }
        bool IsClosed() const { return !IsOpen(); }
        bool IsUnknown() const { return m_Status[SOURCE_STATUS_UNKNOWN]; }
        SourceStatus& SetThreadID(const std::string& threadId) { m_ThreadId = threadId; return *this; }
        SourceStatus& SetSourceHandle(CTL_ITwainSource* Source) { m_pSource = Source; return *this; }
        std::string GetThreadID() const { return m_ThreadId; }
        CTL_ITwainSource* GetSourceHandle() const { return m_pSource; }
    };

    template <typename SourceFunc, typename StringPtrType>
    int32_t GetSourceInfoImpl(CTL_ITwainSource* p, SourceFunc pFunc, StringPtrType szInfo, int32_t nMaxLen)
    {
        return CopyInfoToCString((p->*pFunc)(), szInfo, nMaxLen);
    }

    class CTL_TwainDLLHandle;
    std::vector<CTL_ITwainSource*> GetOpenSources(const CTL_TwainDLLHandle* pHandle);
}
#endif