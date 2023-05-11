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
#include "cppfunc.h"
#include "ctltwmgr.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_GetSourceAcquisitions(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(NULL)
    const DTWAIN_ARRAY AcqArray = p->GetAcquisitionArray();
    if (!AcqArray)
        LOG_FUNC_EXIT_PARAMS(NULL)
    LOG_FUNC_EXIT_PARAMS(AcqArray)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_BOOL dynarithmic::DTWAIN_GetAllSourceDibs(DTWAIN_SOURCE Source, DTWAIN_ARRAY pArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);
    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(false)

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
            DTWAIN_ArrayAdd(pArray, hDib);
    }
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetAllSourceDibsEx(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY  pArray)
{
    LOG_FUNC_ENTRY_PARAMS((Source, pArray))
    const DTWAIN_ARRAY DibArray = DTWAIN_ArrayCreate(DTWAIN_ARRAYHANDLE, 0);
    if (DibArray)
    {
        DTWAIN_GetAllSourceDibs(Source, DibArray);
        *pArray = DibArray;
        LOG_FUNC_EXIT_PARAMS(true)
    }
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

HANDLE DLLENTRY_DEF DTWAIN_GetCurrentAcquiredImage(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);
    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(NULL)

    const int nCount = pSource->GetNumDibs();
    if (nCount == 0)
        LOG_FUNC_EXIT_PARAMS(NULL)
    LOG_FUNC_EXIT_PARAMS((HANDLE)pSource->GetDibHandle(nCount - 1))
    CATCH_BLOCK(HANDLE(0))
}

LONG DLLENTRY_DEF DTWAIN_GetCurrentPageNum(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);
    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(-1L)

        // return the file name that would be acquired
    const LONG retval = static_cast<LONG>(pSource->GetPendingImageNum());
    LOG_FUNC_EXIT_PARAMS(retval)
    CATCH_BLOCK(-1L)
}

DTWAIN_ARRAY DLLENTRY_DEF DTWAIN_CreateAcquisitionArray()
{
    LOG_FUNC_ENTRY_PARAMS(())
    const DTWAIN_ARRAY AcqArray = static_cast<DTWAIN_ARRAY>(DTWAIN_ArrayCreate(DTWAIN_ARRAYOFHANDLEARRAYS, 0));
    LOG_FUNC_EXIT_PARAMS(AcqArray)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

// class whose purpose is to destroy the image data array
struct NestedAcquisitionDestroyer
{
    bool m_bDestroyDibs;
    NestedAcquisitionDestroyer(bool bDestroyDibs) : m_bDestroyDibs(bDestroyDibs) {}

    void operator()(DTWAIN_ARRAY ImagesArray) const
    {
        // we want this array destroyed when we're finished
        DTWAINArrayLL_RAII raii(ImagesArray);

        // Test if the DIB data should also be destroyed
        if (m_bDestroyDibs)
        {
            const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            // get underlying vector of dibs
            auto& vHandles = pHandle->m_ArrayFactory->underlying_container_t<void*>(ImagesArray);

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
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, nullptr);

    const auto& factory = pHandle->m_ArrayFactory;

    // Check if array exists
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !factory->is_valid(aAcq); }, DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);

    // Make sure this array is destroyed when we exit this function
    DTWAINArrayLL_RAII raiiMain(aAcq);

    // get instance of acquisition destroy class
    const NestedAcquisitionDestroyer acqDestroyer(bDestroyDibs ? true : false);

    // underlying images array
    auto& vImagesArray = factory->underlying_container_t<CTL_ArrayFactory::tagged_array_voidptr*>(aAcq);

    // for each image array, destroy it
    std::for_each(vImagesArray.begin(), vImagesArray.end(), acqDestroyer);

    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

static bool SetBitDepth(CTL_ITwainSource *p, LONG BitDepth);

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ForceAcquireBitDepth(DTWAIN_SOURCE Source, LONG BitDepth)
{
    LOG_FUNC_ENTRY_PARAMS((Source, BitDepth))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (!p)
        LOG_FUNC_EXIT_PARAMS(false)

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{ return !CTL_TwainAppMgr::IsSourceOpen(p); },
                DTWAIN_ERR_SOURCE_NOT_OPEN, false, FUNC_MACRO);

    const DTWAIN_BOOL bRet = SetBitDepth(p, BitDepth);
    LOG_FUNC_EXIT_PARAMS(bRet)
    CATCH_BLOCK(false)
}

static bool SetBitDepth(CTL_ITwainSource *p, LONG BitDepth)
{
    if (BitDepth == 1 ||
        BitDepth == 4 ||
        BitDepth == 8 ||
        BitDepth == 24)
    {
        p->SetForcedImageBpp(BitDepth);
        return true;
    }
    else
        p->SetForcedImageBpp(0);
    return true;
}
