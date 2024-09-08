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
#include "dtwain.h"
#include "ctliface.h"
#include "dtwain_resource_constants.h"
#include "ctltwmgr.h"
#include "transym_ocrinterface.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4505)
#endif
// OCR routines
static bool OCREngineExists(CTL_TwainDLLHandle* pHandle, OCREngine* pEngine);
static bool OCRIsActive(const OCREngine* pEngine);
static LONG GetOCRTextSupport(OCREngine* pEngine, LONG fileType, LONG pixelType, LONG bitDepth);

typedef CTL_StringType(OCREngine::*OCRINFOFUNC)() const;

HANDLE DLLENTRY_DEF DTWAIN_GetOCRText(DTWAIN_OCRENGINE Engine,
    LONG nPageNo,
    LPTSTR Data,
    LONG dSize,
    LPLONG pActualSize,
    LONG nFlags)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, nPageNo, Data, dSize, pActualSize, nFlags))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    // Check if OCR is active
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCRIsActive(pEngine); }, DTWAIN_ERR_OCR_NOTACTIVE, NULL, FUNC_MACRO);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !pEngine->IsValidOCRPage(nPageNo); }, DTWAIN_ERR_OCR_INVALIDPAGENUM, NULL, FUNC_MACRO);

    const std::string sText = pEngine->GetOCRText(nPageNo);

    // Copy actual size data to parameter
    if (pActualSize)
        *pActualSize = static_cast<long>(sText.length());
    const int localActualSize = static_cast<int>(sText.length());

    // Return the handle if that is all user wants to do
    if (nFlags & DTWAINOCR_RETURNHANDLE)
    {
        HANDLE theHandle;
        const TW_MEMREF hMem = pHandle->m_TwainMemoryFunc->AllocateMemoryPtr(localActualSize*sizeof(TCHAR), &theHandle);
        memcpy(hMem, sText.c_str(), localActualSize*sizeof(TCHAR));
        pHandle->m_TwainMemoryFunc->UnlockMemory(theHandle);
        LOG_FUNC_EXIT_NONAME_PARAMS(theHandle)
    }
    else
        if (nFlags & DTWAINOCR_COPYDATA)
        {
            if (!Data)
            {
                // cache the info
                LOG_FUNC_EXIT_NONAME_PARAMS(HANDLE(1))
            }
            int nMinCopy;

            if (dSize == -1)
                nMinCopy = localActualSize;
            nMinCopy = (std::max)((std::min)(dSize, static_cast<LONG>(localActualSize)), static_cast<LONG>(0));

            memcpy(Data, sText.data(), nMinCopy * sizeof(TCHAR));
            LOG_FUNC_EXIT_NONAME_PARAMS(HANDLE(1))
        }
    LOG_FUNC_EXIT_NONAME_PARAMS(NULL)
    CATCH_BLOCK(HANDLE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetOCRCapValues(DTWAIN_OCRENGINE Engine,LONG OCRCapValue,LONG GetType,LPDTWAIN_ARRAY CapValues)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, OCRCapValue, GetType, CapValues))

    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    if (pEngine->IsCapSupported(OCRCapValue))
    {
        const LONG nDataType = pEngine->GetCapDataType(OCRCapValue);
        if (nDataType == -1)
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        switch (nDataType)
        {
            case DTWAIN_ARRAYLONG:
            {
                OCREngine::OCRLongArrayValues vals;
                pEngine->GetCapValues(OCRCapValue, GetType, vals);
                const DTWAIN_ARRAY theArray = CreateArrayFromFactory(DTWAIN_ARRAYLONG, static_cast<LONG>(vals.size()));

                for (LONG i = 0; i < static_cast<LONG>(vals.size()); ++i)
                    DTWAIN_ArraySetAtLong(theArray, i, vals[i]);
                *CapValues = theArray;
                LOG_FUNC_EXIT_NONAME_PARAMS(true)
            }
            break;
            case DTWAIN_ARRAYSTRING:
            {
                OCREngine::OCRStringArrayValues vals;
                pEngine->GetCapValues(OCRCapValue, GetType, vals);
                const DTWAIN_ARRAY theArray = CreateArrayFromFactory(DTWAIN_ARRAYSTRING, static_cast<LONG>(vals.size()));
                for (LONG i = 0; i < static_cast<LONG>(vals.size()); ++i)
                    DTWAIN_ArraySetAtStringA(theArray, i, vals[i].c_str());
                *CapValues = theArray;
                LOG_FUNC_EXIT_NONAME_PARAMS(true)
            }
            break;
        default: ;
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetOCRCapValues(DTWAIN_OCRENGINE Engine, LONG OCRCapValue, LONG SetType, DTWAIN_ARRAY CapValues)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, OCRCapValue, SetType, CapValues))

    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    if (pEngine->IsCapSupported(OCRCapValue))
    {
        const LONG nDataType = pEngine->GetCapDataType(OCRCapValue);
        if (nDataType == -1)
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        switch (nDataType)
        {
            case DTWAIN_ARRAYLONG:
            {
                OCREngine::OCRLongArrayValues vals;
                auto& factory = pHandle->m_ArrayFactory;
                const LONG nCount = static_cast<LONG>(factory->size(CapValues));
                if (nCount < 1)
                    LOG_FUNC_EXIT_NONAME_PARAMS(false)
                vals.resize(nCount);
                auto ArrayStart = reinterpret_cast<LONG *>(factory->get_buffer(CapValues, 0));
                std::copy_n(ArrayStart, nCount, vals.begin());
                const BOOL bRet = pEngine->SetCapValues(OCRCapValue, SetType, vals);
                LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
            }
            break;

            case DTWAIN_ARRAYSTRING:
            {
                OCREngine::OCRStringArrayValues vals;
                auto& factory = pHandle->m_ArrayFactory;
                const LONG nCount = static_cast<LONG>(factory->size(CapValues));
                if (nCount < 1)
                    LOG_FUNC_EXIT_NONAME_PARAMS(false)
                vals.resize(nCount);
                for (LONG i = 0; i < nCount; ++i)
                {
                    char buffer[1024];
                    DTWAIN_ArrayGetAtStringA(CapValues, i, buffer);
                    vals[i] = buffer;
                }
                const BOOL bRet = pEngine->SetCapValues(OCRCapValue, SetType, vals);
                LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
            }
        default: ;
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}

//////////////////// OCR information functions /////////////////////////
static LONG GetOCRInfo(OCREngine *pEngine,OCRINFOFUNC pFunc,LPTSTR szInfo, LONG nMaxLen);

LONG GetOCRInfo(OCREngine *pEngine, OCRINFOFUNC pFunc, LPTSTR szInfo, LONG nMaxLen)
{
    const CTL_StringType pName = (pEngine->*pFunc)();
    const int nLen = static_cast<int>(pName.length());
    if (szInfo == nullptr)
        return static_cast<LONG>(nLen);
    const int nRealLen = (std::min)(static_cast<int>(nMaxLen), nLen);
    StringTraits::CopyN(szInfo, pName.c_str(), nRealLen);
    szInfo[nRealLen] = _T('\0');
    return nRealLen;
}

LONG   DLLENTRY_DEF DTWAIN_GetOCRManufacturer(DTWAIN_OCRENGINE Engine,LPTSTR szMan,LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, szMan, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    if (pEngine)
    {
        const LONG Ret = GetOCRInfo(pEngine, reinterpret_cast<OCRINFOFUNC>(&OCREngine::GetManufacturer),
                                    szMan, nMaxLen);
        LOG_FUNC_EXIT_NONAME_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG   DLLENTRY_DEF DTWAIN_GetOCRProductFamily(DTWAIN_OCRENGINE Engine,LPTSTR szMan,LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, szMan, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
                DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    if (pEngine)
    {
        const LONG Ret = GetOCRInfo(pEngine, reinterpret_cast<OCRINFOFUNC>(&OCREngine::GetProductFamily), szMan, nMaxLen);
        LOG_FUNC_EXIT_NONAME_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

LONG   DLLENTRY_DEF DTWAIN_GetOCRProductName(DTWAIN_OCRENGINE Engine,LPTSTR szMan,LONG nMaxLen)
{
    if (szMan)
        szMan[0] = '\0';
    LOG_FUNC_ENTRY_PARAMS((Engine, szMan, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    if (pEngine)
    {
        const LONG Ret = GetOCRInfo(pEngine, reinterpret_cast<OCRINFOFUNC>(&OCREngine::GetProductName), szMan, nMaxLen);
        LOG_FUNC_EXIT_NONAME_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(-1L)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ExecuteOCR(DTWAIN_OCRENGINE Engine, LPCTSTR szFileName, LONG nStartPage, LONG nEndPage)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, szFileName, nStartPage, nEndPage))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    // Check if OCR is active
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCRIsActive(pEngine); }, DTWAIN_ERR_OCR_NOTACTIVE, false, FUNC_MACRO);

    if (nStartPage > nEndPage)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)

    const CTL_StringType sFileName = szFileName;
    int stat;
    const LONG nPages = pEngine->GetNumPagesInFile(sFileName, stat);
    if (nPages < 0)
    {
        std::string s = pEngine->GetReturnCodeString(stat);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }
    if (nStartPage == -1)
    {
        nStartPage = 0;
        nEndPage = nPages - 1;
    }
    else
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return nStartPage >= nPages; },
        DTWAIN_ERR_OCR_INVALIDPAGENUM, false, FUNC_MACRO);

    const LONG minEndPage = (std::min)(nEndPage, nPages - 1);

    LONG curPage = nStartPage;
    pEngine->ClearCharacterInfoMap();
    while (curPage <= minEndPage)
    {
        // Reset that everything is OK so far
        pEngine->SetOkErrorCode();
        pEngine->SetCurrentPageNumber(curPage);
        const LONG retCode = pEngine->StartOCR(sFileName);

        // Check for an error here
        if (!pEngine->IsReturnCodeOk(retCode))
        {
            // No good, we need to get out and report the error
            pEngine->SetLastError(retCode);
            DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; },
                DTWAIN_ERR_OCR_RECOGNITIONERROR, false, FUNC_MACRO);
        }
        ++curPage;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_OCRTEXTINFOHANDLE DLLENTRY_DEF DTWAIN_GetOCRTextInfoHandle(DTWAIN_OCRENGINE Engine, LONG nPageNo)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, nPageNo))
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    // Check if OCR is active
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCRIsActive(pEngine); }, DTWAIN_ERR_OCR_NOTACTIVE, false, FUNC_MACRO);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !pEngine->IsValidOCRPage(nPageNo); }, DTWAIN_ERR_OCR_INVALIDPAGENUM, NULL, FUNC_MACRO);

    // If nNumInfo is not NULL, fill it in with the number of items
    int status;
    std::vector<OCRCharacterInfo>& cInfo = pEngine->GetCharacterInfo(nPageNo, status);

    OCRCharacterInfo* pInfo = &cInfo[0];
    const auto pReturn = static_cast<DTWAIN_OCRTEXTINFOHANDLE>(pInfo);
    LOG_FUNC_EXIT_NONAME_PARAMS(pReturn)
    CATCH_BLOCK(DTWAIN_OCRTEXTINFOHANDLE())
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetOCRTextInfoLong(DTWAIN_OCRTEXTINFOHANDLE OCRTextInfo,LONG nCharPos,LONG nWhichItem,LPLONG pInfo)
{
    LOG_FUNC_ENTRY_PARAMS((OCRTextInfo, nCharPos, nWhichItem, pInfo))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !pInfo; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    const OCRCharacterInfo *cInfo = static_cast<OCRCharacterInfo*>(OCRTextInfo);

    switch (nWhichItem)
    {
    case DTWAIN_OCRINFO_CHAR:
        *pInfo = cInfo->iChar[nCharPos];
        break;

    case DTWAIN_OCRINFO_CHARXPOS:
        *pInfo = cInfo->xPosition[nCharPos];
        break;

    case DTWAIN_OCRINFO_CHARYPOS:
        *pInfo = cInfo->yPosition[nCharPos];
        break;

    case DTWAIN_OCRINFO_CHARXWIDTH:
        *pInfo = cInfo->xWidth[nCharPos];
        break;

    case DTWAIN_OCRINFO_CHARYWIDTH:
        *pInfo = cInfo->yWidth[nCharPos];
        break;

    case DTWAIN_OCRINFO_PAGENUM:
        *pInfo = cInfo->nPage;
        break;

    case DTWAIN_OCRINFO_TEXTLENGTH:
        *pInfo = static_cast<LONG>(cInfo->iChar.size());
        break;

    default:
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    break;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetOCRTextInfoFloat(DTWAIN_OCRTEXTINFOHANDLE OCRTextInfo,LONG nCharPos,LONG nWhichItem,LPDTWAIN_FLOAT pInfo)
{
    LOG_FUNC_ENTRY_PARAMS((OCRTextInfo, nCharPos, nWhichItem, pInfo))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !pInfo; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    const OCRCharacterInfo *cInfo = static_cast<OCRCharacterInfo*>(OCRTextInfo);

    switch (nWhichItem)
    {
    case DTWAIN_OCRINFO_CHARCONFIDENCE:
        *pInfo = cInfo->dConfidence[nCharPos];
        break;

    default:
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return TRUE; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    break;

    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetOCRTextInfoLongEx(DTWAIN_OCRTEXTINFOHANDLE OCRTextInfo,LONG nWhichItem,LPLONG pInfo,LONG bufSize)
{

    LOG_FUNC_ENTRY_PARAMS((OCRTextInfo, nWhichItem, pInfo, bufSize))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !pInfo; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    auto cInfo = static_cast<OCRCharacterInfo*>(OCRTextInfo);

    auto itStart = cInfo->iChar.end();
    auto itEnd = cInfo->iChar.end();

    switch (nWhichItem)
    {
    case DTWAIN_OCRINFO_CHAR:
        itStart = cInfo->iChar.begin();
        itEnd = cInfo->iChar.end();
        break;

    case DTWAIN_OCRINFO_CHARXPOS:
        itStart = cInfo->xPosition.begin();
        itEnd = cInfo->xPosition.end();
        break;

    case DTWAIN_OCRINFO_CHARYPOS:
        itStart = cInfo->yPosition.begin();
        itEnd = cInfo->yPosition.end();
        break;

    case DTWAIN_OCRINFO_CHARXWIDTH:
        itStart = cInfo->xWidth.begin();
        itEnd = cInfo->xWidth.end();
        break;

    case DTWAIN_OCRINFO_CHARYWIDTH:
        itStart = cInfo->yWidth.begin();
        itEnd = cInfo->yWidth.end();
        break;

    default:
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    break;
    }
    LONG actualSize = 0;
    if (itStart != itEnd)
    {
        --itEnd;
        const LONG realSize = static_cast<LONG>(std::distance(itStart, itEnd)) + 1;
        actualSize = (std::min)(bufSize, realSize);
    }
    std::copy_n(itStart, actualSize, pInfo);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_GetOCRTextInfoFloatEx(DTWAIN_OCRTEXTINFOHANDLE OCRTextInfo,LONG nWhichItem,LPDTWAIN_FLOAT pInfo,LONG bufSize)
{
    LOG_FUNC_ENTRY_PARAMS((OCRTextInfo, nWhichItem, pInfo, bufSize))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !pInfo; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);

    auto cInfo = static_cast<OCRCharacterInfo*>(OCRTextInfo);
    const LONG realSize = static_cast<LONG>(std::distance(cInfo->dConfidence.begin(), cInfo->dConfidence.end()));
    const LONG actualSize = (std::min)(bufSize, realSize);
    switch (nWhichItem)
    {
    case DTWAIN_OCRINFO_CHARCONFIDENCE:
        std::copy_n(cInfo->dConfidence.begin(), actualSize, pInfo);
        break;

    default:
    {
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [] { return 1; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }
    break;

    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
        CATCH_BLOCK(false)
}

LONG DLLENTRY_DEF DTWAIN_SetPDFOCRConversion(DTWAIN_OCRENGINE Engine,LONG PageType, LONG FileType,LONG PixelType, LONG BitDepth, LONG Options)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, PageType, FileType, PixelType, BitDepth))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    // check if PageType is OK
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !(PageType == 0 || PageType == 1); },
        DTWAIN_ERR_INVALID_PARAM, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    // Check if BW format, pixel type, and bit depth are supported
    const LONG bRet = GetOCRTextSupport(pEngine, FileType, PixelType, BitDepth);
    if (bRet == 0)
    {
        pEngine->m_OCRPDFInfo.FileType[PageType] = FileType;
        pEngine->m_OCRPDFInfo.PixelType[PageType] = PixelType;
        pEngine->m_OCRPDFInfo.BitDepth[PageType] = BitDepth;
        pEngine->SetBaseOption(OCROPTION_STORECLEANTEXT1, Options & OCROPTION_STORECLEANTEXT1);
        LOG_FUNC_EXIT_NONAME_PARAMS(1)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK(-1L)
}

LONG GetOCRTextSupport(OCREngine* pEngine, LONG fileType, LONG pixelType, LONG bitDepth)
{
    // Get file type support
    OCREngine::OCRLongArrayValues vals;
    bool bOK = pEngine->GetCapValues(DTWAIN_OCRCV_IMAGEFILEFORMAT, DTWAIN_CAPGET, vals);
    if (!bOK)
        return DTWAIN_ERR_OCR_INVALIDFILETYPE;
    if (vals.empty())
        return DTWAIN_ERR_OCR_INVALIDFILETYPE;
    auto it = std::find(vals.begin(), vals.end(), fileType);
    if (it == vals.end())
        return DTWAIN_ERR_OCR_INVALIDFILETYPE;

    // File type exists, so see if pixel type exists
    bOK = pEngine->GetCapValues(DTWAIN_OCRCV_PIXELTYPE, DTWAIN_CAPGET, vals);
    if (!bOK)
        return DTWAIN_ERR_OCR_INVALIDPIXELTYPE;
    if (vals.empty())
        return DTWAIN_ERR_OCR_INVALIDPIXELTYPE;
    it = std::find(vals.begin(), vals.end(), pixelType);
    if (it == vals.end())
        return DTWAIN_ERR_OCR_INVALIDPIXELTYPE;

    // Now select the pixel type, but remember the last one to reset it.
    pEngine->GetCapValues(DTWAIN_OCRCV_PIXELTYPE, DTWAIN_CAPGETCURRENT, vals);
    const LONG lastPixelType = vals[0];
    vals[0] = pixelType;
    pEngine->SetCapValues(DTWAIN_OCRCV_PIXELTYPE, DTWAIN_CAPSET, vals);

    // Get the bit depths for this type
    bOK = pEngine->GetCapValues(DTWAIN_OCRCV_BITDEPTH, DTWAIN_CAPGET, vals);
    LONG retVal = 0;
    if (!bOK || vals.empty())
        retVal = DTWAIN_ERR_OCR_INVALIDBITDEPTH;
    if (retVal == 0)
    {
        const OCREngine::OCRLongArrayValues::iterator it2 = std::find(vals.begin(), vals.end(), bitDepth);
        if (it2 == vals.end())
            retVal = DTWAIN_ERR_OCR_INVALIDBITDEPTH;
    }

    // reset pixel type
    vals.resize(1);
    vals[0] = lastPixelType;
    pEngine->SetCapValues(DTWAIN_OCRCV_PIXELTYPE, DTWAIN_CAPSET, vals);
    return retVal;
}

LONG DLLENTRY_DEF DTWAIN_GetOCRVersionInfo(DTWAIN_OCRENGINE Engine, LPTSTR buffer, LONG maxBufSize)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, buffer, maxBufSize))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);
    std::string sVersion = pEngine->GetOCRVersionInfo();
    const auto retVal = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(sVersion), buffer, maxBufSize);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(0)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumOCRSupportedCaps(DTWAIN_OCRENGINE Engine, LPDTWAIN_ARRAY SupportedCaps)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, SupportedCaps))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const OCREngine *pEngine = static_cast<OCREngine*>(Engine);

    OCREngine::OCRLongArrayValues vals;
    pEngine->GetSupportedCaps(vals);
    const DTWAIN_ARRAY theArray = CreateArrayFromFactory(DTWAIN_ARRAYLONG, static_cast<LONG>(vals.size()));
    if (!theArray)
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(theArray);
    std::copy(vals.begin(), vals.end(), vValues.begin());
    *SupportedCaps = theArray;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

bool OCREngineExists(CTL_TwainDLLHandle* pHandle, OCREngine* pEngine)
{
    return
        std::find_if(pHandle->m_OCRInterfaceArray.begin(),
        pHandle->m_OCRInterfaceArray.end(),
        SmartPointerFinder<OCREnginePtr>(pEngine)) != pHandle->m_OCRInterfaceArray.end();
}

bool OCRIsActive(const OCREngine* pEngine)
{
    return pEngine->IsActivated();
}
void dynarithmic::LoadOCRInterfaces(CTL_TwainDLLHandle *pHandle)
{
    pHandle->m_OCRProdNameToEngine.clear();

    const OCREnginePtr pInterface = std::make_shared<TransymOCR>(pHandle);
    if (pInterface->IsInitialized())
        pHandle->m_OCRInterfaceArray.push_back(pInterface);

    // Add other engines here.

    // Call virtuals to set up the product names.
    auto it = pHandle->m_OCRInterfaceArray.begin();
    while (it != pHandle->m_OCRInterfaceArray.end())
    {
        (*it)->SetOCRVersionIdentity();
        pHandle->m_OCRProdNameToEngine.insert({(*it)->GetProductName(), *it});
        ++it;
    }

    // Set first OCR engine to be the default engine
    if (!pHandle->m_OCRInterfaceArray.empty())
        pHandle->m_pOCRDefaultEngine = *pHandle->m_OCRInterfaceArray.begin();
}

void dynarithmic::UnloadOCRInterfaces(CTL_TwainDLLHandle *pHandle)
{
    pHandle->m_OCRInterfaceArray.clear();
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngineByName(LPCTSTR lpszName)
{
    LOG_FUNC_ENTRY_PARAMS((lpszName))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    const std::string sName = StringConversion::Convert_NativePtr_To_Ansi(lpszName);

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);

    // Get the OCR engine associated with the name
    const auto it = pHandle->m_OCRProdNameToEngine.find(sName);
    OCREnginePtr SelectedEngine;
    DTWAIN_OCRENGINE ocrEngine_ = nullptr;
    if (it != pHandle->m_OCRProdNameToEngine.end())
    {
        SelectedEngine = it->second;
        if (SelectedEngine)
        {
            pHandle->m_pOCRDefaultEngine = SelectedEngine;

            if (!SelectedEngine->IsActivated())
                SelectedEngine->StartupOCREngine();
            ocrEngine_ = SelectedEngine.get();
        }
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(ocrEngine_)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_InitOCRInterface()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    if (pHandle->m_OCRInterfaceArray.empty())
        LoadOCRInterfaces(pHandle);

    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumOCRInterfaces(LPDTWAIN_ARRAY OCRArray)
{
    LOG_FUNC_ENTRY_PARAMS((OCRArray))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    if (pHandle->m_OCRInterfaceArray.empty())
        *OCRArray = nullptr;
    else
    {
        const DTWAIN_ARRAY theArray = CreateArrayFromFactory(DTWAIN_ARRAYOCRENGINE, 0);
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !theArray; }, DTWAIN_ERR_OUT_OF_MEMORY, false, FUNC_MACRO);

        const auto& factory = pHandle->m_ArrayFactory;
        auto& vEnum = factory->underlying_container_t<OCREngine*>(theArray);
        vEnum.clear();

        for (auto &ocr : pHandle->m_OCRInterfaceArray)
            vEnum.push_back(ocr.get());

        *OCRArray = theArray;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_ShutdownOCREngine(DTWAIN_OCRENGINE Engine)
{
    LOG_FUNC_ENTRY_PARAMS((Engine))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);
    int status;
    pEngine->ShutdownOCR(status);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_StartupOCREngine(DTWAIN_OCRENGINE Engine)
{
    LOG_FUNC_ENTRY_PARAMS((Engine))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, false, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);
    //    int status;
    if (!pEngine->IsActivated())
    {
        const LONG bRet = pEngine->StartupOCREngine();
        if (bRet != 0)
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectDefaultOCREngine()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);

    // Get the OCR engine associated with the name
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return pHandle->m_OCRInterfaceArray.empty(); },
        DTWAIN_ERR_OCR_NOTACTIVE, 0, FUNC_MACRO);
    const auto SelectedEngine = static_cast<DTWAIN_OCRENGINE>(pHandle->m_pOCRDefaultEngine.get());

    const auto pEngine = static_cast<OCREngine*>(SelectedEngine);
    if (!pEngine->IsActivated())
        pEngine->StartupOCREngine();

    LOG_FUNC_EXIT_NONAME_PARAMS(SelectedEngine)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsOCREngineActivated(DTWAIN_OCRENGINE Engine)
{
    LOG_FUNC_ENTRY_PARAMS((Engine))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    LONG retVal = FALSE;
    if (pEngine)
    {
        const bool bRet = pEngine->IsActivated();
        retVal = bRet ? TRUE : FALSE;
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal)
    CATCH_BLOCK(false)
}


LONG DLLENTRY_DEF DTWAIN_GetOCRLastError(DTWAIN_OCRENGINE Engine)
{
    LOG_FUNC_ENTRY_PARAMS((Engine))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);
    if (pEngine)
    {
        const LONG bRet = pEngine->GetLastError();
        LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(0)
    CATCH_BLOCK(0)
}

LONG DLLENTRY_DEF DTWAIN_GetOCRErrorString(DTWAIN_OCRENGINE Engine, LONG lError, LPTSTR lpszBuffer, LONG nMaxLen)
{
    LOG_FUNC_ENTRY_PARAMS((Engine, lError, lpszBuffer, nMaxLen))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, -1L, nullptr);

    // check if Engine exists
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !OCREngineExists(pHandle, static_cast<OCREngine*>(Engine)); },
        DTWAIN_ERR_OCR_INVALIDENGINE, NULL, FUNC_MACRO);

    const auto pEngine = static_cast<OCREngine*>(Engine);

    if (pEngine)
    {
        if (lError < 0)
        {
            // This is a DTWAIN error, not an OCR specific error
            const LONG retval = DTWAIN_GetErrorString(lError, lpszBuffer, nMaxLen);
            LOG_FUNC_EXIT_NONAME_PARAMS(retval)
        }
        const LONG nTotalBytes = StringWrapper::CopyInfoToCString(StringConversion::Convert_Ansi_To_Native(pEngine->GetErrorString(lError)), lpszBuffer, nMaxLen);
        LOG_FUNC_EXIT_NONAME_PARAMS(nTotalBytes)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(-1)
    CATCH_BLOCK(-1)
}

#ifdef _WIN32

bool NewOCRJob(OCREngine *pEngine, LPCSTR szFileName)
{
    std::string s1 = pEngine->GetCachedFile();
    std::string s2 = szFileName;
    s1 = StringWrapperA::TrimAll(s1);
    s1 = StringWrapperA::MakeLowerCase(s1);
    s2 = StringWrapperA::TrimAll(s2);
    s2 = StringWrapperA::MakeLowerCase(s2);
    return s1 != s2;
}

LRESULT CALLBACK DisplayOCRDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


DTWAIN_OCRENGINE DLLENTRY_DEF DTWAIN_SelectOCREngine()
{
    LOG_FUNC_ENTRY_NONAME_PARAMS()
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex( pHandle, NULL, FUNC_MACRO);

    // Get the resource for the Twain dialog
    const HGLOBAL hglb = LoadResource(CTL_StaticData::s_DLLInstance,
                                      static_cast<HRSRC>(FindResource(CTL_StaticData::s_DLLInstance,
                                                                      MAKEINTRESOURCE(10000), RT_DIALOG)));
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return  !hglb;}, DTWAIN_ERR_NULL_WINDOW, NULL, FUNC_MACRO);

    const auto lpTemplate = static_cast<LPDLGTEMPLATE>(LockResource(hglb));

    SelectStruct S;
    S.CS.xpos = 0;
    S.CS.ypos = 0;
    S.CS.nOptions = DTWAIN_DLG_CENTER_SCREEN;
    S.CS.hWndParent = nullptr;
    S.nItems = 0;
    S.CS.sTitle = _T("Select OCR Engine");
    const INT_PTR bRet = DialogBoxIndirectParam(CTL_StaticData::s_DLLInstance,
                                                lpTemplate, nullptr,
                                                reinterpret_cast<DLGPROC>(DisplayOCRDlgProc),
                                                reinterpret_cast<LPARAM>(&S));
    if ( bRet == -1 )
        LOG_FUNC_EXIT_NONAME_PARAMS(0)

    // See if cancel was selected
    if ( S.SourceName.empty() || S.nItems == 0 )
        LOG_FUNC_EXIT_NONAME_PARAMS(0)

    const DTWAIN_OCRENGINE SelectedEngine = DTWAIN_SelectOCREngineByName(S.SourceName.c_str());
    LOG_FUNC_EXIT_NONAME_PARAMS((DTWAIN_OCRENGINE)SelectedEngine)
    CATCH_BLOCK(DTWAIN_OCRENGINE())
}



/////////////////////////////////////////////////////////////////////////////////
/// DTWAIN OCR Dialog procedure
static void DisplayLocalString(HWND hWnd, int nID, int resID);

LRESULT CALLBACK DisplayOCRDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND lstSources;
    static SelectStruct *pS;
    switch (message)
    {
    case WM_INITDIALOG:
        {
            pS = reinterpret_cast<SelectStruct*>(lParam);

            if ( pS->CS.nOptions & DTWAIN_DLG_CENTER_SCREEN )
                CenterWindow(hWnd, nullptr);
            else
            if ( pS->CS.nOptions & DTWAIN_DLG_CENTER)
                CenterWindow(hWnd, GetParent(hWnd));
            else
                SetWindowPos(hWnd, nullptr, pS->CS.xpos, pS->CS.ypos, 0, 0, SWP_NOSIZE);

            lstSources = GetDlgItem(hWnd, IDC_LSTSOURCES);

            // Set the title
            ::SetWindowText(hWnd, pS->CS.sTitle.c_str());

            // Fill the list box with the sources
            DTWAIN_ARRAY Array = nullptr;
            DTWAIN_EnumOCRInterfaces(&Array);
            const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
            DTWAINArrayLowLevel_RAII raii(pHandle, Array);
            const int nCount = DTWAIN_ArrayGetCount(Array);
            pS->nItems = nCount;
            if ( nCount == 0 )
            {
                const HWND hWndSelect = GetDlgItem(hWnd, IDOK);
                if ( hWndSelect )
                    EnableWindow(hWndSelect, FALSE);
                return TRUE;
            }

            constexpr TCHAR DefName[255] = {0};
            std::vector<CTL_StringType> SourceNames;
            SourceNames.reserve(nCount);
            int i;
            DTWAIN_OCRENGINE TempOCR;
            for ( i = 0; i < nCount; i++ )
            {
                DTWAIN_ArrayGetAt(Array, i, &TempOCR);
                TCHAR ProdName[256];
                DTWAIN_GetOCRProductName(TempOCR, ProdName, 255);
                SourceNames.push_back(ProdName);
            }

            // Sort the names
            if ( pS->CS.nOptions & DTWAIN_DLG_SORTNAMES)
                sort(SourceNames.begin(), SourceNames.end());

            LRESULT DefIndex = 0;
            for ( i = 0; i < nCount; i++ )
            {
                const LRESULT index = SendMessage(lstSources, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(SourceNames[i].c_str()));
                if ( lstrcmp(SourceNames[i].c_str(), static_cast<LPCTSTR>(DefName)) == 0)
                    DefIndex = index;
            }
            if ( i > 0 )
                SendMessage(lstSources, LB_SETCURSEL, DefIndex, 0);

            // Display the local strings if they are available:
            DisplayLocalString(hWnd, IDOK, IDS_SELECT_TEXT);
            DisplayLocalString(hWnd, IDCANCEL, IDS_CANCEL_TEXT);
            DisplayLocalString(hWnd, IDC_SOURCETEXT, IDS_SOURCES_TEXT);
            return TRUE;
        }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            TCHAR sz[255];
            const LRESULT nSel = SendMessage(lstSources, LB_GETCURSEL, 0, 0);
            SendMessage(lstSources, LB_GETTEXT, nSel, reinterpret_cast<LPARAM>(sz));
            pS->SourceName = sz;
            EndDialog(hWnd, LOWORD(wParam));
            return TRUE;
        }
        else
            if (LOWORD(wParam) == IDCANCEL)
            {
                pS->SourceName = {};
                EndDialog(hWnd, LOWORD(wParam));
                return TRUE;
            }
            break;
    }
    return FALSE;
}

static std::string GetTwainDlgTextFromResource(int nID, int& status)
{
    char buffer[256];
    status = static_cast<int>(GetResourceStringA(nID, buffer, 255));
    return buffer;
}

static void DisplayLocalString(HWND hWnd, int nID, int resID)
{
    std::string sText;
    int status;
    sText = GetTwainDlgTextFromResource(resID, status);
    if ( status )
    {
        const HWND hWndControl = GetDlgItem(hWnd, nID);
        if ( hWndControl )
            SetWindowTextA(hWndControl, sText.c_str());
    }
}

#endif
