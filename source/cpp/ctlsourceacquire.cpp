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
#include <boost/format.hpp>

#include "cppfunc.h"
#include "ctltwmgr.h"
#include "sourceacquireopts.h"
#include "errorcheck.h"
#include "sourceselectopts.h"
#include "arrayfactory.h"
#include "ctltr040.h"
#include "dtwstrfn.h"
#include "ctltwainmsgloop.h"
#ifdef _MSC_VER
#pragma warning (disable:4702)
#endif

using namespace dynarithmic;

template <typename PtrType>
static void ParseFileNames(DTWAIN_ARRAY FileList, PtrType lpszFiles, DTWAIN_ARRAY pArray)
{
    if (FileList)
    {
        DTWAIN_ArrayCopy(FileList, pArray);
        return;
    }

    const CTL_StringType szParseDelim(_T(",;| "));
    const CTL_StringType strTemp(lpszFiles);
    std::vector<CTL_StringType> strArray;

    const int nTokens = StringWrapper::TokenizeQuoted(strTemp, szParseDelim.c_str(), strArray);
    DTWAIN_ArrayRemoveAll(pArray);
    for_each(strArray.begin(), strArray.begin() + nTokens, [&](const CTL_StringType& s)
    {
        DTWAIN_ArrayAddString(pArray, s.c_str());
    });
}

DTWAIN_ACQUIRE CTL_TwainDLLHandle::GetNewAcquireNum()
{
    const auto iter = std::find(s_aAcquireNum.begin(), s_aAcquireNum.end(), -1L);
    if (iter != s_aAcquireNum.end())
    {
        const DTWAIN_ACQUIRE num { std::distance(s_aAcquireNum.begin(), iter) };
        *iter = num;
        return num;
    }

    const size_t nSize = s_aAcquireNum.size();
    s_aAcquireNum.push_back(static_cast<LONG_PTR>(nSize));
    return static_cast<DTWAIN_ACQUIRE>(nSize);
}


void CTL_TwainDLLHandle::EraseAcquireNum(DTWAIN_ACQUIRE nNum)
{
    const size_t nSize = s_aAcquireNum.size();
    if (nNum >= static_cast<LONG_PTR>(nSize) || nNum < 0)
        return;
    s_aAcquireNum[nNum] = -1;
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceAcquiring(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (p)
    {
        // out of a TWAIN loop testing
        const bool Ret = p->IsAcquireAttempt();
        LOG_FUNC_EXIT_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsSourceAcquiringEx(DTWAIN_SOURCE Source, BOOL bUIOnly)
{
    LOG_FUNC_ENTRY_PARAMS((Source, bUIOnly))
    if ( !Source )
        LOG_FUNC_EXIT_PARAMS(true)
    const auto pHandle = static_cast<CTL_TwainDLLHandle*>(GetDTWAINHandle_Internal());
    const CTL_ITwainSource* p = VerifySourceHandle(pHandle, Source);
    if (p)
    {
        if (bUIOnly)
            LOG_FUNC_EXIT_PARAMS(p->IsUIOpen() ? true : false);
        const bool stillAcquiring = (!pHandle->m_bTransferDone == true && !pHandle->m_bSourceClosed == true);
        LOG_FUNC_EXIT_PARAMS(stillAcquiring)
    }
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}


DTWAIN_BOOL DLLENTRY_DEF DTWAIN_IsMemFileXferSupported(DTWAIN_SOURCE Source)
{
    LOG_FUNC_ENTRY_PARAMS((Source))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (p)
    {
        const bool Ret = CTL_TwainAppMgr::IsMemFileTransferSupported(p);
        LOG_FUNC_EXIT_PARAMS(Ret)
    }
    LOG_FUNC_EXIT_PARAMS(false)
    CATCH_BLOCK(false)
}

DTWAIN_ARRAY  dynarithmic::SourceAcquire(SourceAcquireOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(opts.getHandle());
    opts.setStatus(0);
    bool bSessionPreStarted = false;

    // See if DLL Handle exists
    DTWAIN_Check_Bad_Handle_Ex(pHandle, NULL, FUNC_MACRO);

    // Check if Source is valid
    const CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, opts.getSource());
    if (!pSource)
    {
        opts.setStatus(DTWAIN_ERR_BAD_SOURCE);
        DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_BAD_SOURCE, NULL, FUNC_MACRO);
    }

    if (!pHandle->m_bSessionAllocated)
    {
        if (!DTWAIN_StartTwainSession(nullptr, nullptr))
        {
            opts.setStatus(DTWAIN_ERR_NO_SESSION);
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, opts.getStatus(), NULL, FUNC_MACRO);
        }
    }
    else
        bSessionPreStarted = true;
    auto p = static_cast<CTL_ITwainSource *>(opts.getSource());
    DTWAIN_SOURCE pRealSource;
    bool bSourcePreOpened = true;
    if (!DTWAIN_IsSourceOpen(p))
    {
        bSourcePreOpened = false;
        SourceSelectionOptions selOpts(SELECTSOURCEBYNAME, p->GetProductName().c_str());
        pRealSource = SourceSelect(selOpts);
        if (!pRealSource)
        {
            if (!bSessionPreStarted)
                DTWAIN_EndTwainSession();
            LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
        }
        if (!DTWAIN_OpenSource(pRealSource))
        {
            if (!bSessionPreStarted)
                DTWAIN_EndTwainSession();
            LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
        }
    }
    else
        pRealSource = p;

    // Set the PixelType capability.  If we need to set the pixel type, then DTWAIN must default to use
    // the default bit depth.  The user should use DTWAIN_SetPixelType and DTWAIN_SetBitDepth before
    // calling the DTWAIN_Acquirexxx() function to override this behavior.
    LONG PixelType = opts.getPixelType();
    bool bWriteMisc = (CTL_TwainDLLHandle::s_lErrorFilterFlags & DTWAIN_LOG_MISCELLANEOUS)?true:false;
    if (PixelType != DTWAIN_PT_DEFAULT && opts.getAcquireType() != ACQUIREAUDIONATIVE)
    {
        CTL_StringType sBuf;
        if ( bWriteMisc)
            CTL_TwainAppMgr::WriteLogInfoA("Verifying Current Pixel Type ...\n");

        if (DTWAIN_IsPixelTypeSupported(pRealSource, PixelType))
        {
            if ( bWriteMisc )
            {
                StringStreamA strm;
                strm << boost::format("Pixel Type of %1% is supported.  Checking if we need to set it...") % PixelType;
                CTL_TwainAppMgr::WriteLogInfoA(strm.str());
            }
            LONG curPixelType;
            LONG curBitDepth;

            // Now check if current pixel type is the same as desired pixel type
            if (DTWAIN_GetPixelType(pRealSource, &curPixelType, &curBitDepth, TRUE))
            {
                if ( bWriteMisc)
                {
                    StringStreamA strm2;
                    strm2 << boost::format("Current pixel type is %1%, bit depth is %2%\n") % curPixelType % curBitDepth;
                    CTL_TwainAppMgr::WriteLogInfoA(strm2.str());
                }
                // set the pixel type if not the same
                if (curPixelType != PixelType)
                {
                    if ( bWriteMisc )
                        CTL_TwainAppMgr::WriteLogInfoA("Current and desired pixel type not equal.  Setting to desired...");
                    if (!DTWAIN_SetPixelType(pRealSource, PixelType, DTWAIN_DEFAULT, TRUE))
                    {
                        if ( bWriteMisc)
                            CTL_TwainAppMgr::WriteLogInfoA("Could not set pixel type!");
                        opts.setStatus(DTWAIN_ERR_BAD_PIXTYPE);
                        DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_BAD_PIXTYPE, NULL, FUNC_MACRO);
                    }
                }
                else
                    // pixel type is supported
                {
                    if (bWriteMisc)
                        CTL_TwainAppMgr::WriteLogInfoA("Current and desired pixel type equal.  End processing pixel type and bit depth...");
                }
            }
            else
            {
                if ( bWriteMisc )
                    CTL_TwainAppMgr::WriteLogInfoA("Could not get current pixel type!");
                opts.setStatus(DTWAIN_ERR_BAD_PIXTYPE);
                DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_BAD_PIXTYPE, NULL, FUNC_MACRO);
            }
        }
        else
        {
            if ( bWriteMisc )
            {
                // pixel type not supported
                StringStreamA strm2;
                strm2 << boost::format("Pixel Type of %1% is not supported.  Setting to default...") % PixelType;
                CTL_TwainAppMgr::WriteLogInfoA(strm2.str());
            }
            if (!DTWAIN_SetPixelType(pRealSource, DTWAIN_PT_DEFAULT, DTWAIN_DEFAULT, TRUE))
            {
                opts.setStatus(DTWAIN_ERR_BAD_PIXTYPE);
                DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_BAD_PIXTYPE, NULL, FUNC_MACRO);
            }
        }
    }

    // Set the max # of pages to retrieve
    auto pAcquireSource = static_cast<CTL_ITwainSource*>(pRealSource);

    pAcquireSource->SetDibAutoDelete(FALSE);
    pAcquireSource->SetAcquireCount(0);
    pAcquireSource->SetDeleteDibOnScan(opts.getDiscardDibs());

#ifdef _WIN32
    DTWAIN_CALLBACK oldCall = pHandle->m_CallbackMsg;
    DTWAIN_SetCallbackProc(DTWAIN_AcquireProc, DTWAIN_CallbackMESSAGE);
#endif

    DTWAIN_ARRAY aAcquisitionArray = SourceAcquireWorkerThread(opts);
    if (DTWAIN_GetTwainMode() == DTWAIN_MODELESS)
    {
        LOG_FUNC_EXIT_PARAMS(aAcquisitionArray)
    }

    if (aAcquisitionArray)
    {
        auto& vValues = CTL_TwainDLLHandle::s_ArrayFactory->
                            underlying_container_t<CTL_ArrayFactory::tagged_array_voidptr*>(aAcquisitionArray);
        if (!vValues.empty())
            opts.setStatus(DTWAIN_TN_ACQUIREDONE);
    }

    #ifdef _WIN32
    DTWAIN_SetCallbackProc(oldCall, DTWAIN_CallbackMESSAGE);
    #endif
    if (!bSessionPreStarted)
        DTWAIN_EndTwainSession();
    if (!bSourcePreOpened)
        DTWAIN_CloseSource(pRealSource);
    LOG_FUNC_EXIT_PARAMS(aAcquisitionArray)
    CATCH_BLOCK(DTWAIN_ARRAY(0))
}

DTWAIN_ARRAY dynarithmic::SourceAcquireWorkerThread(SourceAcquireOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    DTWAIN_ARRAY Array = nullptr;
    DTWAIN_ARRAY aAcquisitionArray = nullptr;

    DTWAINArrayLL_RAII a1;

    CTL_ITwainSource *pSource = VerifySourceHandle(opts.getHandle(), opts.getSource());
    pSource->ResetAcquisitionAttempts(nullptr);
    aAcquisitionArray = static_cast<DTWAIN_ARRAY>(DTWAIN_ArrayCreate(DTWAIN_ARRAYOFHANDLEARRAYS, 0));
    DTWAINArrayLL_RAII aAcq(aAcquisitionArray);

    pSource->m_pUserPtr = nullptr;
    CTL_TwainDLLHandle *pDLLHandle = static_cast<CTL_TwainDLLHandle *>(opts.getHandle());
    pDLLHandle->m_bTransferDone = false;
    pDLLHandle->m_bSourceClosed = false;
    pDLLHandle->m_lLastError = 0;

    if (DTWAIN_GetTwainMode() == DTWAIN_MODELESS)
    {
        Array = DTWAIN_ArrayCreate(DTWAIN_ARRAYHANDLE, 0);
        a1.reset(Array);
        const auto pHandle = static_cast<CTL_TwainDLLHandle *>(opts.getHandle());
        if (!Array)
        {
            opts.setStatus(DTWAIN_ERR_OUT_OF_MEMORY);
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_OUT_OF_MEMORY, NULL, FUNC_MACRO);
        }
    }
    else
        pSource->ResetAcquisitionAttempts(aAcquisitionArray);

    switch (opts.getAcquireType())
    {
        case ACQUIRENATIVE:
        case ACQUIRENATIVEEX:
            if (DTWAIN_LLAcquireNative(opts) == -1L)
            {
                opts.setStatus(DTWAIN_TN_ACQUIREFAILED);
                LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
            }
            if (opts.getAcquireType() == ACQUIRENATIVEEX)
                pSource->SetUserAcquisitionArray(opts.getUserArray());
            break;

        case ACQUIREBUFFER:
        case ACQUIREBUFFEREX:
            if (DTWAIN_LLAcquireBuffered(opts) == -1L)
            {
                opts.setStatus(DTWAIN_TN_ACQUIREFAILED);
                LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
            }
            if (opts.getAcquireType() == ACQUIREBUFFEREX)
                pSource->SetUserAcquisitionArray(opts.getUserArray());
            break;

        case ACQUIRECLIPBOARD:
            if (DTWAIN_LLAcquireToClipboard(opts) == -1L)
            {
                opts.setStatus(DTWAIN_TN_ACQUIREFAILED);
                LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
            }
            break;

        case ACQUIREFILE:
            if (DTWAIN_LLAcquireFile(opts) == -1L)
            {
                opts.setStatus(DTWAIN_TN_ACQUIREFAILED);
                LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
            }
            break;

        case ACQUIREAUDIONATIVE:
        case ACQUIREAUDIONATIVEEX:
            if (DTWAIN_LLAcquireAudioNative(opts) == -1L)
            {
                opts.setStatus(DTWAIN_TN_ACQUIREFAILED);
                LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
            }
            if (opts.getAcquireType() == ACQUIREAUDIONATIVEEX)
                pSource->SetUserAcquisitionArray(opts.getUserArray());
            break;

        case ACQUIREAUDIOFILE:
            if (DTWAIN_LLAcquireAudioFile(opts) == -1L)
            {
                opts.setStatus(DTWAIN_TN_ACQUIREFAILED);
                LOG_FUNC_EXIT_PARAMS((DTWAIN_ARRAY)NULL)
            }
            break;

    }

    if (Array)  // This is an immediate return
    {
        // turn off RAII here
        a1.release();
        aAcq.release();
        pSource->ResetAcquisitionAttempts(aAcquisitionArray);
        pDLLHandle->m_lLastAcqError = DTWAIN_TN_ACQUIRESTARTED;
        opts.setStatus(DTWAIN_TN_ACQUIRESTARTED);
        pSource->m_pUserPtr = Array;
        LOG_FUNC_EXIT_PARAMS(Array)
    }

    pSource->ResetAcquisitionAttempts(aAcquisitionArray);

    opts.setStatus(pDLLHandle->m_lLastAcqError);
    // Get the array of Dibs
    const auto& vValues = CTL_TwainDLLHandle::s_ArrayFactory->
                            underlying_container_t<CTL_ArrayFactory::tagged_array_voidptr*>(aAcquisitionArray);

    if (vValues.empty() && opts.getAcquireType() != ACQUIREFILE)
    {
        pSource->ResetAcquisitionAttempts(nullptr);
        LOG_FUNC_EXIT_PARAMS(NULL)
    }
    aAcq.release();
    LOG_FUNC_EXIT_PARAMS(aAcquisitionArray)
    CATCH_BLOCK(DTWAIN_ARRAY())
}

bool dynarithmic::AcquireExHelper(SourceAcquireOptions& opts)
{
    DTWAIN_ARRAY aDibs = SourceAcquire(opts);
    DTWAINArrayLL_RAII arr(aDibs);
    const auto& vValues = CTL_TwainDLLHandle::s_ArrayFactory->underlying_container_t<void*>(aDibs);

    bool bRet = false;
    if (aDibs)
        bRet = !vValues.empty() ? true: false;
    if (opts.getStatus() == DTWAIN_TN_ACQUIRESTARTED && aDibs)
        bRet = true;
    auto pSource = reinterpret_cast<CTL_ITwainSource*>(opts.getSource());

    // Destroy internally generated acquisition array and
    // utilize only user-defined acquisition array
    pSource->ResetAcquisitionAttempts(nullptr);
    return bRet;
}

DTWAIN_ACQUIRE  dynarithmic::LLAcquireImage(SourceAcquireOptions& opts)
{
    LOG_FUNC_ENTRY_PARAMS((opts))
    DTWAIN_ACQUIRE nNum = -1;
    LONG ClipboardTransferType = -1;
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    const DTWAIN_SOURCE Source = opts.getSource();
    CTL_ITwainSource *pSource = VerifySourceHandle(pHandle, Source);

    if (!pSource)
        LOG_FUNC_EXIT_PARAMS((DTWAIN_ACQUIRE)nNum)

    // Open the source (if source is closed)
    if (!CTL_TwainAppMgr::OpenSource(pHandle->m_pTwainSession, pSource))
        LOG_FUNC_EXIT_PARAMS((DTWAIN_ACQUIRE)nNum)

    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return DTWAIN_IsSourceAcquiring(Source); },
    DTWAIN_ERR_SOURCE_ACQUIRING, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
    // Negotiate transfer
    CTL_TwainAppMgr::SetTransferCount(pSource, opts.getMaxPages());
    pSource->SetSpecialTransferMode(opts.getTransferMode());
    if (opts.getActualAcquireType() == TWAINAcquireType_File)
    {
        CTL_StringType strFile;
        int nFileType;
        LONG lFileFlags = opts.getFileFlags();
        const LONG lFileType = opts.getFileType();
        // Check if the DTWAIN_USESOURCEMODE flag is set
        const bool bUseSourceMode = lFileFlags & DTWAIN_USESOURCEMODE ? true : false;
        const bool bUseMemFile = lFileFlags & DTWAIN_USEMEMFILE ? true : false;
        if (bUseSourceMode || bUseMemFile )
        {
            // Source must support file transfers
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !DTWAIN_IsFileXferSupported(Source, lFileType); },
                DTWAIN_ERR_NO_FILE_XFER, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);

            // Turn off NATIVE and BUFFERED modes if set
            lFileFlags = lFileFlags & ~(DTWAIN_USENATIVE | DTWAIN_USEBUFFERED);

            CTL_TwainAppMgr::GetFileTransferDefaults(pSource, nFileType);
        }

        if ( bUseMemFile )
        {
            // Source must support file transfers
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&] { return !DTWAIN_IsMemFileXferSupported(Source); },
                                              DTWAIN_ERR_NO_MEMFILE_XFER, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
            // Turn off USESOURCEMODE if buffered file mode is selected
            lFileFlags = lFileFlags & ~(DTWAIN_USESOURCEMODE);
        }
        
        // Check if the file type is supported
        // check if defaults were specified
        if (lFileFlags & (DTWAIN_USENAME | DTWAIN_USELONGNAME) &&
            !(lFileFlags & DTWAIN_USELIST))
            strFile = opts.getFileName();

        nFileType = lFileType;

        if (bUseSourceMode || /*lFileFlags & */CTL_TwainAppMgr::IsSupportedFileFormat(pSource,
            nFileType))
        {
            opts.setActualAcquireType(CTL_TwainAppMgr::GetCompatibleFileTransferType(pSource));
            if (!bUseSourceMode)
                opts.setActualAcquireType(TWAINAcquireType_FileUsingNative);

            pSource->SetAcquireType(static_cast<CTL_TwainAcquireEnum>(opts.getActualAcquireType()), strFile.c_str());
            pSource->SetAcquireFileType(static_cast<CTL_TwainFileFormatEnum>(nFileType));

            LONG lMode = lFileFlags;

            if (!bUseSourceMode)
            {
                lMode = lFileFlags & DTWAIN_USEBUFFERED;
                if (!lMode)
                    lMode = DTWAIN_USENATIVE;
                else
                if (lFileFlags & DTWAIN_USENATIVE && lFileFlags & DTWAIN_USEBUFFERED)
                    lMode = DTWAIN_USENATIVE;
            }
            else
            {
                // Set the image file format here for file transfers.  If this returns false, the Source is
                // buggy to have gotten this far!  The results will be written to the log,
                // transfer will continue!
                DTWAIN_SetFileXferFormat(Source, nFileType, TRUE);

                // See if compression needs to be set and if so,
                // it must be done here for file transfers
                LONG Compression;
                if (DTWAIN_GetCompressionType(Source, &Compression, DTWAIN_CAPGETCURRENT))
                    DTWAIN_SetCompressionType(Source, Compression, TRUE);
            }

            // Tiles not supported yet, so let user know this
            if (lMode == DTWAIN_USEBUFFERED)
            {
                DTWAINScopedLogControllerExclude sLogger(DTWAIN_LOG_ERRORMSGBOX);
                if (DTWAIN_IsCapSupported(Source, DTWAIN_CV_ICAPTILES))
                {
                    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return TileModeOn(Source); },
                        DTWAIN_ERR_TILES_NOT_SUPPORTED, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
                }
            }

            pSource->SetSpecialTransferMode(lMode);

            // Determine the naming convention
            bool bUsePrompt = false;
            if (lFileFlags & DTWAIN_USEPROMPT)
                bUsePrompt = true;
            else
                if (!(lFileFlags & (DTWAIN_USENAME | DTWAIN_USELONGNAME)))
                    bUsePrompt = true;

            if (bUsePrompt)
                lFileFlags = lMode | DTWAIN_USEPROMPT;
            else
            {
                // Check file naming option
                if (lFileFlags & DTWAIN_USENAME)
                    lFileFlags = lMode | DTWAIN_USENAME;
                else
                    lFileFlags = lMode | DTWAIN_USELONGNAME;

                // Allocate for array
                DTWAIN_ARRAY pArray = static_cast<DTWAIN_ARRAY>(pSource->GetFileEnumerator());
                if (!pArray)
                    pArray = DTWAIN_ArrayCreate(DTWAIN_ARRAYSTRING, 0);
                if (!pArray)
                {
                    // Check if array exists
                    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !pArray; }, DTWAIN_ERR_BAD_ARRAY, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
                }

                // Parse the filename string into the array
                ParseFileNames(opts.getFileList(), opts.getFileName(), pArray);
                CTL_StringType szName;
                const size_t nFileCount = CTL_TwainDLLHandle::s_ArrayFactory->size(pArray);
                if (nFileCount > 0)
                {
                    CTL_TwainDLLHandle::s_ArrayFactory->get_value(pArray, 0, &szName);
                }

                if (nFileCount == 0 || szName.empty())
                {
                    DTWAIN_ArrayDestroy(pArray);
                    // Check if at least one file is in array
                    DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{ return true; }, DTWAIN_ERR_EMPTY_ARRAY, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
                }

                pSource->SetFileEnumerator(pArray);

                // Check if auto-increment is on.  If so, set up the various file numbering
                // data
                if (pSource->IsFileAutoIncrement())
                {
                    // Get the first name in the list.
                    pSource->InitFileAutoIncrementData(szName);
                }
            }
            pSource->SetAcquireFileFlags(lFileFlags);

        }
        else
            DTWAIN_Check_Error_Condition_0_Ex(pHandle, []{return true; }, DTWAIN_ERR_FILE_FORMAT, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
    }
    else
    if (opts.getActualAcquireType() == TWAINAcquireType_Clipboard)
    {
        if (opts.getTransferMode() == DTWAIN_USENATIVE)
            ClipboardTransferType = TWSX_NATIVE;
        else
            ClipboardTransferType = TWSX_MEMORY;
        pSource->SetAcquireType(static_cast<CTL_TwainAcquireEnum>(opts.getActualAcquireType()));
    }
    else
        pSource->SetAcquireType(static_cast<CTL_TwainAcquireEnum>(opts.getActualAcquireType()));

    // Get the new acquire number
    nNum = pHandle->GetNewAcquireNum();
    pSource->SetAcquireNum(nNum);

    // Erase old DIBs
    pSource->RemoveAllDibs();

    // set the user interface to on/off
    pSource->SetUIOpenOnAcquire(opts.getShowUI());
    pSource->SetAcquireAttempt(true);

    if (opts.getMaxPages() <= 0)
        pSource->SetMaxAcquireCount(-1);
    else
        pSource->SetMaxAcquireCount(static_cast<int>(opts.getMaxPages()));
    // Set the file transfer mechanism here
    CTL_TwainAppMgr::SetTransferMechanism(pSource, pSource->GetAcquireType(), ClipboardTransferType);


    // Enable the document feeder here
    {
    // Remember the current error flags
        DTWAINScopedLogControllerExclude sLogger(DTWAIN_LOG_ERRORMSGBOX);
        CTL_TwainAppMgr::SetupFeeder(pSource, opts.getMaxPages());
    }

    // Reset manual duplex mode
    if (!pSource->IsMultiPageModeContinuous())
        pSource->ResetManualDuplexMode();

    // Send message that interface is about to be shown
    CTL_TwainAppMgr::SendTwainMsgToWindow(pHandle->m_pTwainSession, nullptr, DTWAIN_TN_UIOPENING, reinterpret_cast<LPARAM>(pSource));

    // Show the user interface, or start the acquisition process immediately if no user interface is shown
    const bool bUIOk = CTL_TwainAppMgr::ShowUserInterface(pSource);
    if (!bUIOk)
        pSource->SetAcquireAttempt(false);

    // Terminate if there is an error showing the UI (or acquiring with no UI)
    DTWAIN_Check_Error_Condition_0_Ex(pHandle, [&]{return !bUIOk; }, DTWAIN_ERR_UI_ERROR, static_cast<DTWAIN_ACQUIRE>(-1), FUNC_MACRO);
    pSource->SetOpenAfterAcquire(opts.getRemainOpen());
    if (!opts.getShowUI())
        pSource->SetMaxAcquisitions(1);
    else
        pSource->SetMaxAcquisitions(pSource->GetUIMaxAcquisitions());

    // Set the pending image and job numbers
    pSource->SetPendingImageNum(0);
    pSource->SetPendingJobNum(0);
    pSource->SetBlankPageCount(0);

    // Set the array to save the image data to the user-provided array
    if (opts.getUserArray())
        pSource->SetUserAcquisitionArray(opts.getUserArray());

    // Notify that we started the acquisition
    CTL_TwainAppMgr::SendTwainMsgToWindow(pHandle->m_pTwainSession, nullptr, DTWAIN_TN_ACQUIRESTARTED, reinterpret_cast<LPARAM>(pSource));

    // return the Acquire Number
    LOG_FUNC_EXIT_PARAMS((DTWAIN_ACQUIRE)nNum)
    CATCH_BLOCK(DTWAIN_FAILURE1)
}

bool dynarithmic::TileModeOn(DTWAIN_SOURCE Source)
{
    BOOL bMode;
    const auto pHandle = static_cast<CTL_TwainDLLHandle *>(GetDTWAINHandle_Internal());
    CTL_ITwainSource *p = VerifySourceHandle(pHandle, Source);
    if (p)
    {
        if (CTL_TwainAppMgr::GetOneTwainCapValue(p, &bMode, DTWAIN_CV_ICAPTILES, CTL_GetTypeGET, TWTY_BOOL))
            return static_cast<TW_BOOL>(bMode)?true:false;
    }
    return false;
}


