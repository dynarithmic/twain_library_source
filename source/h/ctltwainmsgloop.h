/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2023 Dynarithmic Software.

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
#ifndef CTLTWAINMSGLOOP_H
#define CTLTWAINMSGLOOP_H

#include "ctliface.h"

namespace dynarithmic
{
    class TwainMessageLoopImpl
    {
    private:
        CTL_TwainDLLHandle* m_pDLLHandle;

    protected:
        DTWAIN_ACQUIRE m_AcquireNum;
        SourceAcquireOptions sOpts;
        virtual bool IsSourceOpen(CTL_ITwainSource* pSource, bool bUIOnly);
        bool IsAcquireTerminated(CTL_ITwainSource* pSource, bool bUIOnly);
        virtual bool CanEnterDispatch(MSG* /*pMsg*/) { return true; }

    public:
        TwainMessageLoopImpl(CTL_TwainDLLHandle* pHandle) : m_pDLLHandle(pHandle), m_AcquireNum{} {}
        TwainMessageLoopImpl(const TwainMessageLoopImpl&) = delete;
        TwainMessageLoopImpl& operator=(const TwainMessageLoopImpl&) = delete;
        TwainMessageLoopImpl(TwainMessageLoopImpl&& rhs) noexcept :
            m_pDLLHandle(rhs.m_pDLLHandle) {
            rhs.m_pDLLHandle = nullptr;
        }
        TwainMessageLoopImpl& operator=(TwainMessageLoopImpl&& rhs) = delete;

        virtual ~TwainMessageLoopImpl() = default;
        virtual void PrepareLoop() {}
        void SetAcquireOptions(SourceAcquireOptions opts) { sOpts = opts; }
        virtual void PerformMessageLoop(CTL_ITwainSource* /*pSource*/, bool /*bUIOnly*/) {}
        DTWAIN_ACQUIRE GetAcquireNum() const { return m_AcquireNum; }
    };

    class TwainMessageLoopWindowsImpl : public TwainMessageLoopImpl
    {
    public:
        TwainMessageLoopWindowsImpl(CTL_TwainDLLHandle* pHandle) : TwainMessageLoopImpl(pHandle) {}
        void PrepareLoop() override
        {
#ifdef WIN32
            MSG msg;
            // Call this so that we have a queue to deal with
            PeekMessage(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
#endif
        }

        void PerformMessageLoop(CTL_ITwainSource* pSource, bool bUIOnly) override;
    };

    class TwainMessageLoopV1 : public TwainMessageLoopWindowsImpl
    {
    public:
        TwainMessageLoopV1(CTL_TwainDLLHandle* pHandle) : TwainMessageLoopWindowsImpl(pHandle) {}
        bool CanEnterDispatch(MSG* pMsg) override { return !DTWAIN_IsTwainMsg(pMsg); }
    };

    class TwainMessageLoopV2 : public TwainMessageLoopWindowsImpl
    {
    public:
        static std::queue<MSG> s_MessageQueue;

        static TW_UINT16 TW_CALLINGSTYLE TwainVersion2MsgProc(
            pTW_IDENTITY pOrigin,
            pTW_IDENTITY pDest,
            TW_UINT32 DG_,
            TW_UINT16 DAT_,
            TW_UINT16 MSG_,
            TW_MEMREF pData
        );

        TwainMessageLoopV2(CTL_TwainDLLHandle* pHandle) : TwainMessageLoopWindowsImpl(pHandle) {}
        void PrepareLoop() override
        {
            // remove elements from the queue
            std::queue<MSG> empty;
            std::swap(s_MessageQueue, empty);
        }

        bool IsSourceOpen(CTL_ITwainSource* pSource, bool bUIOnly) override;
        bool CanEnterDispatch(MSG* pMsg) override { return !DTWAIN_IsTwainMsg(pMsg); }
    };


    std::pair<bool, DTWAIN_ACQUIRE> StartModalMessageLoop(DTWAIN_SOURCE Source, SourceAcquireOptions& opts);
}

#endif