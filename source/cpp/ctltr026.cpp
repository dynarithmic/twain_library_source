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
#include <bitset>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <fstream>
#include <array>
#include "ctltr010.h"
#include "ctltr026.h"
#include "ctltr027.h"
#include "ctltr025.h"
#include "ctltwainmanager.h"
#include "imagexferfilewriter.h"
#include "ctldib.h"
#include "dtwain.h"
#include "arrayfactory.h"
#include "ctlfileutils.h"
#include "resamplefactory.h"
#include "ctlfilesave.h"
#include "cppfunc.h"

using namespace dynarithmic;

static void SendFileAcquireError(CTL_ITwainSource* pSource, const CTL_ITwainSession* pSession,
                                 LONG Error, LONG ErrorMsg, const std::string& extraInfo = "");
static bool IsState7InfoNeeded(CTL_ITwainSource *pSource);

CTL_ImageXferTriplet::CTL_ImageXferTriplet(CTL_ITwainSession *pSession,
                                           CTL_ITwainSource* pSource,
                                           TW_UINT16 nType)
                     :  CTL_ImageTriplet(pSession, pSource),
                     m_pImgHandler(nullptr),
                     m_hDataHandle(nullptr),
                     m_nTotalPagesSaved(0),
                     m_bJobControlPageRecorded(false),
                     m_bJobMarkerNeedsToBeWritten(false),
                     m_bScanPending(true),
                     m_nTotalPages(0),
                     m_nTransferType(nType),
                     m_nFailAction(0),
                     m_bPendingXfersDone(false),
                     m_PendingXfers{},
                     m_lastPendingXferCode(0),
                     m_hDataHandleFromDevice(nullptr),
                     m_IsBuffered(false),
                     m_bEndTwainUI(true)
{
    switch( nType )
    {
        case DAT_IMAGENATIVEXFER:
        case DAT_AUDIONATIVEXFER:
            InitVars(nType, CTL_GetTypeGET, &m_hDataHandleFromDevice);
        break;
        case DAT_IMAGEFILEXFER:
        case DAT_IMAGEMEMFILEXFER:
            InitVars(nType, CTL_GetTypeGET, nullptr );
        break;
    }
}


TW_UINT16 CTL_ImageXferTriplet::Execute()
{
    // Check if document feeder is to be used.  If it can, check if there
    // really is a Document feeder

    CTL_ITwainSource *pSource = GetSourcePtr();
    CTL_ITwainSession *pSession = GetSessionPtr();
    TW_UINT16 rc = CTL_ImageTriplet::Execute();
    bool    bPageDiscarded = false;

    ImageXferFileWriter FileWriter(this, pSession, pSource);

    m_bJobControlPageRecorded = false;
    int errfile = 0;

    switch (rc)
    {
        case TWRC_XFERDONE:
        {
            size_t nLastDib = 0;
            bool bKeepPage = true;
            bool bInClip = false;
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,
                                                  DTWAIN_TN_TRANSFERDONE,
                                                  reinterpret_cast<LPARAM>(pSource));

            if (m_nTransferType == DAT_IMAGENATIVEXFER)
            {
                // We need to clone the DIB if we're doing a native XFER on a DSM2 Data Source
                if (CTL_TwainAppMgr::IsVersion2DSMUsed())
                {
                    auto sessionHandle = GetSessionPtr()->GetTwainDLLHandle();
                    // Lock the DIB returned by the device
                    BITMAPINFOHEADER* thisBitmap =
                        (BITMAPINFOHEADER*)sessionHandle->m_TwainMemoryFunc->LockMemory((HBITMAP)m_hDataHandleFromDevice);

                    if (!thisBitmap)
                    {
                        // This is an error if the source didn't give us a valid handle on return.
                        // Since this is a native transfer, there is no point in going further trying
                        // to acquire images.
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_INVALID_TWAINDSM2_BITMAP, 
                                                              reinterpret_cast<LPARAM>(pSource));

                        // Log the error
                        auto sErr = dynarithmic::GetResourceStringFromMap(-DTWAIN_ERR_TWAINDSM2_BADBITMAP);
                        sErr += " (" + pSource->GetProductNameA() + ")";
                        CTL_TwainAppMgr::SetAndLogError(DTWAIN_ERR_TWAINDSM2_BADBITMAP, sErr, true);
                        pSource->SetLastAcquireError(DTWAIN_ERR_TWAINDSM2_BADBITMAP);

                        // Stop the acquisition
                        pSource->SetShutdownAcquire(true);
                        FailAcquisition();
                        AbortTransfer(true, errfile);
                        return TWRC_FAILURE;
                    }

                    // Make sure we unlock and free this memory once out of this scope
                    auto dsmPair = DSMPair(sessionHandle, m_hDataHandleFromDevice);
                    DTWAINDSM2LockAndFree_RAII raii(&dsmPair);

                    // Create a local bitmap
                    DWORD dwSize = sizeof(BITMAPINFOHEADER) + ((((thisBitmap->biWidth * thisBitmap->biBitCount + 31) / 32) * 4) * 
                        thisBitmap->biHeight) + thisBitmap->biClrUsed * sizeof(RGBQUAD);
                    HANDLE hDIB = GlobalAlloc(GHND, dwSize);

                    if (hDIB)
                    {
                        BYTE* img = (BYTE*)GlobalLock(hDIB);

                        // Make sure we unlock this handle once out of this scope
                        DTWAINGlobalHandle_RAII raii2(hDIB);

                        // Copy the image over
                        memcpy(img, thisBitmap, dwSize);
                        m_hDataHandle = hDIB;
                    }
                    else
                    {
                        // Not enough memory to copy the DIB
                        CTL_TwainAppMgr::SetAndLogError(DTWAIN_ERR_OUT_OF_MEMORY, "", true);
                        FailAcquisition();
                        return rc;
                    }
                }
                else
                {
                    // Just assign the handle returned from the device if the DSM is version 1.x
                    m_hDataHandle = m_hDataHandleFromDevice;
                }
            }
            
            m_bJobMarkerNeedsToBeWritten = false;
            // Check if more images are pending (job control only)
            SetPendingXfersDone(false);

            bool bEndOfJobDetected=false;

            // Get the current page number of image being transferred
            size_t nCurImage = pSource->GetPendingImageNum();

            // See if we need to check the job control status via TWAIN
            if ( pSource->IsUIOpenOnAcquire() && nCurImage == 0 )
            {
                LONG JobControl;
                // Get the current job control if the user may have changed it
                // in the UI of the TWAIN driver
                DTWAINScopedLogControllerExclude sLogerr(DTWAIN_LOG_ERRORMSGBOX);
                if ( DTWAIN_GetJobControl(pSource,&JobControl, TRUE) != FALSE )
                    pSource->SetCurrentJobControl( static_cast<TW_UINT16>(JobControl) );
            }

            if ( pSource->GetCurrentJobControl() != TWJC_NONE )
            {
                bool bSuccess = false;
                if ( pSource->IsTwainJobControl())
                {
                    TW_PENDINGXFERS& Pending = GetLocalPendingXferInfo();
                    SetLastPendingInfoCode(GetImagePendingInfo(&Pending));
                    if (GetLastPendingInfoCode() == TWRC_SUCCESS)
                    {
                        bSuccess = true;
                        // Indicate that pending xfers has been executed
                        SetPendingXfersDone(true);
                        bEndOfJobDetected = Pending.EOJ == static_cast<TW_UINT32>(pSource->GetEOJDetectedValue());
                    }
                }
                else
                {
                    // temporary
                    if (GetDAT() != DAT_AUDIONATIVEXFER)
                    {
                        CTL_TwainDib theDib;
                        theDib.SetHandle(m_hDataHandle);
                        bEndOfJobDetected = theDib.IsBlankDIB(pSource->GetBlankPageThreshold());
                    }
                    bSuccess = true;
                }
                if ( bSuccess )
                {
                    if ( bEndOfJobDetected )
                    {
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,DTWAIN_TN_EOJDETECTED_XFERDONE,reinterpret_cast<LPARAM>(pSource));
                        // Now check if special job control file handling is done
                        if ( pSource->IsJobFileHandlingOn() &&
                            !pSource->CurrentJobIncludesPage())
                        {
                            // Indicate that a job control marker
                            // needs to be written after the real page has
                            // been written
                            m_bJobMarkerNeedsToBeWritten = true;
                        }
                    }
                    else
                    if ( nCurImage == 0 )
                    {
                        // Write a job control marker now.  This is the start of a
                        // new job.
                        FileWriter.CopyDuplexDibToFile(CTL_TwainDibPtr(), true);
                    }
                }
            }

            bool bExecuteEOJPageHandling = bEndOfJobDetected && pSource->IsJobFileHandlingOn();

            pSource->SetTransferDone(true);
            // Store DIB in object

            // Get the image if native transfer
            CTL_TwainDibPtr CurDib;
            CTL_TwainDibArray* pArray = nullptr;
            if (m_nTransferType == DAT_IMAGENATIVEXFER)
            {
                bool bProcessDibEx = true;
                // Get the array of current array of DIBS (this pointer allows changes to Source's internal DIB array)
                // If this is an audio transfer, the the "DIB" is actually WAV data (for Windows)
                pArray = pSource->GetDibArray();

                // Let Source set the handle (Source knows if this is a new or old DIB to replace)
                // Emit an error if m_hDib is NULL
                if (!m_hDataHandle)
                {
                    bPageDiscarded = true;
                    // Set the error code
					CTL_TwainAppMgr::SetAndLogError(DTWAIN_ERR_BAD_DIB_PAGE, "", false);
                    break;
                }

                if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_DECODE_BITMAP)
                {
                    std::string sOut = "\nOriginal bitmap from device: \n";
                    sOut += "{\n" + CTL_ErrorStructDecoder::DecodeBitmap(m_hDataHandle) + "\n}\n";
                    LogWriterUtils::WriteMultiLineInfoIndentedA(sOut, "\n");
                }

                pSource->SetDibHandle(m_hDataHandle, nCurImage);

                // Get the dib from the array (must get last dib generated)
                nLastDib = pArray->GetSize() - 1;
                CurDib = pArray->GetAt(nLastDib);

                if (CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,DTWAIN_TN_PROCESSEDDIB,reinterpret_cast<LPARAM>(pSource)) == 0)
                {
                    // User does not want to process the image further.
                    // They are satisfied with the DIB as-is.
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_PROCESSDIBACCEPTED, reinterpret_cast<LPARAM>(pSource));
                    bProcessDibEx = false;
                }

                // Here we can do a check for blank page.
                if (ProcessBlankPage(pSession, pSource, CurDib, false, DTWAIN_TN_BLANKPAGEDETECTED1, DTWAIN_TN_BLANKPAGEDISCARDED1, DTWAIN_BP_AUTODISCARD_IMMEDIATE) == 0)
                {
                    bPageDiscarded = true;
                    break;  // The page is discarded
                }

                auto sessionHandle = GetSessionPtr()->GetTwainDLLHandle();

                // Callback function for access to change DIB
                if (sessionHandle->m_pDibUpdateProc != nullptr && GetDAT() != DAT_AUDIONATIVEXFER)
                {
                    HANDLE hRetDib =
                        (sessionHandle->m_pDibUpdateProc)
                        (pSource, static_cast<LONG>(nLastDib), m_hDataHandle);
                    if (hRetDib && hRetDib != m_hDataHandle)
                    {
                        // Application changed DIB.  So make this the current dib
                        #ifdef _WIN32
                        GlobalFree(m_hDataHandle);
                        #endif
                        m_hDataHandle = hRetDib;
                        pSource->SetDibHandle(m_hDataHandle, nLastDib);
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_APPUPDATEDDIB, reinterpret_cast<LPARAM>(pSource));
                    }
                }

                // Change bpp if necessary
                if (bProcessDibEx && GetDAT() != DAT_AUDIONATIVEXFER)
                    ModifyAcquiredDib();

                WPARAM wParamToUse[] = { DTWAIN_TN_PROCESSEDDIBFINAL, DTWAIN_TN_PROCESSDIBFINALACCEPTED };
                if ( GetDAT() == DAT_AUDIONATIVEXFER )
                {
                    wParamToUse[0] = DTWAIN_TN_PROCESSEDAUDIOFINAL;
                    wParamToUse[1] = DTWAIN_TN_PROCESSAUDIOFINALACCEPTED;
                }

                if (CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, wParamToUse[0], reinterpret_cast<LPARAM>(pSource)) == 0)
                {
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, wParamToUse[1], reinterpret_cast<LPARAM>(pSource));
                    // user is satisfied with the image, so break
                    bProcessDibEx = false;
                }

                if (ProcessBlankPage(pSession, pSource, CurDib, true, DTWAIN_TN_BLANKPAGEDETECTED2, DTWAIN_TN_BLANKPAGEDISCARDED2, DTWAIN_BP_AUTODISCARD_AFTERPROCESS) == 0)
                {
                    bPageDiscarded = true;
                    break;  // The page is discarded
                }

                // Now see if we want to keep the bitmap for purely native transfers (not file saves or file saves that use native transfers)
                // Query if the page should be thrown away
                bKeepPage = QueryAndRemoveDib(TWAINAcquireType_Native, nLastDib);
                if (!bKeepPage)
                    break;
            }

            // Use for native and native/file transfers
            switch( pSource->GetAcquireType() )
            {
                // Check if source acquire is file using native mode
                case TWAINAcquireType_FileUsingNative:
                {
                    if ( m_IsBuffered )
                        break;
                    pSource->SetPromptPending(false);
                    long lFlags   = pSource->GetAcquireFileFlags();
                    if ( lFlags & DTWAIN_USEPROMPT )
                    {
                        pSource->SetPromptPending(true);
                    }

                    // resample the acquired dibs
                    ResampleAcquiredDib();

                    // Check if multi page file is being used
                    bool bIsMultiPageFile = dynarithmic::IsFileTypeMultiPage(pSource->GetAcquireFileType());

                    // Query if the page should be thrown away
                    bKeepPage = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_QUERYPAGEDISCARD, reinterpret_cast<LPARAM>(pSource))?true:false;
                    // Keep the page

                    if ( bKeepPage )
                    {
                        int nMultiStage = 0;
                        if ( bIsMultiPageFile || pSource->IsMultiPageModeSaveAtEnd())
                        {
                            // This is the first page of the acquisition
                            if ( nLastDib == 0 || (pSource->IsNewJob() && pSource->IsJobFileHandlingOn()))
                                nMultiStage = DIB_MULTI_FIRST;
                            else
                            // This is a subsequent page of the acquisition
                                nMultiStage = DIB_MULTI_NEXT;

                            // Now check if this we are in manual duplex mode
                            // or in continuous mode
                            if ( pSource->IsManualDuplexModeOn() ||
                                 pSource->IsMultiPageModeContinuous() ||
                               (pSource->IsMultiPageModeSaveAtEnd() && !bIsMultiPageFile))
                            {
                                // We need to copy the data to a file and store info in
                                // vector of the source
                                if ( !bEndOfJobDetected || // Not end -of-job
                                    (bExecuteEOJPageHandling && !m_bJobControlPageRecorded) // write job control page
                                    )
                                    errfile = FileWriter.CopyDuplexDibToFile(CurDib, bExecuteEOJPageHandling);

                                if ( !m_bJobControlPageRecorded && bExecuteEOJPageHandling)
                                    m_bJobControlPageRecorded = true;
                            }
                            else
                                errfile = FileWriter.CopyDibToFile(CurDib, nMultiStage, pSource->GetImageHandlerPtr(), 0);
                        }
                        else
                           errfile = FileWriter.CopyDibToFile(CurDib, nMultiStage, pSource->GetImageHandlerPtr(), 0);
                        m_nTotalPagesSaved++;
                    }
                    else
                    {
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,DTWAIN_TN_PAGEDISCARDED,reinterpret_cast<LPARAM>(pSource));
                    }
                    // Delete temporary bitmap here
                    if ( !bKeepPage || pSource->IsDeleteDibOnScan() )
                    {
                        // Let array class handle deleting of the DIB (Global memory will be freed only)
                        if ( pArray )
                            pArray->DeleteDibMemory( nLastDib );
                    }
                }
                break;

                case TWAINAcquireType_File:
                case TWAINAcquireType_AudioFile:
                {
                    if ( GetDAT() != DAT_AUDIOFILEXFER )
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,DTWAIN_TN_PROCESSEDDIB,reinterpret_cast<LPARAM>(pSource));
                    else
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_PROCESSEDAUDIOFILE, reinterpret_cast<LPARAM>(pSource));
                    long lFlags = pSource->GetAcquireFileFlags();

                    if ( lFlags & TWAINFileFlag_PROMPT )
                    {
                        CTL_StringType strTempFile = PromptForFileName(pSource->GetDTWAINHandle(), pSource->GetAcquireFileType());
                        StringWrapper::TrimAll(strTempFile);
                        if ( strTempFile.empty())
                        {
                            SendFileAcquireError(pSource, pSession, DTWAIN_ERR_BAD_FILENAME, DTWAIN_TN_FILESAVECANCELLED,
                                                 StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
                            pSource->SetLastAcquiredFileName( StringWrapper::traits_type::GetEmptyString() );
                        }
                        else
                        {
                            // Copy default file name to the new file
                            if ( CTL_TwainAppMgr::CopyFile(pSource->GetAcquireFile(), strTempFile) != 1 )
                            {
                                // Error in copying the file
                                SendFileAcquireError(pSource, pSession, DTWAIN_ERR_FILEWRITE, DTWAIN_TN_FILESAVEERROR,
                                                     StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
                                pSource->SetLastAcquiredFileName( StringWrapper::traits_type::GetEmptyString() );
                            }
                            else
                                pSource->SetLastAcquiredFileName( strTempFile );

                            // Remove the temporary file
                            if (delete_file(pSource->GetAcquireFile().c_str()))
                                pSource->SetAcquireFile(StringWrapper::traits_type::GetEmptyString());

                        }
                        pSource->SetLastAcquiredFileName( strTempFile );
                    }
                    else
                    {
                        // We can't get here if the file copy did not work, so assume success
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_FILESAVEOK,
                                                              static_cast<LPARAM>(pSource->GetAcquireNum()));
                    }
                }
                break;

                case TWAINAcquireType_Clipboard:
                    if ( pSource->GetSpecialTransferMode() == DTWAIN_USENATIVE )
                        bInClip = CopyDibToClipboard( pSession, m_hDataHandle );
                break;
                default:
                    break;
            }

            if ( bInClip )
                CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,DTWAIN_TN_CLIPTRANSFERDONE,static_cast<LPARAM>(pSource->GetAcquireNum()));

            if ( errfile != 0 )
               SendFileAcquireError(pSource, pSession, errfile, DTWAIN_TN_FILESAVEERROR, 
                                    StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
            break;
        }

        case TWRC_CANCEL:
        {
            CancelAcquisition();
            break;
        }

        case TWRC_FAILURE:
        {
            m_hDataHandle = nullptr;
            FailAcquisition();
            AbortTransfer(false, errfile);
            return rc;
        }
        case TWRC_SUCCESS:
        {
            return rc;
        }
        default:
        {
            StringStreamA strm;
            strm << "Unknown return code " << rc << " from DSM during transfer!  Twain driver unstable!";
            LogWriterUtils::WriteLogInfoIndentedA(strm.str());
            m_hDataHandle = nullptr;
            break;
        }
    }

    if (m_IsBuffered)
        return rc;

    bool bRetval = true;
    bool bForceClose;

    pSource->SetBlankPageCount(pSource->GetBlankPageCount() + (bPageDiscarded?1:0));

    if ( !bPageDiscarded && pSource->IsPromptPending())
    {
        pSource->SetPromptPending(false);
    }

    // Force a close if Prompting returned false.
    if ( bRetval == true )
        bForceClose = false;
    else
        bForceClose = true;
    AbortTransfer(bForceClose, errfile);
    return rc;

}

bool CTL_ImageXferTriplet::CancelAcquisition()
{
    CTL_ITwainSource* pSource = GetSourcePtr();
    CTL_ITwainSession* pSession = GetSessionPtr();

    pSource->SetState(SOURCE_STATE_UIENABLED); // Transition to state 5
    pSource->SetTransferDone(false);
    m_hDataHandle = nullptr;
    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,
        DTWAIN_TN_TWAINPAGECANCELLED,
        reinterpret_cast<LPARAM>(pSource));

    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr,
        DTWAIN_TN_PAGECANCELLED,
        reinterpret_cast<LPARAM>(pSource));

    if (pSource->GetAcquireType() == TWAINAcquireType_FileUsingNative)
    {
        // Remove the image file
        ImageXferFileWriter(this, pSession, pSource).EndProcessingImageFile(
            pSource->IsFileIncompleteSave());
    }
    return true;
}


bool CTL_ImageXferTriplet::FailAcquisition()
{
    CTL_ITwainSource* pSource = GetSourcePtr();
    CTL_ITwainSession* pSession = GetSessionPtr();
    pSource->SetTransferDone(false);

    const int nRetryMax = pSource->GetMaxRetryAttempts();
    int nCurRetry = pSource->GetCurrentRetryCount();

    pSource->SetState(SOURCE_STATE_UIENABLED); // Transition to state 5
    pSource->SetTransferDone(false);

    const CTL_TwainAcquireEnum nAcquireType = pSource->GetAcquireType();
    if (nAcquireType == TWAINAcquireType_File) // The TWAIN source is solely responsible for the file handling
    {
        // Send notification that file save failed
        CTL_TwainAppMgr::SendTwainMsgToWindow(pSource->GetTwainSession(),
            nullptr, DTWAIN_TN_FILESAVEERROR, reinterpret_cast<LPARAM>(pSource));
    }

    // Get what to do, either from user-notification or from default
    // TWAIN Window Proc
    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_TWAINPAGEFAILED, reinterpret_cast<LPARAM>(pSource));

    int bContinue = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_PAGEFAILED,
                                                          reinterpret_cast<LPARAM>(pSource));

    if (bContinue == 0)
    {
        bContinue = DTWAIN_PAGEFAIL_TERMINATE;
        SetAcquireFailAction(DTWAIN_PAGEFAIL_TERMINATE);
    }
    else
    if (bContinue == DTWAIN_RETRY_EX || // Means not a user notification
        bContinue == 2)               // Means notifications are on and user wants
        // default behavior
    {
        // Check if retrying forever
        if (nRetryMax == DTWAIN_RETRY_FOREVER)
            bContinue = DTWAIN_PAGEFAIL_RETRY;
        else
        // Check if max retries have been reached
        if (nCurRetry == nRetryMax)
        {
            bContinue = DTWAIN_PAGEFAIL_TERMINATE;
        }
        else
        {
            // Increment retry count and try again
            pSource->SetCurrentRetryCount(++nCurRetry);
            bContinue = DTWAIN_PAGEFAIL_RETRY;
        }
    }
    else
    {
        // Increment retry count and try again
        pSource->SetCurrentRetryCount(++nCurRetry);
        if (pSource->GetCurrentRetryCount() >= pSource->GetMaxRetryAttempts())
            bContinue = DTWAIN_PAGEFAIL_TERMINATE;
        else
            bContinue = DTWAIN_PAGEFAIL_RETRY;
    }
    return ImageXferFileWriter(this, pSession, pSource).ProcessFailureCondition(bContinue);
}

HANDLE  CTL_ImageXferTriplet::GetDibHandle() const
{
    return m_hDataHandle;
}

TW_UINT16 CTL_ImageXferTriplet::GetImagePendingInfo(TW_PENDINGXFERS *pPI, TW_UINT16 nMsg  /* =MSG_ENDXFER */)
{
    CTL_ImagePendingTriplet Pending(GetSessionPtr(),
                                    GetSourcePtr(),
                                    nMsg);
    const TW_UINT16 rc = Pending.Execute();

    if ( rc == TWRC_SUCCESS )
        memcpy(pPI, Pending.GetPendingXferBuffer(), sizeof(TW_PENDINGXFERS));
    return rc;
}



std::pair<bool, bool> CTL_ImageXferTriplet::AbortTransfer(bool bForceClose, int errFile)
{
    if ( CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
        LogWriterUtils::WriteLogInfoIndentedA("Potentially aborting transfer..");
    CTL_ITwainSession *pSession = GetSessionPtr();
    CTL_ITwainSource *pSource = GetSourcePtr();
    ImageXferFileWriter FileWriter(this, pSession, pSource);

    TW_PENDINGXFERS pPending = {};
    TW_PENDINGXFERS* ptrPending = {};
    TW_UINT16 rc = {};
    if ( !IsPendingXfersDone() )
    {
        rc = GetImagePendingInfo( &pPending );
        ptrPending = &pPending;
    }
    else
    {
        rc = GetLastPendingInfoCode();
        ptrPending = &GetLocalPendingXferInfo();
    }

    int nContinue = 1;
    bool bUserCancelled = false;
    bool bJobControlContinue = false;
    bool bEndOfJobDetected = false;
    bool bProcessSinglePage = (!dynarithmic::IsFileTypeMultiPage(pSource->GetAcquireFileType()) &&
                               errFile == 0);
    switch( rc )
    {
        case TWRC_SUCCESS:
            bEndOfJobDetected = pSource->GetCurrentJobControl() != TWJC_NONE &&
                ptrPending->EOJ == pSource->GetEOJDetectedValue();

            if ( bEndOfJobDetected )
                CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                      nullptr, DTWAIN_TN_ENDOFJOBDETECTED,
                                                  reinterpret_cast<LPARAM>(pSource));

            bJobControlContinue = IsJobControlPending(ptrPending);
            m_nTotalPages++;

            // Make sure that job control is "on"
            pSource->StartJob();

            if ( ptrPending->Count != 0) // More to transfer
            {
                LogWriterUtils::WriteLogInfoIndentedA("More To Transfer...");
                // Check if max pages has been reached.  Some Sources do not detect when
                // Count has been set to a specific number, so enforce the test here.
                if ( pSource->GetMaxAcquireCount() == pSource->GetPendingImageNum() + 1 )
                    nContinue = 0;
                else
                if ( !bForceClose )
                {
                    // Send message to User App
                    nContinue = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                                      nullptr, DTWAIN_TN_PAGECONTINUE,
                                                                      reinterpret_cast<LPARAM>(pSource));
                    if ( !nContinue)
                        bUserCancelled = true;
                }
                if ( !nContinue || bForceClose )
                {
                    ResetTransfer(MSG_RESET);
                    // Check if canceled by user
                    if ( bUserCancelled )
                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                              nullptr, DTWAIN_TN_PAGECANCELLED,
                                                              reinterpret_cast<LPARAM>(pSource));
                }
                else
                // Check if the user wants to just stop the feeder
                {
                    if ( nContinue == 2 )// Stop the feeder
                        StopFeeder();
                    // Remain in state 6, even if user wanted to stop the feeder
                    m_bScanPending = true;

                    if ( bEndOfJobDetected )
                    {
                        if ( pSource->IsJobFileHandlingOn() )
                            SaveJobPages(FileWriter);

                        // Increment job number
                        pSource->SetPendingJobNum(pSource->GetPendingJobNum() + 1);

                        // Set everything for next job
                        pSource->ResetJob();
                        return { true, false };
                    }
                    return { true, false };
                }
            }

            if ( ptrPending->Count == 0 || !nContinue || bForceClose || bEndOfJobDetected || bProcessSinglePage)
            {
                struct UIShutDown
                {
                    CTL_ITwainSession* pSession;
                    CTL_ITwainSource* pSource;
                    bool bShutdown;
                    UIShutDown(CTL_ITwainSession* pSes, CTL_ITwainSource* pSrc, bool bClose)
                        : pSession(pSes), pSource(pSrc), bShutdown(bClose) {}
                    ~UIShutDown()
                    {
                        if (bShutdown)
                            CTL_TwainAppMgr::EndTwainUI(pSession, pSource);
                    }
                };

                // Prompt to save image here

                // Send a message to close things down if
                // there was no user interface chosen
                bool keepProcessingSinglePage = bProcessSinglePage && (ptrPending->Count > 0);
                // If there are no more images pending for single page image types, and
                // the device is not showing the user-interface, and there are no pages in 
                // the feeder, shut the UI down.
                UIShutDown uiCloser(pSession, pSource, !keepProcessingSinglePage && !pSource->IsUIOpenOnAcquire());

                if ( pSource->GetAcquireType() == TWAINAcquireType_FileUsingNative)
                {
                    if ( !bForceClose )
                    {
                        // Check if we've acquired any pages successfully
                        if ( pSource->GetPendingImageNum() + 1 - pSource->GetBlankPageCount() > 0)
                        {
                            if ( pSource->IsMultiPageModeSaveAtEnd() &&
                                 !dynarithmic::IsFileTypeMultiPage( pSource->GetAcquireFileType() ))
                            {
                                pSource->ProcessMultipageFile();
                            }
                            else
                            if ( (!pSource->IsMultiPageModeContinuous()) ||
                                 (pSource->IsMultiPageModeContinuous() && !dynarithmic::IsFileTypeMultiPage(pSource->GetAcquireFileType())))
                            {
                                if ( !pSource->GetTransferDone())
                                {
                                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                                          nullptr, DTWAIN_TN_FILESAVECANCELLED,
                                                                          reinterpret_cast<LPARAM>(pSource));
                                }
                                else
                                {
                                    const int notification = (errFile == 0?DTWAIN_TN_FILESAVEOK:DTWAIN_TN_CLOSEDIBFAILED);
                                    if ( FileWriter.CloseMultiPageDibFile() == 0 )
                                        CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                                              nullptr, notification,
                                                                              reinterpret_cast<LPARAM>(pSource));
                                  else
                                  {
                                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                                          nullptr, DTWAIN_TN_FILESAVEERROR,
                                                                          reinterpret_cast<LPARAM>(pSource));
                                    pSource->ClearPDFText(); // clear the text elements
                                  }
                              }
                            }
                        }
                    }
                }
                if ( bEndOfJobDetected)
                {
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr, DTWAIN_TN_ENDOFJOBDETECTED,
                                                          reinterpret_cast<LPARAM>(pSource));
                }
                m_bScanPending = ptrPending->Count != 0;
                return { true, true };
            }
        break;

        case TWRC_FAILURE:
        {
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, TWRC_FAILURE, CTL_TwainAppMgr::GetInstance()->GetLastConditionCodeError());
            if ( !pSource->IsUIOpenOnAcquire())
                CTL_TwainAppMgr::EndTwainUI(pSession, pSource);
            *ptrPending = {};
        }
        break;
    }
    m_bScanPending = ptrPending->Count != 0;
    return { false, ptrPending->Count == 0};
}

void CTL_ImageXferTriplet::SaveJobPages(const ImageXferFileWriter& FileWriter)
{
    CTL_ITwainSource *pSource = GetSourcePtr();
    const CTL_ITwainSession *pSession = GetSessionPtr();


    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                          nullptr, DTWAIN_TN_EOJBEGINFILESAVE,
                                          reinterpret_cast<LPARAM>(pSource));

    if ( m_nTotalPagesSaved > 0)
    {
        if ( pSource->IsMultiPageModeSaveAtEnd() &&
             !dynarithmic::IsFileTypeMultiPage( pSource->GetAcquireFileType() ))
        {
            pSource->ProcessMultipageFile();
        }
        else
        if ( !pSource->IsMultiPageModeContinuous() )
        {
          if ( FileWriter.CloseMultiPageDibFile() == 0 )
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                  nullptr, DTWAIN_TN_FILESAVEOK,
                                              reinterpret_cast<LPARAM>(pSource));
          else
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                  nullptr, DTWAIN_TN_FILESAVEERROR,
                                              reinterpret_cast<LPARAM>(pSource));
        }
    }
    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                          nullptr, DTWAIN_TN_EOJENDFILESAVE,
                                          reinterpret_cast<LPARAM>(pSource));
}

int CTL_ImageXferTriplet::GetTotalScannedPages() const
{
    return m_nTotalPages;
}

bool CTL_ImageXferTriplet::IsScanPending() const
{
    return m_bScanPending;
}

bool CTL_ImageXferTriplet::IsJobControlPending(TW_PENDINGXFERS *pPending) const
{
    const CTL_ITwainSource* pSource = GetSourcePtr();

    // If no job control, return false;
    if ( pSource->GetCurrentJobControl() == TWJC_NONE )
        return false;

    // If job control is set, check the TW_PENDINGXFERS data for non-zero
    if (pPending && pPending->EOJ != pSource->GetEOJDetectedValue())
       return true;  // Expect more data
    return false;   // No job control
}

std::string CTL_ImageXferTriplet::GetPageFileName(const std::string &strBase, int nCurImage ) const
{
    const std::string strFormat = std::to_string(nCurImage);
    std::string strTemp;
    const size_t nLenFormat = strFormat.length();

    StringArray aTokens;
    // Adjust name

    StringWrapperA::Tokenize(strBase, ".", aTokens);

    // Make sure that you take the "last" token
    const size_t nTokens = aTokens.size();
    size_t nLen;

    if ( nTokens == 0 )
    {
        nLen = strBase.length();
        strTemp = StringWrapperA::Left(strTemp, nLen -  nLenFormat ) + strFormat;
        return strTemp;
    }

    if ( nTokens == 1 )
    {
        nLen = aTokens[0].length();
        strTemp = StringWrapperA::Left(aTokens[0], nLen - nLenFormat) + strFormat;
        return strTemp;
    }

    else
    {
        for ( size_t i =0; i < nTokens - 1; i++ )
        {
            strTemp += aTokens[i];
            strTemp += ".";
        }
        nLen = strTemp.length();
        strTemp = StringWrapperA::Left(strTemp,  nLen - 1 - nLenFormat);
        strTemp += strFormat;
        strTemp += ".";
        strTemp += aTokens[nTokens-1];
    }
    return strTemp;
}

int CTL_ImageXferTriplet::GetTransferType() const
{
    return m_nTransferType;
}

bool CTL_ImageXferTriplet::StopFeeder()
{
    return ResetTransfer(MSG_STOPFEEDER);
}

bool CTL_ImageXferTriplet::ResetTransfer(TW_UINT16 Msg/*=MSG_RESET*/)
{
    CTL_ITwainSession* pSession = GetSessionPtr();
    CTL_ImagePendingTriplet Pending(pSession, GetSourcePtr(), Msg);
    const TW_UINT16 rc = Pending.Execute();

    switch( rc )
    {
        case TWRC_SUCCESS:
            switch (Msg )
            {
                case MSG_RESET:
                    LogWriterUtils::WriteLogInfoIndentedA("Transfer reset and ended.  ADF may eject page...");
                break;

                case MSG_ENDXFER:
                    LogWriterUtils::WriteLogInfoIndentedA("Ending transfer...");
                break;

                case MSG_STOPFEEDER:
                    LogWriterUtils::WriteLogInfoIndentedA("stopping feeder...");
            }
            return true;

        case TWRC_FAILURE:
        {
            LogWriterUtils::WriteLogInfoIndentedA("Reset Transfer failed...");
            auto ccode = CTL_TwainAppMgr::GetLastConditionCodeError();
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, TWRC_FAILURE, ccode);
            return false;
        }
    }
    return false;
}

// Always called for the last bitmap scanned for File Transfer using the prompt
int CTL_ImageXferTriplet::PromptAndSaveImage(size_t nImageNum)
{
    // Get the image if native transfer
    CTL_TwainDibPtr CurDib;
    CTL_TwainDibArray* pArray = nullptr;
    CTL_ITwainSource* pSource = GetSourcePtr();
    CTL_ITwainSession* pSession = GetSessionPtr();
    const ImageXferFileWriter FileWriter(this, pSession, pSource);

    // Check if multi page file is being used
    const bool bIsMultiPageFile = dynarithmic::IsFileTypeMultiPage(pSource->GetAcquireFileType());
    int nMultiStage = 0;
    if ( bIsMultiPageFile )
    {
        // This is the fist page of the acquisition
        if ( nImageNum == 0 )
            nMultiStage = DIB_MULTI_FIRST;
        else
        // This is a subsequent page of the acquisition
            nMultiStage = DIB_MULTI_NEXT;
    }

    switch( pSource->GetAcquireType() )
    {
        // Check if source acquire is file using native mode
        case TWAINAcquireType_FileUsingNative:
        {
            // Get the array of current array of DIBS
            pArray = pSource->GetDibArray();

            // Get the dib from the array
            CurDib = pArray->GetAt( nImageNum );
        }
        break;
        default:
            break;
    }

    CTL_StringType strTempFile;
    if ( nMultiStage == 0 || nMultiStage == DIB_MULTI_FIRST)
    {
        strTempFile = PromptForFileName(pSource->GetDTWAINHandle(), pSource->GetAcquireFileType());
        if ( strTempFile.empty() )
        {
            SendFileAcquireError(pSource, pSession,
                                 DTWAIN_ERR_BAD_FILENAME, DTWAIN_TN_FILESAVECANCELLED,
                                 StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
            return 0;
        }
        const filesys::path p{ strTempFile };
        std::ofstream ofs(p.c_str());
        if ( !ofs )
        {
            SendFileAcquireError(pSource, pSession, DTWAIN_ERR_FILEWRITE, DTWAIN_TN_FILESAVEERROR,
                                 StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
            return 0;
        }
        ofs.close();
        filesys::remove(strTempFile);
    }
    switch( pSource->GetAcquireType() )
    {
        case TWAINAcquireType_FileUsingNative:
        {

            // Write this file if using Native Mode transfer
            int retval = 0;

            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                  nullptr, DTWAIN_TN_FILEPAGESAVING,
                                          reinterpret_cast<LPARAM>(pSource));
            DTWAINImageInfoEx ImageInfo;

            // Get any relevant JPEG or TIFF Information
            pSource->GetImageInfoEx(ImageInfo);

            // Set information for possible TIFF file transfer
            ImageInfo.ResolutionX = 300;
            ImageInfo.ResolutionY = 300;

            if (!DTWAIN_GetSourceUnit(pSource, &ImageInfo.UnitOfMeasure))
                ImageInfo.UnitOfMeasure = DTWAIN_INCHES;

            ResolveImageResolution( pSource, &ImageInfo );

            // Now check for Postscript file types.  We alias these
            // types as TIFF format
            const CTL_TwainFileFormatEnum FileType = pSource->GetAcquireFileType();
            if ( dynarithmic::IsFileTypePostscript( FileType ) )
            {
                ImageInfo.IsPostscript = true;
                ImageInfo.IsPostscriptMultipage =
                    dynarithmic::IsFileTypeMultiPage( FileType );
                ImageInfo.PostscriptType = static_cast<LONG>(FileType);
            }

            // Write single page if just a dib
            if (!CurDib)
            {
                SendFileAcquireError(pSource, pSession, DTWAIN_ERR_INVALIDBMP, DTWAIN_TN_FILEPAGESAVEERROR,
                                     StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
                return DTWAIN_ERR_INVALIDBMP;
            }

            if ( nMultiStage == 0)
                retval = CurDib->WriteDibBitmap(ImageInfo, strTempFile.c_str(), pSource->GetAcquireFileType() );
            else
            {
                // Write a multi page file
                CTL_ImageIOHandlerPtr& pHandler = pSource->GetImageHandlerPtr();

                if ( nMultiStage == DIB_MULTI_FIRST)
                    pHandler = CurDib->WriteFirstPageDibMulti(ImageInfo, strTempFile.c_str(),
                                                               pSource->GetAcquireFileType(), false, 0, retval);
                else
                    CurDib->WriteNextPageDibMulti(pHandler, retval, ImageInfo);
            }

            if ( retval != 0)
            {
                SendFileAcquireError(pSource, pSession, retval, DTWAIN_TN_INVALIDIMAGEFORMAT,
                                     StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
                if ( nMultiStage )
                    SendFileAcquireError(pSource, pSession, retval, DTWAIN_TN_FILEPAGESAVEERROR,
                                         StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
                else
                    SendFileAcquireError(pSource, pSession, retval, DTWAIN_TN_FILESAVEERROR,
                                         StringConversion::Convert_Native_To_Ansi(pSource->GetActualFileName()));
            }
            else
            {
                if ( !nMultiStage || nMultiStage == DIB_MULTI_FIRST )
                    pSource->SetLastAcquiredFileName( strTempFile );
                if ( nMultiStage )
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr, DTWAIN_TN_FILEPAGESAVEOK,
                                                          reinterpret_cast<LPARAM>(pSource));
                else
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr, DTWAIN_TN_FILESAVEOK,
                                                          reinterpret_cast<LPARAM>(pSource));
            }
            // Check if there were any other images
            if (nMultiStage != 0 && !IsScanPending())
            {
                // This is the last page
                if ( FileWriter.CloseMultiPageDibFile() == 0 )
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr, DTWAIN_TN_FILESAVEOK,
                                                          reinterpret_cast<LPARAM>(pSource));
                else
                    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession,
                                                          nullptr, DTWAIN_TN_FILESAVEERROR,
                                                          reinterpret_cast<LPARAM>(pSource));
                return retval;
            }
        }
        break;

        case TWAINAcquireType_File:
        {
            // Copy default file name to the new file
            // Check if default file exists
            if (file_exists( pSource->GetAcquireFile().c_str()))
                CTL_TwainAppMgr::CopyFile(pSource->GetAcquireFile(), strTempFile);
            else
                return 0;
        }
        break;
        default:
        break;
    }
    // Delete temporary bitmap here
    if ( pSource->IsDeleteDibOnScan() && pArray )
        // Let array class handle deleting of the DIB (Global memory will be freed only)
        pArray->DeleteDibMemory( nImageNum );
    return 1;
}

bool CTL_ImageXferTriplet::CopyDibToClipboard(CTL_ITwainSession * /*pSession*/, HANDLE hDib)
{
#ifdef _WIN32
    if (hDib)
    {
        // Open the clipboard
        if (OpenClipboard(nullptr/*hWnd*/ ))
        {
            // Empty the clipboard
            if (EmptyClipboard() )
            {
                SetClipboardData(CF_DIB, hDib);
                CloseClipboard();
                return true;
            }
            CloseClipboard();
        }
    }
#endif
    return false;
}

bool CTL_ImageXferTriplet::CropDib(CTL_ITwainSession *pSession,
                                   const CTL_ITwainSource *pSource,
                                   const CTL_TwainDibPtr &CurDib)
{
    // Possibly crop the DIB
    LONG SourceUnit;
    FloatRect Requested;
    FloatRect Actual;
    LONG flags;
    LONG DestUnit;
    pSource->GetAlternateAcquireArea(Requested, DestUnit, flags);
    if (flags & CTL_ITwainSource::CROP_FLAG)
    {
        if (!pSource->IsImageLayoutValid())
        {
            LogWriterUtils::WriteLogInfoIndentedA("Image layout is invalid.  Image cannot be cropped");
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_CROPFAILED, reinterpret_cast<LPARAM>(pSource));
            return false;
        }
        // Get the actual acquisition area
        pSource->GetImageLayout(&Actual);
        bool bUndefinedSize = false;
        if ( Actual.left < 0.0 && Actual.right < 0.0 &&
                Actual.top < 0.0 && Actual.bottom < 0.0 )
        {
            // the image information is undefined.  Should get the information
            // now from the image info and assume pixels.
            bUndefinedSize = true;

            // Get the image info
            CTL_ImageInfoTriplet ImageInfo(pSession, const_cast<CTL_ITwainSource*>(pSource));
            if ( ImageInfo.Execute() == TWRC_SUCCESS )
            {
                // Set the rectangle to the image info.  This is in
                // pixels
                const TW_IMAGEINFO *pInfo = ImageInfo.GetImageInfoBuffer();
                Actual.left = Actual.top = 0.0;
                Actual.right = pInfo->ImageWidth;
                Actual.bottom = pInfo->ImageLength;
            }
        }

        // Now get the unit of measure
        if ( DTWAIN_GetSourceUnit(const_cast<CTL_ITwainSource*>(pSource), &SourceUnit) )
        {
            // Get the image resolution
            double Resolution;
            DTWAIN_GetResolution(const_cast<CTL_ITwainSource*>(pSource), &Resolution);

            // Crop the dib here
            if (CurDib->CropDib(Actual, Requested, SourceUnit, DestUnit, static_cast<int>(Resolution),
                                bUndefinedSize))
                return true;
        }
    }
    return false;
}

bool CTL_ImageXferTriplet::NegateDib(CTL_ITwainSession * /*pSession*/,
                                     const CTL_ITwainSource *pSource,
                                     const CTL_TwainDibPtr& CurDib)
{
    if ( pSource->IsImageNegativeOn() )
    {
        if (CurDib->NegateDib())
            return true;
    }
    return false;
}

bool CTL_ImageXferTriplet::ResampleDib(CTL_ITwainSession * /*pSession*/,
                                        const CTL_ITwainSource *pSource,
                                        const CTL_TwainDibPtr& CurDib)
{
    double xscale;
    double yscale;
    LONG flags;
    pSource->GetImageScale(xscale, yscale, flags);
    if (flags & CTL_ITwainSource::SCALE_FLAG)
    {
        if (CurDib->ResampleDib(xscale, yscale))
            return true;
    }
    return false;
}


bool CTL_ImageXferTriplet::ChangeBpp(CTL_ITwainSession*,
                                     const CTL_ITwainSource* pSource,
                                     const CTL_TwainDibPtr& CurDib)
{
    const LONG bpp = pSource->GetForcedImageBpp();
    bool bRetval = false;
    if ( bpp != 0 )
    {
        const int depth = CurDib->GetDepth();
        if ( bpp > depth )
            bRetval = CurDib->IncreaseBpp(bpp);
        else
        if (bpp < depth)
            bRetval = CurDib->DecreaseBpp(bpp);
    }
    return bRetval;
}

int CTL_ImageXferTriplet::ProcessBlankPage(CTL_ITwainSession *pSession,
                                           CTL_ITwainSource *pSource,
                                           const CTL_TwainDibPtr& CurDib,
                                           bool resampled,
                                           LONG message_to_send1,
                                           LONG message_to_send2,
                                           LONG option_to_test) const
{
    if (GetDAT() == DAT_AUDIONATIVEXFER)
        return 1;
    const bool bIsBlankPage = IsPageBlank(pSession, pSource, resampled, CurDib)?true:false;
    if (bIsBlankPage)
    {
        LONG bKeepPage = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, static_cast<WPARAM>(message_to_send1), reinterpret_cast<LPARAM>(pSource));

        if ( pSource->IsBlankPageAutoDetectOn() )
        {
            LONG options = pSource->GetBlankPageAutoDetect();
            bKeepPage = options & option_to_test?0:1;
        }
        if ( !bKeepPage )
        {
            // remove dib from array and delete the memory for the DIB
            CurDib->SetAutoDelete(true);
            pSource->GetDibArray()->RemoveDib(m_hDataHandle);
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, static_cast<WPARAM>(message_to_send2),reinterpret_cast<LPARAM>(pSource));
            return 0;  // DIB is thrown away
        }
    }
    return 1; // keep the DIB
}


bool CTL_ImageXferTriplet::IsPageBlank(CTL_ITwainSession*,
                                       const CTL_ITwainSource* pSource,
                                       bool resampled,
                                       const CTL_TwainDibPtr& CurDib)
{
    if ( pSource->IsBlankPageDetectionOn() )
    {
        if ( resampled && pSource->IsBlankPageDetectionSampleOn() ||
            !resampled && pSource->IsBlankPageDetectionNoSampleOn())
            return CurDib->IsBlankDIB(pSource->GetBlankPageThreshold());
    }
    return false;
}

CTL_TwainFileFormatEnum CTL_ImageXferTriplet::GetFileTypeFromCompression(int nCompression)
{
    switch (nCompression)
    {
        case TWCP_GROUP31D:
        case TWCP_GROUP31DEOL:
        case TWCP_GROUP32D:
            return TWAINFileFormat_TIFFGROUP3;

        case TWCP_GROUP4:
            return TWAINFileFormat_TIFFGROUP4;

        case TWCP_PACKBITS:
            return TWAINFileFormat_TIFFPACKBITS;

        case TWCP_JPEG:
            return TWAINFileFormat_JPEG;

        case TWCP_LZW:
            return TWAINFileFormat_TIFFLZW;

        case TWCP_JBIG:
            return TWAINFileFormat_JBIG;

        case TWCP_PNG:
            return TWAINFileFormat_PNG;

        case TWCP_RLE4:
        case TWCP_RLE8:
        case TWCP_BITFIELDS:
            return TWAINFileFormat_BMP;


    }
    return TWAINFileFormat_RAW;
}

void SendFileAcquireError(CTL_ITwainSource* pSource, const CTL_ITwainSession* pSession,
                          LONG Error, LONG ErrorMsg, const std::string& extraInfo)
{
    CTL_TwainAppMgr::SetError(Error, extraInfo, true);
    if ( CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_DTWAINERRORS)
    {
        char szBuf[DTWAIN_USERRES_MAXSIZE + 1];
        CTL_TwainAppMgr::GetLastErrorString(szBuf, DTWAIN_USERRES_MAXSIZE);
        LogWriterUtils::WriteLogInfoIndentedA(szBuf);
    }
    CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, static_cast<WPARAM>(ErrorMsg), reinterpret_cast<LPARAM>(pSource));
}

void CTL_ImageXferTriplet::ResolveImageResolution(CTL_ITwainSource *pSource,  DTWAINImageInfoEx* ImageInfo)
{
    double Resolution;

    // Get the image info
    // First try the actual image
    double ResolutionX, ResolutionY;

    // First, check if we need to get the info from state 7
    bool bGotResolution = false;
    bool bGetResFromDriver = false;

    if ( IsState7InfoNeeded(pSource) )
    {
      if ( DTWAIN_GetImageInfo(pSource,
                              &ResolutionX,
                              &ResolutionY,
                              nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr))
        {
            bool bWriteMisc = CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS;
            std::string sError;
            if (bWriteMisc)
            {
                sError = "Image resolution available in state 7.";
                LogWriterUtils::WriteLogInfoIndentedA(sError);
            }
            ImageInfo->ResolutionX = static_cast<LONG>(ResolutionX);
            ImageInfo->ResolutionY = static_cast<LONG>(ResolutionY);
            if ( bWriteMisc )
            {
                StringStreamA strm;
                strm << boost::format("x-Resolution=%1%, y-Resolution=%2%") % ImageInfo->ResolutionX % ImageInfo->ResolutionY;
                sError = strm.str();
                LogWriterUtils::WriteLogInfoIndentedA(sError);
            }
            bGotResolution = true;
        }
        else
        // Try to get resolution from the driver
           bGetResFromDriver = true;
    }

    if ( !bGotResolution && !bGetResFromDriver )
    {
        // Get the image info from when we started
        bool bWriteMisc = CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS;
        if ( bWriteMisc )
        {
            LogWriterUtils::WriteLogInfoIndentedA("Getting image resolution from state 6.");
        }
        TW_IMAGEINFO II;
        pSource->GetImageInfo( &II );

        ImageInfo->ResolutionX = static_cast<LONG>(Fix32ToFloat(II.XResolution));
        ImageInfo->ResolutionY = static_cast<LONG>(Fix32ToFloat(II.YResolution));
        if ( bWriteMisc )
        {
            StringStreamA strm;
            strm << boost::format("x-Resolution=%1%, y-Resolution=%2%\n") % ImageInfo->ResolutionX % ImageInfo->ResolutionY;
            LogWriterUtils::WriteLogInfoIndentedA(strm.str());
        }
        bGotResolution = true;
    }

    if ( !bGotResolution )
    {
        bool bWriteMisc = CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS;
        // Try TWAIN driver setting
        if ( DTWAIN_GetResolution(pSource, &Resolution) )
        {
            if ( bWriteMisc )
            {
                std::string sError = "Image resolution obtained from TWAIN driver";
                LogWriterUtils::WriteLogInfoIndentedA(sError);
            }
            ImageInfo->ResolutionX = static_cast<LONG>(Resolution);
            ImageInfo->ResolutionY = static_cast<LONG>(Resolution);
            if ( bWriteMisc )
            {
                StringStreamA strm;
                strm << boost::format("x-Resolution=%1%, y-Resolution=%2%") % ImageInfo->ResolutionX % ImageInfo->ResolutionY;
                LogWriterUtils::WriteLogInfoIndentedA(strm.str());
            }
        }
        else
        {
            if ( bWriteMisc )
            {
                // Tried everything, just set the resolution to the default resolution
                std::string sError = "Could not obtain resolution in state 6/7 or through TWAIN.  Image resolution defaulted to 100 DPI";
                LogWriterUtils::WriteLogInfoIndentedA(sError);
            }
            ImageInfo->ResolutionX = 100;
            ImageInfo->ResolutionY = 100;
        }
    }
}

bool CTL_ImageXferTriplet::ModifyAcquiredDib()
{
    CTL_ITwainSession* pSession = GetSessionPtr();
    CTL_ITwainSource* pSource = GetSourcePtr();
    CTL_TwainDibArray* pArray = pSource->GetDibArray();

    size_t nLastDib = pArray->GetSize() - 1;
    CTL_TwainDibPtr CurDib = pArray->GetAt(nLastDib);


    typedef bool (*AdjustFn)(CTL_ITwainSession*, const CTL_ITwainSource*, const CTL_TwainDibPtr&);
    std::array<AdjustFn, 4> adjfn = { &CTL_ImageXferTriplet::ChangeBpp, &CTL_ImageXferTriplet::CropDib,
                                      &CTL_ImageXferTriplet::ResampleDib, &CTL_ImageXferTriplet::NegateDib };

    constexpr const char *msg[] = { "Bitmap after change to bits-per-pixel: \n",
                                    "Bitmap after cropping: \n",
                                    "Bitmap after resampling: \n",
                                    "Bitmap after negating image: \n" };

    for (unsigned i = 0; i < std::size(adjfn); ++i)
    {
        // call the function to adjust bitmap
        if (adjfn[i](pSession, pSource, CurDib))
        {
            // reset the dib handle if adjusted
            pSource->SetDibHandle(m_hDataHandle = CurDib->GetHandle(), nLastDib);
            if (CTL_StaticData::GetLogFilterFlags() & DTWAIN_LOG_MISCELLANEOUS)
            {
                std::string sOut = msg[i];
                sOut += CTL_ErrorStructDecoder::DecodeBitmap(m_hDataHandle);
                LogWriterUtils::WriteMultiLineInfoIndentedA(sOut, "\n");
            }
            return true;
        }
    }
    return false;
}

// This function is used only for file transfers.  We will need to
// modify the DIB if the Bit-per-pixel of the original DIB is different 
// than what the image type expects.
bool CTL_ImageXferTriplet::ResampleAcquiredDib()
{
    if (!CTL_StaticData::IsResamplingDone())
        return false;
    CTL_ITwainSource* pSource = GetSourcePtr();
    const int nFileType = pSource->GetAcquireFileType();

    CTL_TwainDibArray* pArray = pSource->GetDibArray();

    if (pArray->GetSize() == 0)
        return false;

    const size_t nLastDib = pArray->GetSize() - 1;
    CTL_TwainDibPtr CurDib = pArray->GetAt(nLastDib);

    // Get the appropriate image resampler for the image
    auto ptr = ResampleFactory::GetResampler(nFileType);
    if (ptr)
    {
        // resample image
        bool bResampled = ptr->Resample(*CurDib);
        if (bResampled)
        {
            // set the handle to the resampled image
            pSource->SetDibHandle(m_hDataHandle = CurDib->GetHandle(), nLastDib);
            return true;
        }
        if ( ptr->GetResampleStatus() != DTWAIN_NO_ERROR )
        {
            // This is an error if the image could not be resampled
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSource->GetTwainSession(), nullptr, DTWAIN_TN_IMAGE_RESAMPLE_FAILURE,
                                                  reinterpret_cast<LPARAM>(pSource));
        }
    }
    return false;
}

bool CTL_ImageXferTriplet::QueryAndRemoveDib(CTL_TwainAcquireEnum acquireType, size_t nWhich)
{
    if (GetDAT() == DAT_AUDIONATIVEXFER)
        return true;
    CTL_ITwainSource* pSource = GetSourcePtr();
    CTL_TwainDibArray* pArray = pSource->GetDibArray();
    const CTL_ITwainSession* pSession = GetSessionPtr();
    bool bKeepPage = true;

    if (pSource->GetAcquireType() == acquireType)
    {
        bKeepPage = CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_QUERYPAGEDISCARD, reinterpret_cast<LPARAM>(pSource)) ? true : false;
        // Do not keep the page
        if (!bKeepPage)
        {
            // throw this dib away (remove from the dib array)
            pArray->DeleteDibMemory(nWhich);
            pArray->RemoveDib(nWhich);
            CTL_TwainAppMgr::SendTwainMsgToWindow(pSession, nullptr, DTWAIN_TN_PAGEDISCARDED, reinterpret_cast<LPARAM>(pSource));
        }
    }
    return bKeepPage;
}

void CTL_ImageXferTriplet::SetBufferedTransfer(bool bSet)
{
    m_IsBuffered = bSet;
}

bool CTL_ImageXferTriplet::IsBufferedTransfer() const
{
    return m_IsBuffered;
}

bool IsState7InfoNeeded(CTL_ITwainSource *pSource)
{
    bool bRetval = false;
    DTWAIN_ARRAY A = nullptr;
    DTWAINScopedLogControllerExclude scopedLog(DTWAIN_LOG_ERRORMSGBOX);
    if ( DTWAIN_GetCapValuesEx2(pSource, DTWAIN_CV_ICAPUNDEFINEDIMAGESIZE, DTWAIN_CAPGETCURRENT, DTWAIN_CONTDEFAULT, DTWAIN_DEFAULT, &A))
    {
        const auto pHandle = pSource->GetDTWAINHandle();
        DTWAINArrayLowLevel_RAII raii(pHandle, A);
        const auto& vValues = pHandle->m_ArrayFactory->underlying_container_t<LONG>(A);
        if ( !vValues.empty())
            bRetval = vValues[0] > 0;
    }
    return bRetval;
}
///////////////////////////////////////////////////////////////////////////
