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
#include <algorithm>
#include "ctltwainmanager.h"
#include "arrayfactory.h"
#include "errorcheck.h"
#include "sourceacquireopts.h"
#include "ctlfileutils.h"
#include "ctltwainmsgloop.h"
#include "ctlsourcedibs.h"

#include "cppfunc.h"
#include "sourceselectopts.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#pragma warning (disable:4714)
#endif

using namespace dynarithmic;

template <typename T, typename SW>
static bool CheckForAnyBlankNames(const T& vect)
{
    for (auto& oneName : vect)
    {
        if (SW::IsAllSpace(oneName))
            return true;
    }
    return false;
}

template <typename StringType, typename StringArrayType, typename StringWrapperType, typename CopyFn>
static int CheckValidNames(CTL_TwainDLLHandle* pHandle, DTWAIN_ARRAY aFileNames, LPDTWAIN_ARRAY tempNames, CopyFn fn)
{
    int bRetval = DTWAIN_NO_ERROR;
    auto& vect = pHandle->m_ArrayFactory->underlying_container_t<StringType>(aFileNames);
    if (!vect.empty())
    {
        if (!CheckForAnyBlankNames<StringArrayType, StringWrapperType>(vect))
        {
            auto retVal = dynarithmic::CreateArrayFromFactory(pHandle, DTWAIN_ARRAYSTRING, 0);
            if (!retVal.second)
                bRetval = retVal.first;
            else
            {
                *tempNames = retVal.second;
                fn(pHandle, aFileNames, *tempNames);
            }
        }
        else
            bRetval = DTWAIN_ERR_BLANKNAMEDETECTED;
    }
    else
        bRetval = DTWAIN_ERR_EMPTY_ARRAY;
    return bRetval;
}

static bool IsSupportedFileType(DTWAIN_SOURCE Source, LONG lFileType, LONG lFileFlags)
{
    bool bFileGood = true;
    // Check if the file format is valid
    auto& availableFileTypes = CTL_StaticData::GetAvailableFileFormatsMap();
    if (availableFileTypes.find(lFileType) == availableFileTypes.end())
    {
        // Not a universal file type, so see if this is a type supported
        // by the Source's file transfer
        if (lFileFlags & DTWAIN_USESOURCEMODE)
            bFileGood = DTWAIN_IsFileXferSupported(Source, lFileType) ? true : false;
        else
            bFileGood = false;
    }
    return bFileGood;
}

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
    int bRetval = DTWAIN_NO_ERROR;
    auto [pHandle, pSource] = VerifyHandles(Source);
    
    AcquireAttemptRAII aRaii(pSource);

    // Check for null aFileNames
    if (!aFileNames)
    {
        DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_BAD_ARRAY, false, FUNC_MACRO);
    }

    // Check if file format is supported
    if (!IsSupportedFileType(Source, lFileType, lFileFlags))
    {
        DTWAIN_SetLastError(DTWAIN_ERR_FILE_FORMAT);
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }

    DTWAIN_ARRAY tempNames = nullptr;
    DTWAINArrayPtr_RAII tempRAII(pHandle, &tempNames);
    DTWAIN_ARRAY arrayToUse = aFileNames;
    std::vector<LONG> validTypes = {DTWAIN_ARRAYSTRING, DTWAIN_ARRAYANSISTRING, DTWAIN_ARRAYWIDESTRING};
    auto& factory = pHandle->m_ArrayFactory;

    const LONG Type = factory->tagtype_to_arraytype(factory->tag_type(aFileNames));
    const auto itArrType = std::find(validTypes.begin(), validTypes.end(), Type);
    DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return itArrType == validTypes.end(); }, DTWAIN_ERR_WRONG_ARRAY_TYPE, false, FUNC_MACRO);
    const auto idx = std::distance(validTypes.begin(), itArrType);

    if ( idx > 0 )
    {
        if (idx == 1)
        {
            // Check for empty array and for blank entries (both are not allowed)
            bRetval = CheckValidNames<std::string, StringArray, StringWrapperA>(pHandle, aFileNames, &tempNames, &ArrayCopyAnsiToNative);
        }
        else
        {
            // Check for empty array and for blank entries (both are not allowed)
            bRetval = CheckValidNames<std::wstring, StringArrayW, StringWrapperW>(pHandle, aFileNames, &tempNames, &ArrayCopyWideToNative);
        }
        if (tempNames)
            arrayToUse = tempNames;
    }

    // Return error if array is empty or if there are blank entries
    DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return bRetval != DTWAIN_NO_ERROR; }, bRetval, false, FUNC_MACRO);

    SourceAcquireOptions opts = SourceAcquireOptions().setHandle(pHandle).setSource(Source).setFileType(lFileType).setFileFlags(lFileFlags | DTWAIN_USELIST).
                setFileList(arrayToUse).setPixelType(PixelType).setMaxPages(lMaxPages).setShowUI(bShowUI ? true : false).
                setRemainOpen(!(bCloseSource ? true : false));

    bool bRetval2 = AcquireFileHelper(opts, ACQUIREFILE);
    if (pStatus)
        *pStatus = opts.getStatus();
    if (opts.getStatus() == DTWAIN_TN_ACQUIRECANCELED)
        CTL_TwainAppMgr::SetError(DTWAIN_ERR_ACQUISITION_CANCELED, "", false);
    else
    if (pSource->GetLastAcquireError() != 0)
        CTL_TwainAppMgr::SetError(pSource->GetLastAcquireError(), "", false);
    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval2)
    CATCH_BLOCK_LOG_PARAMS(false)
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
    auto [pHandle, pSource] = VerifyHandles(Source);

    // Check for null filename
    if (!lpszFile)
    {
        DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&] { return true; }, DTWAIN_ERR_INVALID_PARAM, false, FUNC_MACRO);
    }

    DTWAIN_ARRAY aFileNames = CreateArrayFromFactory(pHandle, DTWAIN_ARRAYSTRING, 0).second;
    DTWAINArrayPtr_RAII tempRAII(pHandle, &aFileNames);

    // Parse the file name list into separate items.  
    ParseFileNames(pHandle, NULL, lpszFile, &aFileNames);

    auto bRetval = DTWAIN_AcquireFileEx(Source, aFileNames, lFileType, lFileFlags, PixelType, lMaxPages, bShowUI, bCloseSource, pStatus);

    LOG_FUNC_EXIT_DEREFERENCE_POINTERS((pStatus))
    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetFileAutoIncrement(DTWAIN_SOURCE Source, LONG nValue, DTWAIN_BOOL bResetOnAcquire, DTWAIN_BOOL bSet)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nValue, bResetOnAcquire, bSet))
    auto [pHandle, pSource] = VerifyHandles(Source);
    pSource->SetFileAutoIncrement(bSet ? true : false, nValue);
    pSource->SetFileAutoIncrementFlags(bResetOnAcquire ? DTWAIN_INCREMENT_DYNAMIC : DTWAIN_INCREMENT_DEFAULT);
    pSource->SetFileAutoIncrementBase(0);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

LONG DLLENTRY_DEF DTWAIN_GetFileSavePageCount(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source);
    LOG_FUNC_EXIT_NONAME_PARAMS(pSource->GetAcquireFileStatusRef().GetFileSavePageCount())
    CATCH_BLOCK_LOG_PARAMS(-1)
}

DTWAIN_ACQUIRE dynarithmic::DTWAIN_LLAcquireFile(SourceAcquireOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    const DTWAIN_ARRAY FileList = opts.getFileList();
    if (FileList)
        opts.setFileFlags(opts.getFileFlags() | DTWAIN_USELIST);
    if ( opts.getAcquireType() != TWAINAcquireType_AudioFile)
        opts.setActualAcquireType(TWAINAcquireType_File);
    auto pSource = reinterpret_cast<CTL_ITwainSource*>(opts.getSource());
    const auto pHandle = pSource->GetDTWAINHandle();
    if (pHandle->m_lAcquireMode == DTWAIN_MODELESS)
        return LLAcquireImage(opts);
    auto pr = dynarithmic::StartModalMessageLoop(pSource, opts);
    DTWAIN_Check_Error_Condition_NoThrow_Ex(pHandle, [&] { return pr.first != DTWAIN_NO_ERROR; }, pr.first, DTWAIN_FAILURE1, FUNC_MACRO);
    if (pr.first != DTWAIN_NO_ERROR)
    {
        CTL_TwainAppMgr::DisableUserInterface(pSource);
        LOG_FUNC_EXIT_NONAME_PARAMS(DTWAIN_FAILURE1)
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(pr.second)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

template <typename T>
static std::vector<T> FileListToVector(SourceAcquireOptions& opts)
{
    std::vector<T> allNames;
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(opts.getHandle());
    auto fileList = opts.getFileList();
    if (fileList)
        allNames = pHandle->m_ArrayFactory->underlying_container_t<T>(fileList);
    else
        allNames.push_back(opts.getFileName());
    return allNames;
}

static std::string GetDirectoryCreationError(CTL_StringViewType fileName)
{
    return  GetResourceStringFromMap(IDS_LOGMSG_ERRORTEXT) + ": DTWAIN_AcquireFile: " +
            GetResourceStringFromMap(-DTWAIN_ERR_CREATE_DIRECTORY) + ": " +
            StringConversion::Convert_NativePtr_To_Ansi(fileName.data());
}

bool dynarithmic::AcquireFileHelper(SourceAcquireOptions& opts, LONG AcquireType)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    CTL_ITwainSource *pSource = reinterpret_cast<CTL_ITwainSource*>(opts.getSource());

    DumpArrayContents(opts.getFileList(), 0, false, false);
    #ifdef _UNICODE
        auto vTest = FileListToVector<std::wstring>(opts);
    #else
        auto vTest = FileListToVector<std::string>(opts);
    #endif

    opts.setDiscardDibs(!(opts.getFileFlags() & DTWAIN_NODELETEDIBS));
    opts.setAcquireType(AcquireType);

    // set the auto create directory if indicated
    bool bCreateDir = opts.getFileFlags() & DTWAIN_CREATE_DIRECTORY;
    pSource->SetFileAutoCreateDirectory(bCreateDir);

    // if the auto-create is not on, let's do a quick test to see if the file can be written to the
    // directory specified.
    const auto pHandle = pSource->GetDTWAINHandle();

    // Set the total file saving page count to 0
    pSource->GetAcquireFileStatusRef().SetFileSavePageCount(0);

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
                    LogWriterUtils::WriteLogInfoIndentedA(GetDirectoryCreationError(dynarithmic::get_parent_directory(fileName.c_str(), false)));
                    DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&]{ return true; }, DTWAIN_ERR_INVALID_DIRECTORY, false, FUNC_MACRO);
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
                        LogWriterUtils::WriteLogInfoIndentedA(GetDirectoryCreationError(testDir));
                        DTWAIN_Check_Error_Condition_Throw_Ex(pHandle, [&]
                            { return dirCreated.first == false;  }, DTWAIN_ERR_CREATE_DIRECTORY, false, FUNC_MACRO);
                    }
                }
            }
        }
    }
    const DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    if (opts.getStatus() < 0 && !aDibs)
    {
        LOG_FUNC_EXIT_NONAME_PARAMS(false)
    }

    bool bRetval = false;
    if (aDibs)
    {
        bRetval = true;
        if (pHandle->m_lAcquireMode == DTWAIN_MODAL)
        {
            if (!(opts.getFileFlags() & DTWAIN_NODELETEDIBS))
            {
                dynarithmic::DestroyAcquisitionArray(pHandle, aDibs, TRUE);
                pSource->ResetAcquisitionAttempts(nullptr);
            }
            else
            {
                pSource->SetUserAcquisitionArray(aDibs);
                pSource->SetAcquisitionAttempts(nullptr);
            }
        }
        bRetval = true;
    }

    if (pHandle->m_lAcquireMode == DTWAIN_MODAL)
    {
        if (!aDibs)
            bRetval = false;
        else
        if (opts.getStatus() == DTWAIN_TN_ACQUIREDONE)
            bRetval = true;
    }
    else
    if (pHandle->m_lAcquireMode == DTWAIN_MODELESS)
        pSource->m_pUserPtr = nullptr;

    LOG_FUNC_EXIT_NONAME_PARAMS(bRetval)
    CATCH_BLOCK(false)
}

