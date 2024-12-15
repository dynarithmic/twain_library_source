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
#include "cppfunc.h"
#include "dtwain.h"
#include "ctliface.h"
#include "ctltwainmanager.h"
#include "ctltr037.h"
#include "errorcheck.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

#define GET_FIRST           0
#define GET_NEXT            1
#define GET_CLOSE           2
#define GET_FILE_TYPE       3
#define CHANGE_DIRECTORY    4
#define CREATE_DIRECTORY    5
#define COPY_DIRECTORY      6
#define RENAME_DIRECTORY    7
#define DELETE_DIRECTORY    8
#define FORMAT_MEDIA        9

using FileSysRetType = std::pair<LONG, TW_MEMREF>;
static FileSysRetType FSDirectory(CTL_ITwainSource* pSource, LPCTSTR sDir, LONG nWhich);
static FileSysRetType FSGetFile(CTL_ITwainSource* pSource, LPTSTR sDir, TW_MEMREF FSHandle, LONG nWhich);
static DTWAIN_BOOL FSGetFileInfo(CTL_ITwainSource* Source, LPCTSTR szFileName, TW_FILESYSTEM* pFS);
static bool FSGetCameras(CTL_ITwainSource *pSource, LPDTWAIN_ARRAY Cameras, TW_UINT16 CameraType );
static bool GetResults(CTL_ITwainSource* pSource, CTL_FileSystemTriplet* pFS, TW_UINT16 rc);
static FileSysRetType FSFileOp(CTL_ITwainSource* pSource, LPCTSTR sInput, LPCTSTR sOutput, LONG nWhich);
static DTWAIN_BOOL FSFileOp2(CTL_ITwainSource* pSource, LPCTSTR sInput, DTWAIN_BOOL bRecursive, LONG nWhich);

typedef bool (*WALKFSPROC)(TW_FILESYSTEM* p, LPARAM UserParam);
static bool WalkFileSystem(WALKFSPROC pProc, CTL_ITwainSource* pSource, LPCTSTR szDir, LPARAM UserParam);
static bool EnumCameraProc(TW_FILESYSTEM* p, LPARAM UserParam);

static int CheckFileSystemSupport(CTL_ITwainSource* pSource)
{
    // If source is not open, return an error
    if (!CTL_TwainAppMgr::IsSourceOpen(pSource))
        return DTWAIN_ERR_SOURCE_NOT_OPEN;

    auto getSupport = pSource->IsFileSystemSupported();

    // If already determined that source does not support
    // file system, return error.
    if (!getSupport)
        return DTWAIN_ERR_FILESYSTEM_NOT_SUPPORTED;

    if (getSupport.value == boost::tribool::indeterminate_value)
    {
        // Test if source supports file system
        const DTWAIN_BOOL bRet = FSDirectory(pSource, _T("/"), CHANGE_DIRECTORY).first;
        pSource->SetFileSystemSupported(bRet);

        // return results
        return bRet?DTWAIN_NO_ERROR:DTWAIN_ERR_FILESYSTEM_NOT_SUPPORTED;
    }

    // Already tested and determined that source supports file system
    return DTWAIN_NO_ERROR;
}

/**********************************************************************************/
/**********************************************************************************/
/**********************************************************************************/
struct GetFileOpTraits
{
    static FileSysRetType DoOperation(CTL_ITwainSource* pSource, LPTSTR sz, LPTSTR /* */, TW_MEMREF FSHandle, int operation)
    {
        return FSGetFile(pSource, sz, FSHandle, operation);
    }
};

struct FileOpTraits
{
    static FileSysRetType DoOperation(CTL_ITwainSource* pSource, LPCTSTR sz, LPCTSTR sz2, TW_MEMREF /*FSHandle*/, int operation)
    {
        return FSFileOp(pSource, sz, sz2, operation);
    }
};

struct DirectoryOpTraits
{
    static FileSysRetType DoOperation(CTL_ITwainSource* pSource, LPCTSTR sz, LPCTSTR sz2, TW_MEMREF /*FSHandle*/, int operation)
    {
        return FSDirectory(pSource, sz, operation);
    }
};

template <typename StringType=LPTSTR, typename Fn = GetFileOpTraits>
static std::pair<int, FileSysRetType> PerfomFileSystemOperation(DTWAIN_SOURCE Source, StringType szDir, StringType szDir2, TW_MEMREF FSHandle, int operation)
{
    CTL_ITwainSource* pSource = static_cast<CTL_ITwainSource*>(Source);
    int fsSupported = CheckFileSystemSupport(pSource);
    if (fsSupported != DTWAIN_NO_ERROR)
        return { fsSupported, {false, {} } };
    auto bRet = Fn::DoOperation(pSource, szDir, szDir2, FSHandle, operation); 
    return { DTWAIN_NO_ERROR, {bRet.first, bRet.second } };
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsFileSystemSupported(DTWAIN_SOURCE Source )
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    int fsSupported = CheckFileSystemSupport(pSource);
    DTWAIN_Check_Error_Condition_1_Ex(pSource->GetDTWAINHandle(), [&] { return fsSupported != DTWAIN_NO_ERROR; },
                                      fsSupported, false, FUNC_MACRO);

    LOG_FUNC_EXIT_NONAME_PARAMS(fsSupported == DTWAIN_NO_ERROR)
    CATCH_BLOCK_LOG_PARAMS(false)
}

TW_MEMREF DTWAIN_FSGetFirstFile(DTWAIN_SOURCE Source, LPTSTR szDir)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szDir))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPTSTR>(pSource, szDir, nullptr, nullptr, GET_FIRST);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                      retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.second)
    CATCH_BLOCK_LOG_PARAMS(nullptr)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSGetNextFile(DTWAIN_SOURCE Source, LPTSTR szDir, TW_MEMREF FSHandle)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szDir, FSHandle))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPTSTR>(pSource, szDir, nullptr, FSHandle, GET_NEXT);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                      retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSGetClose(DTWAIN_SOURCE Source, TW_MEMREF FSHandle)
{
    LOG_FUNC_ENTRY_PARAMS((Source, FSHandle))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPTSTR>(pSource, nullptr, nullptr, FSHandle, GET_CLOSE);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                      retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSCreateDirectory(DTWAIN_SOURCE Source, LPCTSTR szNewDir)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szNewDir))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPCTSTR, DirectoryOpTraits>(pSource, szNewDir, nullptr, 0, CREATE_DIRECTORY);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                      retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSChangeDirectory(DTWAIN_SOURCE Source, LPCTSTR szNewDir)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szNewDir))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPCTSTR, DirectoryOpTraits>(pSource, szNewDir, nullptr, 0, CHANGE_DIRECTORY);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                      retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSFormat(DTWAIN_SOURCE Source, LPCTSTR szDir)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szDir))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPCTSTR, DirectoryOpTraits>(pSource, szDir, nullptr, 0, FORMAT_MEDIA);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                                    retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSRename(DTWAIN_SOURCE Source, LPCTSTR szInput, LPCTSTR szOutput)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szInput, szOutput))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPCTSTR, FileOpTraits>(pSource, szInput, szOutput, 0, RENAME_DIRECTORY);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                                    retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSCopy(DTWAIN_SOURCE Source, LPCTSTR szInput, LPCTSTR szOutput)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szInput, szOutput))
    auto [pHandle, pSource] = VerifyHandles(Source);
    auto retVal = PerfomFileSystemOperation<LPCTSTR, FileOpTraits>(pSource, szInput, szOutput, 0, COPY_DIRECTORY);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return retVal.first != DTWAIN_NO_ERROR; },
                                    retVal.first, false, FUNC_MACRO);
    LOG_FUNC_EXIT_NONAME_PARAMS(retVal.second.first)
    CATCH_BLOCK_LOG_PARAMS(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSDelete(DTWAIN_SOURCE Source, LPCTSTR szInput,
                                         DTWAIN_BOOL bRecursive)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szInput, bRecursive))
    auto [pHandle, pSource] = VerifyHandles(Source);
    int fsSupported = CheckFileSystemSupport(pSource);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return fsSupported != DTWAIN_NO_ERROR; },
        DTWAIN_ERR_FILESYSTEM_NOT_SUPPORTED, fsSupported, FUNC_MACRO);
    const DTWAIN_BOOL bRet = FSFileOp2(pSource, szInput, bRecursive, DELETE_DIRECTORY);

    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_FSGetFileType(DTWAIN_SOURCE Source, LPCTSTR szFileName, LPLONG pFileType)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szFileName, pFileType))
    auto [pHandle, pSource] = VerifyHandles(Source);
    int fsSupported = CheckFileSystemSupport(pSource);

    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return fsSupported != DTWAIN_NO_ERROR; },
        fsSupported, false, FUNC_MACRO);

    TW_FILESYSTEM* pFS = pSource->GetFileSystem();
    const DTWAIN_BOOL bRet = FSGetFileInfo(pSource, szFileName, pFS);
    if (bRet)
        *pFileType = pFS->FileType;
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}

struct CameraStruct {
    DTWAIN_ARRAY aCameras;
    TW_UINT16 CameraType;
};

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumCamerasEx(DTWAIN_SOURCE Source, LONG nWhichCamera, LPDTWAIN_ARRAY Cameras)
{
    LOG_FUNC_ENTRY_PARAMS((Source, nWhichCamera, Cameras))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    int fsSupported = CheckFileSystemSupport(pSource);
    DTWAIN_Check_Error_Condition_1_Ex(pSource->GetDTWAINHandle(), [&] {return fsSupported != DTWAIN_NO_ERROR; },
                                      fsSupported, false, FUNC_MACRO);
    FSGetCameras(pSource, Cameras, static_cast<TW_UINT16>(nWhichCamera));
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumTopCameras(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Cameras)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Cameras))
    auto retval = DTWAIN_EnumCamerasEx(Source, DTWAIN_FT_CAMERATOP, Cameras);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumBottomCameras(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Cameras)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Cameras))
    auto retval = DTWAIN_EnumCamerasEx(Source, DTWAIN_FT_CAMERABOTTOM, Cameras);
    LOG_FUNC_EXIT_NONAME_PARAMS(retval)
    CATCH_BLOCK(false)
}

#define DTWAIN_FT_ALLCAMERAS     0xFFFF

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_EnumCameras(DTWAIN_SOURCE Source, LPDTWAIN_ARRAY Cameras)
{
    LOG_FUNC_ENTRY_PARAMS((Source, Cameras))
    auto [pHandle, pSource] = VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    int fsSupported = CheckFileSystemSupport(pSource);
    DTWAIN_Check_Error_Condition_1_Ex(pHandle, [&] {return fsSupported != DTWAIN_NO_ERROR; },
        fsSupported, false, FUNC_MACRO);
    FSGetCameras(pSource, Cameras, DTWAIN_FT_ALLCAMERAS);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK_LOG_PARAMS(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_SetCamera(DTWAIN_SOURCE Source, LPCTSTR szCamera)
{
    LOG_FUNC_ENTRY_PARAMS((Source, szCamera))
    VerifyHandles(Source, DTWAIN_TEST_SOURCEOPEN_SETLASTERROR);
    const DTWAIN_BOOL bRet = DTWAIN_FSChangeDirectory(Source, szCamera);
    LOG_FUNC_EXIT_NONAME_PARAMS(bRet)
    CATCH_BLOCK_LOG_PARAMS(false)
}


bool FSGetCameras(CTL_ITwainSource *pSource, LPDTWAIN_ARRAY Cameras, TW_UINT16 CameraType )
{
    LOG_FUNC_ENTRY_PARAMS((pSource, Cameras, CameraType))
    const DTWAIN_ARRAY aCameras = CreateArrayFromFactory(pSource->GetDTWAINHandle(), DTWAIN_ARRAYANSISTRING, 0);
    CameraStruct CS{};
    CS.aCameras = aCameras;
    CS.CameraType = CameraType;
    WalkFileSystem(EnumCameraProc, pSource, _T("/"), reinterpret_cast<LPARAM>(&CS));
    *Cameras = CS.aCameras;
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

bool EnumCameraProc(TW_FILESYSTEM* p, LPARAM UserParam)
{
    LOG_FUNC_ENTRY_PARAMS((p, UserParam))
    const CameraStruct *pCS = reinterpret_cast<CameraStruct*>(UserParam);
    if ( pCS->CameraType == DTWAIN_FT_ALLCAMERAS || p->FileType == pCS->CameraType )
        DTWAIN_ArrayAdd(pCS->aCameras, p->OutputName);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

bool WalkFileSystem(WALKFSPROC pProc, CTL_ITwainSource* pSource, LPCTSTR szStart, LPARAM UserParam)
{
    LOG_FUNC_ENTRY_PARAMS((pProc, pSource, szStart, UserParam))
    std::vector<TCHAR> szCurDir(256, 0);
    bool bRes = FSDirectory(pSource, szStart, CHANGE_DIRECTORY).first;
    TW_MEMREF Context = nullptr;
    StringWrapper::SafeStrcpy(&szCurDir[0], szStart);
    TW_FILESYSTEM *pFS = pSource->GetFileSystem();

    // Get the first file in the directory
    auto results = FSGetFile(pSource, szCurDir.data(), nullptr, GET_FIRST);
    bRes = results.first;
    Context = results.second;
    while ( bRes )
    {
        auto FileType = static_cast<TW_UINT16>(pFS->FileType);
        if ( !(*pProc)(pFS, UserParam))
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        switch ( FileType )
        {
            case TWFY_DIRECTORY:
                bRes = WalkFileSystem(pProc, pSource, &szCurDir[0], UserParam);
                if ( bRes )
                {
                    FSGetFile( pSource, nullptr, static_cast<LPLONG>(pFS->Context), GET_CLOSE );
                    LOG_FUNC_EXIT_NONAME_PARAMS(bRes ? true : false)
                }
            break;
        }
        results = FSGetFile(pSource, szCurDir.data(), Context, GET_NEXT);
        bRes = results.first;
        Context = results.second;
    }

    FSGetFile(pSource, nullptr, static_cast<LPLONG>(pFS->Context), GET_CLOSE);
    LOG_FUNC_EXIT_NONAME_PARAMS(true)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL FSGetFileInfo(CTL_ITwainSource* pSource, LPCTSTR szFileName, TW_FILESYSTEM* pFS)
{
    LOG_FUNC_ENTRY_PARAMS((pSource, szFileName, pFS))
    const auto pSession = pSource->GetTwainSession();
    CTL_FileSystemTriplet FS(pSession, pSource);
    const TW_UINT16 rc = FS.GetInfo(szFileName);
    switch(rc)
    {
        case TWRC_SUCCESS:
            *pFS = FS.GetTWFileSystem();
            LOG_FUNC_EXIT_NONAME_PARAMS(TRUE)
        break;

        case TWRC_FAILURE:
        {
            const TW_UINT16 cc = CTL_TwainAppMgr::GetConditionCode(pSession, pSource);
            CTL_TwainAppMgr::ProcessConditionCodeError(cc);
            LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
        }
        break;
        default: {}
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(FALSE)
    CATCH_BLOCK(FALSE)
}

FileSysRetType FSDirectory(CTL_ITwainSource* pSource, LPCTSTR sDir, LONG nWhich)
{
    const auto pSession = pSource->GetTwainSession();
    CTL_FileSystemTriplet FS(pSession, pSource);
    TW_UINT16 rc = 0;
    switch(nWhich)
    {
        case CHANGE_DIRECTORY:
            rc = FS.ChangeDirectory(sDir);
        break;

        case CREATE_DIRECTORY:
            rc = FS.CreateDirectory(sDir);
        break;

        case FORMAT_MEDIA:
            rc = FS.FormatMedia(sDir);
        break;
        default: ;
    }

    DTWAIN_BOOL results = GetResults(pSource, &FS, rc);
    return { results,nullptr };
}


FileSysRetType FSFileOp(CTL_ITwainSource* pSource, LPCTSTR sInput, LPCTSTR sOutput, LONG nWhich)
{
    const auto pSession = pSource->GetTwainSession();
    CTL_FileSystemTriplet FS(pSession, pSource);
    TW_UINT16 rc = 0;
    switch(nWhich)
    {
        case COPY_DIRECTORY:
            rc = FS.CopyFile(sInput, sOutput);
        break;

        case RENAME_DIRECTORY:
            rc = FS.Rename(sInput, sOutput);
        break;
        default: ;
    }

    DTWAIN_BOOL results = GetResults(pSource, &FS, rc);
    return { results,nullptr };
}


DTWAIN_BOOL FSFileOp2(CTL_ITwainSource* pSource, LPCTSTR sInput, DTWAIN_BOOL bRecursive, LONG nWhich)
{
    LOG_FUNC_ENTRY_PARAMS((pSource, sInput, bRecursive, nWhich))
    const auto pSession = pSource->GetTwainSession();
    CTL_FileSystemTriplet FS(pSession, pSource);
    TW_UINT16 rc = 0;
    switch(nWhich)
    {
        case DELETE_DIRECTORY:
            rc = FS.DeleteFile(sInput, bRecursive?true:false);
        break;
        default: ;
    }
    DTWAIN_BOOL results = GetResults(pSource, &FS, rc);
    LOG_FUNC_EXIT_NONAME_PARAMS(results)
    CATCH_BLOCK(FALSE)
}


FileSysRetType FSGetFile(CTL_ITwainSource* pSource, LPTSTR sDir, TW_MEMREF FSHandle, LONG nWhich)
{
    const auto pSession = pSource->GetTwainSession();
    CTL_FileSystemTriplet FS(pSession, pSource);
    TW_UINT16 rc = 0;
    TW_MEMREF returnedContext = nullptr;
    switch(nWhich)
    {
        case GET_FIRST:
            rc = FS.GetFirstFile();
        break;

        case GET_NEXT:
            rc = FS.GetNextFile(FSHandle);
        break;

        case GET_CLOSE:
            rc = FS.GetClose();
        break;
    }

    const bool bRet = GetResults(pSource, &FS, rc);
    if ( bRet )
    {
        TW_FILESYSTEM *pFS = pSource->GetFileSystem();
        if (nWhich != GET_CLOSE)
        {
            returnedContext = pFS->Context;
            StringWrapper::SafeStrcpy(sDir, StringConversion::Convert_AnsiPtr_To_Native(pFS->OutputName).c_str());
        }
    }
    return { bRet, returnedContext };
}

bool GetResults(CTL_ITwainSource* pSource, CTL_FileSystemTriplet* pFST, TW_UINT16 rc)
{
    LOG_FUNC_ENTRY_PARAMS((pSource, pFST, rc))
    const auto pSession = pSource->GetTwainSession();
    switch (rc)
    {
        case TWRC_SUCCESS:
        {
            TW_FILESYSTEM *pFS = pSource->GetFileSystem();
            *pFS = pFST->GetTWFileSystem();
            LOG_FUNC_EXIT_NONAME_PARAMS(true)
        }

        case TWRC_FAILURE:
        {
            const TW_UINT16 cc = CTL_TwainAppMgr::GetConditionCode(pSession, pSource);
            CTL_TwainAppMgr::ProcessConditionCodeError(cc);
            LOG_FUNC_EXIT_NONAME_PARAMS(false)
        }

        default:{}
    }
    LOG_FUNC_EXIT_NONAME_PARAMS(false)
    CATCH_BLOCK(false)
}
