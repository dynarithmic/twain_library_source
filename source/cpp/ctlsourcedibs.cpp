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
#include "cppfunc.h"
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_GetSourceAcquisitions(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const DTWAIN_ARRAY AcqArray = pSource->GetAcquisitionArray();
    if (!AcqArray)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    LOG_FUNC_EXIT_NONAME_PARAMS(AcqArray)
    CATCH_BLOCK_LOG_PARAMS(DTWAIN_ARRAY(0))
}

DTWAIN_BOOL dynarithmic::DTWAIN_GetAllSourceDibs(DTWAIN_SOURCE Source, DTWAIN_ARRAY pArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pArray))
    CTL_ITwainSource* pSource = static_cast<CTL_ITwainSource*>(Source);
    const auto pHandle = pSource->GetDTWAINHandle();
    const auto& factory = pHandle->m_ArrayFactory;

    // Check if array is of the correct type
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !factory->is_valid(pArray, CTL_ArrayFactory::arrayTag::VoidPtrType); },
                                                    DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
    const DTWAIN_ARRAY pDTWAINArray = pArray;
    factory->clear(pDTWAINArray);

    // Copy DIBs to the array
    const int nCount = pSource->GetNumDibs();
    HANDLE hDib;
    for (int i = 0; i < nCount; i++)
    {
        hDib = pSource->GetDibHandle(i);
        if (hDib)
            factory->add_to_back(pArray, hDib, 1); 
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAllSourceDibsEx(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY  pArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pArray))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const DTWAIN_ARRAY DibArray = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYHANDLE, 0);
    if (DibArray)
    {
        DTWAIN_GetAllSourceDibs(Source, DibArray);
        *pArray = DibArray;
        LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

HANDLE DLLENTRY_DEF DTWAIN_GetCurrentAcquiredImage(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const int nCount = pSource->GetNumDibs();
    if (nCount == 0)
        LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    LOG_FUNC_EXIT_NONAME_PARAMS((HANDLE)pSource->GetDibHandle(nCount - 1))
    CATCH_BLOCK_LOG_PARAMS(HANDLE(0))
}

LONG DLLENTRY_DEF DTWAIN_GetCurrentPageNum(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    const LONG retval = static_cast<LONG>(pSource->GetPendingImageNum());
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK_LOG_PARAMS(-1L)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_CreateAcquisitionArray()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);
    const DTWAIN_ARRAY AcqArray = static_cast<DTWAIN_ARRAY>(CreateArrayFromFactory(pHandle, DTWAIN_ARRAYOFHANDLEARRAYS, 0));
    LOG_FUNC_EXIT_NONAME_PARAMS(AcqArray)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

// class whose purpose is to destroy the image data array
struct NestedAcquisitionDestroyer
{
    CTL_TwainDLLHandle* m_pHandle;
    bool m_bDestroyDibs;
    NestedAcquisitionDestroyer(CTL_TwainDLLHandle* pHandle, bool bDestroyDibs) : m_pHandle(pHandle), m_bDestroyDibs(bDestroyDibs) {}

    void operator()(DTWAIN_ARRAY ImagesArray) const
    {
        // we want this array destroyed when we're finished
        DTWAINArrayLowLevel_RAII raii(m_pHandle, ImagesArray);

        // Test if the DIB data should also be destroyed
        if (m_bDestroyDibs)
        {
            // get underlying vector of dibs
            auto& vHandles = m_pHandle->m_ArrayFactory->underlying_container_t<void*>(ImagesArray);

            // for each dib, destroy the data
            std::for_each(vHandles.begin(), vHandles.end(), DestroyDibData);
        }
    }

    static void DestroyDibData(HANDLE hImageData)
    {
        #ifdef _WIN32
        const UINT nCount = GlobalFlags(hImageData) & GMEM_LOCKCOUNT;
        #else
        UINT nCount = 1;
        #endif
        for (UINT k = 0; k < nCount; k++)
            ImageMemoryHandler::GlobalUnlock(hImageData);
        ImageMemoryHandler::GlobalFree(hImageData);
    }
};

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_DestroyAcquisitionArray(DTWAIN_ARRAY aAcq, DTWAIN_BOOL bDestroyDibs)
{
    LOG_FUNC_ENTRY_PARAMS((aAcq))
    auto [pHandle, pSource] = VerifyHandles(nullptr, DTWAIN_VERIFY_DLLHANDLE);

    const auto& factory = pHandle->m_ArrayFactory;

    // Check if array exists
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !factory->is_valid(aAcq); }, DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);

    // Make sure this array is destroyed when we exit this function
    DTWAINArrayLowLevel_RAII raiiMain(pHandle, aAcq);

    // get instance of acquisition destroy class
    const NestedAcquisitionDestroyer acqDestroyer(pHandle, bDestroyDibs ? true : false);

    // underlying images array
    auto& vImagesArray = factory->underlying_container_t<CTL_ArrayFactory::tagged_array_voidptr*>(aAcq);

    // for each image array, destroy it
    std::for_each(vImagesArray.begin(), vImagesArray.end(), acqDestroyer);

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

static bool SetBitDepth(CTL_ITwainSource *p, LONG BitDepth);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ForceAcquireBitDepth(DTWAIN_SOURCE Source, LONG BitDepth)
{
    LOG_FUNC_ENTRY_PARAMS((Source, BitDepth))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const DTWAIN_BOOL bRet = SetBitDepth(pSource, BitDepth);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

static bool SetBitDepth(CTL_ITwainSource *pTheSource, LONG BitDepth)
{
    if (BitDepth == 1 ||
        BitDepth == 4 ||
        BitDepth == 8 ||
        BitDepth == 24)
    {
        pTheSource->SetForcedImageBpp(BitDepth);
        return true;
    }
    else
        pTheSource->SetForcedImageBpp(0);
    return true;
}
