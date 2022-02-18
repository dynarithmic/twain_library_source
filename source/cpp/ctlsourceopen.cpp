/*
    This file is part of the Dynarithmic TWAIN Library (DTWAIN).
    Copyright (c) 2002-2022 Dynarithmic Software.

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
#include "cppfunc.h"
#include "ctltwmgr.h"
#include "enumeratorfuncs.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

static void LogAndCachePixelTypes(CTL_ITwainSource *p);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_OpenSourcesOnSelect(DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((bSet))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    pHandle->m_bOpenSourceOnSelect = bSet ? true : false;
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsOpenSourcesOnSelect(VOID_PROTOTYPE)
{
    LOG_FUNC_ENTRY_PARAMS(())
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    const bool retVal = pHandle->m_bOpenSourceOnSelect;
    LOG_FUNC_EXIT_PARAMS(retVal)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_OpenSource(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto p = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    bool bRetval = false;
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    bRetval = CTL_TwainAppMgr::OpenSource(pHandle->m_pTwainSession, p);
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !bRetval || !p; }, DTWAIN_ERR_BAD_SOURCE, false, FUNC_MACRO);

    // Cache the pixel types and bit depths
    LogAndCachePixelTypes(p);

    DTWAIN_ARRAY arr = nullptr;
    DTWAINArrayPtr_RAII raii(&arr);
    DTWAIN_EnumSupportedCaps(Source, &arr);

    // if any logging is turned on, then get the capabilities and log the values
    if (CTL_TwainDLLHandle::s_lErrorFilterFlags)
    {
        CTL_StringType msg = _T("Source: ") + p->GetProductName() + _T(" has been opened successfully");
        CTL_TwainAppMgr::WriteLogInfo(msg);

        // Log the caps if logging is turned on
        CTL_StringType sName;

        auto& vCaps = EnumeratorVector<LONG>(arr);
        std::vector<std::string> VecString(vCaps.size());

        // copy the names
        std::transform(vCaps.begin(), vCaps.end(), VecString.begin(), [](LONG n) { return CTL_TwainAppMgr::GetCapNameFromCap(n); });

        // Sort the names
        std::sort(VecString.begin(), VecString.end());
        CTL_StringStreamType strm;
        strm << vCaps.size();
        sName = _T("\n\nSource \"");
        sName += p->GetProductName();
        sName += _T("\" contains the following ");
        sName += strm.str() + _T(" capabilities: \n{\n");
        if (vCaps.empty())
            sName += _T(" No capabilities:\n");
        else
        {
            sName += _T("    ");
            sName += StringConversion::Convert_Ansi_To_Native(StringWrapperA::Join(VecString, "\n    "));
        }
        sName += _T("\n}\n");

        CTL_TwainAppMgr::WriteLogInfo(sName);
    }

    LOG_FUNC_EXIT_PARAMS(bRetval)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceOpen(DTWAIN_SOURCE Source)
{
    //    TRY_BLOCK
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(false)
    const DTWAIN_BOOL bRet = CTL_TwainAppMgr::IsSourceOpen(p);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

void LogAndCachePixelTypes(CTL_ITwainSource *p)
{
    static const int MaxMessage = 1024;
    if (p->PixelTypesRetrieved())
        return;

    p->SetCurrentlyProcessingPixelInfo(true);
    char szName[MaxMessage + 1];
    LONG oldflags = CTL_TwainDLLHandle::s_lErrorFilterFlags;

    if (oldflags)
        DTWAIN_GetSourceProductNameA(p, szName, MaxMessage);

    std::string sName = szName;
    std::string sBitDepths;
    DTWAIN_ARRAY PixelTypes;

    // enumerate all of the pixel types
    DTWAIN_BOOL bOK = DTWAIN_GetCapValues(p, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPGET, &PixelTypes);
    if (bOK)
    {
        DTWAINArrayLL_RAII arrP(PixelTypes);
        auto vPixelTypes = EnumeratorVectorPtr<LONG>(PixelTypes);

        LONG nCount = vPixelTypes ? static_cast<LONG>(vPixelTypes->size()) : 0;
        if (nCount > 0)
        {
            // create an array of 1
            DTWAIN_ARRAY vCurPixType = DTWAIN_ArrayCreate(DTWAIN_ARRAYLONG, 1);
            DTWAINArrayLL_RAII raii(vCurPixType);

            // get pointer to internals of the array
            auto& vCurPixTypePtr = EnumeratorVector<LONG>(vCurPixType);

            for (LONG i = 0; i < nCount; ++i)
            {
                // current pixel type
                vCurPixTypePtr[0] = (*vPixelTypes)[i];
                LONG& curPixType = vCurPixTypePtr[0];
                // Set the pixel type temporarily
                if (DTWAIN_SetCapValues(p, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPSET, vCurPixType))
                {
                    // Add to source list
                    // Now get the bit depths for this pixel type
                    DTWAIN_ARRAY BitDepths = nullptr;
                    if (DTWAIN_GetCapValues(p, DTWAIN_CV_ICAPBITDEPTH, DTWAIN_CAPGET, &BitDepths))
                    {
                        DTWAINArrayLL_RAII arr(BitDepths);

                        // Get the total number of bit depths.
                        auto vBitDepths = EnumeratorVectorPtr<LONG>(BitDepths);

                        LONG nCountBPP = vBitDepths ? static_cast<LONG>(vBitDepths->size()) : 0;
                        if (oldflags)
                        {
                            StringStreamA strm;
                            strm << "\nFor source \"" << sName << "\", there are (is) " <<
                                nCountBPP << " available bit depth(s) for pixel type " <<
                                curPixType << "\n";
                            sBitDepths += strm.str();
                        }
                        LONG nCurBitDepth;
                        for (LONG j = 0; j < nCountBPP; ++j)
                        {
                            nCurBitDepth = (*vBitDepths)[j];
                            p->AddPixelTypeAndBitDepth(curPixType, nCurBitDepth);
                            if (oldflags)
                            {
                                StringStreamA strm;
                                strm << "Bit depth[" << j << "] = " << nCurBitDepth << "\n";
                                sBitDepths += strm.str();
                            }
                        }
                    }
                }
            }
            DTWAIN_SetCapValues(p, DTWAIN_CV_ICAPPIXELTYPE, DTWAIN_CAPRESET, nullptr);
        }
    }
    if (oldflags && bOK )
        CTL_TwainAppMgr::WriteLogInfoA(sBitDepths);
    else
    if (!bOK)
        CTL_TwainAppMgr::WriteLogInfoA("Could not retrieve bit depth information\n");
    p->SetCurrentlyProcessingPixelInfo(false);
}
