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
#include <algorithm>
#include "ctltwmgr.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "sourceacquireopts.h"
#include "ctlfileutils.h"
#include "ctltwainmsgloop.h"

#include "cppfunc.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#pragma warning (disable:4714)
#endif

using namespace dynarithmic;

DTWAIN_BOOL       DLLENTRY_DEF DTWAIN_AcquireFileEx(DTWAIN_SOURCE Source,
                                                    DTWAIN_ARRAY aFileNames,
                                                    LONG     lFileType,
                                                    LONG     lFileFlags,
                                                    LONG     PixelType,
                                                    LONG     lMaxPages,
                                                    DTWAIN_BOOL bShowUI,
                                                    DTWAIN_BOOL bCloseSource,
                                                    LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, aFileNames, lFileType, lFileFlags, PixelType, lMaxPages, bShowUI,bCloseSource, pStatus))
    auto bRetval = true;
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    CTL_ITwainSource* pSource = VerifySourceHandle(pHandle, Source);
    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(false)

    // Check if the file format is valid
    auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
    if (availableFileTypes.find(lFileType) == availableFileTypes.end())
    {
        DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
        LOG_FUNC_EXIT_PARAMS(false)
    }

    DTWAIN_ARRAY tempNames = nullptr;
    DTWAINArrayPtr_RAII tempRAII(&tempNames);
    DTWAIN_ARRAY arrayToUse = aFileNames;
    if (aFileNames)
    {
        std::vector<LONG> validTypes = {DTWAIN_ARRAYSTRING, DTWAIN_ARRAYANSISTRING, DTWAIN_ARRAYWIDESTRING};
        auto& factory = pHandle->m_ArrayFactory;
        const LONG Type = factory->tagtype_to_arraytype(factory->tag_type(aFileNames));
        const auto itArrType = std::find(validTypes.begin(), validTypes.end(), Type);
        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return itArrType == validTypes.end(); }, DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
        const auto idx = std::distance(validTypes.begin(), itArrType);
        if ( idx > 0 )
        {
            tempNames = CreateArrayFromFactory(DTWAIN_ARRAYSTRING, 0);
            if ( idx == 1 )
                ArrayCopyAnsiToNative(aFileNames, tempNames);
            else
                ArrayCopyWideToNative(aFileNames, tempNames);
            arrayToUse = tempNames;
        }
    }
    else
        bRetval = false;

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] { return !bRetval; }, DTWAIN_ERR_BAD_ARRAY, false, FUNC_MACRO);

    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(GetDTWAINHandle_Internal()).setSource(Source).setFileType(lFileType).setFileFlags(lFileFlags | DTWAIN_USELIST).
                setFileList(arrayToUse).setPixelType(PixelType).setMaxPages(lMaxPages).setShowUI(bShowUI ? true : false).
                setRemainOpen(!(bCloseSource ? true : false));

    bRetval = AcquireFileHelper(opts, ACQUIREFILE);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_PARAMS(bRetval)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL       DLLENTRY_DEF DTWAIN_AcquireFile(DTWAIN_SOURCE Source,
                                                  LPCTSTR   lpszFile,
                                                  LONG     lFileType,
                                                  LONG     lFileFlags,
                                                  LONG     PixelType,
                                                  LONG     lMaxPages,
                                                  DTWAIN_BOOL bShowUI,
                                                  DTWAIN_BOOL bCloseSource,
                                                  LPLONG pStatus)
{
    LOG_FUNC_ENTRY_PARAMS((Source, lpszFile, lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus))

    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    DTWAIN_Check_Bad_Handle_Ex(pHandle, false, FUNC_MACRO);
    CTL_ITwainSource* pSource = VerifySourceHandle(pHandle, Source);
    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(false)

    // Check if the file format is valid
    auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
    if (availableFileTypes.find(lFileType) == availableFileTypes.end())
    {
        DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
        LOG_FUNC_EXIT_PARAMS(false)
    }

    lFileFlags &= ~DTWAIN_USELIST;
    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(GetDTWAINHandle_Internal()).setSource(Source).
        setFileName(lpszFile).setFileType(lFileType).setFileFlags(lFileFlags).setPixelType(PixelType).
        setMaxPages(lMaxPages).setShowUI(bShowUI ? true : false).setRemainOpen(!(bCloseSource ? true : false)).setAcquireType(ACQUIREFILE);
    const bool bRetval = AcquireFileHelper(opts, ACQUIREFILE);
    if (pStatus)
        *pStatus = opts.getStatus();
    LOG_FUNC_EXIT_PARAMS(bRetval)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileAutoIncrement(DTWAIN_SOURCE Source, LONG nValue, DTWAIN_BOOL bResetOnAcquire, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nValue, bResetOnAcquire, bSet))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);
    if (!pSource)
        LOG_FUNC_EXIT_PARAMS(false)

    pSource->SetFileAutoIncrement(bSet ? true : false, nValue);
    pSource->SetFileAutoIncrementFlags(bResetOnAcquire ? DTWAIN_INCREMENT_DYNAMIC : DTWAIN_INCREMENT_DEFAULT);
    /*    if ( nInitial < -1 )
    nInitial = 0;                                    */
    pSource->SetFileAutoIncrementBase(0); //nInitial );
    LOG_FUNC_EXIT_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireFile(SourceAcquireOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    const DTWAIN_ARRAY FileList = opts.getFileList();
    if (FileList)
        opts.setFileFlags(opts.getFileFlags() | DTWAIN_USELIST);
    if ( opts.getAcquireType() != TWAINAcquireType_AudioFile)
        opts.setActualAcquireType(TWAINAcquireType_File);
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    if (pHandle->m_lAcquireMode == DTWAIN_MODELESS)
        return LLAcquireImage(opts);
    auto pr = dynarithmic::StartModalMessageLoop(opts.getSource(), opts);
    LOG_FUNC_EXIT_PARAMS(pr.second)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

template <typename T>
static std::vector<T> FileListToVector(SourceAcquireOptions& opts)
{
    std::vector<T> allNames;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    auto fileList = opts.getFileList();
    if (fileList)
        allNames = pHandle->m_ArrayFactory->underlying_container_t<T>(fileList);
    else
        allNames.push_back(opts.getFileName());
    return allNames;
}

static std::string GetDirectoryCreationError(CTL_StringType fileName)
{
    return  GetResourceStringFromMap(IDS_LOGMSG_ERRORTEXT) + ": DTWAIN_AcquireFile: " +
            GetResourceStringFromMap(DTWAIN_ERR_CREATE_DIRECTORY_) + ": " +
            StringConversion::Convert_Native_To_Ansi(fileName.c_str());
}

bool dynarithmic::AcquireFileHelper(SourceAcquireOptions& opts, LONG AcquireType)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    CTL_ITwainSource *pSource = VerifySourceHandle(GetDTWAINHandle_Internal(), opts.getSource());

    DumpArrayContents(opts.getFileList(), 0);
    #ifdef _UNICODE
        auto vTest = FileListToVector<std::wstring>(opts);
    #else
        auto vTest = FileListToVector<std::string>(opts);
    #endif

    opts.setAcquireType(AcquireType);
    opts.setDiscardDibs(true); // make sure we remove acquired dibs for file handling
    // set the auto create directory if indicated
    bool bCreateDir = opts.getFileFlags() & DTWAIN_CREATE_DIRECTORY;
    pSource->SetFileAutoCreateDirectory(bCreateDir);

    // if the auto-create is not on, let's do a quick test to see if the file can be written to the
    // directory specified.
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    bool bUsePrompt = opts.getFileFlags() & DTWAIN_USEPROMPT;
    if (!bUsePrompt)
    {
        // The following loop makes sure that all of the files specified have directories that are writable
        for (auto& fileName : vTest)
        {
            if (!bCreateDir)
            {
                // Check for existing writable directory
                if (!dynarithmic::directory_writeable(fileName.c_str()))
                {
                    CTL_TwainAppMgr::WriteLogInfoA(GetDirectoryCreationError(dynarithmic::get_parent_directory(fileName.c_str(), false)));
                    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]{ return true; }, DTWAIN_ERR_INVALID_DIRECTORY, false, FUNC_MACRO);
                }
            }
            else
            {
                if (!parent_directory_exists(fileName.c_str()).first)
                {
                    auto testDir = dynarithmic::get_parent_directory(fileName.c_str(), false);

                    // auto-create the directory
                    const auto dirCreated = dynarithmic::create_directory(dynarithmic::get_parent_directory(fileName.c_str(), false).c_str());

                    if (!dirCreated.first)
                    {
                        // directory creation failed for one of the files.  
                        CTL_TwainAppMgr::WriteLogInfoA(GetDirectoryCreationError(testDir));
                        DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&]
                            { return dirCreated.first == false;  }, DTWAIN_ERR_CREATE_DIRECTORY, false, FUNC_MACRO);
                    }
                }
            }
        }
    }
    const DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    if (opts.getStatus() < 0 && !aDibs)
    {
        LOG_FUNC_EXIT_PARAMS(false)
    }

    bool bRetval = false;
    if (aDibs)
    {
        bRetval = TRUE;
        if (DTWAIN_GetTwainMode() == DTWAIN_MODAL)
        {
            auto& factory = pHandle->m_ArrayFactory;
            auto pVariant = aDibs;
            const auto& vDibs = 
                factory->underlying_container_t<CTL_ArrayFactory::tagged_array_voidptr*>(pVariant);
            std::for_each(begin(vDibs), end(vDibs), [&](HANDLE dib) {factory->destroy(dib); });
            pSource->ResetAcquisitionAttempts(nullptr);
            if (factory->is_valid(pVariant))
                factory->destroy(pVariant);
        }
    }

    if (DTWAIN_GetTwainMode() == DTWAIN_MODAL)
    {
        if (!aDibs)
            bRetval = false;
        else
        if (opts.getStatus() == DTWAIN_TN_ACQUIREDONE)
            bRetval = true;
    }
    else
    if (DTWAIN_GetTwainMode() == DTWAIN_MODELESS)
        pSource->m_pUserPtr = nullptr;

    LOG_FUNC_EXIT_PARAMS(bRetval)
    CATCH_BLOCK(false)
}

