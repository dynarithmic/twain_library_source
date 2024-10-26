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
#include "ctltwainmanager.h"
#include "ctltmpl5.h"
#include "ctliface.h"
#include "cppfunc.h"
#include "ctlreg.h"

#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif
#undef min
#undef max

using namespace dynarithmic;

#define DTWAIN_STATE4   8

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetQueryCapSupport(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL dynarithmic::DTWAIN_CacheCapabilityInfo(CTL_ITwainSource *p, CTL_TwainDLLHandle *pHandle, TW_UINT16 nCapToCache)
{
    CTL_EnumeratorNode<LONG>::container_base_type vCaps(1, nCapToCache);
    return DTWAIN_CacheCapabilityInfo(p, pHandle, &vCaps);
}

DTWAIN_BOOL dynarithmic::DTWAIN_CacheCapabilityInfo(CTL_ITwainSource *pSource, CTL_TwainDLLHandle *pHandle, CTL_EnumeratorNode<LONG>::container_pointer_type vCaps)
{
    if (pSource->RetrievedAllCaps())
        return true;
    struct CapFinder
    {
        CTL_ITwainSource *m_ps;
        LONG m_nCap;
        CapFinder(CTL_ITwainSource *ps, LONG nCap) : m_ps(ps), m_nCap(nCap) {}
        bool operator()(const CTL_CapInfo& CapInfo) const
        {
            if (static_cast<int>(std::get<CAPINFO_IDX_CAPABILITY>(CapInfo)) == m_nCap)
            {
                m_ps->SetCapCached(static_cast<TW_UINT16>(m_nCap), true);
                return true;
            }
            return false;
        }
    };

    // Check if this source has had capabilities negotiated and tested
    int nWhere;

    CTL_CapInfoMapPtr pArray;
    bool bNewArray = false;

    // get the array of cap info for this source
    pArray = GetCapInfoArray(pHandle, pSource);
    if (!pArray)
    {
        // create a new one
        pArray.reset(new CTL_CapInfoMap);
        bNewArray = true;
    }

    static const ContainerMap mapContainer = {{"TW_ONEVALUE", DTWAIN_CONTONEVALUE},
                                                  {"TW_ENUMERATION", DTWAIN_CONTENUMERATION},
                                                  {"TW_RANGE", DTWAIN_CONTRANGE},
                                                  {"TW_ARRAY", DTWAIN_CONTARRAY},
                                                  {"-1", static_cast<UINT>(-1)},
                                                  {"0", 0}};

    FindFirstValue(pSource->GetProductName(), &pHandle->m_aSourceCapInfo, &nWhere);

    auto vIt = vCaps->begin();
    CTL_SourceCapInfo InfoSource;
    for (; vIt != vCaps->end(); ++vIt)
    {
        if (nWhere != -1 && pSource->IsCapabilityCached(static_cast<TW_UINT16>(*vIt))) // Already negotiated
            continue;

        bool bCanQuerySupport = true;
        // Not found, so test capabilities
        // Create these dynamically whenever a new source is opened
        // and source cap info does not exist.  Add cap info statically.

        bool bGetINIEntry = true;

        bool bOk = false;

        // search the current cap array for the cap value to be tested
        if (pArray->find(static_cast<TW_UINT16>(*vIt)) != pArray->end()) //find_if(pArray->begin(), pArray->end(), CapFinder(pSource, *vIt)) != pArray->end() )
        {
            pSource->SetCapCached(static_cast<TW_UINT16>(*vIt), true);
            continue;
        }

        // if we get here, then the cap was never tested.
        auto nCap = static_cast<TW_UINT16>(*vIt);

        // Add capabilities where the state info is set
        pSource->AddCapToStateInfo(CAP_CUSTOMDSDATA, DTWAIN_STATE4);

        // Test the capability and see which container works.
        UINT cGet = 0;
        UINT cGetCurrent = 0;
        UINT cGetDefault = 0;
        UINT cSetCurrent = 0;
        UINT cSetAvailable = 0;
        UINT cQuerySupport = 0;
        UINT cEOJValue = 0;
        UINT cEntryFound = 0;
        UINT cResetSupport = 0;
        UINT cQueryContainer = 0;
        bool bContainerInfoFound = false;

        TW_UINT16 cStateInfo = 0xFF;
        UINT nDataType = 0;
        std::string strName = CTL_TwainAppMgr::GetCapNameFromCap(nCap);
        std::string sProdNameA = StringConversion::Convert_Native_To_Ansi(pSource->GetProductName());

        bOk = GetCapInfoFromIni(strName, sProdNameA, static_cast<UINT>(nCap), cGet, cGetCurrent,
                                cGetDefault, cSetCurrent, cSetAvailable, cResetSupport, cQueryContainer, cQuerySupport,
                                cEOJValue, cStateInfo, nDataType, cEntryFound, bContainerInfoFound, mapContainer);

        bCanQuerySupport = cQuerySupport ? true : false;

        if (cEntryFound)
            pSource->SetEOJDetectedValue(cEOJValue);

        if (!cEntryFound)
            bGetINIEntry = false;
        else
        {
            if (cStateInfo != 0xFF)
                pSource->AddCapToStateInfo(nCap, cStateInfo);
        }

        if (bOk)
        {
            if (CTL_StaticData::GetLogFilterFlags())
            {
                StringStreamOutA strm;
                strm << "Using capability info from DTWAIN32.INI (Source="
                     << sProdNameA << ", Cap=" << CTL_TwainAppMgr::GetCapNameFromCap(nCap) << ")\n";

                CTL_TwainAppMgr::WriteLogInfoA(strm.str());
            }

            if (bContainerInfoFound)
            {
                CTL_CapStruct capData;
                capData.m_nGetContainer = cGet;

                CTL_CapInfo Info(static_cast<CTL_EnumCapability>(nCap), cGet, cSetCurrent, nDataType, 0, cGetCurrent, 
                                                            cGetDefault, cSetAvailable, cResetSupport, cQueryContainer);
                pArray->insert(make_pair(nCap, Info));
                return true;
            }
        }

        auto& allCapInfo = CTL_StaticData::GetGeneralCapInfo();
        auto thisCapInfo = allCapInfo.find(nCap);
        if (thisCapInfo != allCapInfo.end())
        {
            auto& capData = thisCapInfo->second;
            CTL_CapInfo Info(static_cast<CTL_EnumCapability>(nCap),
                capData.m_nGetContainer, capData.m_nSetContainer, capData.m_nDataType, 0, capData.m_nGetCurrentContainer,
                capData.m_nGetDefaultContainer, capData.m_nSetConstraintContainer, capData.m_nResetContainer, capData.m_nQuerySupportContainer);
            pArray->insert(make_pair(nCap, Info));
        }
        else
        {
            // This is probably a custom capability
            CTL_CapInfo Info(static_cast<CTL_EnumCapability>(nCap), 0, 0, 0, 0, 0, 0, 0, 0, 0);
            pArray->insert(make_pair(nCap, Info));
        }

        if (bNewArray)
            InfoSource = CTL_SourceCapInfo(pSource->GetProductName(), pArray, 0, 0, 0, 0, 0, 0,0,0);

        pSource->SetCapCached(static_cast<TW_UINT16>(*vIt), true);
    }

    if (bNewArray)
        pHandle->m_aSourceCapInfo.push_back(InfoSource);
    return true;
}
