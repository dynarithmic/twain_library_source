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
#ifndef CTLTWAINSESSION_H
#define CTLTWAINSESSION_H
#include <vector>
#include <memory>
#include <unordered_set>
#include "ctlobstr.h"
#include "ctlenum.h"
#include "ctltwainsource.h"
#include "ctltwainidentity.h"

namespace dynarithmic
{
  class CTL_ITwainSession;
  class CTL_TwainDLLHandle;
  class CTL_ITwainSource;
  using CTL_ITwainSessionPtr = std::unique_ptr<CTL_ITwainSession>;
  using CTL_TwainSourceSet = std::unordered_set<CTL_ITwainSource*>;

  class CTL_ITwainSession
  {
    public:
        static CTL_ITwainSession*  Create(CTL_TwainDLLHandle *pHandle, LPCTSTR pAppName, HWND* hAppWnd);

        static void Destroy( CTL_ITwainSessionPtr& pSession );

        CTL_ITwainSession(CTL_TwainDLLHandle* pHandle, LPCTSTR pszAppName, HWND* hAppWnd );

        HWND*               GetWindowHandlePtr() const { return const_cast<HWND*>(&m_AppWnd); }
        TW_IDENTITY*        GetAppIDPtr()              { return &m_AppId.get_identity(); }
        CTL_ITwainSource*    CreateTwainSource( LPCTSTR pProduct );
        bool                AddTwainSource( CTL_ITwainSource *pSource );
        void                CopyAllSources( CTL_TwainSourceSet & rArray );
        int                 GetNumSources();
        void                EnumSources();
        bool                SelectSource(const CTL_ITwainSource* pSource);
        bool                SelectSource(LPCTSTR strName);
        bool                OpenSource(const CTL_ITwainSource* pSource);
        bool                CloseSource(const CTL_ITwainSource* pSource,
                                        bool bForce );
        CTL_ITwainSource*   GetSelectedSource() const;
        CTL_ITwainSource*   Find(const CTL_ITwainSource* pSource);
        void                SetSelectedSource(CTL_ITwainSource* pSource);
        CTL_ITwainSource*   GetDefaultSource();
        bool                IsValidSource(const CTL_ITwainSource* pSource) const;
        void                SetTwainMessageFlag(bool bSet = true);
        bool                IsTwainMsgOn() const
                                    { return m_bTwainMessageFlag; }
        bool                IsAllSourcesRetrieved() const { return m_bAllSourcesRetrieved; }
        void                DestroyOneSource(CTL_ITwainSource *pSource);
        void                DestroyAllSources();
        CTL_TwainDLLHandle* GetTwainDLLHandle() { return m_pTwainDLLHandle; }
        void                SetTwainDLLHandle(CTL_TwainDLLHandle* pHandle) { m_pTwainDLLHandle = pHandle; }
        const CTL_TwainSourceSet& GetTwainSources();
        const CTL_TwainSourceSet& GetCurrentTwainSources() const { return m_arrTwainSource; }
        virtual ~CTL_ITwainSession();

    protected:
        CTL_ITwainSource* IsSourceSelected( LPCTSTR pPsourceName);

        static HWND     CreateTwainWindow();
        bool            IsTwainWindowActive() const;
        void            DestroyTwainWindow();
        void            UpdateStatusMap();


    private:
        bool       m_bAllSourcesRetrieved;
        HWND       m_AppWnd;
        CTL_StringType m_AppName;
        CTL_TwainIdentity m_AppId;          // Twain Identity structure
        CTL_TwainSourceSet m_arrTwainSource;
        CTL_ITwainSource *m_pSelectedSource;
        CTL_TwainDLLHandle *m_pTwainDLLHandle;
        
        bool        m_bTwainWindowCreated;
        bool        m_bTwainMessageFlag;
};

typedef std::vector< CTL_ITwainSessionPtr > CTL_TwainSessionArray;
}
#endif
